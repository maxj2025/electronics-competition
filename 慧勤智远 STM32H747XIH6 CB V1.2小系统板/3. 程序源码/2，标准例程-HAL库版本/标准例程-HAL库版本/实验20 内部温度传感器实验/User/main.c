/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       内部温度传感器 实验
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
#include "./BSP/ADC/adc3.h"


int main(void)
{ 
    short temp;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    adc3_init();                            /* 初始化ADC3(使能内部温度传感器) */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "Temperature TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
  
    lcd_show_string(30, 120, 200, 16, 16, "TEMPERATE: 00.00C", BLUE);
    
    while (1)
    {        
        temp = adc3_get_temperature();      /* 得到温度值 */

        if (temp < 0)
        {
            temp = -temp;
            lcd_show_string(30 + 10 * 8, 120, 16, 16, 16, "-", BLUE);   /* 显示负号 */
        }
        else
        {
            lcd_show_string(30 + 10 * 8, 120, 16, 16, 16, " ", BLUE);   /* 无符号 */
        }
        
        lcd_show_xnum(30 + 11 * 8, 120, temp / 100, 2, 16, 0, BLUE);    /* 显示整数部分 */
        lcd_show_xnum(30 + 14 * 8, 120, temp % 100, 2, 16, 0X80, BLUE); /* 显示小数部分 */

        LED0_TOGGLE();                      /* LED0(绿灯)闪烁,提示程序运行 */
        delay_ms(250);
        
        printf(" the temperature data is %2d.%2d degrees Celsius\r\n", temp /100, temp % 100);
    }
}




