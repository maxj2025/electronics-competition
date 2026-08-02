#include "nlms.h"
#include "main.h"
#include <string.h>

/* ================================================================== */
/*  DTCM 数据区（不经过 D-Cache，CPU 直接访问最快）                      */
/* ================================================================== */

/* NLMS 系数（训练 + 实时共享） */
NLMS_DTCM_DATA static float32_t g_coeff[NLMS_NUM_TAPS];

/* NLMS 训练延迟线（循环缓冲区，仅训练阶段使用） */
NLMS_DTCM_DATA static float32_t g_trainHist[NLMS_NUM_TAPS];

/*
 * arm_fir 状态缓冲区（实时阶段使用）
 * arm_fir_f32 要求: numTaps + blockSize - 1 = 128 + 256 - 1 = 383
 */
NLMS_DTCM_DATA static float32_t g_firState[NLMS_NUM_TAPS + NLMS_BLOCK_SIZE - 1U];

/*
 * ★ arm_fir 实例也必须放 DTCM！
 *
 * 如果放在普通 RAM (AXI SRAM)，会经过 D-Cache：
 *   arm_fir_init_f32 写入 → 数据在 D-Cache 中
 *   arm_fir_f32 读取 → 如果 cache line 被驱逐，会从 RAM 读到旧值！
 * 放在 DTCM 则完全绕过 D-Cache，避免此问题。
 */
NLMS_DTCM_DATA static arm_fir_instance_f32 g_firInstance;

/* ================================================================== */
/*  状态变量                                                            */
/* ================================================================== */

static uint32_t g_trainHistIndex = 0U;
static uint32_t g_trainCount = 0U;
static uint8_t  g_trainingDone = 0U;

/* 性能监控 */
static uint32_t g_maxCycles = 0U;
static uint32_t g_overrunCount = 0U;

/* 训练质量监控：最终均方误差 */
static float32_t g_finalMSE = 0.0f;

/* ================================================================== */
/*  内联辅助：限幅函数                                                   */
/* ================================================================== */

