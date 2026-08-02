/**
 * @file dpsd.c
 * @brief DPSD 数字相敏检测实现（STM32H7 CORDIC加速版）
 */

#include "dpsd.h"
#include <string.h>
#include <math.h>

/* ==================== 内部状态变量 ==================== */

// CORDIC 句柄
static CORDIC_HandleTypeDef *g_hcordic = NULL;

// 当前目标频率
static float g_current_freq = DPSD_TARGET_FREQ;

// 频率跟踪：上一次相位
static float g_last_phase = 0.0f;

// 初始化标志
static uint8_t g_initialized = 0;

#if !DPSD_USE_CORDIC
// 参考信号查找表（仅在不使用CORDIC时）
static dpsd_lut_t g_lut_cos[DPSD_POINTS_PER_CYCLE];
static dpsd_lut_t g_lut_sin[DPSD_POINTS_PER_CYCLE];
#endif

#if DPSD_ENABLE_PROFILING
// 性能统计
static dpsd_profile_t g_profile = {0};
#endif

/* ==================== CORDIC Q1.31 格式定义 ==================== */

#define CORDIC_Q31_SCALE    2147483648.0f  // 2^31
#define PI_F                3.14159265358979323846f

/* ==================== 内部辅助宏 ==================== */

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

/* ==================== CORDIC 硬件函数实现 ==================== */

#if DPSD_USE_CORDIC

void DPSD_CORDIC_SinCos(float angle, float *sin_out, float *cos_out)
{
    int32_t angle_q31, result[2];
    
    // 将角度限制到 -π 到 π
    while (angle > PI_F) angle -= 2.0f * PI_F;
    while (angle < -PI_F) angle += 2.0f * PI_F;
    
    // 转换为 Q1.31 格式
    angle_q31 = (int32_t)(angle / PI_F * CORDIC_Q31_SCALE);
    
    // 配置 CORDIC 为 COSINE 模式（同时输出 cos 和 sin）
    if (HAL_CORDIC_Configure(g_hcordic,
        &(CORDIC_ConfigTypeDef){
            .Function   = CORDIC_FUNCTION_COSINE,
            .Precision  = DPSD_CORDIC_PRECISION,
            .Scale      = CORDIC_SCALE_0,
            .NbWrite    = CORDIC_NBWRITE_1,
            .NbRead     = CORDIC_NBREAD_2,
            .InSize     = CORDIC_INSIZE_32BITS,
            .OutSize    = CORDIC_OUTSIZE_32BITS
        }) != HAL_OK)
    {
        *cos_out = 0.0f;
        *sin_out = 0.0f;
        return;
    }
    
    // 执行计算
    if (HAL_CORDIC_CalculateZO(g_hcordic, &angle_q31, result, 1, 100) == HAL_OK)
    {
        *cos_out = (float)result[0] / CORDIC_Q31_SCALE;
        *sin_out = (float)result[1] / CORDIC_Q31_SCALE;
    }
    else
    {
        *cos_out = 0.0f;
        *sin_out = 0.0f;
    }
    
#if DPSD_ENABLE_PROFILING
    g_profile.cordic_calls++;
#endif
}

float DPSD_CORDIC_Atan2(float y, float x)
{
    int32_t input[2], result;
    float magnitude, angle;
    
    // 计算模长用于归一化
    magnitude = sqrtf(x * x + y * y);
    
    if (magnitude < 1e-9f)
    {
        return 0.0f;
    }
    
    // 归一化到 [-1, 1] 范围
    x /= magnitude;
    y /= magnitude;
    
    // 转换为 Q1.31 格式
    input[0] = (int32_t)(x * CORDIC_Q31_SCALE);
    input[1] = (int32_t)(y * CORDIC_Q31_SCALE);
    
    // 配置 CORDIC 为 PHASE 模式
    if (HAL_CORDIC_Configure(g_hcordic,
        &(CORDIC_ConfigTypeDef){
            .Function   = CORDIC_FUNCTION_PHASE,
            .Precision  = DPSD_CORDIC_PRECISION,
            .Scale      = CORDIC_SCALE_0,
            .NbWrite    = CORDIC_NBWRITE_2,
            .NbRead     = CORDIC_NBREAD_1,
            .InSize     = CORDIC_INSIZE_32BITS,
            .OutSize    = CORDIC_OUTSIZE_32BITS
        }) != HAL_OK)
    {
        return 0.0f;
    }
    
    // 执行计算
    if (HAL_CORDIC_Calculate(g_hcordic, input, &result, 1, 100) == HAL_OK)
    {
        angle = (float)result / CORDIC_Q31_SCALE * PI_F;
    }
    else
    {
        angle = 0.0f;
    }
    
#if DPSD_ENABLE_PROFILING
    g_profile.cordic_calls++;
#endif
    
    return angle;
}

