/**
 * @file dpsd.h
 * @brief DPSD 数字相敏检测接口（STM32H7 CORDIC加速版）
 */

#ifndef DPSD_H
#define DPSD_H

#include "dpsd_config.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "cordic.h"
/* ==================== 初始化函数 ==================== */

/**
 * @brief 初始化 DPSD 模块
 * @param hcordic CORDIC 句柄指针（使用CORDIC时必须提供）
 * @return 0：成功，-1：失败
 */
int32_t DPSD_Init(CORDIC_HandleTypeDef *hcordic);

/**
 * @brief 反初始化 DPSD 模块
 */
void DPSD_DeInit(void);

/* ==================== 处理函数 ==================== */

/**
 * @brief 处理一组 ADC 采样数据
 * @param adc_data ADC 数据缓冲区（长度必须为 DPSD_BLOCK_SIZE）
 * @param result 输出结果指针
 * @return 0：成功，-1：失败
 */
int32_t DPSD_Process(const dpsd_adc_t *adc_data, dpsd_result_t *result);

/* ==================== 配置函数 ==================== */

/**
 * @brief 设置目标频率
 * @param freq_hz 目标频率（Hz）
 * @return 0：成功，-1：失败
 */
int32_t DPSD_SetTargetFrequency(float freq_hz);

/**
 * @brief 获取当前目标频率
 * @return 当前目标频率（Hz）
 */
float DPSD_GetTargetFrequency(void);

/**
 * @brief 重置频率跟踪状态
 */
void DPSD_ResetFrequencyTracking(void);

/* ==================== 性能统计函数 ==================== */

#if DPSD_ENABLE_PROFILING

/**
 * @brief 获取性能统计信息
 * @param profile 性能统计结果指针
 */
void DPSD_GetProfile(dpsd_profile_t *profile);

/**
 * @brief 重置性能统计
 */
void DPSD_ResetProfile(void);

#endif

/* ==================== CORDIC 辅助函数 ==================== */

#if DPSD_USE_CORDIC

/**
 * @brief 使用 CORDIC 计算 sin 和 cos
 * @param angle 角度（弧度，-π 到 π）
 * @param sin_out 输出 sin 值
 * @param cos_out 输出 cos 值
 */
void DPSD_CORDIC_SinCos(float angle, float *sin_out, float *cos_out);

/**
 * @brief 使用 CORDIC 计算 atan2
 * @param y Y坐标
 * @param x X坐标
 * @return 角度（弧度，-π 到 π）
 */
float DPSD_CORDIC_Atan2(float y, float x);

/**
 * @brief 使用 CORDIC 计算模长 sqrt(x^2 + y^2)
 * @param x X坐标
 * @param y Y坐标
 * @return 模长
 */
float DPSD_CORDIC_Magnitude(float x, float y);

#endif

#endif /* DPSD_H */
