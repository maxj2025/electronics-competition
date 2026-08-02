#include "fft_app.h"

#define CACHE_LINE_SIZE     32U

arm_cfft_instance_f32 fft_inst;


/*采样率*/
#define ADC_SAMPLE_RATE  1000000.0f

/* DSP buffer: CPU 高速访问，放 DTCM */
__attribute__((section(".DTCM_DATA")))
float32_t fft_buffer[FFT_LEN * 2U];

__attribute__((section(".DTCM_DATA")))
float32_t mag_buffer[FFT_LEN];

float32_t time_buffer[FFT_LEN];

static float32_t FFT_Parabolic_Interpolate(const float32_t *mag,uint32_t peak_idx,uint32_t fft_len);
static void Single_Point_DFT_Real_Fast(const float32_t *x,uint32_t len,float32_t bin,float32_t *out_re,float32_t *out_im);
__attribute__((section(".ITCM_CODE")))
void DCache_Invalidate_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr;
    uint32_t end_addr;

    start_addr = (uint32_t)addr;
    end_addr = start_addr + size;

    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);

    SCB_InvalidateDCache_by_Addr((uint32_t *)start_addr,
                                 (int32_t)(end_addr - start_addr));
}

__attribute__((section(".ITCM_CODE")))
void DCache_Clean_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr;
    uint32_t end_addr;

    start_addr = (uint32_t)addr;
    end_addr = start_addr + size;

    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);

    SCB_CleanDCache_by_Addr((uint32_t *)start_addr,
                            (int32_t)(end_addr - start_addr));
}


/*fft初始化*/
void dsp_fft_init(uint16_t len)
{
	if (arm_cfft_init_f32(&fft_inst, len) != ARM_MATH_SUCCESS)
    {
        Error_Handler();
    }
}

/*去adc直流*/
uint16_t adc_to_mind(const uint16_t *adc_buffer,uint16_t len)
{
	uint32_t adc_sum = 0;
	uint16_t adc_mind = 0;
	for(int i=0;i<len;i++)
	{
		adc_sum+=adc_buffer[i];
	}
	adc_mind = adc_sum/len;
	return adc_mind;
}


__attribute__((section(".ITCM_CODE")))
void Process_fft(uint16_t *adc_buffer, uint16_t len, float32_t *phase, float32_t *freq)
{
    if (adc_buffer == NULL || phase == NULL || freq == NULL)
        return;

    if (len != FFT_LEN)
        return;

    float32_t maxA_val;
    uint32_t idx_A;
    float32_t real_idx;
    float32_t v;
    float32_t dft_re;
    float32_t dft_im;
    float32_t dft_phase;
    uint32_t block_bytes;

    block_bytes = len * sizeof(uint16_t);
    DCache_Invalidate_By_Addr(adc_buffer, block_bytes);

    uint16_t adc_mind = adc_to_mind(adc_buffer, len);

    for (uint32_t i = 0; i < len; i++)
    {
        v = (float32_t)((int32_t)adc_buffer[i] - (int32_t)adc_mind) * 3.3f / 4096.0f;

        time_buffer[i] = v;
        fft_buffer[2U * i]     = v;
        fft_buffer[2U * i + 1U] = 0.0f;
    }

    arm_cfft_f32(&fft_inst, fft_buffer, 0, 1);
    arm_cmplx_mag_f32(fft_buffer, mag_buffer, FFT_LEN / 2U);

    for (uint32_t i = 0; i < 4U; i++)
    {
        mag_buffer[i] = 0.0f;
    }

    arm_max_f32(mag_buffer, FFT_LEN / 2U, &maxA_val, &idx_A);

    real_idx = FFT_Parabolic_Interpolate(mag_buffer, idx_A, FFT_LEN);

    Single_Point_DFT_Real_Fast(time_buffer,
                               FFT_LEN,
                               real_idx,
                               &dft_re,
                               &dft_im);

    dft_phase = atan2f(dft_im, dft_re);

    if (dft_phase > PI)
        dft_phase -= PI2;
    else if (dft_phase < -PI)
        dft_phase += PI2;

    *freq = real_idx * ADC_SAMPLE_RATE / (float32_t)FFT_LEN;
    *phase = dft_phase;
}


