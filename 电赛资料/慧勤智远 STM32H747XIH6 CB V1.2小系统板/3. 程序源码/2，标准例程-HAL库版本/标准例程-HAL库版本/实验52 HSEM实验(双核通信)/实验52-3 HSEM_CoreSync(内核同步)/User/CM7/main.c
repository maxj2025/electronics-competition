/**
 ****************************************************************************************************
 * @file        Cortex-M7的main.c
 * @version     V1.0
 * @brief       HSEM内核同步 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"


#define HSEM_ID_0  0   /* 使用信号量0 */


int main(void)
{
    uint32_t timeout;
    uint16_t adcx;
    float temp;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
  
    /* 系统初始化结束后CPU1(Cortex-M7)通过HSEM通知的方式唤醒CPU2(Cortex-M4) */
    __HAL_RCC_HSEM_CLK_ENABLE();            /* HSEM时钟使能 */

    HAL_HSEM_FastTake(HSEM_ID_0);           /* 通过1步读方式获取信号量 */ 
  
    HAL_HSEM_Release(HSEM_ID_0, 0);         /* 释放信号量以唤醒CPU2 */
  
    timeout = 0xFFFF;
  
    /* 等待D2域时钟准备就绪，即CPU2从停止模式中唤醒 */
    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && timeout)
    {
        timeout--;       
    }
    
    usart_init(115200);                     /* 初始化USART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    adc_init();                             /* 初始化ADC */
 
    /* CM7完成它的任务后通知CM4 */
    HAL_HSEM_FastTake(HSEM_ID_0);           /* 快速获取信号量 */   
    HAL_HSEM_Release(HSEM_ID_0, 0);         /* 释放信号量以通知CPU2 */

    /* CM7等待CM4完成它的任务并锁定硬件信号量0 */
    while (HAL_HSEM_IsSemTaken(HSEM_ID_0) == 0);
 
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "ADC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "ADC1_CH17_VAL:", BLUE);
    lcd_show_string(30, 130, 200, 16, 16, "ADC1_CH17_VOL:0.000V", BLUE); /* 先在固定位置显示小数点 */
    
    while (1)
    {               
        /* CM7等待CM4完成ADC转换并锁定硬件信号量0 */
        while (HAL_HSEM_IsSemTaken(HSEM_ID_0) != 0)
        {
            adcx = HAL_ADC_GetValue(&g_adc_handle);            /* 获取最近一次ADC规则通道的转换结果 */
            lcd_show_xnum(142, 110, adcx, 5, 16, 0, BLUE);     /* 显示ADC采样后的原始值 */
            printf("\r\n ADC sampling value   = %d \r\n", adcx);
          
            temp = (float)adcx * (3.3 / 65536);                /* 获取计算后的带小数的实际电压值(16位分辨率)，比如3.1111 */
            adcx = temp;                                       /* 赋值整数部分给adcx变量，因为adcx为uint16_t整形 */
            lcd_show_xnum(142, 130, adcx, 1, 16, 0, BLUE);     /* 显示电压值的整数部分，3.1111的话，这里就是显示3 */
            printf(" ADC sampling voltage = %5.3fV \r\n", temp);

            temp -= adcx;                                      /* 把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111 */
            temp *= 1000;                                      /* 小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数 */
            lcd_show_xnum(158, 130, temp, 3, 16, 0X80, BLUE);  /* 显示小数部分（前面转换为了整形显示），这里显示的就是111 */

            LED0_TOGGLE();                                     /* LED0(绿灯)闪烁 */
        }
    }
}




