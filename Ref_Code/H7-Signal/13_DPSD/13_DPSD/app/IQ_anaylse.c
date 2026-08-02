#include "IQ_anaylse.h"
#include <math.h>

#define TWO_PI  (6.28318530717958647692f)

__attribute__((section(".RAM_SDRAM"), zero_init)) float32_t step_cos[SIGSEP_MAX_BIN + 1];

__attribute__((section(".RAM_SDRAM"), zero_init)) float32_t step_sin[SIGSEP_MAX_BIN + 1];

/*IQ初始化,将phase_step存储*/
void IQ_init(void)
{
    uint32_t freq_hz;
    float32_t phase_step_rad;

    step_sin[0] = 0.0f;
    step_cos[0] = 1.0f;

    for (uint16_t i = 1U; i <= SIGSEP_MAX_BIN; i++)
    {
        freq_hz = (uint32_t)i * SIGSEP_FREQ_STEP_HZ;
        phase_step_rad = TWO_PI *
                         ((float32_t)freq_hz /
                          (float32_t)SIGSEP_SAMPLE_RATE_HZ);

        step_sin[i] = sinf(phase_step_rad);
        step_cos[i] = cosf(phase_step_rad);
    }
}

float32_t Frame_Mean(const uint16_t *samples)
{
	float32_t mean = 0.0f;
	float32_t val_sum = 0.0f;
	for(int i=0;i<SIGSEP_FRAME_LEN;i++)
	{
		val_sum+=(float32_t)samples[i];
	}
	mean = val_sum/SIGSEP_FRAME_LEN;
	return mean;
}

/*IQ分析*/
/*
samples:采样数组
mean:直流DC
freq_hz:需要分析的频率
amp:该频率的幅度
phase:该频率的相位
*/
void Measure_Component(const uint16_t *samples, float32_t mean,
uint32_t freq_hz, float32_t *amp,
float32_t *phase)
{
	uint16_t bin = freq_hz/SIGSEP_FREQ_STEP_HZ;
	float32_t s = 0.0f;//sin初始值
	float32_t c = 1.0f;//cos初始值
	float32_t sum_s = 0.0f;//Q累加
	float32_t sum_c = 0.0f;//I累加
	float32_t x = 0.0f;
	float32_t next_c = 0.0f;
	float32_t mag = 0.0f;
	for(int i=0;i<SIGSEP_FRAME_LEN;i++)
	{
		x = (float32_t)(samples[i]-mean*1.0f);
		sum_s+=x*s;
		sum_c+=x*c;
		next_c = (c * step_cos[bin]) - (s * step_sin[bin]);
		s =  (s * step_cos[bin]) + (c * step_sin[bin]);
		c = next_c;
	}
	mag = sqrtf(sum_s * sum_s + sum_c * sum_c);
	*amp = (2.0f * mag) / (float32_t)SIGSEP_FRAME_LEN;
	*phase = atan2f(-sum_s, sum_c);
}
