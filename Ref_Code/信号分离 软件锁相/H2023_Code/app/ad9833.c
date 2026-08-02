#include "gpio.h"
#include "ad9833.h"
#include "stm32f4xx_hal.h"

void AD9833_Init(void)
{
	
	//MX_GPIO_Init();
	HAL_Delay(100);
	AD9833_Set_Register(AD9833_ALL, AD9833_Reg_Cmd | AD9833_Reset);
	HAL_Delay(10);
}




void SPI_Write_16bit(uint16_t data)
{
	uint8_t i;
	uint16_t Send_Data = data;
	
	for(i=0;i<16;i++)
	{
		if(Send_Data&0x8000)
			AD9833_SDA_H;
		else
			AD9833_SDA_L;
		AD9833_SCLK_L;
		Send_Data<<=1;
		AD9833_SCLK_H;
	}
	
}

/**
 * @brief       设置AD9833寄存器的值
 * @param       ch：要设置的通道，reg：要设置寄存器的值
 * @retval      无
 */
void AD9833_Set_Register(uint16_t ch, uint16_t reg)
{
	if(ch == AD9833_ALL)
	{
		AD9833_CS1_L;	 
		AD9833_CS2_L;
		SPI_Write_16bit(reg);
		AD9833_CS1_H;
		AD9833_CS2_H;
	}
	else if(ch == AD9833_CH1)
	{
		AD9833_CS1_L;	 
		SPI_Write_16bit(reg);
		AD9833_CS1_H;
	}
	else if(ch == AD9833_CH2)
	{
		AD9833_CS2_L;	 
		SPI_Write_16bit(reg);
		AD9833_CS2_H;
	}
}

/**
 * @brief       设置AD9833的频率寄存器
 * @param       ch：要设置的通道，Type：波形类型 Frequency：频率的值
 * @retval      无
 */
void AD9833_Set_Frequency(uint16_t ch, uint16_t Type, uint32_t Frequency)
{
	uint16_t  Fre_H = AD9833_Reg_Freq0;
	uint16_t  Fre_L = AD9833_Reg_Freq0;
	
	Fre_H |= (Frequency & 0xFFFC000) >> 14 ;
	Fre_L |= (Frequency & 0x3FFF);

	AD9833_Set_Register(ch, AD9833_B28);
	AD9833_Set_Register(ch, Fre_L);
	AD9833_Set_Register(ch, Fre_H);
	AD9833_Set_Register(ch, AD9833_Reg_Cmd | AD9833_B28 | Type);
}

/**
 * @brief       设置AD9833的相位
 * @param       type：波形类型，Frequency：频率的值，Phase1：通道1的相位值，Phase2：通道2的相位值
 * @retval      无
 */
void AD9833_Set_Phase(uint16_t Type, uint32_t Frequency, float Phase1, float Phase2)
{
	uint16_t  Fre_H = AD9833_Reg_Freq0;
	uint16_t  Fre_L = AD9833_Reg_Freq0;
	uint16_t  Value1 = AD9833_Reg_Phase0;
	uint16_t  Value2 = AD9833_Reg_Phase0;
	uint16_t  P1 = 0;
	uint16_t  P2 = 0;
	
	if(Phase1 >= 360 || Phase1 == 0)
		P1 = 0;
	else
		P1 = (uint16_t)((float)Phase1/360.0*4095.0);
	
	if(Phase2 >= 360 || Phase2 == 0)
		P2 = 0;
	else
		P2 = (uint16_t)((float)Phase2/360.0*4095.0);
	
	Fre_H |= (Frequency & 0xFFFC000) >> 14 ;
	Fre_L |= (Frequency & 0x3FFF);
	Value1 |= P1 & 0x0FFF;
	Value2 |= P2 & 0x0FFF;

	AD9833_Set_Register(AD9833_ALL, AD9833_B28 | AD9833_Reset);
	AD9833_Set_Register(AD9833_ALL, Fre_L);
	AD9833_Set_Register(AD9833_ALL, Fre_H);
	AD9833_Set_Register(AD9833_CH1, Value1);
	AD9833_Set_Register(AD9833_CH2, Value2);
	AD9833_Set_Register(AD9833_ALL, AD9833_Reg_Cmd | AD9833_B28 | Type);
	
}

void AD9833_Set_FreqPhase(uint16_t ch, uint16_t Type, uint32_t Frequency, float Phase)
{
    uint16_t Fre_H = AD9833_Reg_Freq0 | ((Frequency & 0xFFFC000) >> 14);
    uint16_t Fre_L = AD9833_Reg_Freq0 | (Frequency & 0x3FFF);
    uint16_t PhaseReg = AD9833_Reg_Phase0;
    uint16_t P = 0;
    
    if(!(Phase >= 360 || Phase == 0))
        P = (uint16_t)(Phase / 360.0 * 4095.0);
    
    PhaseReg |= P & 0x0FFF;

    AD9833_Set_Register(ch, AD9833_B28 | AD9833_Reset);
    AD9833_Set_Register(ch, Fre_L);
    AD9833_Set_Register(ch, Fre_H);
    AD9833_Set_Register(ch, PhaseReg);
    AD9833_Set_Register(ch, AD9833_Reg_Cmd | AD9833_B28 | Type);
}
