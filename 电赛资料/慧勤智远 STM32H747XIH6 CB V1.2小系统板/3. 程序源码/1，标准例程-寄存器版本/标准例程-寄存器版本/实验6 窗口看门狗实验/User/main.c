/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       窗口看门狗 实验
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
#include "./BSP/WWDG/wwdg.h"


int main(void)
{  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();							                /* 初始化LED */   
    LED0(0);                                /* 点亮LED0(绿灯) */ 
    delay_ms(300);                  	      /* 延时300ms再初始化看门狗,LED0的变化"可见" */
    wwdg_init(0X7F, 0X5F, 4);               /* 设置计数器值为7f,窗口值为5f,分频系数为16 */
  
    while (1)
    {
        LED0(1);                            /* 关闭LED0(绿灯) */
    }
}




