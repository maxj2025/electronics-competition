/**
 ****************************************************************************************************
 * @file        ad9220.c
 * @author      科一电子
 * @version     V1.0
 * @date        2020-04-21
 * @brief       ad9220驱动程序
 ****************************************************************************************************
 */

#include "./BSP/AD9220/ad9220.h"

/**
 * @brief       ad9220引脚初始化
 * @param       无
 * @retval      无
 */
void ad9220_init(void)
{
	GPIO_InitTypeDef gpio_init_struct;

	__HAL_RCC_GPIOF_CLK_ENABLE();					  /* 开启GPIOF时钟使能 */
	__HAL_RCC_GPIOE_CLK_ENABLE();					  /* 开启GPIOC时钟使能 */	

	gpio_init_struct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
						   GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|
						   GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	gpio_init_struct.Mode = GPIO_MODE_INPUT;      	  /* 输入 */
	gpio_init_struct.Pull = GPIO_PULLUP;            /* 下拉 */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;    /* 高速 */
	HAL_GPIO_Init(GPIOF, &gpio_init_struct);       	  /* 初始化引脚 */
	
	gpio_init_struct.Pin = GPIO_PIN_0;				  /* CLK引脚 */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;      /* 输入 */
	gpio_init_struct.Pull = GPIO_PULLUP;              /* 上拉 */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;    /* 高速 */
	HAL_GPIO_Init(GPIOE, &gpio_init_struct);       	  /* 初始化引脚 */
}

