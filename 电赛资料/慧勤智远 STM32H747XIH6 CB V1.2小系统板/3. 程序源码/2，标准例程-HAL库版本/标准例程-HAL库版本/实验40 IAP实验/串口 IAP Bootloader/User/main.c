/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       串口IAP 实验
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
#include "./BSP/STMFLASH/stmflash.h"
#include "./IAP/iap.h"


int main(void)
{ 
    uint8_t t;
    uint8_t key;
    uint32_t oldcount = 0;                  /* 之前的串口接收数据值 */
    uint32_t applenth = 0;                  /* 接收到的app代码长度 */
    uint8_t clearflag = 0;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USART IAP TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 240, 16, 16, "WK_UP: Copy & Run FLASH APP", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY0: Run SRAM APP", RED); 
    
    while (1)
    {        
        if (g_usart_rx_cnt)
        {
            if (oldcount == g_usart_rx_cnt)     /* 新周期内,没有收到任何数据,认为本次数据接收完成 */
            {
                applenth = g_usart_rx_cnt;
                oldcount = 0;
                g_usart_rx_cnt = 0;
                printf("用户程序接收完成!\r\n");
                printf("代码长度:%dBytes\r\n", applenth);
            }
            else
            {
                oldcount = g_usart_rx_cnt;
            }
        }

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
            if (applenth)
            {
                printf("开始更新固件...\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "Copying APP2FLASH...", BLUE);

                if (((*(volatile uint32_t *)(0X24002000 + 4)) & 0xFF000000) == 0x08000000)  /* 判断是否为0X08XXXXXX */
                {
                    iap_write_appbin(FLASH_APP1_ADDR, g_usart_rx_buf, applenth);            /* 更新FLASH代码 */
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
            printf("开始执行SRAM用户代码!!\r\n\r\n");
            delay_ms(10);

            if (((*(volatile uint32_t *)(0x24002000 + 4)) & 0xFF000000) == 0x24000000)   /* 判断是否为0X24XXXXXX */
            {
                iap_load_app(0x24002000);                                                /* 执行SRAM APP代码 */
            }
            else
            {
                printf("非SRAM应用程序,无法执行!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "Illegal SRAM APP!", BLUE);
            }

            clearflag = 7;      /* 标志更新了显示,并且设置7*300ms后清除显示 */
        }
    }
}




