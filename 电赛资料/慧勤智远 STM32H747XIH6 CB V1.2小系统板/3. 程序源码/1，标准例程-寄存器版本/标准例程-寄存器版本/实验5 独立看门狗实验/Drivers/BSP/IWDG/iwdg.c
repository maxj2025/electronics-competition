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


/**
 * @brief       初始化独立看门狗 
 * @param       prer: 分频数:0~7(只有低3位有效!)
 *   @arg       分频因子 = 4 * 2^prer. 但最大值只能是256!
 * @param       rlr: 自动重装载值,0~0XFFF. 
 * @note        时间计算(大概):Tout=((4 * 2^prer) * rlr) / 32 (ms). 
 * @retval      无
 */
void iwdg_init(uint8_t prer, uint16_t rlr)
{
    IWDG1->KR = 0X5555; /* 使能对IWDG->PR和IWDG->RLR的写访问 */
    IWDG1->PR = prer;   /* 设置分频系数 */
    IWDG1->RLR = rlr;   /* 设置重载寄存器 */
    IWDG1->KR = 0XAAAA; /* 将IWDG->RLR寄存器的值重装载到看门狗计数器 */
    IWDG1->KR = 0XCCCC; /* 使能看门狗 */
}

/**
 * @brief       喂独立看门狗
 * @param       无
 * @retval      无
 */
void iwdg_feed(void)
{
    IWDG1->KR = 0XAAAA; /* 喂狗 */
}







