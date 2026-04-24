/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       LT7680模块显示 实验
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


int main(void)
{  
    uint8_t x = 0;

    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    lcd_port_init();					              /* LCD模块的接口初始化 */
	  LT768_Init();						                /* LT768初始化 */		
    lcd_init();                             /* 初始化显示窗口 */
    Display_ON();						                /* 开启显示 */
    
    while (1)
    {
        switch (x)
        {
            case 0: lcd_clear(WHITE);    break;

            case 1: lcd_clear(BLACK);    break;

            case 2: lcd_clear(BLUE);     break;

            case 3: lcd_clear(RED);      break;

            case 4: lcd_clear(MAGENTA);  break;

            case 5: lcd_clear(GREEN);    break;

            case 6: lcd_clear(CYAN);     break;

            case 7: lcd_clear(YELLOW);   break;

            case 8: lcd_clear(BRRED);    break;

            case 9: lcd_clear(GRAY);     break;

            case 10:lcd_clear(LGRAY);    break;

            case 11:lcd_clear(BROWN);    break;
        }

        lcd_show_string(30, 40, 230, 32, 32, "STM32H747 ^_^", RED);
        lcd_show_string(30, 80, 230, 24, 24, "LT7680 LCD TEST", RED);
        lcd_show_string(30, 110, 230, 24, 24, "WKS SMART", RED);
        x++;
        
        if (x == 12)
        {
            x = 0;
        }
        
        LED0_TOGGLE();   /* LED0(绿灯)不停的闪烁，提示程序已经在运行了 */
        delay_ms(1000);
    }
}




