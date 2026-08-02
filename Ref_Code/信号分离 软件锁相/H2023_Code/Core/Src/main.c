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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
//FFT计算所需头文件
#include "arm_math.h"
#include "arm_const_structs.h"
#include "ad9833.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define M_PI 3.1415926
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



#define FFT_LENGTH		512 		//FFT长度，默认是1024点FFT

uint16_t adc_buffer1[FFT_LENGTH];
uint16_t adc_buffer2[FFT_LENGTH];
uint16_t adc_buffer3[FFT_LENGTH];
float inputfft_1[FFT_LENGTH*2],inputfft_2[FFT_LENGTH*2],inputfft_3[FFT_LENGTH*2]; //傅里叶计算前的值（实部ad值，虚部为0）
float outputfft_1[FFT_LENGTH],outputfft_2[FFT_LENGTH],outputfft_3[FFT_LENGTH];    //傅里叶计算后的值
double fft_phase_out1[FFT_LENGTH],fft_phase_out2[FFT_LENGTH],fft_phase_out3[FFT_LENGTH];
float Fmax;                          //最大值
float Fmax2;                         //次最大值

//默认分离出来的最大频率信号是A信号
//ADC1采集的信号进行FFT，俩个最大幅值对应对索引计算出频率值

unsigned int Amax_pos;									//最大值位置
unsigned int Bmax_pos;								  //次最大值位置

//DDS输出信号A的反馈
unsigned int Feedbackfreq_Amax_pos;									//最大值位置
//DDS输出信号B的反馈
unsigned int Feedbackfreq_Bmax_pos;									//最大值位置
float phaA_err;

float phaseA;          // 相位 (度)
float FeedbackPhase_A; // 相位 (度)
float phaseA_err;       // 相位差 (度)
float phaseB;          // 相位 (度)
float FeedbackPhase_B; // 相位 (度)
float phaseB_err;      // 相位差 (度)
float freq_A;      // 频率 (Hz)
float Feedbackfreq_A;      // 反馈信号A频率 (Hz)
float phase_A;     // 相位 (度)
float freq_B;      // 频率 (Hz)
float Feedbackfreq_B;      // 反馈信号A频率 (Hz)
int Phase=0;


unsigned char flag=0;
uint8_t adc_dma_finish;//所有adc转换完成标志位
float CH1_Phase = 0;
float CH2_Phase = 180;	// 范围0~360

// PID控制器结构体
typedef struct {
    float Kp, Ki, Kd;     // PID参数
    float last_error;      // 上一次误差
    float prev_error;      // 上上次误差
    float max_output;      // 最大输出限制
} SimplePID;

// 相位锁定控制器
typedef struct {
    SimplePID pid;         // PID控制器
    float target_phase;    // 目标相位差(通常为0)
    float current_freq;    // 当前频率
    float min_step;        // 最小调整步长
} PhaseLocker;

// 初始化PID控制器
void PID_Init(SimplePID* pid, float Kp, float Ki, float Kd, float max_out) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->last_error = 0;
    pid->prev_error = 0;
    pid->max_output = max_out;
}

// 计算PID输出
float PID_Calculate(SimplePID* pid, float error) {
    float output = pid->Kp * (error - pid->last_error) 
                 + pid->Ki * error
                 + pid->Kd * (error - 2*pid->last_error + pid->prev_error);
    
    // 更新误差记录
    pid->prev_error = pid->last_error;
    pid->last_error = error;
    
    // 限制输出范围
    if(output > pid->max_output) output = pid->max_output;
    else if(output < -pid->max_output) output = -pid->max_output;
    
    return output;
}

// 初始化相位锁定器
void PhaseLock_Init(PhaseLocker* locker, float Kp, float Ki, float Kd, 
                   float target_phase, float min_step, float max_adjust) {
    PID_Init(&locker->pid, Kp, Ki, Kd, max_adjust);
    locker->target_phase = target_phase;
    locker->min_step = min_step;
    locker->current_freq = 0;
}

// 计算标准化相位差 (-180°到180°)
float NormalizePhase(float phase)
{

    return phase;
}

