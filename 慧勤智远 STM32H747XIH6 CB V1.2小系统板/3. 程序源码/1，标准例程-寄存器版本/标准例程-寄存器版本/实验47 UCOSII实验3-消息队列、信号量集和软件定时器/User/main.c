/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       UCOSII实验3-消息队列、信号量集和软件定时器 实验
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
#include "./BSP/KEY/key.h"
#include "./BSP/TOUCH/touch.h"
#include "./MALLOC/malloc.h"
#include "uc-os2_demo.h"


int main(void)
{  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */  
    tp_dev.init();                          /* 触摸屏初始化 */

    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
  
    uc_os2_demo();                          /* 运行uC/OS-II例程 */
}




