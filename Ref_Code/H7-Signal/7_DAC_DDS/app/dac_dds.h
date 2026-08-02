#ifndef __DAC_DDS_H
#define __DAC_DDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bspsysteam.h"

/* ================= 用户配置区 ================= */

/* DAC 采样率，固定 4296875 MHz */
#define DDS_SAMPLE_RATE_HZ      4296875UL

/* DDS 32 位相位累加器 */
#define DDS_PHASE_BITS          32U
#define DDS_PHASE_MAX_U64       4294967296ULL

/* 正弦查找表大小，1024 点 */
#define DDS_LUT_BITS            10U
#define DDS_LUT_SIZE            (1U << DDS_LUT_BITS)

/* DMA Buffer 大小，必须为偶数 */
#define DDS_DMA_BUF_SIZE        1024U

/* STM32 DAC 12 bit */
#define DDS_DAC_MAX             4095U
#define DDS_DAC_MID             2048U
#define DDS_DAC_HALF_SCALE      2047

/* ================= 对外变量 ================= */

extern uint16_t dds_dac_buf[DDS_DMA_BUF_SIZE];

/* ================= 对外函数 ================= */

void DDS_Init(void);
void DDS_Start(void);
void DDS_Stop(void);

void DDS_SetFrequency(uint32_t freq_hz);
void DDS_SetPhaseDeg(uint32_t phase_deg);
void DDS_SetAmplitudeQ15(uint16_t amp_q15);
void DDS_SetAmplitudePermille(uint16_t amp_permille);
void DDS_SetOffset(uint16_t offset);
void DDS_ResetPhase(void);

void DDS_FillBuffer(uint16_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
