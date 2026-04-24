/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       定时器输入捕获 实验
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
#include "./BSP/CAPTURE/capture.h"


extern uint8_t  g_timxchy_cap_sta;          /* 输入捕获状态 */
extern uint32_t g_timxchy_cap_val;          /* 输入捕获值 */


int main(void)
{  
    uint32_t temp = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    timx_pwm_chy_init(500 - 1, 240 - 1);    /* 设置1Mhz的计数频率,2Khz的PWM */
    timx_cap_chy_init(0xFFFF, 240 - 1);     /* 以1Mhz的频率计数 捕获 */
  
    while (1)
    {
        TIMX_PWM_CHY_CCRX++;
      
        if (TIMX_PWM_CHY_CCRX == 300)
        {
            TIMX_PWM_CHY_CCRX = 0; 	        /* 改变TIMX_PWM_CHY_CCRX的值，从而改变PWM占空比 */ 
        }
        
        if (g_timxchy_cap_sta & 0x80)       /* 成功捕获到了一次高电平 */
        {
            temp = g_timxchy_cap_sta & 0x3F;
            temp *= 65536;                  /* 溢出时间总和 */
            temp += g_timxchy_cap_val;      /* 得到总的高电平时间 */
            printf("HIGH:%d us\r\n", temp); /* 打印总的高电平时间 */
            g_timxchy_cap_sta = 0;          /* 清零，开启下一次捕获 */
        }

        delay_ms(10);
    }
}




