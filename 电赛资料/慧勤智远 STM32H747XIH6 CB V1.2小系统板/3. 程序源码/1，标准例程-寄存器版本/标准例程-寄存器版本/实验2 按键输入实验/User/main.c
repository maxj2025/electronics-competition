/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       按键输入 实验
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
#include "./BSP/KEY/key.h"


int main(void)
{  
    uint8_t key;

    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    led_init();							                /* 初始化LED */   
    key_init();                             /* 初始化按键 */
    LED0(0);                                /* 点亮LED0(绿灯) */ 
  
    while (1)
    {
        key = key_scan(0);                  /* 按键扫描得到键值:0:不支持连续按;1:支持连续按 */
      
        if (key)
        {
            switch (key)
            {
                case KEY0_PRES:             /* 控制LED0(GREEN)翻转 */
                    LED0_TOGGLE();          /* LED0状态取反 */ 
                    delay_ms(100);
                    break;

                case WKUP_PRES:             /* 控制LED1(BLUE)翻转 */ 
                    LED1_TOGGLE();          /* LED1状态取反 */
                    delay_ms(100);
                    break;
            } 
        }
        else
        {
            delay_ms(10);
        }
    }
}




