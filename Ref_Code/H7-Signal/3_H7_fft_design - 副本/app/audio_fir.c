#include "audio_fir.h"

#include <string.h>
#include <math.h>
#include <stdint.h>

/* ============================================================
 * 常量定义
 * ============================================================ */

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define AUDIO_FIR_EPSILON       1.0e-12
#define AUDIO_FIR_Q15_MAX       32767
#define AUDIO_FIR_Q15_MIN      -32768

/* ============================================================
 * 静态全局变量
 * ============================================================ */

/*
 * 输入/输出块缓存
 */
__attribute__((section(".DTCM_DATA"))) static q15_t s_fir_input_q15[AUDIO_FIR_BLOCK_SIZE];
__attribute__((section(".DTCM_DATA"))) static q15_t s_fir_output_q15[AUDIO_FIR_BLOCK_SIZE];

/*
 * FIR 状态缓存：
 * 长度 = 最大抽头数 + 最大块长 - 1
 */
__attribute__((section(".DTCM_DATA"))) static q15_t s_fir_state_q15[AUDIO_FIR_MAX_TAP_NUM + AUDIO_FIR_BLOCK_SIZE - 1];

/*
 * FIR 系数（Q15）
 */
__attribute__((section(".DTCM_DATA"))) static q15_t s_fir_coeff_q15[AUDIO_FIR_MAX_TAP_NUM];

/*
 * 当前 FIR 抽头数
 */
__attribute__((section(".DTCM_DATA"))) static uint16_t s_fir_tap_num = 0;

/* ============================================================
 * 静态函数声明
 * ============================================================ */

static double AudioFIR_Sinc(double x);
static uint16_t AudioFIR_SelectWindowAndTapNum(const TransferParam *param,
                                               FilterType type,
                                               WindowType *window_type);

static void AudioFIR_GenerateIdealLPF(double *h, uint16_t N, double wc);
static void AudioFIR_GenerateIdealHPF(double *h, uint16_t N, double wc);
static void AudioFIR_GenerateIdealBPF(double *h, uint16_t N, double w1, double w2);
static void AudioFIR_GenerateIdealBSF(double *h, uint16_t N, double w1, double w2);

static double AudioFIR_WindowValue(WindowType type, uint16_t n, uint16_t N);
static q15_t AudioFIR_DoubleToQ15(double x);

static void AudioFIR_ADC12ToQ15(const uint16_t *src, q15_t *dst, uint32_t len);
static void AudioFIR_Q15ToDAC12(const q15_t *src, uint16_t *dst, uint32_t len);

static int AudioFIR_CheckParam(const TransferParam *param, FilterType type);
static void AudioFIR_ComputeQ15(const q15_t *src, q15_t *dst, uint32_t blockSize);

/* ============================================================
 * 基础数学工具
 * ============================================================ */

/**
 * @brief  sinc(x) = sin(x)/x
 */
static double AudioFIR_Sinc(double x)
{
    if (fabs(x) < AUDIO_FIR_EPSILON)
    {
        return 1.0;
    }

    return sin(x) / x;
}

/**
 * @brief  根据阻带衰减与过渡带，自动选择窗函数并估算抽头数
 *
 * @note   这里采用经验公式，优先保证简单稳定。
 */
