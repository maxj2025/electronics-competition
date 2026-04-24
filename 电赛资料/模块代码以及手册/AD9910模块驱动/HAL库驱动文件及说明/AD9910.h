#ifndef __AD9910_H__
#define __AD9910_H__

#include "main.h"
#include <stdint.h>

/* 类型定义简化 */
typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint16_t u16;
typedef unsigned char      uchar;
typedef unsigned int       uint;
typedef unsigned long int  ulong;

/**
 * @brief AD9910 RAM 波形枚举
 */
typedef enum
{
    TRIG_WAVE = 0,   // 三角波
    SQUARE_WAVE,     // 方波
    SINC_WAVE,       // Sinc 波
} AD9910_WAVE_ENUM;

/* ---------------- AD9910 控制引脚重新映射 ---------------- */
/* 映射关系完全对齐 gpio.c 中的 MX_GPIO_Init 配置 */

// 电源与复位相关 (GPIOE)
#define AD9910_PWR_GPIO_Port      GPIOE
#define AD9910_PWR_Pin            GPIO_PIN_0// AD9910_PWR_Pin
#define AD9910_MAS_REST_GPIO_Port GPIOE
#define AD9910_MAS_REST_Pin       AD9910_RST_Pin

// SPI 通信相关 (GPIOE)
#define AD9910_SCLK_GPIO_Port     GPIOE
#define AD9910_SCLK_Pin           AD9910_CLK_Pin
#define AD9910_CS_GPIO_Port       GPIOE
#define AD9910_CS_Pin             AD9910_CSB_Pin
#define AD9910_SDIO_GPIO_Port     GPIOE
// #define AD9910_SDIO_Pin           AD9910_SDIO_Pin

// 更新与控制 (GPIOD)
#define AD9910_UP_DAT_GPIO_Port   GPIOD
#define AD9910_UP_DAT_Pin         AD9910_IOUP_Pin
#define AD9910_OSK_GPIO_Port      GPIOD
#define AD9910_OSK_Pin            AD9910_OSK_Pin

// 数字斜率/扫描控制 (Digital Ramp Generator - DRG)
#define AD9910_DRHOLD_GPIO_Port   GPIOE
#define AD9910_DRHOLD_Pin         AD9910_DPH_Pin
#define AD9910_DROVER_GPIO_Port   GPIOC          // 根据 gpio.c，DRO 属于 GPIOC
#define AD9910_DROVER_Pin         AD9910_DRO_Pin
#define AD9910_DRCTL_GPIO_Port    GPIOD          // 根据 gpio.c，DRC 属于 GPIOD
#define AD9910_DRCTL_Pin          AD9910_DRC_Pin

// Profile 切换引脚 (GPIOB)
#define AD9910_PROFILE0_GPIO_Port GPIOB
#define AD9910_PROFILE0_Pin       AD9910_PF0_Pin
#define AD9910_PROFILE1_GPIO_Port GPIOB
#define AD9910_PROFILE1_Pin       AD9910_PF1_Pin
#define AD9910_PROFILE2_GPIO_Port GPIOB
#define AD9910_PROFILE2_Pin       AD9910_PF2_Pin

/* ---------------- GPIO 电平控制宏定义 ---------------- */

#define AD9910_PWR_0      HAL_GPIO_WritePin(AD9910_PWR_GPIO_Port, AD9910_PWR_Pin, GPIO_PIN_RESET)
#define AD9910_PWR_1      HAL_GPIO_WritePin(AD9910_PWR_GPIO_Port, AD9910_PWR_Pin, GPIO_PIN_SET)

#define AD9910_SDIO_0     HAL_GPIO_WritePin(AD9910_SDIO_GPIO_Port, AD9910_SDIO_Pin, GPIO_PIN_RESET)
#define AD9910_SDIO_1     HAL_GPIO_WritePin(AD9910_SDIO_GPIO_Port, AD9910_SDIO_Pin, GPIO_PIN_SET)

