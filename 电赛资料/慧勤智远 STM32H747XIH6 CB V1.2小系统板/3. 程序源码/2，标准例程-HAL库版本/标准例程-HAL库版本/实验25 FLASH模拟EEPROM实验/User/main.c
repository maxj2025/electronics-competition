/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       FLASH模拟EEPROM 实验
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
#include "./BSP/STMFLASH/stmflash.h"


/* 要写入到STM32 FLASH的字符串数组 */
const uint8_t g_text_buf[] = {"STM32H7 FLASH TEST"};

#define TEXT_LENTH   sizeof(g_text_buf)   /* 数组长度 */

/* SIZE表示字长(4字节), 大小必须是4的整数倍, 如果不是的话, 强制对齐到4的整数倍 */
#define SIZE    TEXT_LENTH / 4 + ((TEXT_LENTH % 4) ? 1 : 0)

#define FLASH_SAVE_ADDR 0x08020000  /* 设置FLASH 保存地址(必须为4的整数倍，且所在扇区要大于本代码所占用到的扇区,STM32H7一个扇区是128K).
                                       否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机. */


int main(void)
{ 
    uint8_t key = 0;
    uint16_t i = 0;
    uint8_t datatemp[TEXT_LENTH];
  
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
    lcd_show_string(30, 70, 200, 16, 16, "FLASH EEPROM TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "WK_UP:Write  KEY0:Read", RED);        /* 显示提示信息 */
    
    while (1)
    {        
        key = key_scan(0);                                                       /* 按键扫描 */

        if (key == WKUP_PRES)                                                    /* WK_UP按下,写入STM32 FLASH */
        {
            lcd_fill(0, 150, 239, 319, WHITE);                                   /* 清除半屏 */
            lcd_show_string(30, 150, 200, 16, 16, "Start Write FLASH....", RED);
            stmflash_write(FLASH_SAVE_ADDR, (uint32_t *)g_text_buf, SIZE);       /* 写入数据 */ 
            lcd_show_string(30, 150, 200, 16, 16, "FLASH Write Finished!", RED); /* 提示传送完成 */
        }

        if (key == KEY0_PRES)                                                    /* KEY0按下,读取FLASH数据并显示 */
        {
            lcd_show_string(30, 150, 200, 16, 16, "Start Read FLASH.... ", RED);
            stmflash_read(FLASH_SAVE_ADDR, (uint32_t *)datatemp, SIZE);          /* 读取数据 */ 
            lcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:  ", RED); /* 提示传送完成 */
            lcd_show_string(30, 170, 200, 16, 16, (char *)datatemp, BLUE);       /* 显示读到的字符串 */
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();                                                       /* LED0(绿灯)闪烁，提示系统正在运行 */
            i = 0;
        }
    }
}




