/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "octospi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bspsysteam.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include <string.h>
#include <stdint.h>
/* USER CODE END Includes */


/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */


/* ===================== Cache & DMA 宏 ===================== */
/* USER CODE BEGIN 0 */


/* ===================== Cache & DMA 宏 ===================== */
static void DCache_Invalidate_By_Addr(void *addr, uint32_t size);
static void DCache_Clean_By_Addr(void *addr, uint32_t size);

#define CACHE_LINE_SIZE         32U

#define FFT_LENGTH              4096U
#define ADC_DMA_LENGTH          (FFT_LENGTH * 2U)
#define DAC_DMA_LENGTH          (FFT_LENGTH * 2U)
#define DAC_CENTER              2048.0f

/* ===================== PLL 核心算法宏 ===================== */

/*
 * 真实采样率 —— 必须与 TIM4 触发频率完全一致。
 * TIM4 配置: PSC=274(÷275), ARR=1(周期2), 定时器时钟137.5MHz -> 250kHz。
 * (曾试 ARR=0 凑 500kHz, 但该配置下定时器/ADC 触发不可靠, 已弃用。)
 * 若改硬件定时器/时钟, 必须同步修改此处; 增益在 DSP_Process_Init 中按本宏自动推导。
 */
#define SAMPLE_RATE             500000.0f

/* 输出信号要求的精确固定相移 (rad) */
#define PHASE_SHIFT_A           (PI * 0.694f)     /* A 路目标相移 */
#define PHASE_SHIFT_B           (-PI / 4.0f)      /* B 路目标相移 */

#define PEAK_GUARD_BINS         6
#define DC_IGNORE_BINS          4U

/* ---- 频率获取/跟踪 ---- */
#define FREQ_ALPHA              0.05f      /* 小差时中心频率一阶平滑系数 */
#define AMP_ALPHA               0.05f      /* 幅值平滑系数 */
#define FREQ_GATE_HZ            730.0f     /* 视为同一音的频率门限 (≈6 bin) */
#define MAX_SLEW_HZ             50.0f      /* (保留) 大跳变斜坡 */
#define LOCK_HZ                 5.0f       /* (保留) 频率锁定判据 Hz */
#define MISS_MAX                10U        /* 连续丢帧 N 块后进入 Hold */
#define ACQ_GATE_HZ            100.0f      /* 重获取频率门限 (~0.8 bin, 确保鉴相器在 Hann 窗高增益区) */
#define ACQ_CONFIRM             2U         /* 连续 N 块超门限才重获取, 防噪声误触发 */
#define LOCK_ERR                0.1f       /* 锁定判据:相位误差 (rad, ≈5.7°) */

/* ---- 信号存在性 (滞回双门限) ---- */
#define SIG_HI                  30.0f      /* 退出 Hold / 进入锁定的幅值门限 */
#define SIG_LO                  15.0f      /* 判定信号丢失的幅值门限 ( < SIG_HI ) */

/* ---- 二阶 PLL 增益 (由 fn, zeta 在 init 中推导; fs=500kHz, 块率 fb≈122Hz) ---- */
#define LOOP_FN                 1.5f       /* 环路自然频率 Hz (降低以增强噪声抑制) */
#define LOOP_ZETA               0.707f     /* 阻尼比 */
#define PLL_KP_SLEW             0.5f       /* 频率斜坡过渡期使用的弱比例增益 (Hz/rad) */
#define F_INT_MAX               30.0f      /* 积分项 anti-windup 限幅 (Hz) */

/* ---- 流水线延迟补偿 ---- */
#define PIPE_LATENCY_SAMPLES    8192U      /* 乒乓 DMA 输出滞后样本数 (≈2N, 可标定) */

/* 增益限制防削顶 (0.0~1.0) */
#define OUT_GAIN                0.8f

/* ===================== Cache 操作函数 ===================== */
__attribute__((section(".ITCM_CODE")))
static void DCache_Invalidate_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr = (uint32_t)addr;
    uint32_t end_addr = start_addr + size;
    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);
    SCB_InvalidateDCache_by_Addr((uint32_t *)start_addr, (int32_t)(end_addr - start_addr));
}

