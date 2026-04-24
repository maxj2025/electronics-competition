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
    delay_ms(20);                       /* 消抖 */
    EXTI_D1->PR1 = KEY0_INT_GPIO_PIN;   /* 清除KEY0所在中断线的中断标志位 */

    if (KEY0 == 1)                      /* 按键KEY0按下 */ 
    {
        LED0_TOGGLE();                  /* LED0状态取反 */
    }
}

/**
 * @brief       WK_UP 外部中断服务程序
 * @param       无
 * @retval      无
 */
void WKUP_INT_IRQHandler(void)
{ 
    delay_ms(20);                       /* 消抖 */
    EXTI_D1->PR1 = WKUP_INT_GPIO_PIN;   /* 清除WKUP所在中断线的中断标志位 */

    if (WK_UP == 1)                     /* 按键WK_UP按下 */ 
    {
        LED1_TOGGLE();                  /* LED1状态取反 */
    }
}

/**
 * @brief       外部中断初始化程序
 * @param       无
 * @retval      无
 */
void extix_init(void)
{
    key_init();                                                                 /* 初始化按键对应的IO口 */	
    sys_nvic_ex_config(KEY0_INT_GPIO_PORT, KEY0_INT_GPIO_PIN, SYS_GPIO_RTIR);   /* KEY0配置为上升沿触发中断 */
    sys_nvic_ex_config(WKUP_INT_GPIO_PORT, WKUP_INT_GPIO_PIN, SYS_GPIO_RTIR);   /* WKUP配置为上升沿触发中断 */

    sys_nvic_init(0, 2, KEY0_INT_IRQn, 2); /* 抢占优先级0，子优先级2，组2 */
    sys_nvic_init(1, 2, WKUP_INT_IRQn, 2); /* 抢占优先级1，子优先级2，组2 */
}