static float32_t FFT_Parabolic_Interpolate(const float32_t *mag,
                                           uint32_t peak_idx,
                                           uint32_t fft_len)
{
    float32_t y1;
    float32_t y2;
    float32_t y3;
    float32_t denominator;
    float32_t delta;

    if (peak_idx == 0U || peak_idx >= (fft_len / 2U - 1U))
    {
        return (float32_t)peak_idx;
    }

    y1 = mag[peak_idx - 1U];
    y2 = mag[peak_idx];
    y3 = mag[peak_idx + 1U];

    denominator = y1 - 2.0f * y2 + y3;

    if (denominator == 0.0f)
    {
        delta = 0.0f;
    }
    else
    {
        delta = 0.5f * (y1 - y3) / denominator;
    }

    /* 防止异常情况下偏移过大 */
    if (delta > 1.0f)
    {
        delta = 1.0f;
    }
    else if (delta < -1.0f)
    {
        delta = -1.0f;
    }

    return (float32_t)peak_idx + delta;
}

static void Single_Point_DFT_Real_Fast(const float32_t *x,
                                       uint32_t len,
                                       float32_t bin,
                                       float32_t *out_re,
                                       float32_t *out_im)
{
    float32_t sum_re = 0.0f;
    float32_t sum_im = 0.0f;

    float32_t phase_step = -2.0f * PI * bin / (float32_t)len;
    float32_t step_c = arm_cos_f32(phase_step);
    float32_t step_s = arm_sin_f32(phase_step);

    float32_t c = 1.0f;
    float32_t s = 0.0f;

    for (uint32_t n = 0; n < len; n++)
    {
        float32_t next_c;
        float32_t next_s;

        sum_re += x[n] * c;
        sum_im += x[n] * s;

        next_c = c * step_c - s * step_s;
        next_s = s * step_c + c * step_s;

        c = next_c;
        s = next_s;
    }

    *out_re = sum_re;
    *out_im = sum_im;
}


 float32_t Find_FFT_Bin(uint16_t *adc_buffer, uint16_t len, float32_t *freq)
{
    float32_t maxA_val;
    uint32_t idx_A;
    float32_t real_idx;
    float32_t v;
    uint32_t block_bytes;

    block_bytes = len * sizeof(uint16_t);
    DCache_Invalidate_By_Addr(adc_buffer, block_bytes);

    uint16_t adc_mind = adc_to_mind(adc_buffer, len);

    for (uint32_t i = 0; i < len; i++)
    {
        v = (float32_t)((int32_t)adc_buffer[i] - (int32_t)adc_mind) * 3.3f / 4096.0f;

        time_buffer[i] = v;
        fft_buffer[2U * i]      = v;
        fft_buffer[2U * i + 1U] = 0.0f;
    }

    arm_cfft_f32(&fft_inst, fft_buffer, 0, 1);
    arm_cmplx_mag_f32(fft_buffer, mag_buffer, FFT_LEN / 2U);

    for (uint32_t i = 0; i < 4U; i++)
    {
        mag_buffer[i] = 0.0f;
    }

    arm_max_f32(mag_buffer, FFT_LEN / 2U, &maxA_val, &idx_A);

    real_idx = FFT_Parabolic_Interpolate(mag_buffer, idx_A, FFT_LEN);

    *freq = real_idx * ADC_SAMPLE_RATE / (float32_t)FFT_LEN;

    return real_idx;
}


void Get_DFT_By_Bin(uint16_t *adc_buffer,
                           uint16_t len,
                           float32_t bin,
                           float32_t *out_re,
                           float32_t *out_im)
{
    float32_t v;
    uint32_t block_bytes;

    block_bytes = len * sizeof(uint16_t);
    DCache_Invalidate_By_Addr(adc_buffer, block_bytes);

    uint16_t adc_mind = adc_to_mind(adc_buffer, len);

    for (uint32_t i = 0; i < len; i++)
    {
        v = (float32_t)((int32_t)adc_buffer[i] - (int32_t)adc_mind) * 3.3f / 4096.0f;
        time_buffer[i] = v;
    }

    Single_Point_DFT_Real_Fast(time_buffer,
                               FFT_LEN,
                               bin,
                               out_re,
                               out_im);
}




