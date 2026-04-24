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


/******************************************************************************************/
/* 定时器 定义 */

/* TIMX PWM输出定义 
 * 这里输出的PWM控制LED0(GREEN)的亮度
 * 注意: 通过修改这些宏定义,可以支持能产生PWM信号的定时器的通道输出PWM
 */
#define TIMX_PWM_CHY_GPIO_PORT         GPIOA
#define TIMX_PWM_CHY_GPIO_PIN          SYS_GPIO_PIN3
#define TIMX_PWM_CHY_GPIO_AF           2                           /* AF功能选择 */
#define TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */

#define TIMX_PWM                       TIM5
#define TIMX_PWM_CHY                   4                           /* 通道Y,  1<= Y <=4 */
#define TIMX_PWM_CHY_CCRX              TIM5->CCR4                  /* 通道Y的输出比较寄存器 */
#define TIMX_PWM_CHY_CLK_ENABLE()      do{ RCC->APB1LENR |= 1 << 3; }while(0)  /* TIM5 时钟使能 */

/******************************************************************************************/

void timx_pwm_chy_init(uint32_t arr, uint16_t psc);     /* 定时器TIMX 通道Y PWM输出初始化函数 */

#endif








