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

/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */

#include "arm_math.h"
#include "nlms.h"

/* ===================== 用户参数区 ===================== */

#define ADC_DMA_BUFFER_LENGTH      (2U * NLMS_BLOCK_SIZE)
#define CACHE_LINE_SIZE            (32U)

/*
 * 根据你的实际标定修改。
 * 如果 ADC 输入是 0~3.3V，中点偏置 1.65V，12bit ADC，
 * 默认中点大约是 2048。
 */
#define ADC1_ZERO_CODE             (2048.0f)
#define ADC2_ZERO_CODE             (2048.0f)

/*
 * ADC code 转信号幅值比例。
 * 如果你希望转换到“归一化信号”，可以用 1/2048。
 */
#define ADC1_SIGNAL_GAIN           (1.0f / 2048.0f)
#define ADC2_SIGNAL_GAIN           (1.0f / 2048.0f)

/*
 * DAC 输出中点。
 */
#define DAC_MID_CODE               (2048U)

/*
 * PRBS 激励幅度
 *
 * 0.60 对应 DAC 约 2048 ± 1228，留出足够余量
 * 如果 RLC 电路有增益（谐振），0.80 会导致 ADC 削波
 * 削波后的信号失真会严重损害 NLMS 训练质量
 */
#define PRBS_AMPLITUDE             (0.60f)

/* ===================== 运行模式 ===================== */

typedef enum
{
    APP_MODE_BATCH_TRAIN = 0,
    APP_MODE_WAIT_REWIRE,
    APP_MODE_REALTIME_RUN
} AppMode_t;

static volatile AppMode_t g_appMode = APP_MODE_BATCH_TRAIN;

/* ===================== DMA 缓冲区 ===================== */

/*
 * 实时运行阶段 DMA 缓冲区。
 * 长度为 2 * NLMS_BLOCK_SIZE，用于 half/full 双缓冲。
 */
__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc1_dma_buffer[ADC_DMA_BUFFER_LENGTH];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc2_dma_buffer[ADC_DMA_BUFFER_LENGTH];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t dac_buffer[ADC_DMA_BUFFER_LENGTH];

/*
 * 训练阶段 4096 点缓冲区。
 * 注意：训练和运行都使用 Circular DMA。
 * 训练时采满 4096 点后马上停 TIM/ADC/DAC。
 */
__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc1_train_buffer[NLMS_TRAIN_BLOCK_SIZE];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t adc2_train_buffer[NLMS_TRAIN_BLOCK_SIZE];

__attribute__((section(".RAM_D1"), aligned(32)))
uint16_t dac_train_buffer[NLMS_TRAIN_BLOCK_SIZE];

/* ===================== DSP 临时缓冲区 ===================== */

__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t xBlock[NLMS_BLOCK_SIZE];

__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t dBlock[NLMS_BLOCK_SIZE];

__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t yBlock[NLMS_BLOCK_SIZE];

__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t eBlock[NLMS_BLOCK_SIZE];

/*
 * 训练阶段 4096 点转换缓冲。
 * 如果 DTCM 空间不够，可以把这两个数组改到 RAM_D1。
 */
__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t xTrainBlock[NLMS_TRAIN_BLOCK_SIZE];

__attribute__((section(".DTCM_DATA"), aligned(32)))
static float32_t dTrainBlock[NLMS_TRAIN_BLOCK_SIZE];

/* ===================== DMA 标志位 ===================== */

/* 实时阶段 ADC/DAC half/full 标志 */
volatile uint8_t adc1_half_flag = 0U;
volatile uint8_t adc1_full_flag = 0U;
volatile uint8_t adc2_half_flag = 0U;
volatile uint8_t adc2_full_flag = 0U;

volatile uint8_t dac_half_flag = 0U;
volatile uint8_t dac_full_flag = 0U;

/* 丢块统计 */
volatile uint32_t adc_half_block_lost = 0U;
volatile uint32_t adc_full_block_lost = 0U;

