/**
 ****************************************************************************************************
 * @file        pwr.c
 * @version     V1.0
 * @brief       低功耗模式 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/PWR/pwr.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"


/**
 * @brief       WK_UP按键 外部中断服务程序
 * @param       无
 * @retval      无
 */
void PWR_WKUP_INT_IRQHandler(void)
{
    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;   /* 清除WKUP所在中断线的中断标志位 */
}

/**
 * @brief       低功耗模式下的按键初始化(用于唤醒待机模式)
 * @param       无
 * @retval      无
 */
void pwr_wkup_key_init(void)
{
    PWR_WKUP_GPIO_CLK_ENABLE();     /* WKUP引脚时钟使能 */

    sys_gpio_set(PWR_WKUP_GPIO_PORT, PWR_WKUP_GPIO_PIN,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PD);    /* WKUP引脚模式设置,下拉输入 */

    sys_nvic_ex_config(PWR_WKUP_GPIO_PORT, PWR_WKUP_GPIO_PIN, SYS_GPIO_RTIR);   /* WKUP配置为上升沿触发中断 */ 
    sys_nvic_init(2, 2, PWR_WKUP_INT_IRQn, 2);                                  /* 抢占优先级2，子优先级2，组2 */
}

/**
 * @brief       进入待机模式
 * @param       无
 * @retval      无
 */
void pwr_enter_standby(void)
{
    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;  /* 清除WKUP上的中断标志位 */
    
    /* STM32F4/F7/H7,当开启了RTC相关中断后,必须先关闭RTC中断,再清中断标志位
     * 再进入待机模式才可以正常唤醒,否则会有问题.
     */
    PWR->CR1 |= 1 << 8;                /* 后备区域写使能 */
    
    /* 关闭RTC寄存器写保护 */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(0XF << 12);           /* 关闭RTC所有中断 */
    RTC->ISR &= ~(0X3F << 8);          /* 清除所有RTC中断标志 */

    RTC->WPR = 0xFF;                   /* 使能RTC寄存器写保护 */

    /* 关闭VOS0
     * H747芯片（V版本）设置为Scale0模式时,在进入低功耗模式之前,必须先退出Scale0模式！！ 
     */
    sys_clock_set(160, 5, 2, 4);       /* 设置时钟,400Mhz,降频 */
    RCC->APB4ENR |= 1 << 1;            /* 使能SYSCFG时钟 */
    SYSCFG->PWRCR &= ~(1 << 0);        /* 设置ODEN位为0,关闭Overdrive模式,此时进入Scale1模式, VCORE = 1.2V */
    
    while ((PWR->D3CR & (1 << 13)) == 0);   /* 等待电压稳定 */
    
    LCD_PWREN(0);                      /* 关闭LCD_5V */
   
    sys_standby();                     /* 进入待机模式 */
}








