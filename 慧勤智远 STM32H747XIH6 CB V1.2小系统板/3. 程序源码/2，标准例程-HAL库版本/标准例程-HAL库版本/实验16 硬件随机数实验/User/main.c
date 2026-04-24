/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       RNG(真随机数发生器) 实验
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
#include "./USMART/usmart.h"
#include "./BSP/KEY/key.h"
#include "./BSP/RNG/rng.h"


int main(void)
{ 
    uint32_t random;
    uint8_t t = 0, key;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    usmart_init(240);	                      /* 初始化USMART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "RNG TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
  
    while (rng_init())                      /* 初始化随机数发生器 */
    {
        lcd_show_string(30, 110, 200, 16, 16, "  RNG Error! ", RED);
        delay_ms(200);
        lcd_show_string(30, 110, 200, 16, 16, "RNG Trying...", RED);
    }

    lcd_show_string(30, 110, 200, 16, 16, "RNG Ready!   ", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY0:Get Random Num", RED);
    lcd_show_string(30, 150, 200, 16, 16, "Random Num:", RED);
    lcd_show_string(30, 180, 200, 16, 16, "Random Num[0-9]:", RED); 
    
    while (1)
    {        
        key = key_scan(0);

        if (key == KEY0_PRES)                                     /* KEY0按下 */
        {
            random = rng_get_random_num();                        /* 获取一次随机数 */
            lcd_show_num(30 + 8 * 11, 150, random, 10, 16, BLUE); /* 显示随机数 */
        }

        if ((t % 20) == 0)
        {
            LED0_TOGGLE();                                        /* 每200ms,翻转一次LED0(绿灯) */
            random = rng_get_random_range(0, 9);                  /* 取[0,9]区间的随机数 */
            lcd_show_num(30 + 8 * 16, 180, random, 1, 16, BLUE);  /* 显示随机数 */
        }

        delay_ms(10);
        t++;
    }
}




