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

/* TIMX 输入捕获定义 
 * 这里的输入捕获使用定时器TIM15_CH1,捕获WK_UP按键的输入
 * 注意: 通过修改这些宏定义,可以支持有输入捕获功能的定时器通道做输入捕获
 *       特别要注意:默认用的PA2,设置的是下拉输入!如果改其他IO,对应的上下拉方式也得改!
 */
#define TIMX_CAP_CHY_GPIO_PORT         GPIOA
#define TIMX_CAP_CHY_GPIO_PIN          SYS_GPIO_PIN2
#define TIMX_CAP_CHY_GPIO_AF           4                           /* AF功能选择 */
#define TIMX_CAP_CHY_GPIO_CLK_ENABLE() do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */

#define TIMX_CAP                       TIM15                       
#define TIMX_CAP_IRQn                  TIM15_IRQn
#define TIMX_CAP_IRQHandler            TIM15_IRQHandler
#define TIMX_CAP_CHY                   1                           /* 通道Y,  1<= Y <=4 */
#define TIMX_CAP_CHY_CCRX              TIM15->CCR1                 /* 通道Y的输出比较寄存器 */
#define TIMX_CAP_CHY_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 16; }while(0)  /* TIM15 时钟使能 */

/******************************************************************************************/

void timx_pwm_chy_init(uint32_t arr, uint16_t psc);     /* 定时器TIMX 通道Y PWM输出初始化函数 */
void timx_cap_chy_init(uint32_t arr, uint16_t psc);     /* 定时器TIMX 通道Y 输入捕获初始化函数 */

#endif





