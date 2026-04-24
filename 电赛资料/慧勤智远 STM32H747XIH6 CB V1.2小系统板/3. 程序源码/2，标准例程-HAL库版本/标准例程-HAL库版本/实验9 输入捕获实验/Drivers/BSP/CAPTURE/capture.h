/**
 ****************************************************************************************************
 * @file        capture.h
 * @version     V1.0
 * @brief       输入捕获 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __CAPTURE_H
#define __CAPTURE_H

#include "./SYSTEM/sys/sys.h"


/****************************************************************************************************/
/* 定时器 定义 */

/* TIMX PWM输出定义 
 * 这里输出的PWM控制LED0(GREEN)的亮度
 */
#define TIMX_PWM_CHY_GPIO_PORT         GPIOA
#define TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_3
#define TIMX_PWM_CHY_GPIO_AF           GPIO_AF2_TIM5                                /* AF功能选择 */
#define TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */

#define TIMX_PWM                       TIM5                                         /* TIM5 */
#define TIMX_PWM_CHY                   TIM_CHANNEL_4                                /* 通道Y,  1<= Y <=4 */
#define TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)   /* TIM5 时钟使能 */

/* TIMX 输入捕获定义 
 * 这里的输入捕获使用定时器TIM15_CH1,捕获WK_UP按键的输入
 * 特别要注意:默认用的PA2,设置的是下拉输入!如果改其他IO,对应的上下拉方式也得改!
 */
#define TIMX_CAP_CHY_GPIO_PORT         GPIOA
#define TIMX_CAP_CHY_GPIO_PIN          GPIO_PIN_2
#define TIMX_CAP_CHY_GPIO_AF           GPIO_AF4_TIM15                               /* AF功能选择 */
#define TIMX_CAP_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */

#define TIMX_CAP                       TIM15                       
#define TIMX_CAP_IRQn                  TIM15_IRQn
#define TIMX_CAP_IRQHandler            TIM15_IRQHandler
#define TIMX_CAP_CHY                   TIM_CHANNEL_1                                /* 通道Y,  1<= Y <=4 */
#define TIMX_CAP_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM15_CLK_ENABLE(); }while(0)  /* TIM15 时钟使能 */

/****************************************************************************************************/

void timx_pwm_chy_init(uint32_t arr, uint16_t psc);                                 /* 定时器TIMX 通道Y PWM输出初始化函数 */
void timx_cap_chy_init(uint32_t arr, uint16_t psc);                                 /* 定时器TIMX 通道Y 输入捕获初始化函数 */

#endif