#define AD9910_SCLK_0     HAL_GPIO_WritePin(AD9910_SCLK_GPIO_Port, AD9910_SCLK_Pin, GPIO_PIN_RESET)
#define AD9910_SCLK_1     HAL_GPIO_WritePin(AD9910_SCLK_GPIO_Port, AD9910_SCLK_Pin, GPIO_PIN_SET)

#define AD9910_CS_0       HAL_GPIO_WritePin(AD9910_CS_GPIO_Port, AD9910_CS_Pin, GPIO_PIN_RESET)
#define AD9910_CS_1       HAL_GPIO_WritePin(AD9910_CS_GPIO_Port, AD9910_CS_Pin, GPIO_PIN_SET)

#define AD9910_MAS_REST_0 HAL_GPIO_WritePin(AD9910_MAS_REST_GPIO_Port, AD9910_MAS_REST_Pin, GPIO_PIN_RESET)
#define AD9910_MAS_REST_1 HAL_GPIO_WritePin(AD9910_MAS_REST_GPIO_Port, AD9910_MAS_REST_Pin, GPIO_PIN_SET)

#define AD9910_UP_DAT_0   HAL_GPIO_WritePin(AD9910_UP_DAT_GPIO_Port, AD9910_UP_DAT_Pin, GPIO_PIN_RESET)
#define AD9910_UP_DAT_1   HAL_GPIO_WritePin(AD9910_UP_DAT_GPIO_Port, AD9910_UP_DAT_Pin, GPIO_PIN_SET)

#define AD9910_DRCTL_0    HAL_GPIO_WritePin(AD9910_DRCTL_GPIO_Port, AD9910_DRCTL_Pin, GPIO_PIN_RESET)
#define AD9910_DRCTL_1    HAL_GPIO_WritePin(AD9910_DRCTL_GPIO_Port, AD9910_DRCTL_Pin, GPIO_PIN_SET)

#define AD9910_DPH_0    HAL_GPIO_WritePin(AD9910_DRHOLD_GPIO_Port, AD9910_DRHOLD_Pin, GPIO_PIN_RESET)
#define AD9910_DPH_1    HAL_GPIO_WritePin(AD9910_DRHOLD_GPIO_Port, AD9910_DRHOLD_Pin, GPIO_PIN_SET)

#define AD9910_PROFILE0_0 HAL_GPIO_WritePin(AD9910_PROFILE0_GPIO_Port, AD9910_PROFILE0_Pin, GPIO_PIN_RESET)
#define AD9910_PROFILE0_1 HAL_GPIO_WritePin(AD9910_PROFILE0_GPIO_Port, AD9910_PROFILE0_Pin, GPIO_PIN_SET)
#define AD9910_PROFILE1_0 HAL_GPIO_WritePin(AD9910_PROFILE1_GPIO_Port, AD9910_PROFILE1_Pin, GPIO_PIN_RESET)
#define AD9910_PROFILE1_1 HAL_GPIO_WritePin(AD9910_PROFILE1_GPIO_Port, AD9910_PROFILE1_Pin, GPIO_PIN_SET)
#define AD9910_PROFILE2_0 HAL_GPIO_WritePin(AD9910_PROFILE2_GPIO_Port, AD9910_PROFILE2_Pin, GPIO_PIN_RESET)
#define AD9910_PROFILE2_1 HAL_GPIO_WritePin(AD9910_PROFILE2_GPIO_Port, AD9910_PROFILE2_Pin, GPIO_PIN_SET)

/* ---------------- 函数声明 ---------------- */

void Init_AD9910(void);                        // AD9910 寄存器初始化配置
void AD9910_FreWrite(ulong Freq);              // 写入单点频率 (FTW)
void AD9910_AmpWrite(uint16_t Amp);            // 写入单点振幅 (ASF)
void AD9910_RAM_WAVE_Set(AD9910_WAVE_ENUM wave); // 配置并启动 RAM 波形输出

// DRG（数字扫描发生器）相关
void AD9910_DRG_AMP_Init(void);
void AD9910_DRG_FreInit_AutoSet(FunctionalState autoSweepEn);
void AD9910_DRG_FrePara_Set(u32 lowFre, u32 upFre, u32 posStep, u32 negStep, u16 posRate, u16 negRate);

#endif /* __AD9910_H__ */