static uint16_t AudioFIR_SelectWindowAndTapNum(const TransferParam *param,
                                               FilterType type,
                                               WindowType *window_type)
{
    double trans_bw = 0.0;
    double delta_w;
    double Nf;
    uint32_t N;

    if (param == NULL || window_type == NULL)
    {
        return 0;
    }

    switch (type)
    {
        case LOWPASSFILTER:
            trans_bw = param->fst1 - param->fp1;
            break;

        case HIGHPASSFILTER:
            trans_bw = param->fp1 - param->fst1;
            break;

        case BANDPASSFILTER:
        {
            double bw1 = param->fp1 - param->fst1;
            double bw2 = param->fst2 - param->fp2;
            trans_bw = (bw1 < bw2) ? bw1 : bw2;
            break;
        }

        case BANDSTOPFILTER:
        {
            double bw1 = param->fst1 - param->fp1;
            double bw2 = param->fp2 - param->fst2;
            trans_bw = (bw1 < bw2) ? bw1 : bw2;
            break;
        }

        default:
            return 0;
    }

    if (trans_bw <= 0.0 || param->fs <= 0.0)
    {
        return 0;
    }

    delta_w = 2.0 * M_PI * trans_bw / param->fs;

    /*
     * 按阻带衰减选择窗函数
     */
    if (param->ast <= 21.0)
    {
        *window_type = Rectangle;
        Nf = 4.0 * M_PI / delta_w;
    }
    else if (param->ast <= 25.0)
    {
        *window_type = triangle;
        Nf = 8.0 * M_PI / delta_w;
    }
    else if (param->ast <= 44.0)
    {
        *window_type = Hanning;
        Nf = 8.0 * M_PI / delta_w;
    }
    else if (param->ast <= 53.0)
    {
        *window_type = Hamming;
        Nf = 8.0 * M_PI / delta_w;
    }
    else
    {
        *window_type = Blackman;
        Nf = 12.0 * M_PI / delta_w;
    }

    N = (uint32_t)(Nf + 1.0);

    /*
     * FIR 线性相位常用奇数抽头
     */
    if ((N & 1U) == 0U)
    {
        N++;
    }

    if (N < 3U)
    {
        N = 3U;
    }

    if (N > AUDIO_FIR_MAX_TAP_NUM)
    {
        N = AUDIO_FIR_MAX_TAP_NUM;
        if ((N & 1U) == 0U)
        {
            N--;
        }
    }

    return (uint16_t)N;
}

/* ============================================================
 * 理想滤波器响应生成
 * ============================================================ */

/**
 * @brief  理想低通
 *         截止角频率 wc，单位 rad/sample
 */
static void AudioFIR_GenerateIdealLPF(double *h, uint16_t N, double wc)
{
    int32_t n;
    int32_t m;

    m = (int32_t)(N - 1U) / 2;

    for (n = 0; n < (int32_t)N; n++)
    {
        int32_t k = n - m;

        if (k == 0)
        {
            h[n] = wc / M_PI;
        }
        else
        {
            h[n] = sin(wc * k) / (M_PI * k);
        }
    }
}

/**
 * @brief  理想高通
 */
static void AudioFIR_GenerateIdealHPF(double *h, uint16_t N, double wc)
{
    double hlp[AUDIO_FIR_MAX_TAP_NUM];
    uint16_t n;
    uint16_t m = (N - 1U) / 2U;

    AudioFIR_GenerateIdealLPF(hlp, N, wc);

    for (n = 0; n < N; n++)
    {
        h[n] = -hlp[n];
    }

    h[m] += 1.0;
}

/**
 * @brief  理想带通
 */
static void AudioFIR_GenerateIdealBPF(double *h, uint16_t N, double w1, double w2)
{
    double hlp1[AUDIO_FIR_MAX_TAP_NUM];
    double hlp2[AUDIO_FIR_MAX_TAP_NUM];
    uint16_t n;

    AudioFIR_GenerateIdealLPF(hlp1, N, w1);
    AudioFIR_GenerateIdealLPF(hlp2, N, w2);

    for (n = 0; n < N; n++)
    {
        h[n] = hlp2[n] - hlp1[n];
    }
}

/**
 * @brief  理想带阻
 */
static void AudioFIR_GenerateIdealBSF(double *h, uint16_t N, double w1, double w2)
{
    double hbpf[AUDIO_FIR_MAX_TAP_NUM];
    uint16_t n;
    uint16_t m = (N - 1U) / 2U;

    AudioFIR_GenerateIdealBPF(hbpf, N, w1, w2);

    for (n = 0; n < N; n++)
    {
        h[n] = -hbpf[n];
    }

    h[m] += 1.0;
}

/* ============================================================
 * 窗函数
 * ============================================================ */

static double AudioFIR_WindowValue(WindowType type, uint16_t n, uint16_t N)
{
    double x;

    if (N <= 1U)
    {
        return 1.0;
    }

    x = 2.0 * M_PI * (double)n / (double)(N - 1U);

    switch (type)
    {
        case Rectangle:
            return 1.0;

        case triangle:
        {
            double m = (double)(N - 1U) / 2.0;
            return 1.0 - fabs(((double)n - m) / m);
        }

        case Hanning:
            return 0.5 - 0.5 * cos(x);

        case Hamming:
            return 0.54 - 0.46 * cos(x);

        case Blackman:
            return 0.42 - 0.5 * cos(x) + 0.08 * cos(2.0 * x);

        default:
            return 1.0;
    }
}

