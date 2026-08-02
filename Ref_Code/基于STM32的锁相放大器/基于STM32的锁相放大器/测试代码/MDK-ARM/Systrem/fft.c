#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "arm_math.h"
#include "OLED.h"
#define M_PI 3.1415926
#define N 1024
#define Fs 80000
arm_rfft_fast_instance_f32 S_RFFT;
float amplitude1[N / 2];
float phase1[N / 2];
float amplitude2[N / 2];
float phase2[N / 2];
uint16_t fundamental_idx1;
uint16_t fundamental_idx2;
__attribute__((aligned(4))) float32_t input1[N];
__attribute__((aligned(4))) float32_t output1[N];
__attribute__((aligned(4))) float32_t input2[N];
__attribute__((aligned(4))) float32_t output2[N];
float re1; // 实部
float im1; // 虚部
float re2; // 实部
float im2; // 虚部
void RFFT_System_Init(void)
{

	if (arm_rfft_fast_init_f32(&S_RFFT, N) != ARM_MATH_SUCCESS)
	{

		while (1);
			
	}
}



void FFT_Init(uint16_t *ADCValue, uint16_t Len, float32_t *input)
{
	uint16_t k;
	uint16_t DC_offset = 0;
	uint32_t sum = 0;
	for (k = 0; k < N; k++)
	{

		sum += ADCValue[k];
	}
	DC_offset = sum / N;
	for (k = 0; k < N; k++)
	{
		float32_t sample = (float32_t)(ADCValue[k] - DC_offset);
		float32_t window = 0.5f * (1.0f - cosf(2 * M_PI * k / (N - 1)));
		sample *= window;
		sample /= 2048.0f;
		input[k] = sample;
	}
}


void FFT1()
{
	FFT_Init(Adc1Value, 1024, input1);

	arm_rfft_fast_f32(&S_RFFT, input1, output1, 0);
	
	for (uint16_t i = 0; i < N / 2; i++)
	{
		re1 = (float)output1[2 * i];
		im1 = (float)output1[2 * i + 1];
		amplitude1[i] = sqrtf(re1 * re1 + im1 * im1)*4/N;
		phase1[i] = atan2f(im1, re1) * 180.0f / M_PI;
		
	}
}

void FFT2()
{
	FFT_Init(Adc2Value, 1024, input2);

	arm_rfft_fast_f32(&S_RFFT, input2, output2, 0);
	
	for (uint16_t i = 0; i < N / 2; i++)
	{
		re2 = (float)output2[2 * i];
		im2 = (float)output2[2 * i + 1];
		amplitude2[i] = sqrtf(re2 * re2 + im2 * im2)*4/N; 
		phase2[i] = atan2f(im2, re2) * 180.0f / M_PI;
		
	}
}

float Get_Freq1()
{
	float max_amp = 0;
	float freq;
	for (uint16_t k = 1; k < N / 2; k++)
	{
		if (amplitude1[k] > max_amp)
		{
			max_amp = amplitude1[k];
			fundamental_idx1 = k;
		}
	}
	freq = (float)fundamental_idx1 * Fs / N; // Fn=k*Fs/N
	
	return freq;
}

float Get_Freq2()
{
	float max_amp = 0;
	float freq;
	for (uint16_t k = 1; k < N / 2; k++)
	{
		if (amplitude2[k] > max_amp)
		{
			max_amp = amplitude2[k];
			fundamental_idx2 = k;
		}
	}
	freq = (float)fundamental_idx2 * Fs / N; // Fn=k*Fs/N
	
	return freq;
}

float Get_PhaseDiff()
{
	if (fundamental_idx1 <= 0 || fundamental_idx2 <= 0)
	{

		return -1;
	}

	if (abs(fundamental_idx1 - fundamental_idx2) > 1)
	{

		return -1;
	}

	uint16_t fundamental_idx = fundamental_idx1;

	float p1 = phase1[fundamental_idx1]; // 通道1 的基波相位
	float p2 = phase2[fundamental_idx2]; // 通道2 的基波相位 (使用相同的索引)

	float phase_diff = p1 - p2;

	// 将相位差归到 [-180, 180] 度区间
	while (phase_diff <= -180.0f)
	{
		phase_diff += 360.0f;
	}
	while (phase_diff > 180.0f)
	{
		phase_diff -= 360.0f;
	}

	return phase_diff;
}

float Get_Amplitude1()
{
	if (fundamental_idx1 < N / 2)
	{
		return amplitude1[fundamental_idx1]*(3.3f);
	}
	else
	{
		return 0.0f; // 如果索引超出范围，返回0
	}
}

float Get_Amplitude2()
{
	if (fundamental_idx2 < N / 2)
	{
		return amplitude2[fundamental_idx2]*(3.3f);
	}
	else
	{
		return 0.0f; // 如果索引超出范围，返回0
	}
}