__attribute__((section(".ITCM_CODE")))
static void DCache_Clean_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr = (uint32_t)addr;
    uint32_t end_addr = start_addr + size;
    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);
    SCB_CleanDCache_by_Addr((uint32_t *)start_addr, (int32_t)(end_addr - start_addr));
}

/* ===================== DMA Buffer (RAM_D1) ===================== */
__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc_dma_buf[ADC_DMA_LENGTH];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t dac_A_dma_buf[DAC_DMA_LENGTH];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t dac_B_dma_buf[DAC_DMA_LENGTH];

/* ===================== DSP 计算 Buffer (DTCM) ===================== */
__attribute__((section(".DTCM_DATA"))) float32_t fft_buffer[FFT_LENGTH * 2U];
__attribute__((section(".DTCM_DATA"))) float32_t mag_buffer[FFT_LENGTH / 2U];
__attribute__((section(".DTCM_DATA"))) float32_t window_buf[FFT_LENGTH];

arm_cfft_instance_f32 fft_inst;
static float32_t window_coherent_gain;

/* ===================== 通道 PLL 状态 ===================== */
/*
 * 每个通道(DAC A / DAC B)一套独立的标准二阶 PLL 状态。
 * phase 跨块连续累积, 任何重锁/获取/Hold 都不复位 -> 输出相位绝对连续。
 */
typedef struct {
    float32_t f_hat;     /* NCO 跟踪频率 (Hz), 锁定时由环路积分器平滑驱动 */
    float32_t f_acq;     /* FFT 粗捕频率锚点 (Hz), 仅在(重)获取时更新 */
    float32_t f_int;     /* 环路积分项 (Hz) —— 长期频率修正 */
    float32_t f_nco;     /* 本块 NCO 输出频率 (Hz) = f_acq + f_int + KP*err */
    float32_t phase;     /* NCO 跟踪相位 (rad), 跨块连续, 永不复位 */
    float32_t amp;       /* 跟踪幅值 */
    float32_t shift;     /* 该通道目标相移 (rad) */
    float32_t last_err;  /* 上一次相位误差 (rad), 供调试 */
    uint8_t  acquired;   /* 是否已首次获取频率 */
    uint8_t  locked;     /* 是否锁定 */
    uint8_t  hold;       /* 是否处于 Hold (信号丢失保持) */
    uint8_t  miss_cnt;   /* 连续未匹配块数 */
    uint8_t  reacq_cnt;  /* 重获取确认计数 */
} pll_chan_t;

static pll_chan_t chA, chB;

/* 由 fn/zeta 推导的二阶 PLL 增益, 在 DSP_Process_Init 中计算 */
static float32_t pll_kp = 0.0f;
static float32_t pll_ki = 0.0f;

/* ===================== 调试及中断变量 ===================== */
volatile uint32_t adc_half_count = 0;
volatile uint32_t adc_full_count = 0;
volatile uint32_t adc_half_drop_count = 0;
volatile uint32_t adc_full_drop_count = 0;

volatile uint32_t dac_ch1_half_count = 0;
volatile uint32_t dac_ch1_full_count = 0;
volatile uint32_t dac_ch2_half_count = 0;
volatile uint32_t dac_ch2_full_count = 0;
volatile uint32_t dac_ch1_error_count = 0;
volatile uint32_t dac_ch2_error_count = 0;

volatile uint8_t adc_half_ready = 0;
volatile uint8_t adc_full_ready = 0;
volatile uint8_t dac_half_free  = 0;
volatile uint8_t dac_full_free  = 0;

/* 观察锁相状态 */
volatile float32_t debug_err_A = 0.0f;
volatile float32_t debug_err_B = 0.0f;
volatile float32_t debug_fhat_A = 0.0f;
volatile float32_t debug_fhat_B = 0.0f;
volatile float32_t debug_fnco_A = 0.0f;
volatile float32_t debug_fnco_B = 0.0f;

/* ===================== 算法工具函数 ===================== */