float DPSD_CORDIC_Magnitude(float x, float y)
{
    int32_t input[2], result;
    
    // 转换为 Q1.31 格式
    input[0] = (int32_t)(x * CORDIC_Q31_SCALE);
    input[1] = (int32_t)(y * CORDIC_Q31_SCALE);
    
    // 配置 CORDIC 为 MODULUS 模式
    if (HAL_CORDIC_Configure(g_hcordic,
        &(CORDIC_ConfigTypeDef){
            .Function   = CORDIC_FUNCTION_MODULUS,
            .Precision  = DPSD_CORDIC_PRECISION,
            .Scale      = CORDIC_SCALE_0,
            .NbWrite    = CORDIC_NBWRITE_2,
            .NbRead     = CORDIC_NBREAD_1,
            .InSize     = CORDIC_INSIZE_32BITS,
            .OutSize    = CORDIC_OUTSIZE_32BITS
        }) != HAL_OK)
    {
        return 0.0f;
    }
    
    // 执行计算
    if (HAL_CORDIC_Calculate(g_hcordic, input, &result, 1, 100) == HAL_OK)
    {
        return (float)result / CORDIC_Q31_SCALE;
    }
    
#if DPSD_ENABLE_PROFILING
    g_profile.cordic_calls++;
#endif
    
    return 0.0f;
}

#endif /* DPSD_USE_CORDIC */

/* ==================== 内部辅助函数 ==================== */

#if !DPSD_USE_CORDIC
/**
 * @brief 生成参考信号查找表（仅在不使用CORDIC时）
 */
static void DPSD_GenerateLUT(void)
{
    uint32_t i;
    
    for (i = 0; i < DPSD_POINTS_PER_CYCLE; i++)
    {
        float phase = 2.0f * PI_F * (float)i / (float)DPSD_POINTS_PER_CYCLE;
        
        g_lut_cos[i] = (dpsd_lut_t)(cosf(phase) * (float)DPSD_LUT_SCALE);
        g_lut_sin[i] = (dpsd_lut_t)(sinf(phase) * (float)DPSD_LUT_SCALE);
    }
}
#endif

/**
 * @brief 计算直流分量
 */
static float DPSD_CalculateDC(const dpsd_adc_t *adc_data, uint32_t length)
{
    uint32_t i;
    double sum = 0.0;
    
    for (i = 0; i < length; i++)
    {
        sum += (double)adc_data[i];
    }
    
    return (float)(sum / (double)length);
}

/**
 * @brief ADC 计数值转电压
 */
static inline float DPSD_ADCToVoltage(dpsd_adc_t adc_val, float dc_offset)
{
    float adc_float = (float)adc_val - dc_offset;
    return adc_float * DPSD_ADC_VREF / (float)DPSD_ADC_FULL_SCALE;
}

/**
 * @brief 限制相位到 -π ~ π
 */
static inline float DPSD_WrapPhase(float phase)
{
    while (phase > PI_F) phase -= 2.0f * PI_F;
    while (phase < -PI_F) phase += 2.0f * PI_F;
    return phase;
}

/* ==================== 公共函数实现 ==================== */

int32_t DPSD_Init(CORDIC_HandleTypeDef *hcordic)
{
#if DPSD_USE_CORDIC
    // 使用 CORDIC 时必须提供句柄
    if (hcordic == NULL)
    {
        return -1;
    }
    
    g_hcordic = hcordic;
    
    // 使能 CORDIC 时钟
    __HAL_RCC_CORDIC_CLK_ENABLE();
    
    // 初始化 CORDIC（默认配置）
    if (HAL_CORDIC_Configure(hcordic,
        &(CORDIC_ConfigTypeDef){
            .Function   = CORDIC_FUNCTION_COSINE,
            .Precision  = DPSD_CORDIC_PRECISION,
            .Scale      = CORDIC_SCALE_0,
            .NbWrite    = CORDIC_NBWRITE_1,
            .NbRead     = CORDIC_NBREAD_2,
            .InSize     = CORDIC_INSIZE_32BITS,
            .OutSize    = CORDIC_OUTSIZE_32BITS
        }) != HAL_OK)
    {
        return -1;
    }
#else
    // 不使用 CORDIC 时生成查找表
    DPSD_GenerateLUT();
#endif
    
    // 重置状态
    g_current_freq = DPSD_TARGET_FREQ;
    g_last_phase = 0.0f;
    
#if DPSD_ENABLE_PROFILING
    memset(&g_profile, 0, sizeof(g_profile));
#endif
    
    g_initialized = 1;
    
    return 0;
}

void DPSD_DeInit(void)
{
#if DPSD_USE_CORDIC
    if (g_hcordic != NULL)
    {
        HAL_CORDIC_DeInit(g_hcordic);
        __HAL_RCC_CORDIC_CLK_DISABLE();
        g_hcordic = NULL;
    }
#endif
    
    g_initialized = 0;
}

