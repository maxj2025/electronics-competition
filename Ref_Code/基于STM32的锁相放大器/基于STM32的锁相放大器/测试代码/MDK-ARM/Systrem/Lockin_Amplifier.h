#ifndef __LOCKIN_AMPLIFIER_H__
#define __LOCKIN_AMPLIFIER_H__

#include "main.h"
#include "adc.h"

#define PI 3.14159265358979f
#define DLIA_BLOCK_SIZE 512         // 每次处理的样本数
#define LUT_SIZE 1024               // LUT表大小
#define LUT_MASK (LUT_SIZE - 1)     // LUT掩码
#define LUT_SHIFT 22                // LUT索引右移位数 (32 - log2(LUT_SIZE))

typedef struct{
    float Signal_Freq;        // 信号频率
    float Ref_Freq;           // ADC采样频率
    float LPF_Beta;           // 滤波系数
}DLIA_Config_t;

typedef struct{
    uint32_t PhaseStep;         // 每次采样相位步进
    uint32_t PhaseAccumulator;  // 相位累加器
    float LPF_Val_I;            // 低通滤波器I通道值
    float LPF_Val_Q;            // 低通滤波器Q通道值
}DLIA_State_t;

typedef struct{
    float Output_I;         // 输出I通道值
    float Output_Q;         // 输出Q通道值  
    float Output_Amp;       // 输出振幅值
    float Output_PhaseRad;  // 输出相位值（弧度制）
    float Output_PhaseDeg;  // 输出相位值（角度制）
}DLIA_Output_t;

void DLIA_Init_LUT(void);
void DLIA_Init(DLIA_Config_t* config, DLIA_State_t* state,float freq , float fs , float lpf_beta);
void DLIA_SetFreq(DLIA_Config_t* config, DLIA_State_t* state, float freq);
void DLIA_Process(DLIA_Config_t* config, DLIA_State_t* state, DLIA_Output_t* output, uint16_t* ADCValue, uint32_t length);



#endif // __LOCKIN_AMPLIFIER_H__
