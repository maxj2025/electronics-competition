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
 
#include "./BSP/LED/led.h"
#include "./BSP/PWM/pwm.h"


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
    uint8_t chy = TIMX_PWM_CHY;
    TIMX_PWM_CHY_GPIO_CLK_ENABLE();    /* TIMX 通道Y IO口时钟使能 */
    TIMX_PWM_CHY_CLK_ENABLE();         /* TIMX 时钟使能 */

    sys_gpio_set(TIMX_PWM_CHY_GPIO_PORT, TIMX_PWM_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* TIMX PWM CHY 引脚模式设置 */

    sys_gpio_af_set(TIMX_PWM_CHY_GPIO_PORT, TIMX_PWM_CHY_GPIO_PIN, TIMX_PWM_CHY_GPIO_AF);      /* IO口复用功能选择 必须设置对!! */

    TIMX_PWM->ARR = arr;       /* 设置计数器自动重载值 */
    TIMX_PWM->PSC = psc;       /* 设置预分频器  */
    TIMX_PWM->BDTR |= 1 << 15; /* 使能MOE位(仅TIM1/8/15/16/17 有此寄存器,必须设置MOE才能输出PWM), 其他定时器,
                                * 这个寄存器是无效的, 所以设置/不设置并不影响结果, 为了兼容这里统一设置MOE位
                                */

    if (chy <= 2)
    {
        TIMX_PWM->CCMR1 |= 6 << (4 + 8 * (chy - 1));   /* CH1/2 PWM模式1 */
        TIMX_PWM->CCMR1 |= 1 << (3 + 8 * (chy - 1));   /* CH1/2 预装载使能 */
    }
    else if (chy <= 4)
    {
        TIMX_PWM->CCMR2 |= 6 << (4 + 8 * (chy - 3));   /* CH3/4 PWM模式1 */
        TIMX_PWM->CCMR2 |= 1 << (3 + 8 * (chy - 3));   /* CH3/4 预装载使能 */
    }

    TIMX_PWM->CCER |= 1 << (4 * (chy - 1));            /* OCy 输出使能 */
    TIMX_PWM->CCER |= 1 << (1 + 4 * (chy - 1));        /* OCy 低电平有效 */
    TIMX_PWM->CR1 |= 1 << 7;   /* ARPE使能 */
    TIMX_PWM->CR1 |= 1 << 0;   /* 使能定时器TIMX */
}







