/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       Template工程模板 新建工程使用-寄存器版本
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


int main(void)
{
    uint8_t t = 0;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */

    RCC->AHB4ENR |= 3 << 0;                 /* PA/PB口时钟使能 */

    sys_gpio_set(GPIOA, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* 设置PA3 推挽输出模式 */

    sys_gpio_set(GPIOB, SYS_GPIO_PIN1,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* 设置PB1 推挽输出模式 */
  
    while (1)
    {
        sys_gpio_pin_set(GPIOA, SYS_GPIO_PIN3, 0);  /* PA3置0，LED0(绿灯)亮 */
        sys_gpio_pin_set(GPIOB, SYS_GPIO_PIN1, 1);  /* PB1置1，LED1(蓝灯)灭 */
        delay_ms(500);
        sys_gpio_pin_set(GPIOA, SYS_GPIO_PIN3, 1);  /* PA3置1，LED0(绿灯)灭 */
        sys_gpio_pin_set(GPIOB, SYS_GPIO_PIN1, 0);  /* PB1置0，LED1(蓝灯)亮 */
        delay_ms(500);
      
        printf("t:%d\r\n", t);
        t++;
    }
}




