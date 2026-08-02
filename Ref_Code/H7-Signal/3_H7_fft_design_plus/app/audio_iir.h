#ifndef __AUDIO_IIR_H
#define __AUDIO_IIR_H

#include "bspsysteam.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STM32_HPF_MAX_BIQUAD_STAGE    4U

#define STM32_ADC_BITS                12U
#define STM32_DAC_BITS                12U

#define STM32_ADC_MAX_VALUE           ((1U << STM32_ADC_BITS) - 1U)
#define STM32_DAC_MAX_VALUE           ((1U << STM32_DAC_BITS) - 1U)

#define STM32_ADC_MID_VALUE           2079
#define STM32_DAC_MID_VALUE           (1U << (STM32_DAC_BITS - 1U))

#define STM32_ADC_SCALE_VALUE         (1U << (STM32_ADC_BITS - 1U))
#define STM32_DAC_SCALE_VALUE         ((1U << (STM32_DAC_BITS - 1U)) - 1U)

typedef struct
{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;

    float z1;
    float z2;
} STM32_Biquad;

typedef struct
{
    float fs;
    float fc;

    uint32_t stage_num;

    STM32_Biquad stage[STM32_HPF_MAX_BIQUAD_STAGE];

    float adc_mid;
    float adc_scale;
    float dac_mid;
    float dac_scale;

} STM32_StableHPF;

int STM32_StableHPF_Init(STM32_StableHPF *hpf,
                         float fs,
                         float fc,
                         uint32_t order);

void STM32_StableHPF_SetAdcDacRange(STM32_StableHPF *hpf,
                                    float adc_mid,
                                    float adc_scale,
                                    float dac_mid,
                                    float dac_scale);

void STM32_StableHPF_Reset(STM32_StableHPF *hpf);

float STM32_StableHPF_ProcessFloat(STM32_StableHPF *hpf, float x);

uint16_t STM32_StableHPF_ProcessAdc(STM32_StableHPF *hpf, uint16_t adc_value);

void STM32_StableHPF_ProcessAdcBlock(STM32_StableHPF *hpf,
                                     const uint16_t *adc_input,
                                     uint16_t *dac_output,
                                     uint32_t block_size);

#ifdef __cplusplus
}
#endif

#endif
