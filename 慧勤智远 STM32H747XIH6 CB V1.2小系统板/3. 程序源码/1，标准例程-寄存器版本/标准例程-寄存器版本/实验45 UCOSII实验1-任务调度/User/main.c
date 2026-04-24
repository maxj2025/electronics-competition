/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       UCOSII实验1-任务调度 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "uc-os2_demo.h"


int main(void)
{  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();							                /* 初始化LED */   
  
    uc_os2_demo();                          /* 运行uC/OS-II例程 */
}




