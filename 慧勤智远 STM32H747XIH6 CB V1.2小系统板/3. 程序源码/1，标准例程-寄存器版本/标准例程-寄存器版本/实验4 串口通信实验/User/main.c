/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       串口通信 实验
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


int main(void)
{  
    uint8_t t;
		uint8_t len;	
	  uint16_t times = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
  
    while (1)
    {
        if (g_usart_rx_sta & 0x8000)        /* 接收完了一次数据 */
				{					    
            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
            printf("\r\n您发送的消息为:\r\n");
          
            for (t = 0; t < len; t++)
            {
                while((USART_UX->ISR & 0X40) == 0);  /* 等待发送完成 */
              
                USART_UX->TDR = g_usart_rx_buf[t];   /* 发送接收到的数据 */                
            }
            
            printf("\r\n\r\n");             /* 插入换行 */  
            g_usart_rx_sta = 0;
				}
        else
				{
            times++;
          
            if (times % 5000 == 0)
            {
                printf("\r\n慧勤智远 STM32H7开发板 串口实验\r\n");
                printf("慧勤智远@Waiken-Smart\r\n\r\n\r\n");
            }
            
            if (times % 200 == 0)
            {
                printf("请输入数据,以回车键结束\r\n");  
            }
            
            if (times % 30 == 0) 
            {
                LED0_TOGGLE(); /* 闪烁LED0(绿灯),提示系统正在运行. */
            }
            
            delay_ms(10);   
				} 
    }
}




