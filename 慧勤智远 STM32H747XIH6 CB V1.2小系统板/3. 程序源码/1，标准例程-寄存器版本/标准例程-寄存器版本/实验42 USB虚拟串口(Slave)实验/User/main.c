/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       USB虚拟串口(Slave) 实验
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
#include "./MALLOC/malloc.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"


USBD_HandleTypeDef USBD_Device;             /* USB Device处理结构体 */
extern volatile uint8_t g_device_state;     /* USB连接 情况 */


int main(void)
{  
    uint16_t len;
    uint16_t times = 0;
    uint8_t usbstatus = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USB Virtual USART TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "USB Connecting...", RED); /* 提示USB开始连接 */
    
    USBD_Init(&USBD_Device, &VCP_Desc, 0);                           /* 初始化USB */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);                /* 添加类 */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);        /* 为CDC类添加回调函数 */
    USBD_Start(&USBD_Device);                                        /* 开启USB */
    
    while (1)
    {
        if (usbstatus != g_device_state)   /* USB连接状态发生了改变 */
        {
            usbstatus = g_device_state;    /* 记录新的状态 */

            if (usbstatus == 1)
            {
                lcd_show_string(30, 110, 200, 16, 16, "USB Connected    ", RED); /* 提示USB连接成功 */
                LED1(0);    /* LED1(蓝灯)亮 */
            }
            else
            {
                lcd_show_string(30, 110, 200, 16, 16, "USB disConnected ", RED); /* 提示USB断开 */
                LED1(1);    /* LED1(蓝灯)灭 */
            }
        }

        if (g_usb_usart_rx_sta & 0x8000)
        {
            len = g_usb_usart_rx_sta & 0x3FFF;                                   /* 得到此次接收到的数据长度 */
            usb_printf("\r\n您发送的消息长度为:%d\r\n\r\n", len);
            cdc_vcp_data_tx(g_usb_usart_rx_buffer, len);;
            usb_printf("\r\n\r\n");                                              /* 插入换行 */
            g_usb_usart_rx_sta = 0;
        }
        else
        {
            times++;

            if (times % 5000 == 0)
            {
                usb_printf("\r\n慧勤智远 STM32开发板USB虚拟串口实验\r\n");
                usb_printf("慧勤智远@Waiken-Smart\r\n\r\n");
            }

            if (times % 200 == 0)
            {
                usb_printf("请输入数据,以回车键结束\r\n");
            }

            if (times % 30 == 0)
            {
                LED0_TOGGLE();  /* LED0(绿灯)闪烁,提示程序正在运行 */
            }
            
            delay_ms(10);
        }
    }
}




