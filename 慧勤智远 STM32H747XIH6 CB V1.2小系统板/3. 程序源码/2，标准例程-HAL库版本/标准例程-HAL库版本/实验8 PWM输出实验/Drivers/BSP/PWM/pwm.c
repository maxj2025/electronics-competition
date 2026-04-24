/**
 ****************************************************************************************************
 * @file        pwm.c
 * @version     V1.0
 * @brief       PWM输出 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/PWM/pwm.h"
#include "./BSP/LED/led.h"


TIM_HandleTypeDef g_timx_pwm_chy_handle;     /* 定时器TIMX句柄 */

/**
 * @brief       定时器TIMX 通道Y PWM输出 初始化函数（使用PWM模式1）
 * @note
 *              定时器的时钟来自APB1或APB2,如果D2PPREx对应于1或2分频，
 *              定时器的时钟为rcc_hclk1, 而rcc_hclk1为240M, 所以定时器时钟 = 240Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void timx_pwm_chy_init(uint32_t arr, uint16_t psc)
{
    TIM_OC_InitTypeDef timx_oc_pwm_chy = {0};                                            /* 定时器输出句柄 */
    
    g_timx_pwm_chy_handle.Instance = TIMX_PWM;                                           /* 定时器TIMX */
    g_timx_pwm_chy_handle.Init.Prescaler = psc;                                          /* 设置预分频系数 */
    g_timx_pwm_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;                         /* 递增计数模式 */
    g_timx_pwm_chy_handle.Init.Period = arr;                                             /* 设置自动重装载值 */
    HAL_TIM_PWM_Init(&g_timx_pwm_chy_handle);                                            /* 初始化PWM */

    timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;                                            /* 输出比较模式选择PWM模式1 */
    timx_oc_pwm_chy.Pulse = arr / 2;                                                     /* 设置比较值,此值用来确定占空比 
                                                                                          * 这里默认设置比较值为自动重装载值的一半,即占空比为50% 
                                                                                          */
    timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_LOW;                                     /* 设置捕获比较输出极性为低电平有效 */
    HAL_TIM_PWM_ConfigChannel(&g_timx_pwm_chy_handle, &timx_oc_pwm_chy, TIMX_PWM_CHY);   /* 配置TIMX通道Y */
    HAL_TIM_PWM_Start(&g_timx_pwm_chy_handle, TIMX_PWM_CHY);                             /* 开启TIMX通道Y PWM输出 */
}

/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
 * @note        此函数会被HAL_TIM_PWM_Init()函数调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIMX_PWM)                                 /* PWM输出定时器 */
    {
        GPIO_InitTypeDef gpio_init_struct;
      
        TIMX_PWM_CHY_GPIO_CLK_ENABLE();                             /* TIMX 通道Y IO口时钟使能 */
        TIMX_PWM_CHY_CLK_ENABLE();                                  /* TIMX 时钟使能 */

        gpio_init_struct.Pin = TIMX_PWM_CHY_GPIO_PIN;               /* 通道Y的GPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 快速 */
        gpio_init_struct.Alternate = TIMX_PWM_CHY_GPIO_AF;          /* 定时器TIMX通道Y的GPIO口复用功能选择 */
        HAL_GPIO_Init(TIMX_PWM_CHY_GPIO_PORT, &gpio_init_struct);   /* 初始化通道Y引脚 */
    }
}







