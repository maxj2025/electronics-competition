/**
 ****************************************************************************************************
 * @file        exti.c
 * @version     V1.0
 * @brief       外部中断 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/EXTI/exti.h"


/**
 * @brief       KEY0 外部中断服务程序
 * @param       无
 * @retval      无
 */
void KEY0_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY0_INT_GPIO_PIN);      /* 调用中断处理公用函数,清除KEY0所在中断线的中断标志位 */
    __HAL_GPIO_EXTI_CLEAR_IT(KEY0_INT_GPIO_PIN);      /* 退出时再清一次中断，避免按键抖动误触发 */
}

/**
 * @brief       WK_UP 外部中断服务程序
 * @param       无
 * @retval      无
 */
void WKUP_INT_IRQHandler(void)
{ 
    HAL_GPIO_EXTI_IRQHandler(WKUP_INT_GPIO_PIN);      /* 调用中断处理公用函数,清除WK_UP所在中断线的中断标志位，中断下半部在HAL_GPIO_EXTI_Callback执行 */
    __HAL_GPIO_EXTI_CLEAR_IT(WKUP_INT_GPIO_PIN);      /* HAL库默认先清中断再处理回调，退出时再清一次中断，避免按键抖动误触发 */
}

/**
 * @brief       外部中断回调函数
 * @param       GPIO_Pin: 中断引脚号
 * @note        在HAL库中所有的外部中断服务函数都会调用此函数
 * @retval      无
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    delay_ms(20);                      /* 消抖 */
  
    switch (GPIO_Pin)
    {
        case KEY0_INT_GPIO_PIN:
            if (KEY0 == 1)             /* 按键KEY0按下 */
            {
                LED0_TOGGLE();         /* LED0状态取反 */ 
            }
            break;

        case WKUP_INT_GPIO_PIN:
            if (WK_UP == 1)            /* 按键WK_UP按下 */ 
            {
                LED1_TOGGLE();         /* LED1状态取反 */
            }
            break;

        default : break;
    }
}

/**
 * @brief       外部中断初始化程序
 * @param       无
 * @retval      无
 */
void extix_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    /* 使能时钟 */
    KEY0_INT_GPIO_CLK_ENABLE();                             /* KEY0时钟使能 */
    WKUP_INT_GPIO_CLK_ENABLE();                             /* WKUP时钟使能 */  

    gpio_init_struct.Pin = KEY0_INT_GPIO_PIN;               /* KEY0引脚 */
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;            /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                  /* 下拉 */
    HAL_GPIO_Init(KEY0_INT_GPIO_PORT, &gpio_init_struct);   /* KEY0配置为上升沿触发中断 */
    
    gpio_init_struct.Pin = WKUP_INT_GPIO_PIN;               /* WKUP引脚 */
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;            /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                  /* 下拉 */
    HAL_GPIO_Init(WKUP_GPIO_PORT, &gpio_init_struct);       /* WKUP配置为上升沿触发中断 */

    HAL_NVIC_SetPriority(KEY0_INT_IRQn, 0, 2);              /* 抢占优先级0，子优先级2 */
    HAL_NVIC_EnableIRQ(KEY0_INT_IRQn);                      /* 使能KEY0所在中断线中断 */

    HAL_NVIC_SetPriority(WKUP_INT_IRQn, 1, 2);              /* 抢占优先级1，子优先级2 */
    HAL_NVIC_EnableIRQ(WKUP_INT_IRQn);                      /* 使能WK_UP所在中断线中断 */
}