// 相位锁定处理
void PhaseLock_Process(PhaseLocker* locker, float measured_phase, float current_freq,int ch,uint16_t waveform) {
    // 计算标准化相位差
    float phase_diff = NormalizePhase(measured_phase - locker->target_phase);
  
    // 计算频率调整量
    float adjust = PID_Calculate(&locker->pid, phase_diff);
    
    // 应用最小步长
    if(fabs(adjust) < locker->min_step) {
        adjust = (adjust > 0) ? locker->min_step : -locker->min_step;
    }
    
    // 更新频率
    locker->current_freq = current_freq + adjust;
    
		if(ch==AD9833_CH1)
		{
       // 设置新频率 (AD9833频率寄存器值 = 频率 * 10.73742)
       AD9833_Set_Frequency(AD9833_CH1, waveform, 
                        (uint32_t)(locker->current_freq * 10.73742));
		}
		else if(ch==AD9833_CH2)
		{
		   // 设置新频率 (AD9833频率寄存器值 = 频率 * 10.73742)
      AD9833_Set_Frequency(AD9833_CH2, waveform, 
      (uint32_t)(locker->current_freq * 10.73742));
		}
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)//ad转换完成
{
	  static uint8_t count = 0;
    
    if (hadc->Instance == ADC1) {
        count++;
    } else if (hadc->Instance == ADC2) {
        count++;
    } else if (hadc->Instance == ADC3) {
        count++;
    }
    if (count == 3) {
			  HAL_TIM_Base_Stop(&htim3);
        count = 0;
				adc_dma_finish = 1;  // 通知主循环处理数据
			  
    }//由于要在明显的看到3个信号的区别，所以等3个ad一起转换完成后再一起计算且打印出串口
}

//实时改变采样率函数
void TIM_Config_Sampling(uint32_t signal_freq, uint16_t samples_per_cycle) {
    uint32_t f_timer = 64000000;  // 64 MHz
    uint32_t total_divider = f_timer / (samples_per_cycle * signal_freq);
    
    // 优先最小PSC，最大化ARR（提高精度）
    uint16_t psc = 0;  // 默认不分频
    uint32_t arr = total_divider - 1;
    
    // 如果ARR超过32位限制（实际不可能，因为total_divider最大=64M/1=64M，ARR=64M-1）
    if (arr > 0xFFFFFFFF) {
        // 需要增加PSC（分频）
        psc = (uint16_t)(sqrt(total_divider)) - 1;
        arr = (total_divider / (psc + 1)) - 1;
    }
    
    // 配置定时器
		htim3.Instance->PSC=psc;
    htim3.Instance->ARR = arr;
    HAL_TIM_Base_Start(&htim3);
}
//计算FFT结果每个分量的相位角
void compute_fft_phase(float *fft_output, uint32_t fft_size, double *phase_out) {
    for (uint32_t k = 0; k < fft_size; k++) 
	  {
        float32_t real = fft_output[2*k];     // 实部（CMSIS-DSP中交替存储实/虚部）
        float32_t imag = fft_output[2*k+1];   // 虚部
        
        // 计算相位（弧度）
        phase_out[k] = (double)atan2f(imag, real);

        // 可选：转换为角度
         phase_out[k] *= 180.0f / M_PI;
    }
}
PhaseLocker lockerA;
PhaseLocker lockerB;
uint32_t peak1_idx;
uint32_t peak2_idx;
uint32_t length=FFT_LENGTH;

// 输入: fft_outputbuf（幅度谱数组）, length（数组长度，如4096）
// 输出: peak1_idx（第一个峰值的索引）, peak2_idx（第二个峰值的索引）
void find_peak_indices(float* magnitude, uint32_t length, uint32_t* peak1_idx, uint32_t* peak2_idx) {
    *peak1_idx = 0;
    *peak2_idx = 0;
    float peak1_val = 0;
    float peak2_val = 0;

    // 只检查前一半频点（实数信号对称性）
    for (uint32_t i = 1; i < length/2 - 1; i++) {
        // 检测局部峰值（避免噪声和直流分量）
        if (magnitude[i] > magnitude[i-1] && magnitude[i] > magnitude[i+1]) {
            if (magnitude[i] > peak1_val) {
                // 更新峰值2为旧的峰值1
                peak2_val = peak1_val;
                *peak2_idx = *peak1_idx;
                // 更新峰值1
                peak1_val = magnitude[i];
                *peak1_idx = i;
            } else if (magnitude[i] > peak2_val) {
                // 更新峰值2
                peak2_val = magnitude[i];
                *peak2_idx = i;
            }
        }
    }
}
uint16_t waveform_A;
uint16_t waveform_B;
uint8_t uartdata[50];
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
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */
	//开启定时器
	AD9833_Init();
  HAL_TIM_Base_Start(&htim3);
	//开始进行ad转换
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc_buffer1,FFT_LENGTH);
	HAL_ADC_Start_DMA(&hadc2,(uint32_t *)adc_buffer2,FFT_LENGTH);
	HAL_ADC_Start_DMA(&hadc3,(uint32_t *)adc_buffer3,FFT_LENGTH);
 
  PhaseLock_Init(&lockerA, 0.05f, 0.1f, 0.1f,  0.0f,   -0.8f,    0.8f);//如果要更改 A信号和分离出来A信号的相位偏移，可改下面初始化函数的“偏移相位”
	//             pid结构体   Kp     Ki    Kd   偏移相位 最小步进   最大调整值
	PhaseLock_Init(&lockerB, 0.05f, 0.1f, 0.1f, 0.0f, -0.8f, 0.8f);
	HAL_Delay(100);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

							
		if(adc_dma_finish)//判断完成标志位
		{
		  adc_dma_finish=0;//标志位清0
			for(int i=0;i<FFT_LENGTH;i++)
		  {
				inputfft_1[i*2]=(double)adc_buffer1[i]*3.3/4095;
				inputfft_1[i*2+1]=0;
				inputfft_2[i*2]=(double)adc_buffer2[i]*3.3/4095;
				inputfft_2[i*2+1]=0;
				inputfft_3[i*2]=(double)adc_buffer3[i]*3.3/4095;
				inputfft_3[i*2+1]=0;
		  }
			 //对3路adc采集的数据都进行FFT计算
			 //合成信号A+B
		 	 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_1,0,1);
		   arm_cmplx_mag_f32(inputfft_1,outputfft_1,FFT_LENGTH);    //把运算结果复数求模得幅值 
			 //回馈信号A`
			 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_2,0,1);
			 arm_cmplx_mag_f32(inputfft_2,outputfft_2,FFT_LENGTH);    //把运算结果复数求模得幅值 
			 //回馈信号B`
			 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_3,0,1);
			 arm_cmplx_mag_f32(inputfft_3,outputfft_3,FFT_LENGTH);    //把运算结果复数求模得幅值 
		   outputfft_1[0]=0;//去直流，防止最大值寻找错误
			 outputfft_2[0]=0;
			 outputfft_3[0]=0;
			 find_peak_indices(outputfft_1,length,&peak1_idx,&peak2_idx);
			 
        if(outputfft_1[peak1_idx]>110)
        {
				  waveform_A=AD9833_Out_Sinus; 
				}
				else if(outputfft_1[peak1_idx]<=110)
				{
				  waveform_A=AD9833_Out_Triangle;
				}
        if(outputfft_1[peak2_idx]>110)
        {
				  waveform_B=AD9833_Out_Sinus;
				}
				else if(outputfft_1[peak2_idx]<=110)
				{
				  waveform_B=AD9833_Out_Triangle;
				}
