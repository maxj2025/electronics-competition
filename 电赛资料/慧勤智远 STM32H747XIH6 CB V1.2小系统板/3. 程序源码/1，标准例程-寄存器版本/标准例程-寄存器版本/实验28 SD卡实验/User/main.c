/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       SD卡 实验
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
#include "./BSP/SDMMC/sdmmc_sdcard.h"


/**
 * @brief       通过串口打印SD卡相关信息
 * @param       无
 * @retval      无
 */
void show_sdcard_info(void)
{
    switch (g_sd_card_info.CardType)
    {
        case STD_CAPACITY_SD_CARD_V1_1:
            printf("Card Type:SDSC V1.1\r\n");
            break;

        case STD_CAPACITY_SD_CARD_V2_0:
            printf("Card Type:SDSC V2.0\r\n");
            break;

        case HIGH_CAPACITY_SD_CARD:
            printf("Card Type:SDHC V2.0\r\n");
            break;

        case MULTIMEDIA_CARD:
            printf("Card Type:MMC Card\r\n");
            break;
    }

    printf("Card ManufacturerID:%d\r\n", g_sd_card_info.SD_cid.ManufacturerID);         /* 制造商ID */
    printf("Card RCA:%d\r\n", g_sd_card_info.RCA);                                      /* 卡相对地址 */
    printf("Card Capacity:%d MB\r\n", (uint32_t)(g_sd_card_info.CardCapacity >> 20));   /* 显示容量 */
    printf("Card BlockSize:%d\r\n\r\n", g_sd_card_info.CardBlockSize);                  /* 显示块大小 */
}

/**
 * @brief       测试SD卡的读取
 * @note        从secaddr地址开始,读取seccnt个扇区的数据
 * @param       secaddr : 扇区地址
 * @param       seccnt  : 扇区数
 * @retval      无
 */
void sd_test_read(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
  
    buf = mymalloc(SRAMIN, seccnt * 512);       /* 申请内存,从内部SRAM申请内存 */
    sta = sd_read_disk(buf, secaddr, seccnt);   /* 读取secaddr扇区开始的内容 */

    if (sta == 0)
    {
        lcd_show_string(30, 190, 200, 16, 16, "USART1 Sending Data...", BLUE);
        printf("SECTOR %d DATA:\r\n", secaddr);

        for (i = 0; i < seccnt * 512; i++)
        {
            printf("%x ", buf[i]);              /* 打印secaddr开始的扇区数据 */
        }
        
        printf("\r\nDATA ENDED\r\n");
        lcd_show_string(30, 190, 200, 16, 16, "USART1 Send Data Over!", BLUE);
    }
    else
    {
        printf("err:%d\r\n", sta);
        lcd_show_string(30, 190, 200, 16, 16, "SD read Failure!      ", BLUE);
    }

    myfree(SRAMIN, buf);                        /* 释放内存 */
}

/**
 * @brief       测试SD卡的写入
 * @note        从secaddr地址开始,写入seccnt个扇区的数据
 *              慎用!! 最好写全是0XFF的扇区,否则可能损坏SD卡.
 *
 * @param       secaddr : 扇区地址
 * @param       seccnt  : 扇区数
 * @retval      无
 */
void sd_test_write(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
  
    buf = mymalloc(SRAMIN, seccnt * 512);       /* 从内部SRAM申请内存 */

    for (i = 0; i < seccnt * 512; i++)          /* 初始化写入的数据,是3的倍数. */
    {
        buf[i] = i * 3;
    }

    sta = sd_write_disk(buf, secaddr, seccnt);  /* 从secaddr扇区开始写入seccnt个扇区内容 */

    if (sta == 0)
    {
        printf("Write over!\r\n");
    }
    else 
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);                        /* 释放内存 */
}

int main(void)
{  
    uint8_t key;
    uint8_t t = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    usmart_init(240);	                      /* 初始化USMART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "SD TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Read Sector 0", RED);
    
    while (sd_init())                       /* 检测不到SD卡 */
    {
        lcd_show_string(30, 150, 200, 16, 16, "SD Card Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 150, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();                      /* LED0(绿灯)闪烁 */
    } 

    /* 打印SD卡相关信息 */
    show_sdcard_info(); 
    
    /* 检测SD卡成功 */
    lcd_show_string(30, 150, 200, 16, 16, "SD Card OK    ", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "SD Card Size:      MB", BLUE);
    lcd_show_num(30 + 13 * 8, 170, g_sd_card_info.CardCapacity >> 20, 6, 16, BLUE); /* 显示SD卡容量(MB) */
    
    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)    /* KEY0按下了 */
        {
            sd_test_read(0, 1);  /* 从0扇区读取1*512字节的内容 */
        }

        t++;
        delay_ms(10);

        if (t == 20)
        { 
            LED0_TOGGLE();       /* LED0(绿灯)闪烁 */
            t = 0;
        }
    }
}




