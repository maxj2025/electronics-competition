/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       外部中断 实验
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
#include "./BSP/KEY/key.h"
#include "./BSP/EXTI/exti.h"


int main(void)
{  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    extix_init();                           /* 初始化外部中断输入 */
    LED0(0);                                /* 点亮LED0(绿灯) */ 
  
    while (1)
    {
        printf("OK\r\n");
        delay_ms(1000);
    }
}




