/**
 ****************************************************************************************************
 * @file        timer.c
 * @version     V1.0
 * @brief       定时器中断 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/LED/led.h"
#include "./BSP/TIMER/timer.h"
#include "./SYSTEM/usart/usart.h"


TIM_HandleTypeDef g_timx_handle;      /* 定时器TIMX句柄 */
TIM_HandleTypeDef g_tim7_handle;      /* 定时器TIM7句柄 */

/* 视频播放帧率控制全局变量 */
uint16_t g_avi_frame;                 /* 播放帧率 */
volatile uint8_t g_avi_frameup;       /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */


/**
 * @brief       定时器TIMX定时中断初始化函数
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
void timx_int_init(uint32_t arr, uint16_t psc)
{
    TIMX_INT_CLK_ENABLE();                                    /* 使能TIMX时钟 */

    g_timx_handle.Instance = TIMX_INT;                        /* 定时器TIMX */
    g_timx_handle.Init.Prescaler = psc;                       /* 设置预分频系数 */
    g_timx_handle.Init.CounterMode = TIM_COUNTERMODE_UP;      /* 递增计数模式 */
    g_timx_handle.Init.Period = arr;                          /* 设置自动重载值 */
    HAL_TIM_Base_Init(&g_timx_handle);
    
    HAL_TIM_Base_Start_IT(&g_timx_handle);                    /* 使能定时器TIMX和定时器TIMX更新中断 */
  
    HAL_NVIC_SetPriority(TIMX_INT_IRQn, 1, 3);                /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(TIMX_INT_IRQn);                        /* 使能TIMX中断 */
}

/**
 * @brief       基本定时器TIM7定时中断初始化函数
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
void btim_tim7_int_init(uint16_t arr, uint16_t psc)
{
    __HAL_RCC_TIM7_CLK_ENABLE();                              /* 使能TIM7时钟 */

    g_tim7_handle.Instance = TIM7;                            /* 定时器TIM7 */
    g_tim7_handle.Init.Prescaler = psc;                       /* 设置预分频系数 */
    g_tim7_handle.Init.CounterMode = TIM_COUNTERMODE_UP;      /* 递增计数模式 */
    g_tim7_handle.Init.Period = arr;                          /* 设置自动重载值 */
    g_tim7_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&g_tim7_handle);
    
    HAL_TIM_Base_Start_IT(&g_tim7_handle);                    /* 使能定时器TIM7和定时器TIM7更新中断 */
  
    HAL_NVIC_SetPriority(TIM7_IRQn, 0, 3);                    /* 设置中断优先级，抢占优先级0，子优先级3 */
    HAL_NVIC_EnableIRQ(TIM7_IRQn);                            /* 使能TIM7中断 */
}

/**
 * @brief       定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void TIMX_INT_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_timx_handle);                       /* 调用HAL库定时器中断公共处理函数 */
}

/**
 * @brief       基本定时器TIM7中断服务函数
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_tim7_handle);                       /* 调用HAL库定时器中断公共处理函数 */
}

/**
 * @brief       定时器更新中断回调函数
 * @param       htim  : 定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&g_timx_handle))               /* 定时器TIMX */
    {
        printf("frame:%dfps\r\n", g_avi_frame); /* 打印帧率 */
        g_avi_frame = 0;
        LED0_TOGGLE();                          /* LED0(绿灯)翻转 */
    }
    
    if (htim == (&g_tim7_handle))               /* 定时器TIM7 */
    {
        g_avi_frameup = 1;                      /* 标记时间到 */        
    }
}







