/**
 ****************************************************************************************************
 * @file        wwdg.c
 * @version     V1.0
 * @brief       窗口看门狗 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/WWDG/wwdg.h"
#include "./BSP/LED/led.h"


WWDG_HandleTypeDef g_wwdg_handle;   /* 窗口看门狗句柄 */

/**
 * @brief       初始化窗口看门狗
 * @param       tr    : T[6:0],计数器值
 * @param       wr    : W[6:0],窗口值
 * @param       fprer :分频系数,范围:WWDG_PRESCALER_1~WWDG_PRESCALER_128,表示1~128分频
 * @note        Fwwdg = PCLK3 / (4096 * 2^fprer). 一般PCLK3 = 120Mhz
 *              溢出时间 = (4096 * 2^fprer) * (tr - 0X3F) / PCLK3
 *              假设fprer = 4, tr = 7f, PCLK3 = 120Mhz
 *              则溢出时间 = 4096 * 16 * 64 / 120Mhz = 34.96ms
 * @retval      无
 */
void wwdg_init(uint8_t tr, uint8_t wr, uint32_t fprer)
{
    g_wwdg_handle.Instance = WWDG1;
    g_wwdg_handle.Init.Prescaler = fprer;           /* 设置分频系数 */
    g_wwdg_handle.Init.Window = wr;                 /* 设置窗口值 */
    g_wwdg_handle.Init.Counter = tr;                /* 设置计数器值 */
    g_wwdg_handle.Init.EWIMode = WWDG_EWI_ENABLE;   /* 使能窗口看门狗提前唤醒中断 */
    HAL_WWDG_Init(&g_wwdg_handle);                  /* 初始化WWDG */
}

/**
 * @brief       WWDG底层驱动，时钟配置，中断配置
 * @note        此函数会被HAL_WWDG_Init()调用
 * @param       hwwdg:窗口看门狗句柄
 * @retval      无
 */
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg)
{
    __HAL_RCC_WWDG_CLK_ENABLE();                    /* 使能窗口看门狗时钟 */

    HAL_NVIC_SetPriority(WWDG_IRQn, 2, 3);          /* 抢占优先级2，子优先级为3 */
    HAL_NVIC_EnableIRQ(WWDG_IRQn);                  /* 使能窗口看门狗中断 */
}

/**
 * @brief       窗口看门狗中断服务程序
 * @param       无
 * @retval      无
 */
void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&g_wwdg_handle);            /* 调用WWDG共用中断处理函数 */
}

/**
 * @brief       窗口看门狗提前唤醒中断服务回调函数
 * @note        此函数会被HAL_WWDG_IRQHandler()调用
 * @param       hwwdg  : 窗口看门狗句柄
 * @retval      无
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
    HAL_WWDG_Refresh(&g_wwdg_handle);               /* 更新窗口看门狗计数器的值 */
    LED1_TOGGLE();                                  /* LED1(蓝灯)闪烁 */
}




