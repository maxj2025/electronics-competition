/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       跑马灯 实验
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


int main(void)
{  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();							                /* 初始化LED */   
  
    while (1)
    {
        LED0(0);                            /* LED0(GREEN) 亮 */
        LED1(1);                            /* LED1(BLUE)  灭 */ 
        delay_ms(500);
        LED0(1);                            /* LED0(GREEN) 灭 */
        LED1(0);                            /* LED1(BLUE)  亮 */ 
        delay_ms(500);
    }
}