/* 训练阶段采集完成标志 */
volatile uint8_t train_adc1_done = 0U;
volatile uint8_t train_adc2_done = 0U;
volatile uint8_t train_dac_done  = 0U;

/* PRBS 状态 */
static uint32_t g_prbsState = 0xACE1U;

/* ===================== DWT 计数器，可选 ===================== */

static void DWT_CycleCounterInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}


/* ===================== 信号转换函数 ===================== */

static float32_t AdcCodeToSignal(uint16_t adcCode,
                                 float32_t zeroCode,
                                 float32_t gain)
{
    return (((float32_t)adcCode) - zeroCode) * gain;
}

static uint16_t SignalToDacCode(float32_t x)
{
    float32_t code;

    /*
     * x 是归一化信号。
     * x = 1.0 对应 DAC 增加 2047。
     */
    code = 2048.0f + x * 2047.0f;

    if (code < 0.0f)
    {
        code = 0.0f;
    }
    else if (code > 4095.0f)
    {
        code = 4095.0f;
    }

    return (uint16_t)(code + 0.5f);
}

/* 简单 PRBS，输出 ±PRBS_AMPLITUDE */
static float32_t GeneratePrbsSignal(void)
{
    uint32_t bit;

    bit = ((g_prbsState >> 0U) ^
           (g_prbsState >> 2U) ^
           (g_prbsState >> 3U) ^
           (g_prbsState >> 5U)) & 1U;

    g_prbsState = (g_prbsState >> 1U) | (bit << 15U);

    if ((g_prbsState & 1U) != 0U)
    {
        return PRBS_AMPLITUDE;
    }
    else
    {
        return -PRBS_AMPLITUDE;
    }
}

/* ===================== 外设停止函数 ===================== */

static void Stop_All_Sampling_Output(void)
{
    HAL_TIM_Base_Stop(&htim4);

    HAL_ADC_Stop_DMA(&hadc1);
    HAL_ADC_Stop_DMA(&hadc2);

    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

    HAL_DAC_SetValue(&hdac1,
                     DAC_CHANNEL_1,
                     DAC_ALIGN_12B_R,
                     DAC_MID_CODE);
}

/* ===================== 训练阶段：启动一批 4096 点采样 ===================== */

static void BatchTrain_StartCapture(void)
{
    uint32_t i;
    uint32_t byteCount;

    byteCount = NLMS_TRAIN_BLOCK_SIZE * sizeof(uint16_t);

    train_adc1_done = 0U;
    train_adc2_done = 0U;
    train_dac_done  = 0U;

    /*
     * 填充一批 PRBS 到 DAC buffer。
     * 因为训练阶段 DMA 也是 Circular，
     * 所以我们只关心第一圈 4096 点。
     */
    for (i = 0U; i < NLMS_TRAIN_BLOCK_SIZE; i++)
    {
        dac_train_buffer[i] = SignalToDacCode(GeneratePrbsSignal());
    }

    DCache_Clean_By_Addr(dac_train_buffer, byteCount);

    DCache_Invalidate_By_Addr(adc1_train_buffer, byteCount);
    DCache_Invalidate_By_Addr(adc2_train_buffer, byteCount);

    /*
     * 确保定时器还没开始。
     * 先启动 ADC/DAC DMA，再启动 TIM4。
     */
    HAL_TIM_Base_Stop(&htim4);

    HAL_ADC_Start_DMA(&hadc1,
                      (uint32_t *)adc1_train_buffer,
                      NLMS_TRAIN_BLOCK_SIZE);

    HAL_ADC_Start_DMA(&hadc2,
                      (uint32_t *)adc2_train_buffer,
                      NLMS_TRAIN_BLOCK_SIZE);

    HAL_DAC_Start_DMA(&hdac1,
                      DAC_CHANNEL_1,
                      (uint32_t *)dac_train_buffer,
                      NLMS_TRAIN_BLOCK_SIZE,
                      DAC_ALIGN_12B_R);

    HAL_TIM_Base_Start(&htim4);
}

/* ===================== 训练阶段：处理一批 4096 点 ===================== */

