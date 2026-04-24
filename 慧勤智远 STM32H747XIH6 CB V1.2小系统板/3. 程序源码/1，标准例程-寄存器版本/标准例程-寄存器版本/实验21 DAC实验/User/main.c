/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       DAC输出 实验
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
#include "./USMART/usmart.h"
#include "./BSP/KEY/key.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/DAC/dac.h"


int main(void)
{  
    uint16_t adcx;
    float temp;
    uint8_t t = 0;
    uint16_t dacval = 0;
    uint8_t key;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    usmart_init(240);	                      /* 初始化USMART */    
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    adc_init();                             /* 初始化ADC */
  
    dac_init(2);                            /* 初始化DAC1_OUT2通道 */

    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DAC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "WK_UP:+  KEY0:-", RED);
    
    lcd_show_string(30, 150, 200, 16, 16, "DAC VAL:", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "DAC VOL:0.000V", BLUE);
    lcd_show_string(30, 190, 200, 16, 16, "ADC VOL:0.000V", BLUE);
    
    while (1)
    {
        t++;
        key = key_scan(0);          /* 按键扫描 */

        if (key == WKUP_PRES)       /* 按下WK_UP */
        {
            if (dacval < 4000)dacval += 200;

            DAC1->DHR12R2 = dacval; /* DAC输出增大200 */
        }
        else if (key == KEY0_PRES)  /* 按下KEY0 */ 
        {
            if (dacval > 200)dacval -= 200;
            else dacval = 0;

            DAC1->DHR12R2 = dacval; /* DAC输出减少200 */
        }

        if (t == 30 || key == KEY0_PRES || key == WKUP_PRES)    /* WKUP/KEY0按下了,或者定时时间到了 */
        {
            adcx = DAC1->DHR12R2;                               /* 获取DAC1_OUT2的输出数据 */
            lcd_show_xnum(94, 150, adcx, 4, 16, 0, BLUE);       /* 显示DAC输出值 */
            printf("\r\nDAC VAL:%d\r\n", adcx);
            
            temp = (float)adcx * (3.3 / 4096);                  /* 得到DAC电压值 */
            adcx = temp;
            lcd_show_xnum(94, 170, temp, 1, 16, 0, BLUE);       /* 显示电压值整数部分 */
            
            temp -= adcx;
            temp *= 1000;
            lcd_show_xnum(110, 170, temp, 3, 16, 0X80, BLUE);   /* 显示电压值的小数部分 */
            printf("DAC VOL:%d.%03dV\r\n", adcx, (uint16_t)temp);
            
            adcx = adc_get_result_average(ADC_ADCX_CHY, 20);    /* 得到ADC通道Y的转换结果 */
            temp = (float)adcx * (3.3 / 65536);                 /* 得到ADC电压值(adc是16bit的) */
            adcx = temp;
            lcd_show_xnum(94, 190, temp, 1, 16, 0, BLUE);       /* 显示电压值整数部分 */
            
            temp -= adcx;
            temp *= 1000;
            lcd_show_xnum(110, 190, temp, 3, 16, 0X80, BLUE);   /* 显示电压值的小数部分 */
            printf("ADC VOL:%d.%03dV\r\n", adcx, (uint16_t)temp);
            
            LED0_TOGGLE();  /* LED0(绿灯)闪烁 */
            t = 0;
        }

        delay_ms(10);
    }
}