/* ============================================================
 * 数据格式转换
 * ============================================================ */

/**
 * @brief  double [-1.0, 1.0) 转 Q15
 */
static q15_t AudioFIR_DoubleToQ15(double x)
{
    int32_t y;

    if (x >= 0.999969482421875)   /* 32767 / 32768 */
    {
        return (q15_t)32767;
    }

    if (x <= -1.0)
    {
        return (q15_t)-32768;
    }

    y = (int32_t)(x * 32768.0);

    if (y > 32767)
    {
        y = 32767;
    }
    else if (y < -32768)
    {
        y = -32768;
    }

    return (q15_t)y;
}

/**
 * @brief  ADC12 unsigned -> Q15
 *
 * @note   0~4095, 中点2048
 */
static void AudioFIR_ADC12ToQ15(const uint16_t *src, q15_t *dst, uint32_t len)
{
    uint32_t i;
    int32_t x;
    int32_t q15;

    for (i = 0; i < len; i++)
    {
        x = (int32_t)src[i] - (int32_t)AUDIO_FIR_ADC_MID;

        /*
         * 12bit signed范围大约 [-2048, 2047]
         * 左移4位映射到近似Q15范围 [-32768, 32752]
         */
        q15 = x << 4;

        if (q15 > 32767)
        {
            q15 = 32767;
        }
        else if (q15 < -32768)
        {
            q15 = -32768;
        }

        dst[i] = (q15_t)q15;
    }
}

/**
 * @brief  Q15 -> DAC12 unsigned
 */
static void AudioFIR_Q15ToDAC12(const q15_t *src, uint16_t *dst, uint32_t len)
{
    uint32_t i;
    int32_t x;
    int32_t dac;

    for (i = 0; i < len; i++)
    {
        /*
         * Q15 右移4位恢复到约 12bit signed 幅值
         */
        x = ((int32_t)src[i]) >> 4;
        dac = x + (int32_t)AUDIO_FIR_DAC_MID;

        if (dac < 0)
        {
            dac = 0;
        }
        else if (dac > AUDIO_FIR_DAC_MAX)
        {
            dac = AUDIO_FIR_DAC_MAX;
        }

        dst[i] = (uint16_t)dac;
    }
}

/* ============================================================
 * 参数检查
 * ============================================================ */

static int AudioFIR_CheckParam(const TransferParam *param, FilterType type)
{
    if (param == NULL)
    {
        return 0;
    }

    if (param->fs <= 0.0)
    {
        return 0;
    }

    if (param->ast <= 0.0)
    {
        return 0;
    }

    switch (type)
    {
        case LOWPASSFILTER:
            if (!(param->fp1 > 0.0 &&
                  param->fst1 > 0.0 &&
                  param->fp1 < param->fst1 &&
                  param->fst1 < (param->fs / 2.0)))
            {
                return 0;
            }
            break;

        case HIGHPASSFILTER:
            if (!(param->fst1 > 0.0 &&
                  param->fp1 > 0.0 &&
                  param->fst1 < param->fp1 &&
                  param->fp1 < (param->fs / 2.0)))
            {
                return 0;
            }
            break;

        case BANDPASSFILTER:
            if (!(param->fst1 > 0.0 &&
                  param->fp1 > 0.0 &&
                  param->fp2 > 0.0 &&
                  param->fst2 > 0.0 &&
                  param->fst1 < param->fp1 &&
                  param->fp1 < param->fp2 &&
                  param->fp2 < param->fst2 &&
                  param->fst2 < (param->fs / 2.0)))
            {
                return 0;
            }
            break;

        case BANDSTOPFILTER:
            if (!(param->fp1 > 0.0 &&
                  param->fst1 > 0.0 &&
                  param->fst2 > 0.0 &&
                  param->fp2 > 0.0 &&
                  param->fp1 < param->fst1 &&
                  param->fst1 < param->fst2 &&
                  param->fst2 < param->fp2 &&
                  param->fp2 < (param->fs / 2.0)))
            {
                return 0;
            }
            break;

        default:
            return 0;
    }

    return 1;
}

