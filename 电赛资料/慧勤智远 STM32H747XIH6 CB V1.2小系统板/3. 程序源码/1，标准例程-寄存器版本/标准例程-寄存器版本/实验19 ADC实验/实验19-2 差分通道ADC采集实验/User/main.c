/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       差分通道ADC采集 实验
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


int main(void)
{  
    uint16_t adcx;
    float temp;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    adc_init();                             /* 初始化ADC */
    adc_start_convert(ADC_ADCX_CHY);        /* 开始转换ADC */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "ADC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "ADC1_CH4_VAL: ", BLUE);
    lcd_show_string(30, 130, 200, 16, 16, "ADC1_CH4_VOL: 0.000V", BLUE); /* 先在固定位置显示小数点 */
    
    while (1)
    {
        adcx = adc_get_result_average(20);                 /* 获取ADC通道的转换值，20次取平均 */            
        lcd_show_xnum(142, 110, adcx, 5, 16, 0, BLUE);     /* 显示ADC采样后的原始值 */

        temp = (2 * 3.3f * adcx) / 0xffff - 3.3f;          /* 获取差分输入模式下计算后的带小数的实际电压值(16位分辨率)，比如3.1111 */      
        printf("\r ******  Channel IN4 differential mode  ******\r\n");
        printf(" ADC1 sampling data    = %d \r\n", adcx);
        printf(" ADC1 sampling voltage = %5.3fV \r\n", temp);
      
        if (temp < 0)
        {
            temp = -temp;
            lcd_show_string(134, 130, 16, 16, 16, "-", BLUE);   /* 显示负号 */
        } 
        else
        {
            lcd_show_string(134, 130, 16, 16, 16, " ", BLUE);   /* 无符号 */
        }
        
        adcx = temp;                                       /* 赋值整数部分给adcx变量，因为adcx为uint16_t整形 */
        lcd_show_xnum(142, 130, adcx, 1, 16, 0, BLUE);     /* 显示电压值的整数部分，3.1111的话，这里就是显示3 */

        temp -= adcx;                                      /* 把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111 */
        temp *= 1000;                                      /* 小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数 */
        lcd_show_xnum(158, 130, temp, 3, 16, 0X80, BLUE);  /* 显示小数部分（前面转换为了整形显示），这里显示的就是111 */

        LED0_TOGGLE();
        delay_ms(200);
    }
}




