/**
 ****************************************************************************************************
 * @file        Cortex-M7的main.c
 * @version     V1.0
 * @brief       HSEM内核资源共享 实验
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
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/LCD/lcd.h"


#define HSEM_ID_0  0   /* 使用信号量0 */


int main(void)
{
    uint32_t timeout;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
  
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
    
    usart_init(115200);                     /* 初始化USART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
 
    /* CM7完成它的任务后通知CM4 */
    HAL_HSEM_FastTake(HSEM_ID_0);           /* 快速获取信号量 */   
    HAL_HSEM_Release(HSEM_ID_0, 0);         /* 释放信号量以通知CPU2 */

    /* CM7等待CM4完成它的任务并锁定硬件信号量0 */
    while (HAL_HSEM_IsSemTaken(HSEM_ID_0) == 0);
  
    while (1)
    {       
        /* CM7尝试获取信号量0 */
        if (HAL_HSEM_FastTake(HSEM_ID_0) == HAL_OK)
        {
            ltdc_clear(RED);
            lcd_show_string(10, 40, 230, 32, 32, "STM32H747", RED);
            lcd_show_string(10, 80, 230, 24, 24, "Cortex-M7", RED); 
            LED0(0);                          /* LED0(绿灯) 亮 */
            delay_ms(500);
            LED0(1);                          /* LED0(绿灯) 灭 */
            delay_ms(500);          
            
            HAL_HSEM_Release(HSEM_ID_0, 0);   /* 释放信号量 */
           
            /* 在释放HSEM到下次获取之间加延时，以便其它任务获取 */          
            delay_ms(10);     
        }
    }
}




