/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       视频播放器 实验
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
#include "./MALLOC/malloc.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/SDNAND/sdmmc_sdnand.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./BSP/TIMER/timer.h"
#include "./APP/videoplayer.h"


int main(void)
{    
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
    timx_int_init(10000 - 1, 24000 - 1);    /* 10Khz计数,1秒钟中断一次,输出视频帧率 */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    exfuns_init();                          /* 为fatfs相关变量申请内存 */    
    f_mount(fs[0], "0:", 1);                /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                /* 挂载SPI FLASH */
    
    while (fonts_init())                    /* 检查字库 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 230, 66, WHITE);   /* 清除显示 */
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "慧勤智远STM32开发板", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "视频播放器实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "WKS SMART", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY0:NEXT", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "WK_UP:PREV", 16, 0, RED);
   
    delay_ms(1500);

    while (1)
    {
        video_play();
    }
}




