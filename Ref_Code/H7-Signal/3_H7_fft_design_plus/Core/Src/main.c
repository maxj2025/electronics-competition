/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "octospi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bspsysteam.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include <string.h>
#include <stdint.h>
/* USER CODE END Includes */


/* USER CODE BEGIN 0 */


/* ===================== 参数配置 ===================== */


/* ===================== 参数配置 ===================== */

#define CACHE_LINE_SIZE         32U

#define FFT_LENGTH              4096U
#define ADC_DMA_LENGTH          (FFT_LENGTH * 2U)
#define DAC_DMA_LENGTH          (FFT_LENGTH * 2U)

#define SAMPLE_RATE             512000.0f
#define FFT_BIN_HZ              (SAMPLE_RATE / (float32_t)FFT_LENGTH)

#define DAC_CENTER              2048.0f

#define DC_IGNORE_BINS          4U

/*
 * 512kHz / 4096 = 125Hz
 * 1kHz = 8 个 FFT bin
 */
#define BINS_PER_1KHZ           8U

#define DEFAULT_OUTPUT_GAIN     0.8f

/*
 * 块间连续 + 慢速跟踪的相位修正系数。
 *
 * 0.01f  更平滑，但锁相更慢
 * 0.03f  一般比较折中
 * 0.08f  跟踪更快，但更容易看到块间晃动
 */
#define PHASE_TRACK_ALPHA       0.03f

/*
 * 频点锁定/滞回参数。
 *
 * PEAK_SWITCH_RATIO:
 *     新频点能量必须大于当前锁定频点能量的这个倍数，才允许切换。
 *
 * PEAK_UNLOCK_RATIO:
 *     如果当前锁定频点能量低于候选最大频点能量的这个比例，认为当前锁定失效。
 *
 * PEAK_MIN_MAG2:
 *     太小的信号不锁定，防止噪声乱跳。
 */
#define PEAK_SWITCH_RATIO       1.40f
#define PEAK_UNLOCK_RATIO       0.35f
#define PEAK_MIN_MAG2           1000.0f

/*
 * CMSIS arm_cfft_f32 的 IFFT 一般已经带 1/N 缩放。
 * 如果你的输出幅值大了 4096 倍，把这里改成：
 * #define IFFT_EXTRA_SCALE      (1.0f / (float32_t)FFT_LENGTH)
 */
#define IFFT_EXTRA_SCALE        1.0f

#ifndef PI
#define PI                      3.14159265358979323846f
#endif

/* ===================== 外部句柄 ===================== */

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim4;

/* ===================== 用户运行中可改参数 ===================== */

volatile float32_t user_phase_shift_rad = 0.0f;
volatile float32_t user_output_gain = DEFAULT_OUTPUT_GAIN;

/* ===================== DMA Buffer ===================== */

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc_dma_buf[ADC_DMA_LENGTH];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t dac_dma_buf[DAC_DMA_LENGTH];

/* ===================== DSP Buffer ===================== */

__attribute__((section(".DTCM_DATA"), aligned(32)))
float32_t fft_buffer[FFT_LENGTH * 2U];

static arm_cfft_instance_f32 fft_inst;

/* ===================== 乒乓标志 ===================== */

volatile uint8_t adc_half_ready = 0U;
volatile uint8_t adc_full_ready = 0U;
volatile uint8_t dac_half_free = 0U;
volatile uint8_t dac_full_free = 0U;

/* ===================== 相位连续状态 ===================== */

static float32_t output_phase_state = 0.0f;
static float32_t last_user_phase_shift_rad = 0.0f;
static uint32_t last_peak_bin = 0U;
static uint8_t output_phase_state_valid = 0U;

/* ===================== 频点锁定状态 ===================== */

static uint32_t locked_peak_bin = 0U;
static uint8_t peak_lock_valid = 0U;

/* ===================== Cache 操作 ===================== */

__attribute__((section(".ITCM_CODE")))
static void DCache_Invalidate_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr = (uint32_t)addr;
    uint32_t end_addr = start_addr + size;

    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);

    SCB_InvalidateDCache_by_Addr((uint32_t *)start_addr,
                                 (int32_t)(end_addr - start_addr));
}

__attribute__((section(".ITCM_CODE")))
static void DCache_Clean_By_Addr(void *addr, uint32_t size)
{
    uint32_t start_addr = (uint32_t)addr;
    uint32_t end_addr = start_addr + size;

    start_addr &= ~(CACHE_LINE_SIZE - 1U);
    end_addr = (end_addr + CACHE_LINE_SIZE - 1U) & ~(CACHE_LINE_SIZE - 1U);

    SCB_CleanDCache_by_Addr((uint32_t *)start_addr,
                            (int32_t)(end_addr - start_addr));
}