__attribute__((section(".ITCM_CODE")))
static float32_t estimate_freq_parabolic(uint32_t idx)
{
    float32_t ym1, y0, yp1, denom, delta, bin;

    if ((idx <= 1U) || (idx >= (FFT_LENGTH / 2U - 2U))) {
        return ((float32_t)idx * SAMPLE_RATE) / (float32_t)FFT_LENGTH;
    }

    ym1 = mag_buffer[idx - 1U];
    y0  = mag_buffer[idx];
    yp1 = mag_buffer[idx + 1U];

    denom = ym1 - 2.0f * y0 + yp1;
    if (fabsf(denom) < 1.0e-20f) {
        delta = 0.0f;
    } else {
        delta = 0.5f * (ym1 - yp1) / denom;
    }

    if (delta > 0.5f) delta = 0.5f;
    else if (delta < -0.5f) delta = -0.5f;

    bin = (float32_t)idx + delta;
    return bin * SAMPLE_RATE / (float32_t)FFT_LENGTH;
}

/* ===================== 系统初始化 ===================== */
void DSP_Process_Init(void)
{
    if (arm_cfft_init_f32(&fft_inst, FFT_LENGTH) != ARM_MATH_SUCCESS) {
        Error_Handler();
    }

    /* 预计算 Hann 窗 */
    window_coherent_gain = 0.0f;
    for (uint32_t n = 0; n < FFT_LENGTH; n++) {
        window_buf[n] = 0.5f - 0.5f * arm_cos_f32((2.0f * PI * (float32_t)n) / ((float32_t)FFT_LENGTH - 1.0f));
        window_coherent_gain += window_buf[n];
    }
    window_coherent_gain /= (float32_t)FFT_LENGTH;

    /* 由自然频率/阻尼推导二阶 PLL 增益 (标准 type-2 环):
     *   KP = zeta*wn/pi            [Hz/rad]
     *   KI = wn^2 * Tb / (2*pi)    [Hz/(rad*block)]   Tb = N/fs
     * 保证 fs 改变时增益自动一致 */
    {
        float32_t Tb = (float32_t)FFT_LENGTH / SAMPLE_RATE;
        float32_t wn = 2.0f * PI * LOOP_FN;
        pll_kp = LOOP_ZETA * wn / PI;
        pll_ki = wn * wn * Tb / (2.0f * PI);
    }

    /* 初始化两通道: 上电默认 Hold, 等待信号 */
    chA.f_hat = 0.0f; chA.f_acq = 0.0f; chA.f_int = 0.0f; chA.f_nco = 0.0f; chA.phase = 0.0f;
    chA.amp = 0.0f;  chA.shift = PHASE_SHIFT_A; chA.last_err = 0.0f;
    chA.acquired = 0; chA.locked = 0; chA.hold = 1; chA.miss_cnt = 0; chA.reacq_cnt = 0;

    chB.f_hat = 0.0f; chB.f_acq = 0.0f; chB.f_int = 0.0f; chB.f_nco = 0.0f; chB.phase = 0.0f;
    chB.amp = 0.0f;  chB.shift = PHASE_SHIFT_B; chB.last_err = 0.0f;
    chB.acquired = 0; chB.locked = 0; chB.hold = 1; chB.miss_cnt = 0; chB.reacq_cnt = 0;
}

static void Audio_DMA_Buffer_Init(void)
{
    for (uint32_t i = 0; i < DAC_DMA_LENGTH; i++) {
        dac_A_dma_buf[i] = (uint16_t)DAC_CENTER;
        dac_B_dma_buf[i] = (uint16_t)DAC_CENTER;
    }
    DCache_Invalidate_By_Addr(adc_dma_buf, ADC_DMA_LENGTH * sizeof(uint16_t));
    DCache_Clean_By_Addr(dac_A_dma_buf, DAC_DMA_LENGTH * sizeof(uint16_t));
    DCache_Clean_By_Addr(dac_B_dma_buf, DAC_DMA_LENGTH * sizeof(uint16_t));
}

