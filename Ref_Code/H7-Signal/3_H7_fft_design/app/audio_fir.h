#ifndef __AUDIO_FIR_H__
#define __AUDIO_FIR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bspsysteam.h"

/*
 * ============================================================
 * 说明：
 *
 * 本文件只负责 FIR 滤波处理逻辑，不负责 ADC/DAC DMA、Cache、
 * 回调函数、定时器启动等。
 *
 * 典型使用流程：
 *
 * 1. 系统初始化完成后调用：
 *
 *      AudioFIR_Init(&param, LOWPASSFILTER);
 *
 * 2. 在 ADC DMA 半满/全满回调中：
 *
 *      先做 ADC DMA buffer 的 Cache Invalidate
 *      再调用 AudioFIR_ProcessBlock_U16(...)
 *      最后做 DAC DMA buffer 的 Cache Clean
 *
 * ============================================================
 */


/* ===================== 用户可调参数 ===================== */

/*
 * 每次 FIR 处理的最大数据点数。
 *
 * 如果你 ADC/DAC DMA 使用双缓冲或者 circular 半满/全满方式，
 * 那么通常：
 *
 * DMA 总长度 = AUDIO_FIR_BLOCK_SIZE * 2
 *
 * 半满处理前半块 AUDIO_FIR_BLOCK_SIZE 点；
 * 全满处理后半块 AUDIO_FIR_BLOCK_SIZE 点。
 */
#ifndef AUDIO_FIR_BLOCK_SIZE
#define AUDIO_FIR_BLOCK_SIZE        ADC_DAC_DMA_HALF_LEN
#endif

/*
 * FIR 最大抽头数。
 *
 * 注意：
 * 1. 抽头越多，频率选择性越好；
 * 2. 抽头越多，CPU 计算量越大；
 * 3. STM32H7 性能较强，513 通常可以尝试；
 * 4. 实时性不够时，可以减小该值，比如 257、129。
 */
#ifndef AUDIO_FIR_MAX_TAP_NUM
#define AUDIO_FIR_MAX_TAP_NUM       127
#endif

/*
 * ADC/DAC 默认 12bit。
 *
 * ADC 输入：
 * 0    ~ 4095
 * 中点 2048
 *
 * DAC 输出：
 * 0    ~ 4095*9
 * 中点 2048
 */
#define AUDIO_FIR_ADC_MAX           4095
#define AUDIO_FIR_ADC_MID           2048

#define AUDIO_FIR_DAC_MAX           4095
#define AUDIO_FIR_DAC_MID           2048


/* ===================== 滤波器类型 ===================== */

typedef enum
{
    LOWPASSFILTER   = 0,    /* 低通滤波器 */
    HIGHPASSFILTER  = 1,    /* 高通滤波器 */
    BANDPASSFILTER  = 2,    /* 带通滤波器 */
    BANDSTOPFILTER  = 3     /* 带阻滤波器 */
} FilterType;


/* ===================== 窗函数类型 ===================== */

typedef enum
{
    Rectangle   = 0,        /* 矩形窗 */
    triangle    = 1,        /* 三角窗 */
    Hanning     = 2,        /* 汉宁窗 */
    Hamming     = 3,        /* 海明窗 */
    Blackman    = 4         /* 布莱克曼窗 */
} WindowType;


/* ===================== 滤波器设计参数 ===================== */

/*
 * 不同滤波器参数含义：
 *
 * ------------------------------------------------------------
 * 1. 低通 LOWPASSFILTER
 *
 *      fp1  = 通带截止频率
 *      fst1 = 阻带起始频率
 *
 *      要求：
 *      fp1 < fst1 < fs/2
 *
 *      fp2、fst2 不使用，可以填 0。
 *
 * ------------------------------------------------------------
 * 2. 高通 HIGHPASSFILTER
 *
 *      fst1 = 阻带截止频率
 *      fp1  = 通带起始频率
 *
 *      要求：
 *      fst1 < fp1 < fs/2
 *
 *      fp2、fst2 不使用，可以填 0。
 *
 * ------------------------------------------------------------
 * 3. 带通 BANDPASSFILTER
 *
 *      fst1 = 下阻带截止频率
 *      fp1  = 下通带起始频率
 *      fp2  = 上通带截止频率
 *      fst2 = 上阻带起始频率
 *
 *      要求：
 *      fst1 < fp1 < fp2 < fst2 < fs/2
 *
 * ------------------------------------------------------------
 * 4. 带阻 BANDSTOPFILTER
 *
 *      fp1  = 下通带截止频率
 *      fst1 = 下阻带起始频率
 *      fst2 = 上阻带截止频率
 *      fp2  = 上通带起始频率
 *
 *      要求：
 *      fp1 < fst1 < fst2 < fp2 < fs/2
 *
 * ------------------------------------------------------------
 *
 * ast：
 *      阻带衰减，单位 dB。
 *      代码会根据 ast 自动选择窗函数。
 *
 * fs：
 *      采样频率，单位 Hz。
 */
