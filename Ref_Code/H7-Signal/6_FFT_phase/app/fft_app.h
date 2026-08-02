#ifndef __FFT_APP_H
#define __FFT_APP_H
#include "bspsysteam.h"
#define FFT_LEN 4096
#ifndef PI
#define PI          3.14159265358979323846f
#endif

#define PI2          6.28318530717958647692f
void Process_fft(uint16_t *adc_buffer, uint16_t len, float32_t *phase, float32_t *freq);
void DCache_Invalidate_By_Addr(void *addr, uint32_t size);
void DCache_Clean_By_Addr(void *addr, uint32_t size);
void dsp_fft_init(uint16_t len);
void Get_DFT_By_Bin(uint16_t *adc_buffer,uint16_t len,float32_t bin,float32_t *out_re,float32_t *out_im);
float32_t Find_FFT_Bin(uint16_t *adc_buffer, uint16_t len, float32_t *freq);
#endif