/* ===================== 单通道正交(相干)PLL ===================== */
/*
 * 全新方法 —— 解决高频频飘:
 * 旧法每块用 FFT 重新估计频率并平滑进 f_hat, FFT 抛物线频率噪声随之进入 NCO,
 * 经延迟补偿放大为相位目标噪声 -> 输出频率游走 (高频 SNR 低, 噪声更大, 飘得更狠)。
 *
 * 新法: FFT 仅用于粗捕与(重)获取; 锁定后频率完全由二阶环积分器平滑驱动,
 * 相位误差用"相干 I/Q 相关"高 SNR 检测 (等效于在 NCO 自身频率上的加窗 DFT):
 *   I = Σ w[n]·x[n]·cos(phase_n) ∝ cos(phase-θ),  Q = Σ w[n]·x[n]·sin(phase_n) ∝ sin(phase-θ)
 *   err = atan2(-Q, I) = θ - phase = 输入相位 - NCO相位  (取负保证负反馈)
 * NCO 频率 f = f_acq + f_int + KP·err; 相位逐样本累加, 永不复位 -> 连续。
 * 输出 = cos(phase + 流水线延迟补偿 + 目标相移) (与 I/Q 的 cos 参考同型, 相移精确), 延迟项用平滑的 f_hat。
 * has_tone/amp_fft 来自 FFT (粗捕与信号检测); xzm 为去直流数据。
 */
