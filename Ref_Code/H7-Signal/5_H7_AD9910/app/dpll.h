/**
  ******************************************************************************
  * @file    dpll.h
  * @brief   数字锁相环(DPLL)头文件，基于FFT相位检测 + PI控制器
  *          适配STM32 + CMSIS-DSP库，实现两路信号的相位锁定
  ******************************************************************************
  */

#ifndef __DPLL_H
#define __DPLL_H

#include "main.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "extra_ffts.h"
#include "arm_const_structs_extra.h"
#include "arm_common_tables_extra.h"
#include "math.h"
#include "stdlib.h"

// ==================== 硬件配置参数（请根据实际硬件修改） ====================
/**
 * @brief FFT运算点数
 * @note  必须为256/512/1024/2048，匹配CMSIS-DSP预定义FFT结构体
 *        点数越大 → 频率/相位精度越高，但运算量越大、占用RAM越多
 */
#define FFT_N       2048

/**
 * @brief ADC采样率，单位Hz
 * @note  必须与硬件ADC实际配置的采样频率完全一致，否则频率、相位计算全部失真
 *        需满足奈奎斯特采样定理：FS > 2×输入信号最高频率，建议5~10倍以上以保证精度
 */
#define FS          2200000.0f

/**
 * @brief FFT频率分辨率 = FS / FFT_N，单位Hz
 */
#define FREQ_RESOLUTION  (FS / FFT_N)
// =============================================================================

/**
 * @brief PI控制器结构体
 * @note  输入为相位误差(rad)，输出为频率调整量(Hz)，用于DPLL环路滤波
 */
typedef struct {
    float Kp;         ///< 比例系数，单位：Hz/rad，决定相位偏差的即时响应速度
    float Ki;         ///< 积分系数，单位：Hz/(rad·s)，用于消除稳态相位误差
    float integral;   ///< 积分累积值，单位：Hz，保存历史误差的积分结果
    float out_max;    ///< 输出限幅值，单位：Hz，正负对称，限制最大频率调整量
    float int_max;    ///< 积分限幅值，单位：Hz，防止积分饱和导致的超调与失控
} PI_HandleTypeDef;

/**
 * @brief  计算两路ADC信号的基波相位差（FFT频谱法），同时输出各自基波频率
 * @param  adc_ref : 参考信号原始ADC采样数组，长度为FFT_N
 * @param  adc_fb  : 反馈信号原始ADC采样数组，长度为FFT_N
 * @param  freq_ref: [出参] 参考通道基波频率(Hz)，传NULL忽略
 * @param  freq_fb : [出参] 反馈通道基波频率(Hz)，传NULL忽略
 * @retval 相位差，单位：弧度，范围 (-π, π]
 *         两路信号基波频点不同时返回0（锁频未完成，相位无意义）
 * @note   首次调用自动初始化汉宁窗系数；两路各自独立搜索基波频点
 */
float FFT_Calc_Phase_Diff(uint32_t *adc_ref, uint32_t *adc_fb,
                          float *freq_ref, float *freq_fb);

/**
 * @brief  PI控制器运算函数
 * @param  pi    : PI控制器结构体指针
 * @param  error : 输入误差量，此处为相位差，单位：弧度
 * @retval 频率调整量 delta_f，单位：Hz，可正可负
 * @note   内置积分限幅与输出限幅，具备抗积分饱和能力
 */
float PI_Calc(PI_HandleTypeDef *pi, float error);

#endif /* __DPLL_H */

