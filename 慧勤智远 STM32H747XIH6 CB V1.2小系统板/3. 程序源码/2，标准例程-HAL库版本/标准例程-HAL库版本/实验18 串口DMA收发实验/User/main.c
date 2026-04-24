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
  
    dma_usart_tx_init(DMA2_Stream7, DMA_REQUEST_USART1_TX);   /* 串口TX DMA初始化 */
    dma_usart_rx_init(DMA1_Stream1, DMA_REQUEST_USART1_RX, (uint32_t)&USART_UX->RDR, (uint32_t)rxbuffer);  /* 串口RX DMA初始化,外设为USARTX数据接收寄存器,存储器为rxbuffer */
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

            USART_UX->CR3 &= ~(1 << 6);                                         /* 禁止串口的DMA接收 */

            HAL_UART_Transmit_DMA(&g_uart1_handle, g_sendbuf, SEND_BUF_SIZE);   /* 开始一次DMA传输！ */

            /* 等待DMA传输完成
             * 实际应用中，在等待DMA传输完成期间，可以执行另外的任务
             */
            while (1)
            {
                if (__HAL_DMA_GET_FLAG(&g_usart_tx_dma_handle, DMA_FLAG_TCIF3_7))    /* 等待DMA2_Stream7传输完成 */
                {
                    __HAL_DMA_CLEAR_FLAG(&g_usart_tx_dma_handle, DMA_FLAG_TCIF3_7);  /* 清除DMA2_Stream7传输完成标志 */
                    HAL_UART_DMAStop(&g_uart1_handle);                          /* 传输完成以后关闭串口DMA */
                    USART_UX->CR3 |= 1 << 6;                                    /* 使能串口的DMA接收 */
                    break;
                }

                pro = __HAL_DMA_GET_COUNTER(&g_usart_tx_dma_handle);            /* 得到当前还剩余多少个数据 */
                len = SEND_BUF_SIZE;                                            /* 总长度 */
                pro = 1 - (pro / len);                                          /* 得到百分比 */
                pro *= 100;                                                     /* 扩大100倍 */
                lcd_show_num(30, 150, pro, 3, 16, BLUE);                        /* 显示传输进度 */
            }
            
            lcd_show_num(30, 150, 100, 3, 16, BLUE);                            /* 显示100% */
            lcd_show_string(30, 130, 200, 16, 16, "Transimit Finished!", BLUE); /* 提示传送完成 */
        }

        if (g_transfer_complete == 1)
        {
            g_transfer_complete = 0;
            printf("\n\r%s\n\r", rxbuffer);
          
            __HAL_DMA_DISABLE(&g_usart_rx_dma_handle);                          /* 关闭DMA */
            while (DMA1_Stream1->CR & 0X01);                                    /* 确保DMA可以被设置 */
            __HAL_DMA_SET_COUNTER(&g_usart_rx_dma_handle, USART_RX_DMA_TRANS);  /* 设置DMA需要传输的数据量 */
            __HAL_DMA_ENABLE(&g_usart_rx_dma_handle);                           /* 使能DMA，重新传输 */  
        }
        
        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();        /* LED0(绿灯)闪烁,提示系统正在运行 */
            i = 0;
        }
    }
}