/* ===================== 工具函数 ===================== */

__attribute__((section(".ITCM_CODE")))
static float32_t wrap_to_pi(float32_t x)
{
    while (x > PI) {
        x -= 2.0f * PI;
    }

    while (x < -PI) {
        x += 2.0f * PI;
    }

    return x;
}

__attribute__((section(".ITCM_CODE")))
static float32_t clamp_f32(float32_t x, float32_t min_v, float32_t max_v)
{
    if (x < min_v) {
        return min_v;
    }

    if (x > max_v) {
        return max_v;
    }

    return x;
}

__attribute__((section(".ITCM_CODE")))
static float32_t get_bin_mag2(const float32_t *buf, uint32_t bin)
{
    float32_t re;
    float32_t im;

    if ((bin == 0U) || (bin >= (FFT_LENGTH / 2U))) {
        return 0.0f;
    }

    re = buf[bin * 2U];
    im = buf[bin * 2U + 1U];

    return re * re + im * im;
}

__attribute__((section(".ITCM_CODE")))
static uint32_t snap_bin_to_1khz_multiple(uint32_t bin)
{
    uint32_t snapped;

    if (bin < BINS_PER_1KHZ) {
        return BINS_PER_1KHZ;
    }

    snapped = ((bin + (BINS_PER_1KHZ / 2U)) / BINS_PER_1KHZ) * BINS_PER_1KHZ;

    if (snapped < BINS_PER_1KHZ) {
        snapped = BINS_PER_1KHZ;
    }

    if (snapped >= (FFT_LENGTH / 2U)) {
        snapped = (FFT_LENGTH / 2U) - BINS_PER_1KHZ;
    }

    return snapped;
}

__attribute__((section(".ITCM_CODE")))
static uint32_t find_raw_peak_bin(const float32_t *buf, float32_t *peak_mag2_out)
{
    float32_t max_mag2 = 0.0f;
    uint32_t max_bin = BINS_PER_1KHZ;

    for (uint32_t k = DC_IGNORE_BINS; k < (FFT_LENGTH / 2U); k++) {
        float32_t mag2 = get_bin_mag2(buf, k);

        if (mag2 > max_mag2) {
            max_mag2 = mag2;
            max_bin = k;
        }
    }

    *peak_mag2_out = max_mag2;

    return max_bin;
}

/*
 * 频点锁定/滞回。
 *
 * 已锁定时：
 * 1. 如果候选频点和锁定频点相同，继续保持。
 * 2. 如果候选频点不同，只有候选能量明显更强才切换。
 * 3. 如果锁定频点能量已经很弱，也允许切换。
 */
__attribute__((section(".ITCM_CODE")))
static uint32_t get_locked_peak_bin(const float32_t *buf)
{
    uint32_t raw_peak_bin;
    uint32_t candidate_bin;
    float32_t raw_peak_mag2;
    float32_t candidate_mag2;
    float32_t locked_mag2;

    raw_peak_bin = find_raw_peak_bin(buf, &raw_peak_mag2);
    candidate_bin = snap_bin_to_1khz_multiple(raw_peak_bin);
    candidate_mag2 = get_bin_mag2(buf, candidate_bin);

    if ((peak_lock_valid == 0U) || (candidate_mag2 < PEAK_MIN_MAG2)) {
        locked_peak_bin = candidate_bin;
        peak_lock_valid = 1U;
        return locked_peak_bin;
    }

    locked_mag2 = get_bin_mag2(buf, locked_peak_bin);

    if (candidate_bin == locked_peak_bin) {
        return locked_peak_bin;
    }

    if (locked_mag2 < PEAK_MIN_MAG2) {
        locked_peak_bin = candidate_bin;
        return locked_peak_bin;
    }

    if (locked_mag2 < (candidate_mag2 * PEAK_UNLOCK_RATIO)) {
        locked_peak_bin = candidate_bin;
        return locked_peak_bin;
    }

    if (candidate_mag2 > (locked_mag2 * PEAK_SWITCH_RATIO)) {
        locked_peak_bin = candidate_bin;
        return locked_peak_bin;
    }

    return locked_peak_bin;
}

__attribute__((section(".ITCM_CODE")))
static void zero_complex_spectrum(float32_t *buf)
{
    for (uint32_t i = 0; i < FFT_LENGTH * 2U; i++) {
        buf[i] = 0.0f;
    }
}

