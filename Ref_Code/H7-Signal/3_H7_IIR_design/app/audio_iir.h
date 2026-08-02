#ifndef __AUDIO_IIR_H
#define __AUDIO_IIR_H


#include <stdint.h>
#include "arm_math.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUDIO_IIR_BLOCK_SIZE
#define AUDIO_IIR_BLOCK_SIZE        128U
#endif

#ifndef AUDIO_IIR_ADC_MID
#define AUDIO_IIR_ADC_MID           2048U
#endif

#ifndef AUDIO_IIR_DAC_MID
#define AUDIO_IIR_DAC_MID           2048U
#endif

#ifndef AUDIO_IIR_DAC_MAX
#define AUDIO_IIR_DAC_MAX           4095U
#endif

typedef struct
{
    double fs;
    double fc;
} AudioIIR_LowPassParam;

int AudioIIR_InitLowPass(const AudioIIR_LowPassParam *param);
int AudioIIR_RedesignLowPass(const AudioIIR_LowPassParam *param);

int AudioIIR_ProcessBlock_U16(const uint16_t *adc_src,
                              uint16_t *dac_dst,
                              uint32_t len);

int AudioIIR_ProcessBlock_Q15(const q15_t *input_q15,
                              q15_t *output_q15,
                              uint32_t len);

void AudioIIR_ResetState(void);
uint8_t AudioIIR_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif
