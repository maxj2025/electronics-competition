/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       定时器中断 实验
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
#include "./BSP/TIMER/timer.h"


int main(void)
{
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    led_init();                             /* 初始化LED */
    timx_int_init(5000 - 1, 24000 - 1);     /* 240 000 000 / 24000 = 10KHz 10KHz的计数频率，计数5K次为500ms */
  
    while (1)
    {
        printf("OK\r\n");
        delay_ms(1000);
    }
}




