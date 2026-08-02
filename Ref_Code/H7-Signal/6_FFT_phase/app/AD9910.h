#ifndef __AD9910_H__
#define __AD9910_H__

#include "bspsysteam.h"

#define uchar unsigned char
#define uint  unsigned int	
#define ulong  unsigned long int

 
#define AD9910_PWR(x)       	HAL_GPIO_WritePin(AD9910_PWR_GPIO_Port, AD9910_PWR_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define AD9910_SDIO(x)     		HAL_GPIO_WritePin(AD9910_SDIO_GPIO_Port, AD9910_SDIO_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define DRHOLD(x)   			HAL_GPIO_WritePin(AD9910_DPH_GPIO_Port, AD9910_DPH_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define DROVER(x)      			HAL_GPIO_WritePin(AD9910_DRO_GPIO_Port, AD9910_DRO_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define UP_DAT(x)      			HAL_GPIO_WritePin(AD9910_IOUP_GPIO_Port, AD9910_IOUP_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define PROFILE1(x)      		HAL_GPIO_WritePin(AD9910_PF1_GPIO_Port, AD9910_PF1_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define MAS_REST(x)      		HAL_GPIO_WritePin(AD9910_RST_GPIO_Port, AD9910_RST_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define SCLK(x)    				HAL_GPIO_WritePin(AD9910_SCK_GPIO_Port, AD9910_SCK_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define DRCTL(x)    			HAL_GPIO_WritePin(AD9910_DRC_GPIO_Port, AD9910_DRC_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OSK(x)    				HAL_GPIO_WritePin(AD9910_OSK_GPIO_Port, AD9910_OSK_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define PROFILE0(x)    			HAL_GPIO_WritePin(AD9910_PF0_GPIO_Port, AD9910_PF0_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define PROFILE2(x) 			HAL_GPIO_WritePin(AD9910_PF2_GPIO_Port, AD9910_PF2_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define CS(x)    				HAL_GPIO_WritePin(AD9910_CSB_GPIO_Port, AD9910_CSB_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)


typedef enum {
	TRIG_WAVE = 0,
	SQUARE_WAVE,
	SINC_WAVE,
} AD9910_WAVE_ENUM;


void AD9110_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);										//写频率
void AD9910_RAM_SetFreq(float freq);
void AD9910_RAM_SetWaveAmp(AD9910_WAVE_ENUM wave, float amp);
void AD9910_AmpWrite(uint16_t Amp);									//写幅度
void AD9910_RAM_WAVE_Set(AD9910_WAVE_ENUM wave);		//波形
void AD9910_PhaWrite(float phase);//相位
void AD9910_DRG_FreInit_AutoSet(uint8_t autoSweepEn);//开启AD9910的数字斜坡模式DRG模式使能，使能数字斜坡控制频率，根据参数设置是否自动扫频
void AD9910_RAM_WAVE_Config(AD9910_WAVE_ENUM wave, float freq, float amp);
void AD9910_DRG_FrePara_Set(uint32_t lowFre, uint32_t upFre, uint32_t posStep, uint32_t negStep, uint16_t posRate, uint16_t negRate);
//设置 AD9910 DRG 扫描参数 (频率扫描)
#endif