/* ============================================================
 * 手写 Q15 FIR 块处理
 * ============================================================ */

/**
 * @brief  手写 Q15 FIR 块处理
 *
 * @note
 * 状态组织方式：
 *   state 长度 = numTaps + blockSize - 1
 *
 * 每次处理前：
 *   state[0 ... numTaps-2]         保存上次末尾历史
 *   state[numTaps-1 ... ]          拷贝本次输入
 *
 * 卷积后：
 *   将末尾 numTaps-1 个样本搬到开头，作为下次历史
 */
static void AudioFIR_ComputeQ15(const q15_t *src, q15_t *dst, uint32_t blockSize)
{
    uint32_t n, k;
    uint32_t numTaps;
    q15_t *state;
    int64_t acc;

    if (src == NULL || dst == NULL)
    {
        return;
    }

    numTaps = s_fir_tap_num;
    if (numTaps == 0U)
    {
        return;
    }

    state = s_fir_state_q15;

    /*
     * 新输入追加到状态尾部
     */
    memcpy(&state[numTaps - 1U], src, blockSize * sizeof(q15_t));

    /*
     * 计算每个输出点
     * y[n] = sum(state[n+k] * coeff[k]), k=0..numTaps-1
     *
     * 系数这里按正序存放：
     * coeff[0], coeff[1], ..., coeff[numTaps-1]
     */
    for (n = 0; n < blockSize; n++)
    {
        acc = 0;

        for (k = 0; k < numTaps; k++)
        {
            acc += (int32_t)state[n + k] * (int32_t)s_fir_coeff_q15[k];
        }

        /*
         * Q15 * Q15 = Q30
         * 转回 Q15
         */
        acc >>= 15;

        if (acc > AUDIO_FIR_Q15_MAX)
        {
            acc = AUDIO_FIR_Q15_MAX;
        }
        else if (acc < AUDIO_FIR_Q15_MIN)
        {
            acc = AUDIO_FIR_Q15_MIN;
        }

        dst[n] = (q15_t)acc;
    }

    /*
     * 保留末尾历史样本供下次使用
     */
    memmove(&state[0],
            &state[blockSize],
            (numTaps - 1U) * sizeof(q15_t));
}

/* ============================================================
 * 对外 API
 * ============================================================ */

/**
 * @brief  运行时重新设计 FIR
 */
int AudioFIR_Redesign(const TransferParam *param, FilterType type)
{
    WindowType window_type;
    uint16_t N;
    double h_ideal[AUDIO_FIR_MAX_TAP_NUM];
    double h_real[AUDIO_FIR_MAX_TAP_NUM];
    double wc, w1, w2;
    double sum;
    uint16_t n;

    if (AudioFIR_CheckParam(param, type) == 0)
    {
        return 0;
    }

    N = AudioFIR_SelectWindowAndTapNum(param, type, &window_type);
    if (N == 0U || N > AUDIO_FIR_MAX_TAP_NUM)
    {
        return 0;
    }

    memset(h_ideal, 0, sizeof(h_ideal));
    memset(h_real, 0, sizeof(h_real));

    switch (type)
    {
        case LOWPASSFILTER:
            wc = 2.0 * M_PI * ((param->fp1 + param->fst1) * 0.5) / param->fs;
            AudioFIR_GenerateIdealLPF(h_ideal, N, wc);
            break;

        case HIGHPASSFILTER:
            wc = 2.0 * M_PI * ((param->fp1 + param->fst1) * 0.5) / param->fs;
            AudioFIR_GenerateIdealHPF(h_ideal, N, wc);
            break;

        case BANDPASSFILTER:
            w1 = 2.0 * M_PI * ((param->fst1 + param->fp1) * 0.5) / param->fs;
            w2 = 2.0 * M_PI * ((param->fp2 + param->fst2) * 0.5) / param->fs;
            AudioFIR_GenerateIdealBPF(h_ideal, N, w1, w2);
            break;

        case BANDSTOPFILTER:
            w1 = 2.0 * M_PI * ((param->fp1 + param->fst1) * 0.5) / param->fs;
            w2 = 2.0 * M_PI * ((param->fst2 + param->fp2) * 0.5) / param->fs;
            AudioFIR_GenerateIdealBSF(h_ideal, N, w1, w2);
            break;

        default:
            return 0;
    }

    /*
     * 加窗
     */
    for (n = 0; n < N; n++)
    {
        h_real[n] = h_ideal[n] * AudioFIR_WindowValue(window_type, n, N);
    }

    /*
     * 对低通/带阻做直流增益归一化，避免幅值偏差太大
     * 对高通/带通不做这个直流归一化
     */
    if ((type == LOWPASSFILTER) || (type == BANDSTOPFILTER))
    {
        sum = 0.0;
        for (n = 0; n < N; n++)
        {
            sum += h_real[n];
        }

        if (fabs(sum) > AUDIO_FIR_EPSILON)
        {
            for (n = 0; n < N; n++)
            {
                h_real[n] /= sum;
            }
        }
    }

    /*
     * 转换到 Q15，按正序存放
     */
    for (n = 0; n < N; n++)
    {
        s_fir_coeff_q15[n] = AudioFIR_DoubleToQ15(h_real[n]);
    }

    /*
     * 未使用部分清零，方便调试
     */
    for (n = N; n < AUDIO_FIR_MAX_TAP_NUM; n++)
    {
        s_fir_coeff_q15[n] = 0;
    }

    s_fir_tap_num = N;

    AudioFIR_ResetState();

    return 1;
}

