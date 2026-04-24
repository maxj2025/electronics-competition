/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       U盘(Host) 实验
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
#include "./MALLOC/malloc.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/SDNAND/sdmmc_sdnand.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./PICTURE/piclib.h"
#include "usbh_core.h"
#include "usbh_msc.h"


USBH_HandleTypeDef hUSBHost;     /* USB Host处理结构体 */

/**
 * @brief   USB用户回调函数
 * @param   phost: 指向USBH句柄的指针
 * @param   id: 事件ID
 * @retval  无
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    uint32_t total, free;
    uint8_t res = 0;
    printf("id:%d\r\n", id);

    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            f_mount(0, "3:", 0);                                    /* 卸载U盘 */
            text_show_string(30, 120, 200, 16, "等待U盘插入...", 16, 0, RED);
            lcd_fill(30, 160, 239, 220, WHITE);
            LED1(1);                                                /* LED1(蓝灯)灭 */
            break;

        case HOST_USER_CLASS_ACTIVE:
            f_mount(fs[3], "3:", 1);                                /* 重新挂载U盘 */
            text_show_string(30, 120, 200, 16, "U盘连接成功!  ", 16, 0, RED);
            res = exfuns_get_free("3:", &total, &free);
           
            if (res == 0)
            {
                lcd_show_string(30, 160, 200, 16, 16, "FATFS OK!", BLUE);
                lcd_show_string(30, 180, 200, 16, 16, "U Disk Total Size:      MB", BLUE);
                lcd_show_string(30, 200, 200, 16, 16, "U Disk  Free Size:      MB", BLUE);
                lcd_show_num(174, 180, total >> 10, 6, 16, BLUE);   /* 显示U盘总容量 MB */
                lcd_show_num(174, 200, free >> 10, 6, 16, BLUE);
                LED1(0);                                            /* LED1(蓝灯)亮 */
            }
            else
            {
                printf("U盘存储空间获取失败\r\n");
            }

            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

int main(void)
{ 
    uint8_t t = 0;
  
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
    norflash_init();                        /* 初始化NORFLASH */
    piclib_init();                          /* 初始化画图 */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    exfuns_init();                          /* 为fatfs相关变量申请内存 */    
    f_mount(fs[0], "0:", 1);                /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                /* 挂载SD NAND */
    
    while (fonts_init())                    /* 检查字库 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 230, 66, WHITE);   /* 清除显示 */
        delay_ms(200);
    }
    
    text_show_string(30, 50, 200, 16, "STM32H747", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "USB U盘(Host) 实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "WKS SMART", 16, 0, RED);
    text_show_string(30, 120, 200, 16, "等待U盘插入...", 16, 0, RED);

    USBH_Init(&hUSBHost, USBH_UserProcess, 0);
    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
    USBH_Start(&hUSBHost);
    
    while (1)
    {        
        USBH_Process(&hUSBHost);
        delay_ms(10);
        t++;

        if (t == 50)
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}




