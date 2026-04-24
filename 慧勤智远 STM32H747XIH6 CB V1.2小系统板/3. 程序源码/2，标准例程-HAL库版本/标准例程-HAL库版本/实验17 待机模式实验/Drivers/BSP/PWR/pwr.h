/**
 ****************************************************************************************************
 * @file        pwr.h
 * @version     V1.0
 * @brief       低功耗模式 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __PWR_H
#define __PWR_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* PWR WKUP 按键 引脚和中断 定义 
 * 我们通过WK_UP按键唤醒 MCU, 因此必须定义这个按键及其对应的中断服务函数 
 */

#define PWR_WKUP_GPIO_PORT              GPIOA
#define PWR_WKUP_GPIO_PIN               GPIO_PIN_2
#define PWR_WKUP_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */
  
#define PWR_WKUP_INT_IRQn               EXTI2_IRQn
#define PWR_WKUP_INT_IRQHandler         EXTI2_IRQHandler

/******************************************************************************************/

void pwr_wkup_key_init(void);           /* 唤醒按键初始化 */
void pwr_enter_standby(void);           /* 进入待机模式 */
    
#endif





