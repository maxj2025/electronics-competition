/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       独立看门狗 实验
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
#include "./BSP/IWDG/iwdg.h"


int main(void)
{
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();                             /* 初始化LED */
    key_init();                             /* 初始化按键 */
    delay_ms(100);                          /* 延时100ms再初始化看门狗,LED0的变化"可见" */
    iwdg_init(IWDG_PRESCALER_64, 500);      /* 预分频数为64,重载值为500,溢出时间约为1s */
    LED0(0);                                /* 点亮LED0(绿灯) */ 
  
    while (1)
    {
        if (key_scan(1) == WKUP_PRES)       /* 如果WK_UP按下，则喂狗，支持连按 */
        {
            iwdg_feed();                    /* 喂狗 */
        }

        delay_ms(10);
    }
}