__attribute__((section(".ITCM_CODE")))
static void pll_run_chan(pll_chan_t *c, uint8_t has_tone,
                         float32_t f_meas, float32_t amp_fft,
                         const float32_t *xzm, uint16_t *out, uint32_t off)
{
    /* 1. 信号存在性 (用 FFT 幅值做滞回, 与 I/Q 解耦, 便于先决定获取) */
    if (has_tone && amp_fft >= SIG_HI) {
        c->hold = 0;
    } else if (amp_fft < SIG_LO) {
        c->hold = 1; c->locked = 0;
    }
    if (!has_tone) { if (c->miss_cnt < 255U) c->miss_cnt++; }
    else           { c->miss_cnt = 0; }
    if (c->miss_cnt > MISS_MAX) { c->hold = 1; c->locked = 0; }

    /* 2. (重)获取: 仅当未获取, 或 FFT 频率偏离跟踪频率超门限且持续确认。
     *    锁定时 f_meas 的 FFT 噪声不会进入 f_hat (这是频飘消除的关键)。 */
    if (has_tone && !c->hold) {
        if (!c->acquired) {
            c->f_hat = f_meas; c->f_acq = f_meas; c->f_int = 0.0f; c->acquired = 1;
            c->reacq_cnt = 0;
        } else if (fabsf(f_meas - c->f_hat) > ACQ_GATE_HZ) {
            if (c->reacq_cnt < 255U) c->reacq_cnt++;
            if (c->reacq_cnt >= ACQ_CONFIRM) {
                c->f_hat = f_meas; c->f_acq = f_meas; c->f_int = 0.0f; c->reacq_cnt = 0;
            }
        } else {
            c->reacq_cnt = 0;
        }
    }

    /* 2.5 频差预补偿: 当 FFT 测量值与跟踪频率偏差超过 30 Hz,
     *     逐步软拉 f_acq 向测量值靠拢, 避免鉴相器长期在 Hann 窗低增益区运行。
     *     门限 30 Hz 远小于 Hann 窗第一零点 (244 Hz) 和重捕获门限 (100 Hz),
     *     仅在鉴相器增益尚好时柔和介入, 不影响正常锁定的 PLL 动态。 */
    if (has_tone && !c->hold && c->acquired) {
        float32_t f_track = c->f_acq + c->f_int;
        float32_t f_diff = fabsf(f_meas - f_track);
        if (f_diff > 30.0f) {
            c->f_acq += FREQ_ALPHA * (f_meas - c->f_acq);
            c->f_hat  = c->f_acq + c->f_int;   /* 保持 f_hat 与锚点一致 */
        }
    }

    /* 3. 相干 I/Q 相位检测 (在当前 NCO 频率/相位上, 递推旋转) */
    float32_t omega = 2.0f * PI * c->f_hat / SAMPLE_RATE;
    float32_t cw = arm_cos_f32(c->phase);
    float32_t sw = arm_sin_f32(c->phase);
    float32_t cs = arm_cos_f32(omega);
    float32_t ss = arm_sin_f32(omega);
    float32_t I_acc = 0.0f, Q_acc = 0.0f;
    for (uint32_t n = 0; n < FFT_LENGTH; n++) {
        float32_t xn = xzm[n] * window_buf[n];
        I_acc += xn * cw;
        Q_acc += xn * sw;
        float32_t nc = cw * cs - sw * ss;
        float32_t ns = sw * cs + cw * ss;
        cw = nc; sw = ns;
    }
    float32_t mag = sqrtf(I_acc * I_acc + Q_acc * Q_acc);
    float32_t amp_iq = 2.0f * mag / ((float32_t)FFT_LENGTH * window_coherent_gain);
    /* err = 输入相位 - NCO相位 = atan2(-Q, I) (I∝cos(phase-θ), Q∝sin(phase-θ),
     * 故 atan2(Q,I)=phase-θ; 取负得 θ-phase, 保证 err>0(NCO落后)时增频为负反馈) */
    float32_t err = atan2f(-Q_acc, I_acc);

    /* 4. 二阶环路滤波器 (比例/积分均作用于频率) */
    if (c->acquired && !c->hold) {
        c->f_int += pll_ki * err;
        if (c->f_int >  F_INT_MAX) c->f_int =  F_INT_MAX;
        if (c->f_int < -F_INT_MAX) c->f_int = -F_INT_MAX;
        c->f_nco = c->f_acq + c->f_int + pll_kp * err;
        c->amp   += AMP_ALPHA * (amp_iq - c->amp);
        c->locked  = (fabsf(err) < LOCK_ERR) ? 1U : 0U;
        c->last_err = err;
    } else {
        c->f_nco = c->f_hat;                 /* Hold: 冻结频率 */
        c->last_err = 0.0f;
    }

    /* 5. NCO 连续生成: 逐样本累加跟踪相位 -> 输出相位绝对连续。
     *    输出 = cos(跟踪相位 + 流水线延迟补偿 + 目标相移)。
     *    使用当前块新计算的 f_nco (非上一块的 f_hat), 消除一帧延迟。 */
    {
        float32_t step  = 2.0f * PI * c->f_nco / SAMPLE_RATE;
        float32_t phoff = 2.0f * PI * c->f_nco * (float32_t)PIPE_LATENCY_SAMPLES / SAMPLE_RATE + c->shift;
        float32_t ph = c->phase;
        for (uint32_t n = 0; n < FFT_LENGTH; n++) {
            float32_t val = DAC_CENTER + OUT_GAIN * c->amp * arm_cos_f32(ph + phoff);
            out[off + n] = (uint16_t)__USAT((int32_t)val, 12);
            ph += step;
            if      (ph >= 2.0f * PI) ph -= 2.0f * PI;
            else if (ph <  0.0f)      ph += 2.0f * PI;
        }
        c->phase = ph;                       /* 下一块起始跟踪相位 */
    }

    /* 6. 本块输出频率作为下一块跟踪频率 (Hold 时保持冻结) */
    if (c->acquired && !c->hold) c->f_hat = c->f_nco;
}

