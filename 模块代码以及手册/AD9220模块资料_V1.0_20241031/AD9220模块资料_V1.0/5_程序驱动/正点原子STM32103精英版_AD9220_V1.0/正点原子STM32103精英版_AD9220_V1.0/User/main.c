/**
 ****************************************************************************************************
 * @file        main.c
 * @author      科一电子
 * @version     V1.0
 * @date        2020-04-21
 * @brief       AD9220测试实验
 * 连接说明
		精英板				  AD9220模块
		 5V			-->			+5V
		 GND		-->			GND
		 PE0		-->			CLK
		 PF0		-->			D0
		 PF1		-->			D1
		 PF2		-->			D2
		 PF3		-->			D3
		 PF4		-->			D4
		 PF5		-->			D5
		 PF6		-->			D6
		 PF7		-->			D7
		 PF12		-->			D8
		 PF13		-->			D9
		 PF14		-->			D10
		 PF15		-->			D11
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/AD9220/ad9220.h"

#define Sampling_Number  	10    	// 采样点数
uint16_t AD9220_Data[Sampling_Number];	// 数据缓存区

int main(void)
{
	uint8_t buf[40];
	float A_Data;
	uint16_t D_Data;
	uint16_t num;
	uint16_t *pb = AD9220_Data;

    HAL_Init();                                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);                 /* 设置时钟, 72Mhz */
    delay_init(72);                                     /* 延时初始化 */
    usart_init(115200);                                 /* 串口初始化为115200 */
	key_init();											/* 初始化按键 */
    led_init();                                         /* 初始化LED */
    lcd_init();                                         /* 初始化LCD */
	ad9220_init();										/* AD9220引脚初始化 */
    g_point_color = RED;

	lcd_clear(WHITE);
	lcd_show_string(10, 10 , 240, 24, 24, "AD9220 Test", RED);
	lcd_show_string(10, 50 , 240, 16, 16, "Digital:          ", RED);
	lcd_show_string(10, 70 , 240, 16, 16, "Analog :          mV", RED);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);
    while (1)
    {
		pb = AD9220_Data;
		for(num=0;num<Sampling_Number;num++)
		{
			GPIOE->BSRR = GPIO_PIN_0;	// 拉高 
			*(pb++) = (uint16_t)GPIOF->IDR;		// 读取IO口数据
			GPIOE->BSRR = (uint32_t)GPIO_PIN_0 << 16u;	// 拉低
		}
		
		// 由于AD9220的传输延时3个时钟，前3个时钟的数据舍弃掉
		D_Data = AD9220_Data[9]&0x00FF;
		D_Data |= (AD9220_Data[9]&0xF000)>>4;
		A_Data = ((float)D_Data*10000/4095 - 5000)/0.961;
		sprintf((char*)buf,"0x%04X", D_Data);
		lcd_show_string(74, 50, 240, 16, 16, (char *)buf, BLUE);
		sprintf((char*)buf,"%.2f", A_Data);
		lcd_show_string(74, 70, 240, 16, 16, (char *)buf, BLUE);
		LED0_TOGGLE(); /*红灯闪烁*/
        delay_ms(500);
    }
}
