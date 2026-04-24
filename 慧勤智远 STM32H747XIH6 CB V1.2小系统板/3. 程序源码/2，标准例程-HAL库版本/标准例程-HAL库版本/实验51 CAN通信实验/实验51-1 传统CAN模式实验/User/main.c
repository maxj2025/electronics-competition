/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       FDCAN通信(传统CAN模式) 实验
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
#include "./BSP/FDCAN/fdcan.h"                           


int main(void)
{ 
    uint8_t key;
    uint8_t i = 0, t = 0;
    uint8_t cnt = 0;
    uint8_t canbuf[8];
    uint8_t rxlen = 0;
    uint8_t res;
    uint8_t mode = 1;                       /* FDCAN工作模式: 0,普通模式; 1,环回模式 */
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 12);    /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    usmart_init(240);	                      /* 初始化USMART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    fdcan_init(2, 8, 31, 8, FDCAN_MODE_INTERNAL_LOOPBACK);               /* FDCAN初始化，内部环回模式 */
    printf("\r\nLoopBack Mode\r\n");
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "FDCAN TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 130, 200, 16, 16, "LoopBack Mode", RED);
    lcd_show_string(30, 150, 200, 16, 16, "KEY0:Send WK_UP:Mode", RED);  /* 显示提示信息 */

    lcd_show_string(30, 170, 200, 16, 16, "Count:", BLUE);
    lcd_show_string(30, 190, 200, 16, 16, "Send Data:", BLUE);
    lcd_show_string(30, 250, 200, 16, 16, "Receive Data:", BLUE);  
    
    while (1)
    {        
        key = key_scan(0);

        if (key == KEY0_PRES)              /* KEY0按下,发送一次数据 */
        {
            for (i = 0; i < 8; i++)
            {
                canbuf[i] = cnt + i;       /* 填充发送缓冲区 */

                if (i < 4)
                {
                    lcd_show_xnum(30 + i * 32, 210, canbuf[i], 3, 16, 0X80, BLUE);        /* 显示数据 */
                }
                else 
                {
                    lcd_show_xnum(30 + (i - 4) * 32, 230, canbuf[i], 3, 16, 0X80, BLUE);  /* 显示数据 */
                }
            }

            res = fdcan_send_msg(canbuf, FDCAN_DLC_BYTES_8);   /* 发送8个字节数据 */

            printf("\r\nFDCAN transmit data: \r\n");           /* 通过串口打印发送的数据 */
            
            for(i = 0U; i < 8; i++) 
            {
                printf("%02x ", canbuf[i]);
            }
            
            if (res)
            {
                lcd_show_string(30 + 80, 190, 200, 16, 16, "Failed", BLUE);   /* 提示发送失败 */
            }
            else 
            {
                lcd_show_string(30 + 80, 190, 200, 16, 16, "OK    ", BLUE);   /* 提示发送成功 */
            }
        }
        else if (key == WKUP_PRES)         /* WK_UP按下，改变FDCAN的工作模式 */
        {
            mode = !mode;
          
            if (mode == 0)  
            {
                fdcan_init(2, 8, 31, 8, FDCAN_MODE_NORMAL);                   /* 普通模式,波特率1Mbps */
            }
            else if (mode == 1) 
            {
                fdcan_init(2, 8, 31, 8, FDCAN_MODE_INTERNAL_LOOPBACK);        /* 环回模式,波特率1Mbps */
            }
            
            if (mode == 0)                 /* 普通模式，需要2个开发板 */
            {
                lcd_show_string(30, 130, 200, 16, 16, "Normal Mode  ", BLUE);
                printf("\r\nNormal Mode\r\n");
            }
            else                           /* 环回模式，一个开发板就可以测试了 */
            {
                lcd_show_string(30, 130, 200, 16, 16, "LoopBack Mode", BLUE);
                printf("\r\nLoopBack Mode\r\n");
            }
        }

        rxlen = fdcan_receive_msg(canbuf);

        if (rxlen)                                       /* 检索接收到数据 */
        {
            lcd_fill(30, 270, 160, 310, WHITE);          /* 清除之前的显示 */
          
            for (i = 0; i < rxlen; i++)
            {
                if (i < 4)
                {
                    lcd_show_xnum(30 + i * 32, 270, canbuf[i], 3, 16, 0X80, BLUE);        /* 显示数据 */
                }
                else 
                {
                    lcd_show_xnum(30 + (i - 4) * 32, 290, canbuf[i], 3, 16, 0X80, BLUE);  /* 显示数据 */
                }
            }
         
            printf("\r\nFDCAN receive data: \r\n");      /* 通过串口打印接收到的数据 */
          
            for(i = 0U; i < 8; i++) 
            {
                printf("%02x ", canbuf[i]);
            }
        }

        t++; 
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();                                         /* LED0(绿灯)闪烁，提示系统正在运行 */
            t = 0;
            cnt++;
            lcd_show_xnum(30 + 48, 170, cnt, 3, 16, 0X80, BLUE);   /* 显示计数值 */
        }
    }
}




