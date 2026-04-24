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
#include "./BSP/RTC/rtc.h"


/**
 * @brief       低功耗模式下的按键初始化(用于唤醒待机模式)
 * @param       无
 * @retval      无
 */
void pwr_wkup_key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    PWR_WKUP_GPIO_CLK_ENABLE();                               /* WK_UP引脚时钟使能 */

    gpio_init_struct.Pin = PWR_WKUP_GPIO_PIN;                 /* WK_UP引脚 */
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;              /* 中断,上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                    /* 下拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;            /* 高速 */
    HAL_GPIO_Init(PWR_WKUP_GPIO_PORT, &gpio_init_struct);     /* WK_UP引脚初始化 */

    HAL_NVIC_SetPriority(PWR_WKUP_INT_IRQn, 2, 2);            /* 抢占优先级2，子优先级2 */
    HAL_NVIC_EnableIRQ(PWR_WKUP_INT_IRQn); 
}

/**
 * @brief       WK_UP按键 外部中断服务程序
 * @param       无
 * @retval      无
 */
void PWR_WKUP_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(PWR_WKUP_GPIO_PIN);              /* 调用中断处理函数,清除中断标志位 */
}

/**
 * @brief       外部中断回调函数
 * @param       GPIO_Pin:中断线引脚
 * @note        此函数会被HAL_GPIO_EXTI_IRQHandler()调用
 * @retval      无
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PWR_WKUP_GPIO_PIN)
    {
        /* HAL_GPIO_EXTI_IRQHandler()函数已经清除了中断标志位，所以进了回调函数可以不做任何事 */
    }
}

/**
 * @brief       进入待机模式
 * @param       无
 * @retval      无
 */
void pwr_enter_standby(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(PWR_WKUP_GPIO_PIN);        /* 清除WKUP上的中断标志位 */

    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);          /* 禁止唤醒 */

    __HAL_RCC_BACKUPRESET_FORCE();                      /* 复位备份区域 */
    HAL_PWR_EnableBkUpAccess();                         /* 后备区域访问使能 */  

    __HAL_RTC_WRITEPROTECTION_DISABLE(&g_rtc_handle);   /* 关闭RTC写保护 */

    /* 关闭RTC相关中断，可能在RTC实验打开了 */
    __HAL_RTC_WAKEUPTIMER_DISABLE_IT(&g_rtc_handle, RTC_IT_WUT);
    __HAL_RTC_TIMESTAMP_DISABLE_IT(&g_rtc_handle, RTC_IT_TS);
    __HAL_RTC_ALARM_DISABLE_IT(&g_rtc_handle, RTC_IT_ALRA | RTC_IT_ALRB);

    /* 清除RTC相关中断标志位 */
    __HAL_RTC_ALARM_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_ALRAF | RTC_FLAG_ALRBF);
    __HAL_RTC_TIMESTAMP_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_TSF); 
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_WUTF);

    __HAL_RCC_BACKUPRESET_RELEASE();                    /* 备份区域复位结束 */
    __HAL_RTC_WRITEPROTECTION_ENABLE(&g_rtc_handle);    /* 使能RTC写保护 */

    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2);           /* 使能WK_UP引脚的唤醒功能 */
    PWR->WKUPEPR &= ~(3 << 18);                         /* 清除WKUPPUPD2原来的设置 */
    PWR->WKUPEPR |= 2 << 18;                            /* WKUPPUPD2 = 10, PA2下拉 */
    PWR->WKUPCR |= 0X3F << 0;                           /* 清除所有WKUP唤醒引脚标志 */
    LCD_PWREN(0);                                       /* 关闭LCD_5V */

    HAL_PWR_EnterSTANDBYMode();                         /* 进入待机模式 */
}





