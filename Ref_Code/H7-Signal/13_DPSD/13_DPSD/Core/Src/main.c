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
#include "cordic.h"
#include "dma.h"
#include "fmac.h"
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

#define ADC_DMA_BUFFER_SIZE    (2U*DPSD_BLOCK_SIZE)

__attribute__((section (".RAM_D1"))) dpsd_adc_t g_adc_dma_buffer[ADC_DMA_BUFFER_SIZE];

__attribute__((section (".RAM_D1"))) volatile uint8_t g_full_buffer_ready = 0U;
__attribute__((section (".RAM_D1"))) volatile uint8_t g_half_buffer_ready = 0U;
__attribute__((section (".RAM_D1"))) dpsd_result_t g_dpsd_result;
__attribute__((section (".RAM_D1"))) volatile uint32_t g_dma_overrun_count = 0U;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /*
         * 前半缓冲区再次采集完成时，若主循环还没处理上一次前半缓冲区，
         * 说明处理速度不足，DMA 即将覆盖旧数据。
         */
        if (g_half_buffer_ready != 0U)
        {
            g_dma_overrun_count++;
        }

        g_half_buffer_ready = 1U;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /*
         * 后半缓冲区再次采集完成时，若主循环还没处理，
         * 说明处理速度不足。
         */
        if (g_full_buffer_ready != 0U)
        {
            g_dma_overrun_count++;
        }

        g_full_buffer_ready = 1U;
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        UART1_Printf("ADC DMA Error: 0x%08lX\r\n", hadc->ErrorCode);
        Error_Handler();
    }
}



/* DWT 周期计数器，用于 DPSD_ENABLE_PROFILING */
static void DWT_CycleCounter_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#if defined(DWT_LAR)
    DWT->LAR = 0xC5ACCE55U;
#endif

    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void DPSD_ProcessOneBlock(dpsd_adc_t *buffer)
{
    int32_t status;
    static uint32_t print_div = 0U;

    /*
     * DMA 已写完该半缓冲区。
     * CPU 读取前使对应 Cache 区域失效，确保读取的是 DMA 写入的新数据。
     */
    DCache_Invalidate_By_Addr(buffer,
                              DPSD_BLOCK_SIZE * sizeof(dpsd_adc_t));

    status = DPSD_Process(buffer, &g_dpsd_result);

    if (status != 0)
    {
        UART1_Printf("DPSD_Process error: %ld\r\n", status);
        return;
    }

    /*
     * 不要每个 8192 点块都打印。
     *
     * Fs = 2.5 MHz:
     * block_time = 8192 / 2500000 = 3.2768 ms
     *
     * 50 块约 163.84 ms 输出一次。
     */
    print_div++;

    if (print_div >= 50U)
    {
        print_div = 0U;

			g_dpsd_result.amplitude_pp = g_dpsd_result.amplitude_pp*1.25f;
//        UART1_Printf(
//            "Vpk=%.6f V, Vpp=%.6f V, Vrms=%.6f V, "
//            "Phase=%.2f deg, Freq=%.3f Hz, DC=%.4f V, "
//            "Valid=%lu, Overrun=%lu\r\n",
//            g_dpsd_result.amplitude_peak,
//            g_dpsd_result.amplitude_pp,
//            g_dpsd_result.amplitude_rms,
//            g_dpsd_result.phase_deg,
//            g_dpsd_result.frequency,
//            g_dpsd_result.dc_offset_v,
//            g_dpsd_result.valid,
//            g_dma_overrun_count);

#if DPSD_ENABLE_PROFILING
        {
            dpsd_profile_t profile;

            DPSD_GetProfile(&profile);

//            UART1_Printf(
//                "DPSD: %lu cycles, %lu us, CORDIC calls=%lu, total=%lu\r\n",
//                profile.process_cycles,
//                profile.process_us,
//                profile.cordic_calls,
//                profile.total_processed);
        }
#endif
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
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_ADC2_Init();
  MX_CORDIC_Init();
  MX_FMAC_Init();
  /* USER CODE BEGIN 2 */
	SRAM_Clock_Enable();    // 如果这个函数确实还需要，放在时钟后
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	DPSD_Init(&hcordic);
	DWT_CycleCounter_Init();
	
	scheduler_Init();
	UART1_Printf("\r\n===== DPSD Start =====\r\n");
  UART1_Printf("Fs = %.1f Hz, f0 = %.1f Hz, N = %lu, block time = %.4f ms\r\n",
           DPSD_SAMPLE_RATE,
           DPSD_TARGET_FREQ,
           (uint32_t)DPSD_BLOCK_SIZE,
           1000.0f * (float)DPSD_BLOCK_SIZE / DPSD_SAMPLE_RATE);
	
    /* 初始化 DPSD；hcordic 为 CubeMX 生成的全局句柄 */
    if (DPSD_Init(&hcordic) != 0)
    {
        UART1_Printf("DPSD_Init failed\r\n");
        Error_Handler();
    }

    /* 可选：设置目标频率 */
    if (DPSD_SetTargetFrequency(10000.0f) != 0)
    {
        Error_Handler();
    }
		
		    /* 先开 ADC DMA，再启动定时器触发 ADC */
    if (HAL_ADC_Start_DMA(&hadc1,
                          (uint32_t *)g_adc_dma_buffer,
                          ADC_DMA_BUFFER_SIZE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_Base_Start(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
/*
         * 前半块：[0 ... 8191]
         */
        if (g_half_buffer_ready != 0U)
        {
            g_half_buffer_ready = 0U;

            DPSD_ProcessOneBlock(&g_adc_dma_buffer[0]);
        }

        /*
         * 后半块：[8192 ... 16383]
         *
         * 注意：
         * 必须使用 DPSD_BLOCK_SIZE。
         *
         * 不可写 ADC_DMA_BUFFER_SIZE，
         * 后者是 16384，会越过数组末尾。
         */
        if (g_full_buffer_ready != 0U)
        {
            g_full_buffer_ready = 0U;

            DPSD_ProcessOneBlock(&g_adc_dma_buffer[DPSD_BLOCK_SIZE]);
        }

        /*
         * 不要在此处再次调用 HAL_ADC_Start_DMA()。
         * Circular DMA 会自动继续采样。
         */

        /* 可放置低优先级任务，但不能 HAL_Delay 很长时间。 */
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
