#include "Lockin_Amplifier.h"
#include "arm_math.h"
__attribute__((section("ccmram"))) static float SinLUT[LUT_SIZE];
#define INV_256 0.00390625f

void DLIA_Init_LUT(void)
{
    for(int i = 0; i < LUT_SIZE; i++)
    {
        float angle = (2.0f * PI * (float)i) / (float)LUT_SIZE;
        SinLUT[i] = arm_sin_f32(angle);
    }
}

void DLIA_Init(DLIA_Config_t* config, DLIA_State_t* state,float freq , float fs , float lpf_beta)
{
    config->Signal_Freq = freq;
    config->Ref_Freq = fs;
    config->LPF_Beta = lpf_beta;
	
    state->PhaseStep = (uint32_t)(config->Signal_Freq / config->Ref_Freq * 4294967296.0f); 
    state->PhaseAccumulator = 0;

    state->LPF_Val_I = 0.0f;
    state->LPF_Val_Q = 0.0f;

}

void DLIA_Process(DLIA_Config_t* config, DLIA_State_t* state, DLIA_Output_t* output, uint16_t* ADCValue, uint32_t length)
{
    if(length != DLIA_BLOCK_SIZE)
    {
        return;
    }

    uint32_t phase_acc  = state->PhaseAccumulator;
    uint32_t phase_step = state->PhaseStep;

    float sum_I = 0.0f;
    float sum_Q = 0.0f;

    const float k_convert =  3.3f * 0.000244140625f;  // ADC值转电压: 3.3V参考/4096
    const uint32_t cos_offset = LUT_SIZE >> 2;  // 余弦表偏移: LUT_SIZE/4 (90度相位差)

    float sum_ref_sin = 0.0f;  // 参考正弦累加和
    float sum_ref_cos = 0.0f;  // 参考余弦累加和
    uint32_t sum_adc_raw = 0;  // 原始ADC值累加和 

    for(int i = 0; i < length; i++)
    {
        uint32_t phase_fixed = phase_acc >> (LUT_SHIFT - 8); 

        uint32_t idx0 = (phase_fixed >> 8) & LUT_MASK;  
        uint32_t idx1 = (idx0 + 1) & LUT_MASK;  

        float frac = (float)(phase_fixed & 0xFF) * INV_256;  // 插值分数 (0-1)
        float frac_inv = 1.0f - frac;  // 1-分数

        float val_sin0 = SinLUT[idx0];
        float val_sin1 = SinLUT[idx1];
        float ref_sin  = val_sin0 * frac_inv + val_sin1 * frac;  // 正弦参考信号线性插值

        uint32_t idx_cos0 = (idx0 + cos_offset) & LUT_MASK;  // 余弦索引0
        uint32_t idx_cos1 = (idx1 + cos_offset) & LUT_MASK;  // 余弦索引1

        float val_cos0 = SinLUT[idx_cos0];
        float val_cos1 = SinLUT[idx_cos1];
        float ref_cos  = val_cos0 * frac_inv + val_cos1 * frac;  // 余弦参考信号线性插值

        phase_acc += phase_step;

        float adc_val = (float)ADCValue[i];
        sum_I += adc_val * ref_sin;  // I通道解调
        sum_Q += adc_val * ref_cos;  // Q通道解调

        sum_adc_raw += ADCValue[i];  // 记录原始ADC值用于直流消除
        sum_ref_sin += ref_sin;      // 记录参考信号用于归一化
        sum_ref_cos += ref_cos;     

    }
    state->PhaseAccumulator = phase_acc;

    float avg_adc_raw = (float)sum_adc_raw / (float)length;  // 计算直流分量

    // 消除直流偏移
    float final_sum_I = sum_I - (avg_adc_raw * sum_ref_sin);
    float final_sum_Q = sum_Q - (avg_adc_raw * sum_ref_cos);

    // 归一化处理
    float norm_factor = k_convert * (2.0f / (float)length);
    float avg_I = final_sum_I * norm_factor;
    float avg_Q = final_sum_Q * norm_factor;

    // 一阶低通滤波器
    state->LPF_Val_I += config->LPF_Beta * (avg_I - state->LPF_Val_I);
    state->LPF_Val_Q += config->LPF_Beta * (avg_Q - state->LPF_Val_Q);

    output->Output_I = state->LPF_Val_I;
    output->Output_Q = state->LPF_Val_Q;

    // 计算幅度: sqrt(I^2 + Q^2)
    float sum = output->Output_I * output->Output_I + output->Output_Q * output->Output_Q;
    arm_sqrt_f32(sum, &output->Output_Amp);

    // 计算相位: atan2(Q, I)
    float phase;
    phase = atan2f(output->Output_Q, output->Output_I);

    output->Output_PhaseRad = phase;
    output->Output_PhaseDeg = phase * 57.2957795f;  // 弧度转角度

}