static inline float32_t Clampf(float32_t val, float32_t lo, float32_t hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/* ================================================================== */
/*  初始化                                                              */
/* ================================================================== */

void NLMS_RLC_Init(void)
{
    memset(g_coeff, 0, sizeof(g_coeff));
    memset(g_trainHist, 0, sizeof(g_trainHist));
    memset(g_firState, 0, sizeof(g_firState));
    memset(&g_firInstance, 0, sizeof(g_firInstance));

    g_trainHistIndex = 0U;
    g_trainCount = 0U;
    g_trainingDone = 0U;

    g_maxCycles = 0U;
    g_overrunCount = 0U;
    g_finalMSE = 0.0f;
}

/* ================================================================== */
/*  训练阶段：单次 FIR 滤波（使用循环缓冲区）                             */
/* ================================================================== */

static float32_t NLMS_FilterWithHistory(const float32_t *hist,
                                        uint32_t histIndex)
{
    uint32_t i;
    uint32_t pos;
    float32_t y;

    y = 0.0f;
    pos = histIndex;

    for (i = 0U; i < NLMS_NUM_TAPS; i++)
    {
        y += g_coeff[i] * hist[pos];

        if (pos == 0U)
        {
            pos = NLMS_NUM_TAPS - 1U;
        }
        else
        {
            pos--;
        }
    }

    return y;
}

/* ================================================================== */
/*  训练阶段：NLMS 块处理                                                */
/* ================================================================== */

NLMS_ITCM_CODE
void NLMS_RLC_ProcessBlock(const float32_t *x,
                           const float32_t *d,
                           float32_t *y,
                           float32_t *e)
{
    uint32_t n;
    uint32_t i;
    uint32_t pos;
    uint32_t startCycles;
    uint32_t usedCycles;
    uint32_t blockCycleBudget;
    float32_t yn;
    float32_t en;
    float32_t power;
    float32_t mu;
    float32_t gain;
    float32_t mseAccum;

    startCycles = DWT->CYCCNT;
    mseAccum = 0.0f;

    for (n = 0U; n < NLMS_BLOCK_SIZE; n++)
    {
        /* 插入新样本到循环缓冲区 */
        g_trainHist[g_trainHistIndex] = x[n];

        /* FIR 滤波 */
        yn = NLMS_FilterWithHistory(g_trainHist, g_trainHistIndex);
        en = d[n] - yn;

        y[n] = yn;
        e[n] = en;

        /* 累计均方误差（用于训练质量评估） */
        mseAccum += en * en;

        /* ====== 训练阶段：更新系数 ====== */
        if (g_trainingDone == 0U)
        {
            /* 计算输入功率 ||x[n]||^2 + ε */
            power = NLMS_EPSILON;
            pos = g_trainHistIndex;

            for (i = 0U; i < NLMS_NUM_TAPS; i++)
            {
                power += g_trainHist[pos] * g_trainHist[pos];

                if (pos == 0U)
                {
                    pos = NLMS_NUM_TAPS - 1U;
                }
                else
                {
                    pos--;
                }
            }

            /* 三段式学习率 */
            if (g_trainCount < NLMS_FAST_SAMPLES)
            {
                mu = NLMS_MU_FAST;
            }
            else if (g_trainCount < NLMS_MID_SAMPLES)
            {
                mu = NLMS_MU_MID;
            }
            else
            {
                mu = NLMS_MU_FINE;
            }

            /* NLMS 增益，并限幅防止发散 */
            gain = mu * en / power;
            gain = Clampf(gain, -NLMS_GAIN_MAX_ABS, NLMS_GAIN_MAX_ABS);

            /* 带泄漏的系数更新 + 限幅保护 */
            pos = g_trainHistIndex;

            for (i = 0U; i < NLMS_NUM_TAPS; i++)
            {
                g_coeff[i] = NLMS_LEAKAGE * g_coeff[i]
                            + gain * g_trainHist[pos];

                /* 系数限幅：防止单个系数发散到极大值 */
                g_coeff[i] = Clampf(g_coeff[i],
                                    -NLMS_COEFF_MAX_ABS,
                                     NLMS_COEFF_MAX_ABS);

                if (pos == 0U)
                {
                    pos = NLMS_NUM_TAPS - 1U;
                }
                else
                {
                    pos--;
                }
            }

            g_trainCount++;

            if (g_trainCount >= NLMS_TRAIN_SAMPLES)
            {
                g_trainingDone = 1U;
            }
        }

        /* 更新循环缓冲区索引 */
        g_trainHistIndex++;

        if (g_trainHistIndex >= NLMS_NUM_TAPS)
        {
            g_trainHistIndex = 0U;
        }
    }

    /* 更新最终 MSE（滑动平均） */
    g_finalMSE = 0.9f * g_finalMSE + 0.1f * (mseAccum / (float32_t)NLMS_BLOCK_SIZE);

    /* 性能监控 */
    usedCycles = DWT->CYCCNT - startCycles;

    if (usedCycles > g_maxCycles)
    {
        g_maxCycles = usedCycles;
    }

    blockCycleBudget = (SystemCoreClock / NLMS_FS_HZ) * NLMS_BLOCK_SIZE;

    if (usedCycles > blockCycleBudget)
    {
        g_overrunCount++;
    }
}

/* ================================================================== */
/*  实时阶段：准备 FIR 滤波器                                            */
/* ================================================================== */

void NLMS_RLC_PrepareFIROnly(void)
{
    /*
     * 将 NLMS 训练得到的系数交给 arm_fir_f32
     *
     * g_coeff 和 g_firState 都在 DTCM（不经过 D-Cache）
     * g_firInstance 也在 DTCM，完全避免缓存一致性问题
     *
     * arm_fir_init_f32 会：
     * 1. 设置系数指针 → g_coeff (DTCM)
     * 2. 清零状态缓冲区 → g_firState (DTCM)
     * 3. 设置 numTaps 和 blockSize
     */
    arm_fir_init_f32(&g_firInstance,
                     (uint16_t)NLMS_NUM_TAPS,
                     (float32_t *)g_coeff,
                     g_firState,
                     NLMS_BLOCK_SIZE);
}

/* ================================================================== */
/*  实时阶段：FIR 滤波处理（使用 arm_fir_f32）                           */
/* ================================================================== */

NLMS_ITCM_CODE
void NLMS_RLC_ProcessFIROnly(const float32_t *x,
                             float32_t *y,
                             uint32_t blockSize)
{
    uint32_t startCycles;
    uint32_t usedCycles;
    uint32_t blockCycleBudget;

    startCycles = DWT->CYCCNT;

    /*
     * arm_fir_f32 使用 Cortex-M7 SIMD/流水线优化
     * 所有数据都在 DTCM，无 D-Cache 风险
     */
    arm_fir_f32(&g_firInstance, (float32_t *)x, y, blockSize);

    /* 性能监控 */
    usedCycles = DWT->CYCCNT - startCycles;

    if (usedCycles > g_maxCycles)
    {
        g_maxCycles = usedCycles;
    }

    blockCycleBudget = (SystemCoreClock / NLMS_FS_HZ) * blockSize;

    if (usedCycles > blockCycleBudget)
    {
        g_overrunCount++;
    }
}

/* ================================================================== */
/*  状态查询                                                            */
/* ================================================================== */

uint8_t NLMS_RLC_IsTrainingDone(void)
{
    return g_trainingDone;
}

uint32_t NLMS_RLC_GetTrainCount(void)
{
    return g_trainCount;
}

const float32_t *NLMS_RLC_GetCoefficients(void)
{
    return g_coeff;
}

uint32_t NLMS_RLC_GetMaxCycles(void)
{
    return g_maxCycles;
}

uint32_t NLMS_RLC_GetOverrunCount(void)
{
    return g_overrunCount;
}

void NLMS_RLC_ResetPerformance(void)
{
    g_maxCycles = 0U;
    g_overrunCount = 0U;
}

float32_t NLMS_RLC_GetFinalMSE(void)
{
    return g_finalMSE;
}
