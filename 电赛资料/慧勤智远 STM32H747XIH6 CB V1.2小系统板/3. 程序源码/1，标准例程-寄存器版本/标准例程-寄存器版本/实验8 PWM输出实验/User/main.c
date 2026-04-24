/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       定时器PWM输出 实验
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
#include "./BSP/PWM/pwm.h"


int main(void)
{  
    uint16_t ledrpwmval = 0;
    uint8_t dir = 1;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();							                /* 初始化LED */   
    timx_pwm_chy_init(500 - 1, 240 - 1);    /* 设置1Mhz的计数频率,2Khz的PWM */
  
    while (1)
    {
        delay_ms(10);

        if (dir)
        {
            ledrpwmval++;                   /* dir==1 ledrpwmval递增 */
        }
        else 
        {
            ledrpwmval--;                   /* dir==0 ledrpwmval递减 */
        }

        if (ledrpwmval > 300)
        {
            dir = 0;                        /* ledrpwmval到达300后，方向改为递减 */
        }
        
        if (ledrpwmval == 0)
        {
            dir = 1;                        /* ledrpwmval递减到0后，方向改为递增 */	
        }

        TIMX_PWM_CHY_CCRX = ledrpwmval;     /* 修改比较值控制占空比 */ 
    }
}




