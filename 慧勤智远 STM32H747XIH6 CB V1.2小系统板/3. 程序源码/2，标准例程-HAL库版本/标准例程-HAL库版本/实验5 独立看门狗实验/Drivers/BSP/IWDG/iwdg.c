/**
 ****************************************************************************************************
 * @file        iwdg.c
 * @version     V1.0
 * @brief       独立看门狗 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/IWDG/iwdg.h"


IWDG_HandleTypeDef g_iwdg_handle;                      /* 独立看门狗句柄 */

/**
 * @brief       初始化独立看门狗 
 * @param       prer  : IWDG_PRESCALER_4~IWDG_PRESCALER_256,对应4~256分频
 *   @arg       分频因子 = 4 * 2^prer. 但最大值只能是256!
 * @param       rlr   : 自动重装载值,0~0XFFF. 
 * @note        时间计算(大概):Tout=((4 * 2^prer) * rlr) / 32 (ms). 
 * @retval      无
 */
void iwdg_init(uint32_t prer, uint16_t rlr)
{
    g_iwdg_handle.Instance = IWDG1;
    g_iwdg_handle.Init.Prescaler = prer;               /* 设置IWDG分频系数 */
    g_iwdg_handle.Init.Reload = rlr;                   /* 设置重装载值 */
    g_iwdg_handle.Init.Window = IWDG_WINDOW_DISABLE;   /* 关闭窗口功能 */
    HAL_IWDG_Init(&g_iwdg_handle);                     /* 初始化IWDG并使能 */
}

/**
 * @brief       喂独立看门狗
 * @param       无
 * @retval      无
 */
void iwdg_feed(void)
{
    HAL_IWDG_Refresh(&g_iwdg_handle);                  /* 喂狗 */
}





