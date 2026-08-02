/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"

#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fft.h"
#include "Lockin_Amplifier.h"
#include "OLED.h"
#include "Delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_SAMPLE_RATE 80000.0f  // 采样 80kHz
#define LPF_FACTOR 0.05f           //滤波器系数
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
DLIA_Config_t DLIA_Cfg; //锁相放大器配置结构体

//通道1
DLIA_State_t  State_Ch1; //通道一状态结构体
DLIA_Output_t Out_Ch1;   //通道一输出结构体

//通道2
DLIA_State_t  State_Ch2; //通道二状态结构体
DLIA_Output_t Out_Ch2;   //通道二输出结构体

uint16_t cnt;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM2_Init(41,24);
  /* USER CODE BEGIN 2 */
  OLED_Init();
	if (HAL_ADCEx_MultiModeStart_DMA(&hadc1,(uint32_t *)AdcValue, 1024) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ADC_Start(&hadc1) != HAL_OK) 
  {
    Error_Handler();
  }
  if (HAL_ADC_Start(&hadc2) != HAL_OK) 
  {
    Error_Handler();
  }

  /* USER CODE END 2 */
  OLED_ShowString(2,1,"Amp1:");
  OLED_ShowString(3,1,"Amp2:");
  OLED_ShowString(4,1,"PD:");

  DLIA_Init_LUT();// 生成查表

  //初始化锁相放大器参数
  DLIA_Init(&DLIA_Cfg, &State_Ch1, 10000, ADC_SAMPLE_RATE, LPF_FACTOR);
  DLIA_Init(&DLIA_Cfg, &State_Ch2, 10000, ADC_SAMPLE_RATE, LPF_FACTOR);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		
    if(data_ready > 0) // 数据全部就绪
    {
			if(data_ready == 1 || data_ready == 3)
			{
				for (int i = 0; i < 512; i++)
				{
					// 复制到处理缓冲区的前半部分
					Adc1Value[i] = (uint16_t)(AdcValue[i] & 0xFFFF);
					Adc2Value[i] = (uint16_t)(AdcValue[i] >> 16);
				}
			}
			if(data_ready == 2)
			{
				for (int i = 0; i < 512; i++)
				{
					// 复制到处理缓冲区的后半部分
					Adc1Value[i + 512] = (uint16_t)(AdcValue[i + 512] & 0xFFFF);
					Adc2Value[i + 512] = (uint16_t)(AdcValue[i + 512] >> 16);
				}
			}
			
			
      State_Ch2.PhaseAccumulator = State_Ch1.PhaseAccumulator; // 两个通道的相位累加器同步
      DLIA_Process(&DLIA_Cfg, &State_Ch1, &Out_Ch1, Adc1Value, DLIA_BLOCK_SIZE);
      DLIA_Process(&DLIA_Cfg, &State_Ch2, &Out_Ch2, Adc2Value, DLIA_BLOCK_SIZE);

      float phase_diff = Out_Ch1.Output_PhaseDeg - Out_Ch2.Output_PhaseDeg;

      if (phase_diff < -180.0f) phase_diff += 360.0f;
      if (phase_diff > 180.0f) phase_diff -= 360.0f;

			float PH1 = Out_Ch1.Output_PhaseDeg;
			if(cnt == 0 || cnt == 20)
			{
				OLED_ShowFloat(2,6, Out_Ch1.Output_Amp * 1.106, 3); //1.106校准补偿

				OLED_ShowFloat(3,6, Out_Ch2.Output_Amp * 1.106, 3);

				OLED_ShowFloat(4,5, phase_diff, 3);
				cnt =1;
			}

      cnt++;
//			OLED_ShowNum(4,5,Adc1Value[1],4);
			data_ready = 0;
    }
    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
