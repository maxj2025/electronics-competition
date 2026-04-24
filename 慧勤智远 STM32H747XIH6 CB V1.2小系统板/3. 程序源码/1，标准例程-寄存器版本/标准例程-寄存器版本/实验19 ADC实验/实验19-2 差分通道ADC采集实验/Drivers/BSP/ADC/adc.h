/**
 ****************************************************************************************************
 * @file        adc.h
 * @version     V1.0
 * @brief       ADC 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* ADC及引脚 定义 */

#define ADC_ADCX_CHY_GPIO_PORT              GPIOC
#define ADC_ADCX_CHY_DIFF_PIN1              SYS_GPIO_PIN4
#define ADC_ADCX_CHY_DIFF_PIN2              SYS_GPIO_PIN5
#define ADC_ADCX_CHY_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 2; }while(0)   /* PC口时钟使能 */

#define ADC_ADCX                            ADC1 
#define ADC_ADCX_CHY                        4                                       /* 通道Y,  0 <= Y <= 19 */ 
#define ADC_ADCX_CHY_CLK_ENABLE()           do{ RCC->AHB1ENR |= 1 << 5; }while(0)   /* ADC1/2 时钟使能 */

/******************************************************************************************/

void adc_init(void);                                                /* ADC初始化 */
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime); /* ADC通道设置采样时间 */
void adc_start_convert(uint8_t ch);                                 /* ADC开始转换 */
uint32_t adc_get_result(void);                                      /* 获得某个通道的ADC转换结果 */
uint32_t adc_get_result_average(uint8_t times);                     /* 得到某个通道给定次数采样的平均值 */

#endif 