int32_t DPSD_Process(const dpsd_adc_t *adc_data, dpsd_result_t *result)
{
    uint32_t i;
    float dc_offset = 0.0f;
    double I_sum = 0.0;
    double Q_sum = 0.0;
    float I, Q;
    float amplitude, phase_rad, phase_deg;
    
#if DPSD_ENABLE_PROFILING
    uint32_t start_cycles = DWT->CYCCNT;
#endif
    
    // 检查初始化
    if (!g_initialized)
    {
        return -1;
    }
    
    // 检查参数
    if (adc_data == NULL || result == NULL)
    {
        return -1;
    }
    
    // 清零结果
    memset(result, 0, sizeof(dpsd_result_t));
    
    /* ==================== 步骤1：去除直流分量 ==================== */
    
#if DPSD_ENABLE_DC_REMOVAL
    dc_offset = DPSD_CalculateDC(adc_data, DPSD_BLOCK_SIZE);
    result->dc_offset_v = DPSD_ADCToVoltage((dpsd_adc_t)dc_offset, 0.0f);
#endif
    
    /* ==================== 步骤2：正交混频与积分 ==================== */
    
    for (i = 0; i < DPSD_BLOCK_SIZE; i++)
    {
        float voltage = DPSD_ADCToVoltage(adc_data[i], dc_offset);
        float ref_cos, ref_sin;
        
#if DPSD_USE_CORDIC
        // 使用 CORDIC 计算参考信号
        float phase = 2.0f * PI_F * g_current_freq * (float)i / DPSD_SAMPLE_RATE;
        DPSD_CORDIC_SinCos(phase, &ref_sin, &ref_cos);
#else
        // 使用查找表
        uint32_t lut_idx = i % DPSD_POINTS_PER_CYCLE;
        ref_cos = (float)g_lut_cos[lut_idx] / (float)DPSD_LUT_SCALE;
        ref_sin = (float)g_lut_sin[lut_idx] / (float)DPSD_LUT_SCALE;
#endif
        
        I_sum += (double)(voltage * ref_cos);
        Q_sum += (double)(voltage * ref_sin);
    }
    
    // 归一化
    I = (float)(2.0 * I_sum / (double)DPSD_BLOCK_SIZE);
    Q = (float)(-2.0 * Q_sum / (double)DPSD_BLOCK_SIZE);
    
    /* ==================== 步骤3：计算幅度和相位 ==================== */
    
#if DPSD_USE_CORDIC
    amplitude = DPSD_CORDIC_Magnitude(I, Q);
    phase_rad = DPSD_CORDIC_Atan2(Q, I);
#else
    amplitude = sqrtf(I * I + Q * Q);
    phase_rad = atan2f(Q, I);
#endif
    
    phase_deg = phase_rad * 180.0f / PI_F;
    
    if (phase_deg < 0.0f)
    {
        phase_deg += 360.0f;
    }
    
    /* ==================== 步骤4：频率跟踪 ==================== */
    
#if DPSD_ENABLE_FREQ_TRACK
    
    float phase_increment = DPSD_WrapPhase(phase_rad - g_last_phase);
    
    g_last_phase = phase_rad;
    
    if (fabsf(phase_increment) > DPSD_PHASE_DRIFT_LIMIT)
    {
        float freq_error = phase_increment * DPSD_SAMPLE_RATE / 
                          (2.0f * PI_F * (float)DPSD_BLOCK_SIZE);
        
        g_current_freq += DPSD_FREQ_CORRECTION * freq_error;
        
        // 限制频率范围
        float freq_min = DPSD_TARGET_FREQ * (1.0f - DPSD_FREQ_MAX_DRIFT);
        float freq_max = DPSD_TARGET_FREQ * (1.0f + DPSD_FREQ_MAX_DRIFT);
        
        g_current_freq = MAX(freq_min, MIN(freq_max, g_current_freq));
    }
    
    result->frequency = g_current_freq;
    
#else
    result->frequency = DPSD_TARGET_FREQ;
#endif
    
    /* ==================== 步骤5：输出结果 ==================== */
    
    result->amplitude_peak = amplitude;
    result->amplitude_rms = amplitude / sqrtf(2.0f);
    result->amplitude_pp = amplitude * 2.0f;
    result->phase_rad = phase_rad;
    result->phase_deg = phase_deg;
    result->valid = 1;
    
#if DPSD_ENABLE_PROFILING
    uint32_t end_cycles = DWT->CYCCNT;
    g_profile.process_cycles = end_cycles - start_cycles;
    g_profile.process_us = g_profile.process_cycles / (SystemCoreClock / 1000000);
    g_profile.total_processed++;
#endif
    
    return 0;
}

int32_t DPSD_SetTargetFrequency(float freq_hz)
{
    if (freq_hz <= 0.0f || freq_hz > DPSD_SAMPLE_RATE / 2.0f)
    {
        return -1;
    }
    
    g_current_freq = freq_hz;
    g_last_phase = 0.0f;
    
    return 0;
}

float DPSD_GetTargetFrequency(void)
{
    return g_current_freq;
}

void DPSD_ResetFrequencyTracking(void)
{
    g_current_freq = DPSD_TARGET_FREQ;
    g_last_phase = 0.0f;
}

#if DPSD_ENABLE_PROFILING

void DPSD_GetProfile(dpsd_profile_t *profile)
{
    if (profile != NULL)
    {
        memcpy(profile, &g_profile, sizeof(dpsd_profile_t));
    }
}

void DPSD_ResetProfile(void)
{
    memset(&g_profile, 0, sizeof(dpsd_profile_t));
}

#endif
