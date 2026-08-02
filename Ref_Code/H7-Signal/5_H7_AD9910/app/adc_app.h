#ifndef __ADC_APP_H
#define __ADC_APP_H

#include "bspsysteam.h"

/*
 * ADC 参考电压，单位：V。
 *
 * 该值需要根据你的实际硬件电路修改。
 *
 * 例如：
 *   如果 ADC 参考电压为 3.3V，则设置为 3.3f。
 *   如果 ADC 参考电压为 2.5V，则设置为 2.5f。
 */
#define ADC_VREF 3.3f

/*
 * ADC 分辨率配置。
 *
 * 如果 ADC 配置为 16 位，则设置为 16。
 * 如果 ADC 配置为 12 位，则设置为 12。
 *
 * 该值必须和实际 ADC 初始化配置一致。
 *
 * 例如 STM32 HAL 中：
 *
 *   hadc1.Init.Resolution = ADC_RESOLUTION_16B;
 *
 * 则这里应设置为：
 *
 *   #define ADC_BITS 16
 *
 * ADC_MAX_CODE 会根据 ADC_BITS 自动计算。
 *
 * 16 位 ADC：
 *   ADC_MAX_CODE = 65536
 *
 * 12 位 ADC：
 *   ADC_MAX_CODE = 4096
 */
#define ADC_BITS      16
#define ADC_MAX_CODE  (1u << ADC_BITS)

/*
 * ADC 波形分析结果结构体。
 */
typedef struct {
    float vpp;          /* 峰峰值电压，单位：V */
    float vmax;         /* 最大电压，单位：V */
    float vmin;         /* 最小电压，单位：V */
    float vavg;         /* 平均电压，也可理解为直流偏置，单位：V */
    uint16_t max_adc;   /* 最大 ADC 原始采样值 */
    uint16_t min_adc;   /* 最小 ADC 原始采样值 */
} adc_waveform_info_t;



void adc_proc(void);


void adc_get_vpp(adc_waveform_info_t *info,
                 const uint16_t *data,
                 uint32_t len);


void adc_get_vpp_robust(adc_waveform_info_t *info,
                        const uint16_t *data,
                        uint32_t len,
                        float discard_pct);

void adc_get_vpp_auto(adc_waveform_info_t *info,
                      const uint16_t *data,
                      uint32_t len);

#endif