/*
 * 使用 tan(phi') 公式计算 Re' / Im'，并做象限修正。
 */
__attribute__((section(".ITCM_CODE")))
static void calc_re_im_by_tan_formula(float32_t amp,
                                      float32_t phase,
                                      float32_t *re_out,
                                      float32_t *im_out)
{
    float32_t phase_wrapped = wrap_to_pi(phase);
    float32_t cos_p = arm_cos_f32(phase_wrapped);
    float32_t sin_p = arm_sin_f32(phase_wrapped);
    float32_t tan_p;
    float32_t re_new;
    float32_t im_new;

    if (fabsf(cos_p) < 1.0e-6f) {
        *re_out = 0.0f;
        *im_out = (sin_p >= 0.0f) ? amp : -amp;
        return;
    }

    tan_p = sin_p / cos_p;

    re_new = sqrtf((amp * amp) / (1.0f + tan_p * tan_p));
    im_new = tan_p * re_new;

    if (cos_p < 0.0f) {
        re_new = -re_new;
        im_new = -im_new;
    }

    *re_out = re_new;
    *im_out = im_new;
}

/*
 * 块间相位连续 + 慢速输入相位跟踪。
 */
__attribute__((section(".ITCM_CODE")))
static float32_t get_continuous_block_phase(uint32_t peak_bin,
                                            float32_t measured_phase,
                                            float32_t phase_shift)
{
    float32_t desired_phase;
    float32_t phase_shift_delta;
    float32_t phase_error;

    desired_phase = wrap_to_pi(measured_phase + phase_shift);

    if ((output_phase_state_valid == 0U) || (peak_bin != last_peak_bin)) {
        output_phase_state = desired_phase;
        last_user_phase_shift_rad = phase_shift;
        last_peak_bin = peak_bin;
        output_phase_state_valid = 1U;
        return output_phase_state;
    }

    phase_shift_delta = phase_shift - last_user_phase_shift_rad;

    if (phase_shift_delta > PI) {
        phase_shift_delta -= 2.0f * PI;
    } else if (phase_shift_delta < -PI) {
        phase_shift_delta += 2.0f * PI;
    }

    output_phase_state = wrap_to_pi(output_phase_state + phase_shift_delta);

    phase_error = wrap_to_pi(desired_phase - output_phase_state);

    output_phase_state = wrap_to_pi(output_phase_state +
                                    PHASE_TRACK_ALPHA * phase_error);

    last_user_phase_shift_rad = phase_shift;
    last_peak_bin = peak_bin;

    return output_phase_state;
}

/* ===================== 初始化 ===================== */

void DSP_Process_Init(void)
{
    if (arm_cfft_init_f32(&fft_inst, FFT_LENGTH) != ARM_MATH_SUCCESS) {
        Error_Handler();
    }
}

static void Audio_DMA_Buffer_Init(void)
{
    for (uint32_t i = 0; i < DAC_DMA_LENGTH; i++) {
        dac_dma_buf[i] = (uint16_t)DAC_CENTER;
    }

    DCache_Invalidate_By_Addr(adc_dma_buf,
                              ADC_DMA_LENGTH * sizeof(uint16_t));

    DCache_Clean_By_Addr(dac_dma_buf,
                         DAC_DMA_LENGTH * sizeof(uint16_t));
}

/* ===================== 核心处理 ===================== */

