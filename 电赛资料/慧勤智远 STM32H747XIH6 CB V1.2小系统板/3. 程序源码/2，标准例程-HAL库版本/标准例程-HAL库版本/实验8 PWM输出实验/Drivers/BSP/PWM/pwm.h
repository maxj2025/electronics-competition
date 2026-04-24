/**
 ****************************************************************************************************
 * @file        pwm.h
 * @version     V1.0
 * @brief       PWM输出 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __PWM_H
#define __PWM_H

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

/****************************************************************************************************/

void timx_pwm_chy_init(uint32_t arr, uint16_t psc);                                 /* 定时器TIMX 通道Y PWM输出初始化函数 */

#endif