/* ===================== 核心信号处理 ===================== */
__attribute__((section(".ITCM_CODE")))
void Process_DSP_Block(uint32_t offset)
{
    uint32_t block_bytes = FFT_LENGTH * sizeof(uint16_t);
    float32_t mean = 0.0f;
    float32_t max1_val, max2_val;
    uint32_t idx1, idx2;

    float32_t freq1, freq2, amp1, amp2;             /* 两个峰: 频率(抛物线)与幅值(FFT) */
    float32_t Tf[2], Ta[2];                          /* 规整后: T[0]=低频, T[1]=高频 */
    uint32_t ai, bi;                                  /* 通道 A/B 取的峰索引 */
    uint8_t  a_has, b_has;
    float32_t a_f, a_a, b_f, b_a;

    DCache_Invalidate_By_Addr(&adc_dma_buf[offset], block_bytes);

    /* 1. 去直流 */
    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        mean += (float32_t)adc_dma_buf[offset + i];
    }
    mean /= (float32_t)FFT_LENGTH;

    /* 2. 装载 FFT 缓冲并加窗 (同时保存一份未加窗但去直流的数据在实部，供DFT用) */
    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        float32_t x = (float32_t)adc_dma_buf[offset + i] - mean;
        fft_buffer[i * 2U] = x * window_buf[i];
        fft_buffer[i * 2U + 1U] = 0.0f;
    }

    /* 3. 执行 FFT 找峰值 */
    arm_cfft_f32(&fft_inst, fft_buffer, 0, 1);
    arm_cmplx_mag_f32(fft_buffer, mag_buffer, FFT_LENGTH / 2U);

    for (uint32_t i = 0; i < DC_IGNORE_BINS; i++) {
        mag_buffer[i] = 0.0f;
    }

    /* 4. 寻找两个主频 */
    arm_max_f32(mag_buffer, FFT_LENGTH / 2U, &max1_val, &idx1);
    freq1 = estimate_freq_parabolic(idx1);

    for (int32_t i = (int32_t)idx1 - PEAK_GUARD_BINS; i <= (int32_t)idx1 + PEAK_GUARD_BINS; i++) {
        if (i > 0 && i < (int32_t)FFT_LENGTH / 2) mag_buffer[i] = 0.0f;
    }

    arm_max_f32(mag_buffer, FFT_LENGTH / 2U, &max2_val, &idx2);
    freq2 = estimate_freq_parabolic(idx2);

    /* 5. 由 FFT 峰值估算两峰幅值 (用于信号检测/关联; 相位由 PLL 自身相干检测) */
    amp1 = 2.0f * max1_val / ((float32_t)FFT_LENGTH * window_coherent_gain);
    amp2 = 2.0f * max2_val / ((float32_t)FFT_LENGTH * window_coherent_gain);

    /* 6. 装载去直流数据 (供相干 I/Q 用), 规整 T[0]=低频, T[1]=高频 */
    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        fft_buffer[i] = (float32_t)adc_dma_buf[offset + i] - mean;
    }
    Tf[0] = freq1; Ta[0] = amp1;
    Tf[1] = freq2; Ta[1] = amp2;
    if (Tf[0] > Tf[1]) {
        float32_t t;
        t = Tf[0]; Tf[0] = Tf[1]; Tf[1] = t;
        t = Ta[0]; Ta[0] = Ta[1]; Ta[1] = t;
    }

    /* 7. 峰-通道关联 (最近频率配对防交换; 首次获取低频->A, 高频->B) */
    if (!chA.acquired && !chB.acquired) {
        ai = 0; bi = 1;
    } else {
        float32_t d01 = fabsf(Tf[0] - chA.f_hat) + fabsf(Tf[1] - chB.f_hat);
        float32_t d10 = fabsf(Tf[1] - chA.f_hat) + fabsf(Tf[0] - chB.f_hat);
        if (d01 <= d10) { ai = 0; bi = 1; } else { ai = 1; bi = 0; }
    }
    /* 通道已获取且候选峰远离 f_hat 时, 若信号弱则视为该音缺失(交由 Hold 处理) */
    a_has = 1; a_f = Tf[ai]; a_a = Ta[ai];
    b_has = 1; b_f = Tf[bi]; b_a = Ta[bi];
    if (chA.acquired && fabsf(a_f - chA.f_hat) > FREQ_GATE_HZ && a_a < SIG_HI) a_has = 0;
    if (chB.acquired && fabsf(b_f - chB.f_hat) > FREQ_GATE_HZ && b_a < SIG_HI) b_has = 0;

    /* 8. 逐通道正交 PLL: 频率由环路平滑跟踪, 不受每块 FFT 噪声扰动 */
    pll_run_chan(&chA, a_has, a_f, a_a, fft_buffer, dac_A_dma_buf, offset);
    pll_run_chan(&chB, b_has, b_f, b_a, fft_buffer, dac_B_dma_buf, offset);

    /* 9. 调试观测 */
    debug_err_A  = chA.last_err;  debug_err_B  = chB.last_err;
    debug_fhat_A = chA.f_hat;     debug_fhat_B = chB.f_hat;
    debug_fnco_A = chA.f_nco;     debug_fnco_B = chB.f_nco;

    /* DMA 前 Clean Cache */
    DCache_Clean_By_Addr(&dac_A_dma_buf[offset], block_bytes);
    DCache_Clean_By_Addr(&dac_B_dma_buf[offset], block_bytes);
}