static void BatchTrain_ProcessBlock(void)
{
    uint32_t i;
    uint32_t byteCount;
    uint32_t startIndex;
    uint32_t validLength;
    uint32_t remainLength;
    uint32_t blockCount;

    byteCount = NLMS_TRAIN_BLOCK_SIZE * sizeof(uint16_t);

    DCache_Invalidate_By_Addr(adc1_train_buffer, byteCount);
    DCache_Invalidate_By_Addr(adc2_train_buffer, byteCount);

    startIndex = NLMS_TRAIN_DISCARD_SAMPLES;

    if (startIndex >= NLMS_TRAIN_BLOCK_SIZE)
    {
        startIndex = 0U;
    }

    validLength = NLMS_TRAIN_BLOCK_SIZE - startIndex;

    for (i = 0U; i < validLength; i++)
    {
        xTrainBlock[i] = AdcCodeToSignal(adc1_train_buffer[startIndex + i],
                                         ADC1_ZERO_CODE,
                                         ADC1_SIGNAL_GAIN);

        dTrainBlock[i] = AdcCodeToSignal(adc2_train_buffer[startIndex + i],
                                         ADC2_ZERO_CODE,
                                         ADC2_SIGNAL_GAIN);
    }

    /*
     * 为了兼容你原来的 NLMS_RLC_ProcessBlock()，
     * 这里仍然每次处理 NLMS_BLOCK_SIZE 点。
     */
    remainLength = validLength;
    blockCount = 0U;

    while (remainLength >= NLMS_BLOCK_SIZE)
    {
        NLMS_RLC_ProcessBlock(&xTrainBlock[blockCount * NLMS_BLOCK_SIZE],
                              &dTrainBlock[blockCount * NLMS_BLOCK_SIZE],
                              yBlock,
                              eBlock);

        blockCount++;
        remainLength -= NLMS_BLOCK_SIZE;

        if (NLMS_RLC_IsTrainingDone() != 0U)
        {
            break;
        }
    }
}

/* ===================== 训练阶段：循环训练直到完成 ===================== */

static void BatchTrain_RunUntilDone(void)
{
    while (NLMS_RLC_IsTrainingDone() == 0U)
    {
        g_appMode = APP_MODE_BATCH_TRAIN;

        BatchTrain_StartCapture();

        /*
         * 因为 DMA 是 Circular，所以 full complete 之后会继续转。
         * 这里一旦 ADC1/ADC2 都采满第一圈，马上停止外设。
         */
        while ((train_adc1_done == 0U) ||
               (train_adc2_done == 0U) ||
               (train_dac_done  == 0U))
        {
            /*
             * 等待 4096 点采集完成。
             * 不在中断里做 NLMS 计算。
             */
        }

        Stop_All_Sampling_Output();

        BatchTrain_ProcessBlock();
    }
}

/* ===================== 实时阶段：启动双缓冲 FIR 输出 ===================== */

static void Realtime_Start(void)
{
    uint32_t i;
    uint32_t byteCount;

    byteCount = ADC_DMA_BUFFER_LENGTH * sizeof(uint16_t);

    adc1_half_flag = 0U;
    adc1_full_flag = 0U;
    adc2_half_flag = 0U;
    adc2_full_flag = 0U;

    dac_half_flag = 0U;
    dac_full_flag = 0U;

    adc_half_block_lost = 0U;
    adc_full_block_lost = 0U;

    for (i = 0U; i < ADC_DMA_BUFFER_LENGTH; i++)
    {
        dac_buffer[i] = DAC_MID_CODE;
    }

    DCache_Clean_By_Addr(dac_buffer, byteCount);

    DCache_Invalidate_By_Addr(adc1_dma_buffer, byteCount);
    DCache_Invalidate_By_Addr(adc2_dma_buffer, byteCount);

    g_appMode = APP_MODE_REALTIME_RUN;

    HAL_TIM_Base_Stop(&htim4);

    HAL_ADC_Start_DMA(&hadc1,
                      (uint32_t *)adc1_dma_buffer,
                      ADC_DMA_BUFFER_LENGTH);

    /*
     * 运行阶段不需要 ADC2
     * 不启动 ADC2 DMA，节省带宽和中断开销
     */

    HAL_DAC_Start_DMA(&hdac1,
                      DAC_CHANNEL_1,
                      (uint32_t *)dac_buffer,
                      ADC_DMA_BUFFER_LENGTH,
                      DAC_ALIGN_12B_R);

    HAL_TIM_Base_Start(&htim4);
}

