/**
 * @file dpsd_config.h
 * @brief DPSD 配置参数（STM32H7 CORDIC 硬件加速版）
 * @note 适用于 STM32H743/H750/H7A3/H7B3 等支持 CORDIC 的芯片
 */

#ifndef DPSD_CONFIG_H
#define DPSD_CONFIG_H

#include <stdint.h>

/* ==================== 基本参数 ==================== */

// ADC 采样率（Hz）
#define DPSD_SAMPLE_RATE        2500000.0f

// 目标信号频率（Hz）
#define DPSD_TARGET_FREQ        10000.0f

// 每次处理的 ADC 采样点数
// 建议设置为整数个周期以避免频谱泄漏
// Fs/f0 = 2000000/10000 = 200 点/周期
// 推荐值：8000(40周期)、10000(50周期)、20000(100周期)
#define DPSD_BLOCK_SIZE         8192U

// ADC 数据类型
typedef uint16_t dpsd_adc_t;

// ADC 满量程值（12位ADC：4095，16位ADC：65535）
#define DPSD_ADC_FULL_SCALE     65535

// ADC 参考电压（V）
#define DPSD_ADC_VREF           1.65f

/* ==================== 参考信号生成方式 ==================== */

// 每个信号周期的采样点数
#define DPSD_POINTS_PER_CYCLE   250U

// 参考信号生成方式：
// 0 - 使用查找表（Q15定点数）
// 1 - 使用 CORDIC 实时计算
#define DPSD_USE_CORDIC         0

#if DPSD_USE_CORDIC
// CORDIC 计算精度（4-15，数值越大精度越高但速度越慢）
// STM32H7 CORDIC 迭代次数，推荐值：6
#define DPSD_CORDIC_PRECISION   6
#else
// 查找表 Q15 定点数缩放系数
typedef int16_t dpsd_lut_t;
#define DPSD_LUT_SCALE          32767
#endif

/* ==================== 频率跟踪参数 ==================== */

// 是否启用频率跟踪（1：启用，0：禁用）
#define DPSD_ENABLE_FREQ_TRACK  1

// 频率跟踪相位累积门限（弧度）
#define DPSD_PHASE_DRIFT_LIMIT  0.05f

// 频率校正系数（越小越平滑，建议 0.01 ~ 0.1）
#define DPSD_FREQ_CORRECTION    0.03f

// 频率允许偏移范围（相对于目标频率的百分比）
#define DPSD_FREQ_MAX_DRIFT     0.1f  // ±10%

/* ==================== 直流去除 ==================== */

// 是否启用直流去除（1：启用，0：禁用）
#define DPSD_ENABLE_DC_REMOVAL  1

/* ==================== 性能统计 ==================== */

// 是否启用性能统计（1：启用，0：禁用）
#define DPSD_ENABLE_PROFILING   1

/* ==================== 输出结果结构体 ==================== */

typedef struct {
    float amplitude_peak;   // 峰值幅度（V）
    float amplitude_rms;    // 有效值（V）
    float amplitude_pp;     // 峰峰值（V）
    float phase_deg;        // 相位（0~360度）
    float phase_rad;        // 相位（-π~π弧度）
    float frequency;        // 检测频率（Hz）
    float dc_offset_v;      // 直流偏置（V）
    uint32_t valid;         // 结果有效标志
} dpsd_result_t;

#if DPSD_ENABLE_PROFILING
typedef struct {
    uint32_t process_cycles;    // 处理耗时（CPU周期）
    uint32_t process_us;        // 处理耗时（微秒）
    uint32_t cordic_calls;      // CORDIC调用次数
    uint32_t total_processed;   // 总处理次数
} dpsd_profile_t;
#endif

#endif /* DPSD_CONFIG_H */