/* ===================== 中断乒乓调度与DMA启动 ===================== */
__attribute__((section(".ITCM_CODE")))
void adc_proc(void)
{
    uint8_t do_half = 0U;
    uint8_t do_full = 0U;

    __disable_irq();
    if ((adc_half_ready != 0U) && (dac_half_free != 0U)) {
        adc_half_ready = 0U; dac_half_free = 0U; do_half = 1U;
    }
    if ((adc_full_ready != 0U) && (dac_full_free != 0U)) {
        adc_full_ready = 0U; dac_full_free = 0U; do_full = 1U;
    }
    __enable_irq();

    if (do_half != 0U) {
        Process_DSP_Block(0U);
        adc_half_count++;
    }
    if (do_full != 0U) {
        Process_DSP_Block(FFT_LENGTH);
        adc_full_count++;
    }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        if (adc_half_ready != 0U) adc_half_drop_count++;
        adc_half_ready = 1U;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        if (adc_full_ready != 0U) adc_full_drop_count++;
        adc_full_ready = 1U;
    }
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) {
        dac_ch1_half_count++;
        dac_half_free = 1U;
    }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) {
        dac_ch1_full_count++;
        dac_full_free = 1U;
    }
}

void HAL_DAC_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) dac_ch2_half_count++;
}

void HAL_DAC_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) dac_ch2_full_count++;
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) dac_ch1_error_count++;
}

void HAL_DAC_ErrorCallbackCh2(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance == DAC1) dac_ch2_error_count++;
}

/* 外部句柄声明 (CubeMX 生成在 main.c 中) */
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim4;

static void Audio_DMA_Start(void)
{
    Audio_DMA_Buffer_Init();

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&adc_dma_buf[0], ADC_DMA_LENGTH) != HAL_OK) Error_Handler();
    if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)&dac_A_dma_buf[0], DAC_DMA_LENGTH, DAC_ALIGN_12B_R) != HAL_OK) Error_Handler();
    if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t *)&dac_B_dma_buf[0], DAC_DMA_LENGTH, DAC_ALIGN_12B_R) != HAL_OK) Error_Handler();
    if (HAL_TIM_Base_Start(&htim4) != HAL_OK) Error_Handler();
}

/* USER CODE END 0 */


/* USER CODE END 0 */




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  MX_OCTOSPI1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_DAC1_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	SRAM_Clock_Enable();    // 如果这个函数确实还需要，放在时钟后
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	SRAM_Clock_Enable();

  scheduler_Init();

 DSP_Process_Init();

  /* 3. 启动 ADC 和 DAC 的 DMA 传输 */
  /* 这个函数会一次性启动 ADC、两个 DAC 通道和定时器 */
  Audio_DMA_Start();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	adc_proc();

	/* 周期调试打印 (1s 一次, 非实时关键路径; 115200 baud ~5ms)
	 * 用整数缩放避免依赖浮点 printf: f*=Hz, e*=毫弧度 */
//	{
//		static uint32_t dbg_last = 0xFFFFFFFFU;
//		uint32_t now = HAL_GetTick();
//		if (dbg_last == 0xFFFFFFFFU) dbg_last = now;
//		if ((now - dbg_last) >= 1000U) {
//			dbg_last = now;
//			UART1_Printf("fA=%d fB=%d nA=%d nB=%d eA=%d eB=%d %c%c\r\n",
//			             (int)debug_fhat_A, (int)debug_fhat_B,
//			             (int)debug_fnco_A, (int)debug_fnco_B,
//			             (int)(debug_err_A * 1000.0f), (int)(debug_err_B * 1000.0f),
//			             chA.locked ? 'L' : (chA.hold ? 'H' : '-'),
//			             chB.locked ? 'L' : (chB.hold ? 'H' : '-'));
//		}
//	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0xE0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER5;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
  while (1)
  {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
