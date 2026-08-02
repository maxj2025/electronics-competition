#ifndef __FREQ_MEASURE_H
#define __FREQ_MEASURE_H


#include "bspsysteam.h"



typedef enum {
    WAVE_SINE = 0,      // 正弦波
    WAVE_SQUARE,        // 方波
    WAVE_TRIANGLE,      // 三角波
    WAVE_UNKNOWN        // 未知 / 初始状态
} WaveType_t;

/** 波形测量结果（频率 + 峰峰值 + 波形类型） */
typedef struct {
    float   Freq;       // 频率 (Hz)
    float   Vpp;        // 峰峰值 (V)
    WaveType_t Wave_type;
} Wave_Struct;
/* 频率测量对象（测周法/测频法双模式） */
typedef struct {
    /* ---- 配置（由 Init 写入）---- */
    TIM_HandleTypeDef *htim;
    float              tick_freq;       // TIM2 计数时钟 (Hz)，H7 下 = APB1 定时器时钟 = 275MHz
    float              switch_high_thr; // 高于此频率切“测频法”
    float              switch_low_thr;  // 低于此频率切回“测周法”
    uint32_t           gate_time_ms;    // 测频法闸门时间 (ms)
    /* ---- 内部状态 ---- */
    uint8_t  mode;            // 0=测周法  1=测频法
    uint32_t gate_start_ms;   // 测频法闸门起始时刻
    uint8_t  measuring;       // 测频法是否正在计数
    uint8_t  first_frame;     // 切换后首帧丢弃标志
} FreqMeasure;

/* ===================== 频率测量 API ===================== */
void FreqMeasure_Init(FreqMeasure *self, TIM_HandleTypeDef *htim);
void FreqMeasure_Process(FreqMeasure *self, Wave_Struct *wave);
void my_MX_TIM2_Init(void);




#endif
