/**
 ****************************************************************************************************
 * @file        exti.h
 * @version     V1.0
 * @brief       外部中断 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __EXTI_H
#define __EXTI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 引脚、中断编号和中断服务函数 定义 */ 

#define KEY0_INT_GPIO_PORT              GPIOA
#define KEY0_INT_GPIO_PIN               SYS_GPIO_PIN8
#define KEY0_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */
#define KEY0_INT_IRQn                   EXTI9_5_IRQn
#define KEY0_INT_IRQHandler             EXTI9_5_IRQHandler

#define WKUP_INT_GPIO_PORT              GPIOA
#define WKUP_INT_GPIO_PIN               SYS_GPIO_PIN2
#define WKUP_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */
#define WKUP_INT_IRQn                   EXTI2_IRQn
#define WKUP_INT_IRQHandler             EXTI2_IRQHandler

/******************************************************************************************/

void extix_init(void);     /* 外部中断初始化 */

#endif