/**
 * @brief  初始化 FIR 模块
 */
int AudioFIR_Init(const TransferParam *param, FilterType type)
{
    s_fir_tap_num = 0;
    memset(s_fir_input_q15, 0, sizeof(s_fir_input_q15));
    memset(s_fir_output_q15, 0, sizeof(s_fir_output_q15));
    memset(s_fir_state_q15, 0, sizeof(s_fir_state_q15));
    memset(s_fir_coeff_q15, 0, sizeof(s_fir_coeff_q15));

    return AudioFIR_Redesign(param, type);
}

/**
 * @brief  处理一块 ADC12 数据，输出 DAC12 数据
 */
__attribute__((section(".ITCM_CODE")))
int AudioFIR_ProcessBlock_U16(const uint16_t *adc_src,
                              uint16_t *dac_dst,
                              uint32_t len)
{
    if (adc_src == NULL || dac_dst == NULL)
    {
        return 0;
    }

    if (s_fir_tap_num == 0U)
    {
        return 0;
    }

    if (len == 0U || len > AUDIO_FIR_BLOCK_SIZE)
    {
        return 0;
    }

    /*
     * Step 1:
     * ADC 12bit 数据转换为 Q15
     */
    AudioFIR_ADC12ToQ15(adc_src, s_fir_input_q15, len);

    /*
     * Step 2:
     * FIR
     */
    AudioFIR_ComputeQ15(s_fir_input_q15, s_fir_output_q15, len);

    /*
     * Step 3:
     * Q15 输出转换为 DAC 12bit
     */
    AudioFIR_Q15ToDAC12(s_fir_output_q15, dac_dst, len);

    return 1;
}

/**
 * @brief  处理一块 Q15 数据
 */
int AudioFIR_ProcessBlock_Q15(const q15_t *input_q15,
                              q15_t *output_q15,
                              uint32_t len)
{
    if (input_q15 == NULL || output_q15 == NULL)
    {
        return 0;
    }

    if (s_fir_tap_num == 0U)
    {
        return 0;
    }

    if (len == 0U || len > AUDIO_FIR_BLOCK_SIZE)
    {
        return 0;
    }

    AudioFIR_ComputeQ15(input_q15, output_q15, len);

    return 1;
}

/**
 * @brief  清空 FIR 状态
 */
void AudioFIR_ResetState(void)
{
    memset(s_fir_state_q15, 0, sizeof(s_fir_state_q15));
}

/**
 * @brief  获取当前 FIR 抽头数
 */
uint16_t AudioFIR_GetTapNum(void)
{
    return s_fir_tap_num;
}

/**
 * @brief  获取 Q15 FIR 系数
 */
const q15_t *AudioFIR_GetCoeffQ15(void)
{
    return s_fir_coeff_q15;
}