//       arm_max_f32(outputfft_1, FFT_LENGTH/2, &Fmax, &Amax_pos);//使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0
//			 outputfft_1[Amax_pos]=0;//清除最大值
//			 
//			 arm_max_f32(outputfft_1, FFT_LENGTH/2, &Fmax2, &Bmax_pos); //找次最大值
//			 outputfft_1[Amax_pos]=Fmax;
			 
			 //计算出采集到的加法信号分离出来的AB信号的频率
			 freq_A = peak1_idx*(float)(512000/FFT_LENGTH);
			 freq_B = peak2_idx*(float)(512000/FFT_LENGTH);
      
			 //FFT计算反馈信号A的频率
			 arm_max_f32(outputfft_2, FFT_LENGTH/2, &Fmax, &Feedbackfreq_Amax_pos);     //使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0
			 Feedbackfreq_A=Feedbackfreq_Amax_pos*(float)(512000/FFT_LENGTH);
			 
			 arm_max_f32(outputfft_3, FFT_LENGTH/2, &Fmax, &Feedbackfreq_Bmax_pos);     //使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0
			 Feedbackfreq_B=Feedbackfreq_Bmax_pos*(float)(512000/FFT_LENGTH);	 
			 if(Feedbackfreq_B==freq_B &&Feedbackfreq_A==freq_A  )//判断计算频率相同进入锁相
			 {
			   flag=1;
			 }
			 else
			 {
			    //DDS输出一次AB信号，方便等下反馈计算
			    AD9833_Set_Frequency(AD9833_CH1,  waveform_A,  (uint32_t)(freq_A * 10.73742));
				  AD9833_Set_Frequency(AD9833_CH2,  waveform_B,  (uint32_t)(freq_B * 10.73742));
          HAL_Delay(5);//延时等待一下
			 }							

			  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer1, FFT_LENGTH);
				HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buffer2, FFT_LENGTH);
			  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_buffer3, FFT_LENGTH);
				HAL_TIM_Base_Start(&htim3);

			  while(flag)
			 {

				 if(adc_dma_finish)//判断标志位
				 {
						adc_dma_finish=0;//标志位清0
						for(int i=0;i<FFT_LENGTH;i++)
						{
							inputfft_1[i*2]=(double)adc_buffer1[i]*3.3/4095;
							inputfft_1[i*2+1]=0;
							inputfft_2[i*2]=(double)adc_buffer2[i]*3.3/4095;
							inputfft_2[i*2+1]=0;
							inputfft_3[i*2]=(double)adc_buffer3[i]*3.3/4095;
							inputfft_3[i*2+1]=0;
						}
				    //信号A+B
					 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_1,0,1);
					 arm_cmplx_mag_f32(inputfft_1,outputfft_1,FFT_LENGTH);    //把运算结果复数求模得幅值 
					 //回馈信号A`
					 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_2,0,1);
					 arm_cmplx_mag_f32(inputfft_2,outputfft_2,FFT_LENGTH);    //把运算结果复数求模得幅值 
					 //回馈信号B
					 arm_cfft_f32(&arm_cfft_sR_f32_len512,inputfft_3,0,1);
					 arm_cmplx_mag_f32(inputfft_3,outputfft_3,FFT_LENGTH);    //把运算结果复数求模得幅值 
						
											 
					 outputfft_1[0]=0;//去直流，防止最大值寻找错误
					 outputfft_2[0]=0;
					 outputfft_3[0]=0;
           find_peak_indices(outputfft_1,length,&peak1_idx,&peak2_idx);
			 
						if(outputfft_1[peak1_idx]>110)
						{
							  waveform_A=AD9833_Out_Sinus; 
						}
						else if(outputfft_1[peak1_idx]<=110)
						{
							  waveform_A=AD9833_Out_Triangle;
						}
						if(outputfft_1[peak2_idx]>110)
						{
							  waveform_B=AD9833_Out_Sinus;
						}
						else if(outputfft_1[peak2_idx]<=110)
						{
							  waveform_B=AD9833_Out_Triangle;
						}
