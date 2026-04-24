/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       LT7680模块 SPI FLASH文件烧录 实验
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
#include "./MALLOC/malloc.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/SDNAND/sdmmc_sdnand.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/LT768_FLASH/LT768_w25qxx.h"
#include "./FILEUPD/fileupd.h"
#include "./TEXT/text.h"


int main(void)
{  
    uint8_t key;
    uint16_t id = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_port_init();					              /* LCD模块的接口初始化 */
	  LT768_Init();						                /* LT768初始化 */		
    lcd_init();                             /* 初始化显示窗口 */
    Display_ON();						                /* 开启显示 */
    spi_flash_init();                       /* 初始化LT7680模块上的SPI FLASH */  
    key_init();                             /* 初始化按键 */
    norflash_init();                        /* 初始化NORFLASH */
    fonts_init();                           /* 初始化字体 */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    exfuns_init();                          /* 为fatfs相关变量申请内存 */    
    f_mount(fs[0], "0:", 1);                /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                /* 挂载SPI FLASH */
  
    lcd_show_string(30, 30, 200, 24, 24, "STM32H7", RED);
  
    while (1)    
    {
        id = spi_flash_read_id();     /* 读取FLASH ID */
      
        if (id == W25Q128 || id == W25Q256)     
        {
            break;
        }

        lcd_show_string(30, 60, 200, 24, 24, "FLASH Check Failed!", RED);   /* 检测不到FLASH芯片 */
        delay_ms(500);
        lcd_show_string(30, 60, 200, 24, 24, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();                /* LED0(绿灯)闪烁 */
    }

    lcd_show_string(30, 60, 200, 24, 24, "SPI FLASH Ready!", RED);     

    while (sd_init())                 /* 检测SD卡 */    
    {
        lcd_show_string(30, 90, 200, 24, 24, "SD Card Failed!", RED);
        delay_ms(200);
        lcd_fill(30, 90, 200 + 30, 90 + 24, WHITE);
        delay_ms(200);
        LED0_TOGGLE();                /* LED0(绿灯)闪烁 */
    }

    lcd_show_string(30, 90, 200, 24, 24, "SD Card OK", RED);
    lcd_show_string(30, 120, 300, 24, 24, "Press KEY0 to update", RED);

    while (key_scan(0) != KEY0_PRES); /* 按键确认 */    

    lcd_fill(30, 120, 300 + 30, 120 + 24, WHITE);
    lcd_show_string(30, 120, 300, 24, 24, "File Updating...", RED);
    key = files_update(20, 150, 24, (uint8_t *)"0:", RED);      /* 更新文件 */

    while (key)                                                 /* 更新失败 */
    {
        lcd_show_string(30, 150, 300, 24, 24, "File Update Failed!", RED);
        delay_ms(200);
        lcd_fill(20, 150, 300 + 20, 150 + 24, WHITE);
        delay_ms(200);
    }

    lcd_fill(20, 120, 300 + 20, 150 + 24, WHITE);
    lcd_show_string(30, 150, 300, 24, 24, "File Update Success!   ", RED);
    LED1_TOGGLE();       /* 点亮LED1(蓝灯) */
    delay_ms(1500);
    
    while (1)
    {
        LED0_TOGGLE();   /* LED0(绿灯)闪烁 */
        delay_ms(1000);   
    }
}