/* ===================== 实时阶段：处理半缓冲或满缓冲 ===================== */

/*
 * 实时阶段：ADC1 → 归一化 → FIR → DAC
 *
 * 使用 NLMS_RLC_ProcessFIROnly() 配合 arm_fir_f32
 * 比原来的 NLMS_RLC_ProcessBlock() 快 2~4 倍
 * 不再计算误差和功率，不做系数更新
 */
__attribute__((section(".ITCM_CODE"), noinline))
static void Realtime_ProcessAdcBlock(uint16_t *adc1Ptr,
                                     uint16_t *dacPtr)
{
    uint32_t i;

    DCache_Invalidate_By_Addr(adc1Ptr,
                              NLMS_BLOCK_SIZE * sizeof(uint16_t));

    /*
     * 运行阶段：
     * 外部输入 -> ADC1 -> 归一化 -> 冻结 FIR -> DAC
     */
    for (i = 0U; i < NLMS_BLOCK_SIZE; i++)
    {
        xBlock[i] = AdcCodeToSignal(adc1Ptr[i],
                                    ADC1_ZERO_CODE,
                                    ADC1_SIGNAL_GAIN);
    }

    /*
     * 使用 arm_fir_f32 高速 FIR 计算
     * 系数已在 NLMS_RLC_PrepareFIROnly() 中加载
     * 状态在 arm_fir 实例中自动维护，连续调用无缝衔接
     */
    NLMS_RLC_ProcessFIROnly(xBlock, yBlock, NLMS_BLOCK_SIZE);

    for (i = 0U; i < NLMS_BLOCK_SIZE; i++)
    {
        dacPtr[i] = SignalToDacCode(yBlock[i]);
    }

    DCache_Clean_By_Addr(dacPtr,
                         NLMS_BLOCK_SIZE * sizeof(uint16_t));
}

/* ===================== ADC 回调函数 ===================== */

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (g_appMode != APP_MODE_REALTIME_RUN)
    {
        return;
    }

    if (hadc->Instance == ADC1)
    {
        if (adc1_half_flag == 0U)
        {
            adc1_half_flag = 1U;
        }
        else
        {
            adc_half_block_lost++;
        }
    }
    else if (hadc->Instance == ADC2)
    {
        if (adc2_half_flag == 0U)
        {
            adc2_half_flag = 1U;
        }
        else
        {
            adc_half_block_lost++;
        }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (g_appMode == APP_MODE_BATCH_TRAIN)
    {
        if (hadc->Instance == ADC1)
        {
            train_adc1_done = 1U;
        }
        else if (hadc->Instance == ADC2)
        {
            train_adc2_done = 1U;
        }

        return;
    }

    if (g_appMode == APP_MODE_REALTIME_RUN)
    {
        if (hadc->Instance == ADC1)
        {
            if (adc1_full_flag == 0U)
            {
                adc1_full_flag = 1U;
            }
            else
            {
                adc_full_block_lost++;
            }
        }
        else if (hadc->Instance == ADC2)
        {
            if (adc2_full_flag == 0U)
            {
                adc2_full_flag = 1U;
            }
            else
            {
                adc_full_block_lost++;
            }
        }
    }
}

/* ===================== DAC 回调函数 ===================== */

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (g_appMode == APP_MODE_REALTIME_RUN)
    {
        if (hdac->Instance == DAC1)
        {
            dac_half_flag = 1U;
        }
    }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (g_appMode == APP_MODE_BATCH_TRAIN)
    {
        if (hdac->Instance == DAC1)
        {
            train_dac_done = 1U;
        }

        return;
    }

    if (g_appMode == APP_MODE_REALTIME_RUN)
    {
        if (hdac->Instance == DAC1)
        {
            dac_full_flag = 1U;
        }
    }
}

