/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       SD卡 IAP 实验
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
#include "./BSP/STMFLASH/stmflash.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/SDNAND/sdmmc_sdnand.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./IAP/iap.h"
  

char *const FLASH_APP_PATH = "0:FLASH_APP.bin";     /* FLASH_APP的bin文件存放在SD卡中的路径 */
char *const SRAM_APP_PATH = "0:SRAM_APP.bin";       /* SRAM_APP的bin文件存放在SD卡中的路径 */


int main(void)
{  
    uint8_t t = 0;
    uint8_t key;
    uint8_t clearflag = 0;
    FIL *f_flash;
    FIL *f_sram;
    uint8_t rval = 0;
    uint8_t flash_app_check = 0;            /* FLASH_APP文件检测标志 */
    uint8_t sram_app_check = 0;             /* SRAM_APP文件检测标志 */
    uint32_t bread;                         /* 读取的长度 */
    uint8_t *p;
    uint8_t *pbuf;                          /* 缓存 */
    uint32_t offset = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
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
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "SD IAP TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 240, 16, 16, "WK_UP: Copy & Run FLASH APP", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY0: Run SRAM APP", RED); 

    f_flash = (FIL *)mymalloc(SRAMIN, sizeof(FIL));  /* 分配内存 */
    f_sram = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* 分配内存 */
    pbuf = mymalloc(SRAMIN, 4096);                   /* 申请4K字节内存 */

    rval = f_open(f_flash, (const TCHAR *)FLASH_APP_PATH, FA_READ);  /* 尝试打开FLASH_APP的bin文件 */
  
    if (rval == FR_OK)
    {
        flash_app_check = 1;     /* 标记FLASH APP文件打开成功 */ 
    }

    rval = f_open(f_sram, (const TCHAR *)SRAM_APP_PATH, FA_READ);    /* 尝试打开SRAM_APP的bin文件 */

    if (rval == FR_OK)
    {
        sram_app_check = 1;      /* 标记SRAM APP文件打开成功 */
    }
    
    while (1)
    {
        t++;
        delay_ms(100);

        if (t == 3)
        {
            LED0_TOGGLE();
            t = 0;

            if (clearflag)
            {
                clearflag--;

                if (clearflag == 0)
                {
                    lcd_fill(30, 190, 240, 190 + 16, WHITE);     /* 清除显示 */
                }
            }
        }

        key = key_scan(0);

        if (key == WKUP_PRES)   /* WKUP按下,更新固件到FLASH */
        {            
            if (flash_app_check)
            {
                printf("开始更新固件...\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "Copying APP2FLASH...", BLUE);

                rval = f_read(f_flash, pbuf, 512, (UINT *)&bread); /* 读出前512字节 */

                if (((*(volatile uint32_t *)(pbuf + 4)) & 0xFF000000) == 0x08000000)  /* 判断是否为0X08XXXXXX */
                {
                    f_lseek(f_flash, 0);  /* 偏移到开头 */
                  
                    while (1)             /* 循环方式,读取整个文件 */
                    {
                        rval = f_read(f_flash, pbuf, 4096, (UINT *)&bread);       /* 读出文件内容 */
                      
                        if (rval != FR_OK)
                        {
                            break;        /* 读取错误 */
                        }    
                        
                        iap_write_appbin(FLASH_APP1_ADDR + offset, pbuf, bread);  /* 写入内部FLASH */
                        offset += bread;

                        if (bread != 4096)
                        {
                            break;        /* 读完了 */
                        }                    
                    }
                    
                    lcd_show_string(30, 190, 200, 16, 16, "Copy APP Successed!!", BLUE);
                    printf("固件更新完成!\r\n");
                    delay_ms(1000);
                  
                    printf("flash addr :%x \r\n",(*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000);
                    if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) /* 判断FLASH里面是否有APP,有的话执行 */
                    {
                        printf("开始执行FLASH用户代码!!\r\n\r\n");
                        delay_ms(10);
                        iap_load_app(FLASH_APP1_ADDR); /* 执行FLASH APP代码 */
                    }
                    else
                    {
                        printf("没有可以运行的固件!\r\n");
                        lcd_show_string(30, 190, 200, 16, 16, "No APP!", BLUE);
                    }
                }
                else
                {
                    lcd_show_string(30, 190, 200, 16, 16, "Illegal FLASH APP!  ", BLUE);
                    printf("非FLASH应用程序!\r\n");
                }
            }
            else if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) /* 判断FLASH里面是否有APP,有的话执行 */
            {
                printf("开始执行FLASH用户代码!!\r\n\r\n");
                delay_ms(10);
                iap_load_app(FLASH_APP1_ADDR);         /* 执行FLASH APP代码 */
            }
            else
            {
                printf("没有可以更新的固件!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "No APP!", BLUE);
            }

            clearflag = 7;      /* 标志更新了显示,并且设置7*300ms后清除显示 */
        }

        if (key == KEY0_PRES)   /* KEY0按下 */
        {
            if (sram_app_check)
            {
                printf("开始执行SRAM用户代码!!\r\n\r\n");
                delay_ms(10);
                p = (uint8_t *)SRAM_APP_ADDR;
              
                rval = f_read(f_sram, p, f_sram->obj.objsize, (UINT *)&bread);                  /* 读出BIN文件所有内容 */
          
                if (((*(volatile uint32_t *)(SRAM_APP_ADDR + 4)) & 0xFF000000) == 0x24000000)   /* 判断是否为0X24XXXXXX */
                {
                    iap_load_app(SRAM_APP_ADDR);                                                /* 执行SRAM APP代码 */
                }
                else
                {
                    printf("非SRAM应用程序,无法执行!\r\n");
                    lcd_show_string(30, 190, 200, 16, 16, "Illegal SRAM APP!", BLUE);
                }
            }
            else
            {
                printf("没有可以更新的固件!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "No APP!", BLUE);
            }
            
            clearflag = 7;      /* 标志更新了显示,并且设置7*300ms后清除显示 */
        }    
    }
}




