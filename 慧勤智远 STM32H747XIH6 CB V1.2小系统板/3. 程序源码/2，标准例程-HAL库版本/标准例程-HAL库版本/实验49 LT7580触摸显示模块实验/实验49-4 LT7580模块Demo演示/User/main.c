/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       LT7580模块Demo演示 实验
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
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/LT758_Function/LT758_Function.h"


int main(void)
{  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    lcd_port_init();					              /* LCD模块的接口初始化 */
	  LT758_Init();						                /* LT758初始化 */		
    lcd_init();                             /* 初始化显示窗口 */
    Display_ON();						                /* 开启显示 */ 
    key_init();                             /* 初始化按键 */ 
    tp_dev.init();                          /* 触摸屏初始化 */
  
    Cs_Select();                            /* 识别所使用的SPI FLASH */
    function_main();                        /* 运行功能演示函数 */
}




