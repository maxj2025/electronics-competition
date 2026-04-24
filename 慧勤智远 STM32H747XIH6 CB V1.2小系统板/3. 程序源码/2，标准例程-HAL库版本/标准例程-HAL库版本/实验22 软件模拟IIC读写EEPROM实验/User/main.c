/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       IIC读写EEPROM 实验
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
#include "./BSP/24CXX/24cxx.h"


/* 要写入到24C16的字符串数组 */
const uint8_t g_text_buf[] = {"STM32H7 IIC TEST"};

#define TEXT_SIZE   sizeof(g_text_buf)      /* TEXT字符串长度 */


int main(void)
{ 
    uint8_t key;
    uint16_t i = 0;
    uint8_t datatemp[TEXT_SIZE];
  
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
    bl24cxx_init();                         /* 初始化24CXX */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "IIC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "WK_UP:Write  KEY0:Read", RED);           /* 显示提示信息 */
  
    while (bl24cxx_check())                                                         /* 检测不到24C16 */
    {
        lcd_show_string(30, 130, 200, 16, 16, "24C16 Check Failed!", RED);
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();                                                              /* LED0闪烁 */
    }

    lcd_show_string(30, 130, 200, 16, 16, "24C16 Ready!", RED);
    
    while (1)
    {        
        key = key_scan(0);                                                          /* 按键扫描 */

        if (key == WKUP_PRES)                                                       /* WK_UP按下,写入24C16 */
        {
            lcd_fill(0, 150, 239, 319, WHITE);                                      /* 清除半屏 */
            lcd_show_string(30, 150, 200, 16, 16, "Start Write 24C16....", BLUE);
            bl24cxx_write(0, (uint8_t *)g_text_buf, TEXT_SIZE);                     /* 往24C16里面写数据 */
            lcd_show_string(30, 150, 200, 16, 16, "24C16 Write Finished!", BLUE);   /* 提示传送完成 */
        }

        if (key == KEY0_PRES)                                                       /* KEY0按下,读取字符串并显示 */
        {
            lcd_show_string(30, 150, 200, 16, 16, "Start Read 24C16.... ", BLUE);
            bl24cxx_read(0, datatemp, TEXT_SIZE);                                   /* 读取数据 */
            lcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:  ", BLUE);   /* 提示传送完成 */
            lcd_show_string(30, 170, 200, 16, 16, (char *)datatemp, BLUE);          /* 显示读到的字符串 */
        }

        i++;

        if (i == 20)
        {
            LED0_TOGGLE();      /* LED0(绿灯)闪烁 */
            i = 0; 
        }

        delay_ms(10);
    }
}