__attribute__((section(".ITCM_CODE")))
void Process_FFT_IFFT_Block(uint32_t offset)
{
    uint32_t block_bytes = FFT_LENGTH * sizeof(uint16_t);

    uint32_t peak_bin;
    uint32_t mirror_bin;

    float32_t mean = 0.0f;

    float32_t re;
    float32_t im;
    float32_t amp;
    float32_t phase;

    float32_t output_phase;
    float32_t new_re;
    float32_t new_im;

    float32_t phase_shift;
    float32_t output_gain;

    DCache_Invalidate_By_Addr(&adc_dma_buf[offset], block_bytes);

    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        mean += (float32_t)adc_dma_buf[offset + i];
    }

    mean /= (float32_t)FFT_LENGTH;

    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        fft_buffer[i * 2U] = (float32_t)adc_dma_buf[offset + i] - mean;
        fft_buffer[i * 2U + 1U] = 0.0f;
    }

    arm_cfft_f32(&fft_inst, fft_buffer, 0, 1);

    /*
     * 频点锁定/滞回后的 peak_bin。
     * 这样不会因为高频时能量轻微抖动，在相邻频点之间来回跳。
     */
    peak_bin = get_locked_peak_bin(fft_buffer);
    mirror_bin = FFT_LENGTH - peak_bin;

    re = fft_buffer[peak_bin * 2U];
    im = fft_buffer[peak_bin * 2U + 1U];

    amp = sqrtf(re * re + im * im);
    phase = atan2f(im, re);

    phase_shift = user_phase_shift_rad;
    output_gain = clamp_f32(user_output_gain, 0.0f, 2.0f);

    output_phase = get_continuous_block_phase(peak_bin, phase, phase_shift);

    calc_re_im_by_tan_formula(amp, output_phase, &new_re, &new_im);

    zero_complex_spectrum(fft_buffer);

    fft_buffer[peak_bin * 2U] = new_re;
    fft_buffer[peak_bin * 2U + 1U] = new_im;

    fft_buffer[mirror_bin * 2U] = new_re;
    fft_buffer[mirror_bin * 2U + 1U] = -new_im;

    arm_cfft_f32(&fft_inst, fft_buffer, 1, 1);

    for (uint32_t i = 0; i < FFT_LENGTH; i++) {
        float32_t y = fft_buffer[i * 2U] * IFFT_EXTRA_SCALE;
        float32_t dac_f = DAC_CENTER + output_gain * y;

        dac_dma_buf[offset + i] = (uint16_t)__USAT((int32_t)dac_f, 12);
    }

    DCache_Clean_By_Addr(&dac_dma_buf[offset], block_bytes);
}

/* ===================== 乒乓调度 ===================== */

__attribute__((section(".ITCM_CODE")))
void adc_proc(void)
{
    uint8_t do_half = 0U;
    uint8_t do_full = 0U;

    __disable_irq();

    if ((adc_half_ready != 0U) && (dac_half_free != 0U)) {
        adc_half_ready = 0U;
        dac_half_free = 0U;
        do_half = 1U;
    }

    if ((adc_full_ready != 0U) && (dac_full_free != 0U)) {
        adc_full_ready = 0U;
        dac_full_free = 0U;
        do_full = 1U;
    }

    __enable_irq();

    if (do_half != 0U) {
        Process_FFT_IFFT_Block(0U);
    }

    if (do_full != 0U) {
        Process_FFT_IFFT_Block(FFT_LENGTH);
    }
}

/* ===================== ADC 回调 ===================== */

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_half_ready = 1U;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_full_ready = 1U;
    }
}

/* ===================== DAC 回调 ===================== */

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DAC1) {
        dac_half_free = 1U;
    }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DAC1) {
        dac_full_free = 1U;
    }
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DAC1) {
        Error_Handler();
    }
}

/* ===================== DMA 启动 ===================== */

void Audio_DMA_Start(void)
{
    Audio_DMA_Buffer_Init();

    adc_half_ready = 0U;
    adc_full_ready = 0U;
    dac_half_free = 0U;
    dac_full_free = 0U;

    output_phase_state = 0.0f;
    last_user_phase_shift_rad = user_phase_shift_rad;
    last_peak_bin = 0U;
    output_phase_state_valid = 0U;

    locked_peak_bin = 0U;
    peak_lock_valid = 0U;

    if (HAL_ADC_Start_DMA(&hadc1,
                          (uint32_t *)&adc_dma_buf[0],
                          ADC_DMA_LENGTH) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_DAC_Start_DMA(&hdac1,
                          DAC_CHANNEL_1,
                          (uint32_t *)&dac_dma_buf[0],
                          DAC_DMA_LENGTH,
                          DAC_ALIGN_12B_R) != HAL_OK) {
        Error_Handler();
    }

    /*
     * TIM4 同时触发 ADC 和 DAC。
     *
     * CubeMX 需要设置：
     * ADC1 External Trigger = TIM4_TRGO
     * DAC1 CH1 Trigger      = TIM4_TRGO
     * TIM4 TRGO             = Update Event
     * TIM4 Update Frequency = 512kHz
     */
    if (HAL_TIM_Base_Start(&htim4) != HAL_OK) {
        Error_Handler();
    }
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  MX_OCTOSPI1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_DAC1_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	SRAM_Clock_Enable();    // 如果这个函数确实还需要，放在时钟后
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	SRAM_Clock_Enable();

  scheduler_Init();

DSP_Process_Init();
Audio_DMA_Start();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	adc_proc();
	
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0xE0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER5;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
  while (1)
  {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
