/**
  ******************************************************************************
  * @file    dpll.c
  * @brief   数字锁相环(DPLL)实现，包含FFT相位检测与PI控制器
  *          基于ARM CMSIS-DSP库，硬件适配STM32系列MCU
  ******************************************************************************
  */

#include "dpll.h"
#include "extra_ffts.h"
#include "arm_const_structs_extra.h"
#include "arm_common_tables_extra.h"
#include <math.h>
#include <stdint.h>

/* 圆周率常量定义，提升精度与代码可读性 */
#define PI1          3.14159265358979323846f
#define PI2          6.28318530717958647692f

/**
 * @brief 根据FFT点数自动匹配扩展版 CMSIS-DSP FFT 结构体
 * @note  支持 16~16384 点 FFT，重点支持 4096/8192/16384 长点数 FFT
 */
static arm_cfft_instance_f32_extra *dpll_fft_get_instance(uint16_t fft_n)
{
    switch (fft_n)
    {
        case 16:    return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len16_extra;
        case 32:    return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len32_extra;
        case 64:    return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len64_extra;
        case 128:   return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len128_extra;
        case 256:   return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len256_extra;
        case 512:   return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len512_extra;
        case 1024:  return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len1024_extra;
        case 2048:  return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len2048_extra;
        case 4096:  return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len4096_extra;
        case 8192:  return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len8192_extra;
        case 16384: return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len16384_extra;
        default:    return (arm_cfft_instance_f32_extra *)&arm_cfft_sR_f32_len2048_extra;
    }
}

/* 全局缓冲区定义，放在全局区避免栈溢出 */
__attribute__((section(".RAM_D1"))) float hanning_win[FFT_N];
__attribute__((section(".RAM_D2"))) float fft_buf_ref[FFT_N * 2];
__attribute__((section(".RAM_D3"))) float fft_buf_fb[FFT_N * 2];

/**
 * @brief  单函数计算两路ADC信号的基波相位差，FFT频谱法
 * @param  adc_ref : 参考信号原始ADC数组，长度FFT_N
 * @param  adc_fb  : 反馈信号原始ADC数组，长度FFT_N
 * @param  freq_ref: 参考信号频率输出指针，当前代码未使用
 * @param  freq_fb : 反馈信号频率输出指针，当前代码未使用
 * @retval 相位差，单位弧度，范围：(-π, π]
 */
float FFT_Calc_Phase_Diff(uint32_t *adc_ref, uint32_t *adc_fb, float *freq_ref, float *freq_fb)
{
    static uint8_t init_flag = 0;

    float avg_ref = 0.0f, avg_fb = 0.0f;
    float max_mag_sq_ref = 0.0f, max_mag_sq_fb = 0.0f;
    uint32_t k0_ref = 0, k0_fb = 0;

    /* 防止未使用参数警告，如果后面你要输出频率，可以删掉这两行 */
    (void)freq_ref;
    (void)freq_fb;

    // ==================== 步骤1：首次调用初始化汉宁窗 ====================
    if (init_flag == 0)
    {
        for (uint32_t n = 0; n < FFT_N; n++)
        {
            hanning_win[n] = 0.5f * (1.0f - arm_cos_f32(PI2 * n / (FFT_N - 1)));
        }
        init_flag = 1;
    }

    // ==================== 步骤2：计算直流分量 ====================
    for (uint32_t i = 0; i < FFT_N; i++)
    {
        avg_ref += adc_ref[i];
        avg_fb  += adc_fb[i];
    }
    avg_ref /= FFT_N;
    avg_fb  /= FFT_N;

    // ==================== 步骤3：去直流 + 加窗 + 填充FFT复数数组 ====================
    for (uint32_t i = 0; i < FFT_N; i++)
    {
        float tmp_ref = ((float)adc_ref[i] - avg_ref) * hanning_win[i];
        fft_buf_ref[2 * i]     = tmp_ref;
        fft_buf_ref[2 * i + 1] = 0.0f;

        float tmp_fb = ((float)adc_fb[i] - avg_fb) * hanning_win[i];
        fft_buf_fb[2 * i]     = tmp_fb;
        fft_buf_fb[2 * i + 1] = 0.0f;
    }

    // ==================== 步骤4：两路信号分别执行扩展版FFT正变换 ====================
    arm_cfft_f32_extra(dpll_fft_get_instance(FFT_N), fft_buf_ref, 0, 1);
    arm_cfft_f32_extra(dpll_fft_get_instance(FFT_N), fft_buf_fb,  0, 1);

    // ==================== 步骤5：分别搜索两路各自的基波频点 ====================
    for (uint32_t k = 1; k < FFT_N / 2; k++)
    {
        float re_r = fft_buf_ref[2 * k];
        float im_r = fft_buf_ref[2 * k + 1];
        float ms_r = re_r * re_r + im_r * im_r;
        if (ms_r > max_mag_sq_ref)
        {
            max_mag_sq_ref = ms_r;
            k0_ref = k;
        }
    }

    for (uint32_t k = 1; k < FFT_N / 2; k++)
    {
        float re_f = fft_buf_fb[2 * k];
        float im_f = fft_buf_fb[2 * k + 1];
        float ms_f = re_f * re_f + im_f * im_f;
        if (ms_f > max_mag_sq_fb)
        {
            max_mag_sq_fb = ms_f;
            k0_fb = k;
        }
    }

    // ==================== 步骤6：分别计算两路基波的相位 ====================
    float re_ref = fft_buf_ref[2 * k0_ref];
    float im_ref = fft_buf_ref[2 * k0_ref + 1];
    float phase_ref = atan2f(im_ref, re_ref);

    /* 注意：这里保留你原代码逻辑，用 k0_ref 取反馈相位，便于同频点比较 */
    float re_fb = fft_buf_fb[2 * k0_fb];
    float im_fb = fft_buf_fb[2 * k0_fb + 1];
    float phase_fb = atan2f(im_fb, re_fb);

    // 如果你想用反馈自己的最大频点，可以改成下面两行：
    // float re_fb = fft_buf_fb[2 * k0_fb];
    // float im_fb = fft_buf_fb[2 * k0_fb + 1];

    // ==================== 步骤7：计算相位差并归一化到(-π, π] ====================
    float phase_diff = phase_ref - phase_fb;

    if (phase_diff > PI1)
        phase_diff -= PI2;
    if (phase_diff < -PI1)
        phase_diff += PI2;

    return phase_diff;
}

/**
 * @brief  PI控制器运算，位置式PI，带抗积分饱和
 */
float PI_Calc(PI_HandleTypeDef *pi, float error)
{
    pi->integral += error * pi->Ki;

    if (pi->integral > pi->int_max)
        pi->integral = pi->int_max;
    if (pi->integral < -pi->int_max)
        pi->integral = -pi->int_max;

    float out = pi->Kp * error + pi->integral;

    if (out > pi->out_max)
        out = pi->out_max;
    if (out < -pi->out_max)
        out = -pi->out_max;

    return out;
}