typedef struct
{
    double fp1;
    double fst1;
    double fp2;
    double fst2;
    double ast;
    double fs;
} TransferParam;


/* ===================== API ===================== */

/**
 * @brief  初始化 FIR 模块。
 *
 * @note   本函数会根据传入参数设计 FIR 系数，并初始化 CMSIS-DSP FIR。
 *         本函数内部会清空 FIR 状态缓存。
 *
 * @param  param FIR 设计参数
 * @param  type  滤波器类型
 *
 * @retval 1 成功
 * @retval 0 失败
 */
int AudioFIR_Init(const TransferParam *param, FilterType type);


/**
 * @brief  运行时重新设计 FIR。
 *
 * @note   不建议在 ADC/DMA 中断回调里调用本函数。
 *         因为本函数内部有 double、sin、cos、sqrt 等计算，
 *         耗时较长。
 *
 *         推荐做法：
 *         1. 暂停 ADC/DAC DMA 或者关闭处理；
 *         2. 在主循环/任务中调用本函数；
 *         3. 重新开启实时处理。
 *
 * @param  param FIR 设计参数
 * @param  type  滤波器类型
 *
 * @retval 1 成功
 * @retval 0 失败
 */
int AudioFIR_Redesign(const TransferParam *param, FilterType type);


/**
 * @brief  处理一块 ADC 12bit 数据，并输出 DAC 12bit 数据。
 *
 * @note   本函数就是你在 ADC DMA 半满/全满回调中调用的核心函数。
 *
 *         输入格式：
 *              uint16_t ADC 12bit，范围 0~4095
 *
 *         内部流程：
 *              ADC 12bit -> 去直流 -> Q15
 *              Q15 FIR
 *              Q15 -> 加直流偏置 -> DAC 12bit
 *
 * @param  adc_src ADC 数据输入指针
 * @param  dac_dst DAC 数据输出指针
 * @param  len     本次处理点数，必须 <= AUDIO_FIR_BLOCK_SIZE
 *
 * @retval 1 成功
 * @retval 0 失败
 */
int AudioFIR_ProcessBlock_U16(const uint16_t *adc_src,
                              uint16_t *dac_dst,
                              uint32_t len);


/**
 * @brief  处理一块 Q15 数据。
 *
 * @note   如果你后面不想用 ADC12/DAC12 格式，而是自己已经准备好 Q15 数据，
 *         可以直接调用这个函数。
 *
 * @param  input_q15  Q15 输入数据
 * @param  output_q15 Q15 输出数据
 * @param  len        本次处理点数，必须 <= AUDIO_FIR_BLOCK_SIZE
 *
 * @retval 1 成功
 * @retval 0 失败
 */
int AudioFIR_ProcessBlock_Q15(const q15_t *input_q15,
                              q15_t *output_q15,
                              uint32_t len);


/**
 * @brief  清空 FIR 状态缓存。
 *
 * @note   如果你切换信号源、重新开始采样，或者想去掉历史滤波状态，
 *         可以调用本函数。
 */
void AudioFIR_ResetState(void);


/**
 * @brief  获取当前 FIR 抽头数。
 *
 * @retval 当前抽头数
 */
uint16_t AudioFIR_GetTapNum(void);


/**
 * @brief  获取当前 Q15 FIR 系数指针。
 *
 * @note   系数顺序是 CMSIS-DSP 要求的反序：
 *
 *         b[N-1], b[N-2], ..., b[0]
 *
 * @retval Q15 系数指针
 */
const q15_t *AudioFIR_GetCoeffQ15(void);


#ifdef __cplusplus
}
#endif

#endif
