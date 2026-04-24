/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       QSPI FLASH 实验
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
#include "./BSP/QSPI/qspi.h"
#include "./BSP/NORFLASH/norflash.h"


/* 要写入到QSPI FLASH的字符串数组 */
const uint8_t g_text_buf[] = {"STM32H7 QSPI FLASH TEST"};

#define TEXT_SIZE    sizeof(g_text_buf)     /* TEXT字符串长度 */


int main(void)
{  
    uint8_t key;
    uint16_t i = 0;
    uint8_t datatemp[TEXT_SIZE + 2];
    uint8_t rectemp[TEXT_SIZE + 2];
    uint32_t flashsize;
    uint16_t id = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    usmart_init(240);	                      /* 初始化USMART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    norflash_init();                        /* 初始化NORFLASH */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "QSPI TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "WK_UP:Write  KEY0:Read", RED);   /* 显示提示信息 */
    
    while (1)    
    {
        id = norflash_read_id();    /* 读取FLASH ID */
      
        if (id == W25Q128 || id == W25Q256)     
        {
            break;
        }

        lcd_show_string(30, 130, 200, 16, 16, "FLASH Check Failed!", RED);  /* 检测不到FLASH芯片 */
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();              /* LED0闪烁 */
    }

    lcd_show_string(30, 130, 200, 16, 16, "QSPI FLASH Ready!", BLUE); 
    flashsize = 32 * 1024 * 1024;   /* FLASH 大小为32M字节 */
    
    while (1)
    {
        key = key_scan(0);          /* 按键扫描 */

        if (key == WKUP_PRES)       /* WK_UP按下,写入QSPI FLASH */
        {
            lcd_fill(0, 170, 239, 319, WHITE);  /* 清除半屏 */
            lcd_show_string(30, 150, 200, 16, 16, "Start Write FLASH....", BLUE);
            sprintf((char *)datatemp, "%s%d", (char *)g_text_buf, i);
            norflash_write((uint8_t *)datatemp, flashsize - 200, TEXT_SIZE + 2);   /* 从倒数第200个地址处开始,写入(TEXT_SIZE + 2)长度的数据 */
            lcd_show_string(30, 150, 200, 16, 16, "FLASH Write Finished!", BLUE);  /* 提示传送完成 */
            printf("datatemp:%s  \r\n", datatemp);
        }

        if (key == KEY0_PRES)       /* KEY0按下,读取字符串并显示 */
        {
            lcd_show_string(30, 150, 200, 16, 16, "Start Read QSPI.... ", BLUE);
            norflash_read(rectemp, flashsize - 200, TEXT_SIZE + 2);                /* 从倒数第200个地址处开始,读出(TEXT_SIZE + 2)个字节 */
            lcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:   ", BLUE); /* 提示传送完成 */
            lcd_show_string(30, 170, 210, 16, 16, (char *)rectemp, BLUE);          /* 显示读到的字符串 */
            printf("rectemp:%s  \r\n", rectemp);
        }

        i++;

        if (i == 20)
        {
            LED0_TOGGLE();  /* LED0(绿灯)闪烁 */
            i = 0;
        }

        delay_ms(10);
    }
}