/* USER CODE END 0 */


   	                                                                                              
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
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_ADC2_Init();
  MX_DAC1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	SRAM_Clock_Enable();    // 如果这个函数确实还需要，放在时钟后
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	/* USER CODE BEGIN 2 */

/* USER CODE BEGIN 2 */

DWT_CycleCounterInit();

NLMS_RLC_Init();

/*
 * 阶段 1：批量训练
 *
 * 接线：
 * DAC -> RLC 输入
 * RLC 输入同时接 ADC1
 * RLC 输出接 ADC2
 *
 * 每次采 4096 点。
 * 采满后停止 TIM/ADC/DAC。
 * 然后 CPU 慢慢计算 NLMS。
 */
g_appMode = APP_MODE_BATCH_TRAIN;
BatchTrain_RunUntilDone();

/*
 * 阶段 2：训练完成，停止所有采样和输出。
 */
Stop_All_Sampling_Output();

/*
 * 初始化 arm_fir 实例，使用训练得到的系数
 * 这会将 g_coeff 交给 arm_fir_f32 使用
 * 并清零 FIR 状态缓冲区
 */
NLMS_RLC_PrepareFIROnly();

/* 重置性能计数器，只监控实时阶段 */
NLMS_RLC_ResetPerformance();

g_appMode = APP_MODE_WAIT_REWIRE;

HAL_DAC_SetValue(&hdac1,
                 DAC_CHANNEL_1,
                 DAC_ALIGN_12B_R,
                 DAC_MID_CODE);

/*
 * 在这里打 Keil 断点。
 *
 * 程序停住后手动换线：
 *
 * 外部信号源 -> ADC1
 * DAC 输出    -> 示波器
 * GND 全部共地
 *
 * 外部信号必须带 1.65V 偏置，并且范围不能超过 0~3.3V。
 */
__NOP();

/*
 * 换线完成后，点击 Keil Run。
 * 程序从这里继续，进入实时 FIR 输出阶段。
 */
Realtime_Start();

/* USER CODE END 2 */


/* USER CODE END 2 */

  while (1)
{
    /*
     * 独立处理 ADC half/full 块，不再要求 DAC 标志必须同时到达。
     *
     * ADC 和 DAC 由同一个 TIM4 触发，DMA 进度理论同步。
     * 当中断到达时间偶有偏差时，先处理 ADC 数据，
     * 写入 DAC buffer 时 DAC DMA 通常已经跳过了该区域。
     *
     * 丢块计数器保留，监控实际丢失情况。
     */

    /* --- Half 块 --- */
    if (adc1_half_flag != 0U)
    {
        adc1_half_flag = 0U;

        Realtime_ProcessAdcBlock(&adc1_dma_buffer[0],
                                 &dac_buffer[0]);

        /* DAC half 区域的 DMA 同步：有则清，无则忽略 */
        if (dac_half_flag != 0U)
        {
            dac_half_flag = 0U;
        }
    }

    /* --- Full 块 --- */
    if (adc1_full_flag != 0U)
    {
        adc1_full_flag = 0U;

        Realtime_ProcessAdcBlock(&adc1_dma_buffer[NLMS_BLOCK_SIZE],
                                 &dac_buffer[NLMS_BLOCK_SIZE]);

        if (dac_full_flag != 0U)
        {
            dac_full_flag = 0U;
        }
    }

    /* 清理孤单的 DAC 标志（无对应 ADC 数据时） */
    if (dac_half_flag != 0U)
    {
        dac_half_flag = 0U;
    }

    if (dac_full_flag != 0U)
    {
        dac_full_flag = 0U;
    }

    /* USER CODE BEGIN 3 */
    scheduler_run();
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
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 32;
  PeriphClkInitStruct.PLL2.PLL2N = 129;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
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
  __disable_irq();
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
