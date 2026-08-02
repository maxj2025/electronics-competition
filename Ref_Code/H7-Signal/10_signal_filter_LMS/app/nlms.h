#ifndef __NLMS_H
#define __NLMS_H

#include "arm_math.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================== */
/*  采样率与滤波器参数                                                  */
/* ================================================================== */

/*
 * 采样率 390.625 kHz
 * TIM4: PSC=21, ARR=31, APB1 Timer Clock = 275 MHz
 * fs = 275 MHz / 22 / 32 = 390,625 Hz
 *
 * 频率分辨率 = fs / N
 *  128 阶 @ 390.625 kHz: Δf ≈ 3.05 kHz
 *  128 阶 @ 200 kHz:     Δf ≈ 1.56 kHz (降低采样率效果更好)
 *
 * ★ 降低采样率方案（修改 TIM4 + 本宏）：
 *   TIM4 PSC=54, ARR=24 → fs = 200 kHz
 *   TIM4 PSC=39, ARR=43 → fs = 156.25 kHz
 */
#define NLMS_FS_HZ                  (100000UL)

/*
 * FIR 滤波器阶数
 *
 * 128 阶配合 arm_fir_f32，实时 CPU 占用仅约 5%
 * 如需更高精度可改为 192 或 256
 */
#define NLMS_NUM_TAPS               (64U)

/* 实时 DMA 双缓冲的半缓冲长度 */
#define NLMS_BLOCK_SIZE             (256U)

/* 训练阶段每批采样点数 */
#define NLMS_TRAIN_BLOCK_SIZE       (4096U)

/* 训练阶段丢弃每批前 N 个样本（阶数越大需要丢弃越多） */
#define NLMS_TRAIN_DISCARD_SAMPLES  (128U)

/* d(n) 与 x(n) 之间的时间对齐延迟采样数 */
#define NLMS_D_DELAY_SAMPLES        (0U)

/* ================================================================== */
/*  训练参数                                                            */
/* ================================================================== */

/*
 * 训练时间 10000 ms
 *
 * 390625 Hz × 10 s = 3,906,250 样本
 * 需要更长训练时间来弥补较小的 μ
 */
#define NLMS_TRAIN_TIME_MS          (3000U)
#define NLMS_TRAIN_SAMPLES          \
    ((NLMS_FS_HZ * NLMS_TRAIN_TIME_MS) / 1000UL)

/*
 * 三段式学习率
 *
 * ★★★ 关键：μ 必须满足 M = μ × N < 1.0 才能稳定收敛 ★★★
 *
 * 阶段 1 (0% ~ 40%):   MU_FAST = 0.005  → M=0.64  快速逼近
 * 阶段 2 (40% ~ 75%):  MU_MID  = 0.002  → M=0.256 中等收敛
 * 阶段 3 (75% ~ 100%): MU_FINE = 0.0003 → M=0.038 精细微调
 *
 * 之前的 MU_FAST=0.08 导致 M=10.24，系数疯狂振荡！
 */
#define NLMS_FAST_SAMPLES           \
    ((NLMS_TRAIN_SAMPLES * 40UL) / 100UL)

#define NLMS_MID_SAMPLES            \
    ((NLMS_TRAIN_SAMPLES * 75UL) / 100UL)

#define NLMS_MU_FAST                (0.005f)
#define NLMS_MU_MID                 (0.002f)
#define NLMS_MU_FINE                (0.0003f)

/*
 * 正则化参数 ε
 *
 * 防止输入功率接近零时 gain = μ·e/power 爆炸
 *
 * PRBS幅度0.6时，输入功率约 128×0.36 = 46
 * ε 应为输入功率的 1%~10%，即 0.5~5.0
 * 设为 1.0 提供强正则化，防止训练初期增益爆炸
 */
#define NLMS_EPSILON                (1.0f)

/*
 * 泄漏因子
 *
 * 系统辨识场景下应设为 1.0f（不泄漏）。
 * 训练完成后系数已被冻结，不会漂移。
 * 如果确实需要泄漏（例如时变系统），建议 ≥ 0.999999f。
 */
#define NLMS_LEAKAGE                (1.0f)

/*
 * 系数最大绝对值限制
 *
 * 防止训练异常时系数发散到极大值
 * 典型 RLC 滤波器的 FIR 系数不会超过 5.0
 * 如果训练后某些系数接近此限制，说明训练有问题
 */
#define NLMS_COEFF_MAX_ABS          (10.0f)

/*
 * 增益最大绝对值限制
 *
 * 限制单次系数更新幅度，防止大误差导致系数突变
 * 设为 0.1 表示每个系数每步最多变化 0.1 × |x|
 */
#define NLMS_GAIN_MAX_ABS           (0.1f)

/* ================================================================== */
/*  ITCM / DTCM 属性宏                                                  */
/* ================================================================== */

#if defined(__GNUC__) || defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define NLMS_ITCM_CODE  __attribute__((section(".ITCM_CODE"), noinline))
#define NLMS_DTCM_DATA  __attribute__((section(".DTCM_DATA"), aligned(32)))
#else
#define NLMS_ITCM_CODE
#define NLMS_DTCM_DATA
#endif

/* ================================================================== */
/*  函数声明                                                            */
/* ================================================================== */

void NLMS_RLC_Init(void);

void NLMS_RLC_ProcessBlock(const float32_t *x,
                           const float32_t *d,
                           float32_t *y,
                           float32_t *e);

void NLMS_RLC_PrepareFIROnly(void);

void NLMS_RLC_ProcessFIROnly(const float32_t *x,
                             float32_t *y,
                             uint32_t blockSize);

uint8_t NLMS_RLC_IsTrainingDone(void);
uint32_t NLMS_RLC_GetTrainCount(void);
const float32_t *NLMS_RLC_GetCoefficients(void);
uint32_t NLMS_RLC_GetMaxCycles(void);
uint32_t NLMS_RLC_GetOverrunCount(void);
void NLMS_RLC_ResetPerformance(void);

/*
 * 获取训练完成时的最终均方误差
 * 可用来判断训练质量：MSE 应远小于 0.01
 */
float32_t NLMS_RLC_GetFinalMSE(void);

#ifdef __cplusplus
}
#endif

#endif
