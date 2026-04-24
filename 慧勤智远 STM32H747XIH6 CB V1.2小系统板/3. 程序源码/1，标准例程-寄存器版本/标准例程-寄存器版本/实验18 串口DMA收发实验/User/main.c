/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       DMA 实验
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
#include "./BSP/DMA/dma.h"


const uint8_t TEXT_TO_SEND[] = {"WKS STM32H747 DMA 串口实验"};      /* 要循环发送的字符串 */

#define SEND_BUF_SIZE       (sizeof(TEXT_TO_SEND) + 2) * 200        /* 发送数据长度, 等于sizeof(TEXT_TO_SEND) + 2的200倍. */

uint8_t g_sendbuf[SEND_BUF_SIZE];                                   /* 发送数据缓冲区 */

uint8_t rxbuffer[USART_RX_DMA_TRANS + 1];                           /* 接收数据缓冲区 */


int main(void)
{  
    uint8_t key = 0;
    uint16_t i, k;
    uint16_t len;
    uint8_t mask = 0;
    float pro = 0;                          /* 用于显示进度 */
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
  
    dma_usart_tx_config(DMA2_Stream7, 42, (uint32_t)&USART_UX->TDR, (uint32_t)g_sendbuf); /* DMA2_Stream7配置,外设为USARTX数据发送寄存器,存储器为g_sendbuf */
    dma_usart_rx_config(DMA1_Stream1, 41, (uint32_t)&USART_UX->RDR, (uint32_t)rxbuffer);  /* 串口RX DMA初始化,外设为USARTX数据接收寄存器,存储器为rxbuffer */
    USART_UX->CR3 |= 1 << 6;                /* 使能串口的DMA接收 */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DMA TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Start", RED);
  
    len = sizeof(TEXT_TO_SEND);
    k = 0;
    
    for (i = 0; i < SEND_BUF_SIZE; i++)     /* 填充数据 */
    {
        if (k >= len)                       /* 加入换行符 */
        {
            if (mask)
            {
                g_sendbuf[i] = 0x0a;
                k = 0;
            }
            else
            {
                g_sendbuf[i] = 0x0d;
                mask++;
            }
        }
        else                                /* 复制TEXT_TO_SEND的内容到g_sendbuf */
        {
            mask = 0;
            g_sendbuf[i] = TEXT_TO_SEND[k];
            k++;
        }
    }
 
    i = 0;
    
    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)   /* KEY0按下 */
        {
            printf("\r\nDMA DATA:\r\n");
            lcd_show_string(30, 130, 200, 16, 16, "Start Transimit....", BLUE); /* 显示开始传送 */
            lcd_show_string(30, 150, 200, 16, 16, "   %", BLUE);                /* 显示百分号 */
          
            USART_UX->CR3 |= 1 << 7;                     /* 使能串口的DMA发送 */
            
            dma_enable(DMA2_Stream7, SEND_BUF_SIZE);     /* 开始一次DMA传输！ */

            /* 等待DMA传输完成
             * 实际应用中，在等待DMA传输完成期间，可以执行另外的任务
             */
            while (1)
            {
                if (DMA2->HISR & (1 << 27))              /* 等待DMA2_Steam7传输完成 */
                {
                    DMA2->HIFCR |= 1 << 27;              /* 清除DMA2_Steam7传输完成标志 */
                    break;
                }

                pro = DMA2_Stream7->NDTR;                /* 得到当前还剩余多少个数据 */
                len = SEND_BUF_SIZE;                     /* 总长度 */
                pro = 1 - (pro / len);                   /* 得到百分比 */
                pro *= 100;                              /* 扩大100倍 */
                lcd_show_num(30, 150, pro, 3, 16, BLUE); /* 显示传输进度 */
            } 
            
            lcd_show_num(30, 150, 100, 3, 16, BLUE);     /* 显示100% */
            lcd_show_string(30, 130, 200, 16, 16, "Transimit Finished!", BLUE); /* 提示传送完成 */
        }

        if (g_transfer_complete == 1)
        {
            g_transfer_complete = 0;
            printf("\n\r%s\n\r", rxbuffer);
            dma_enable(DMA1_Stream1, USART_RX_DMA_TRANS);/* 使能DMA1_Stream1，重新传输 */          
        }
        
        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();  /* LED0(绿灯)闪烁,提示系统正在运行 */
            i = 0;
        }
    }
}