//					 arm_max_f32(outputfft_1, FFT_LENGTH/2, &Fmax, &Amax_pos);     //使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0
//           outputfft_1[Amax_pos]=0;
//					 arm_max_f32(outputfft_1, FFT_LENGTH/2, &Fmax2, &Bmax_pos); 

					 freq_A = peak1_idx*(float)(512000/FFT_LENGTH);
					 freq_B = peak2_idx*(float)(512000/FFT_LENGTH);
					 
           
					 phaseA = atan2(inputfft_1[2*peak1_idx+1],inputfft_1[2*peak1_idx])* 180 / PI;
					 phaseB = atan2(inputfft_1[2*peak2_idx+1], inputfft_1[2*peak2_idx])* 180 / PI;
					 
             
					 arm_max_f32(outputfft_2, FFT_LENGTH/2, &Fmax, &Feedbackfreq_Amax_pos);     //使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0					
			     Feedbackfreq_A=Feedbackfreq_Amax_pos*(float)(512000/FFT_LENGTH);	 
					 FeedbackPhase_A = atan2(inputfft_2[2*Feedbackfreq_Amax_pos+1],inputfft_2[2*Feedbackfreq_Amax_pos])* 180 / PI;

					 arm_max_f32(outputfft_3, FFT_LENGTH/2, &Fmax, &Feedbackfreq_Bmax_pos);     //使用Length会让频率点在后半部分，导致频率计算错误，导致ARR->0					
			     Feedbackfreq_B=Feedbackfreq_Bmax_pos*(float)(512000/FFT_LENGTH);	 
					 FeedbackPhase_B = atan2(inputfft_3[2*Feedbackfreq_Bmax_pos+1],inputfft_3[2*Feedbackfreq_Bmax_pos])* 180 / PI;
					 phaseA_err=FeedbackPhase_A-phaseA;
					 phaseA_err=-phaseA_err;//
					 if(phaseA_err<-180)phaseA_err+=360;
					 if(phaseA_err>180)phaseA_err-=360;
					 
					 phaseB_err=FeedbackPhase_B-phaseB;
					 phaseB_err=-phaseB_err;//
					 if(phaseB_err<-180)phaseB_err+=360;
					 if(phaseB_err>180)phaseB_err-=360;
           
					 //printf("%f,%f\n",phaseB_err,0);//打印调试
		
           //pid锁相
					 PhaseLock_Process(&lockerA,phaseA_err, freq_A,AD9833_CH1,waveform_A);
					 PhaseLock_Process(&lockerB,phaseB_err, freq_B,AD9833_CH2,waveform_B);							
					  if(Feedbackfreq_A!=freq_A || Feedbackfreq_B!=freq_B)
					 {
						 flag=0; 
					 }
					 HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer1, FFT_LENGTH);
				   HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buffer2, FFT_LENGTH);
			     HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_buffer3, FFT_LENGTH);
				   HAL_TIM_Base_Start(&htim3);
					 HAL_Delay(1);//1ms延时
					 
			   }						
			
			 }	
				HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer1, FFT_LENGTH);
				HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buffer2, FFT_LENGTH);
				HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_buffer3, FFT_LENGTH);
				HAL_TIM_Base_Start(&htim3);

//        }
		}
			
	
    /* USER CODE END WHILE */

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
  RCC_OscInitStruct.PLL.PLLN = 128;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//printf 重定向
int fputc(int ch,FILE *f)
{
  HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,10);
	return ch;
}

/* USER CODE END 4 */

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
