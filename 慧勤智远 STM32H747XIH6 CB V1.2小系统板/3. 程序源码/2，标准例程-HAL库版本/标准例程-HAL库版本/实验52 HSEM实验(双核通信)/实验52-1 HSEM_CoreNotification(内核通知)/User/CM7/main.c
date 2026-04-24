/**
 ****************************************************************************************************
 * @file        Cortex-M7的main.c
 * @version     V1.0
 * @brief       HSEM内核通知 实验
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


#define HSEM_ID_0  0   /* 使用信号量0 */


int main(void)
{
    uint8_t i = 0;
    uint32_t timeout;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();                             /* 初始化LED */
  
    /* 系统初始化结束后CPU1(Cortex-M7)通过HSEM通知的方式唤醒CPU2(Cortex-M4) */
    __HAL_RCC_HSEM_CLK_ENABLE();            /* HSEM时钟使能 */

    HAL_HSEM_FastTake(HSEM_ID_0);           /* 通过1步读方式获取信号量 */ 
  
    HAL_HSEM_Release(HSEM_ID_0, 0);         /* 释放信号量以唤醒CPU2 */
  
    timeout = 0xFFFF;
  
    /* 等待D2域时钟准备就绪，即CPU2从停止模式中唤醒 */
    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && timeout)
    {
        timeout--;       
    }
       
    while (1)
    {
        HAL_HSEM_FastTake(HSEM_ID_0);       /* 快速获取信号量 */   
      
        /* CM7每200ms翻转一次LED0共5次 */
        for (i = 0; i < 5; i++)
        {
            LED0_TOGGLE();                  /* LED0(绿灯)闪烁 */
            delay_ms(200);
        }

        HAL_HSEM_Release(HSEM_ID_0, 0);     /* 释放信号量以通知CPU2 */
    }
}




