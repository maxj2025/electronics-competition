/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       Template工程模板 新建工程使用-HAL库版本
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


void led_init(void);                                            /* LED初始化函数声明 */

int main(void)
{
    uint8_t t = 0;

    sys_cache_enable();                                         /* 使能L1-Cache */
    HAL_Init();                                                 /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);                         /* 设置时钟, 480Mhz */
    delay_init(480);                                            /* 延时初始化 */
    usart_init(115200);                                         /* 初始化USART */
    led_init();                                                 /* 初始化LED */
  
    while (1)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);     /* PA3置1，LED0(绿灯)灭 */ 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);   /* PB1置0，LED1(蓝灯)亮 */ 
        delay_ms(500);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   /* PA3置0，LED0(绿灯)亮 */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);     /* PB1置1，LED1(蓝灯)灭 */
        delay_ms(500); 
   
        printf("t:%d\r\n", t);
        t++;
    }
}

/**
 * @brief       初始化LED相关IO口, 并使能时钟
 * @param       无
 * @retval      无
 */
void led_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_GPIOA_CLK_ENABLE();                               /* PA口时钟使能 */
    __HAL_RCC_GPIOB_CLK_ENABLE();                               /* PB口时钟使能 */

    gpio_init_struct.Pin = GPIO_PIN_3;                          /* LED0引脚PA3 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;                /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;            /* 中速 */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);                    /* 初始化LED0引脚 */

    gpio_init_struct.Pin = GPIO_PIN_1;                          /* LED1引脚PB1 */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);                    /* 初始化LED1引脚 */
}




