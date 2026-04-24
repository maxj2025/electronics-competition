/******************************************************************************
 * Copyright(c) 2024 Levetop Semiconductor Co., Ltd. All rights reserved
 * @file      LT758_Lib.c
 * @author    Levetop TFT Controller Application Team
 * @version   V1.0.0
 * @date      2024-12-30
 * @brief     LT758x Function Libraries
 *******************************************************************************/

#include "./BSP/LT758_Lib/LT758_Lib.h"
#include "./BSP/LT758/LT758.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"


unsigned char w25q256 = 0;      /* 0:W25Q128; 1:W25Q256; */
unsigned char Flash_Type = 0;   /* 0:norflash; 1:nandflash; */

unsigned char CCLK; // LT758 Core Clock
unsigned char MCLK; // SDRAM Clock
unsigned char SCLK; // LCD Scan Clock

//-----------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_HW_Reset                                                           */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: LT758x Hardware Reset                                                    */
/*---------------------------------------------------------------------------------------*/
void LT758_HW_Reset(void)
{
  GPIO_InitTypeDef gpio_init_struct;

  LCD_RST_GPIO_CLK_ENABLE();                               /* LCD_RST引脚时钟使能 */

  gpio_init_struct.Pin = LCD_RST_GPIO_PIN;                 /* LCD_RST引脚(定义在if_port.h) */
  gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;             /* 推挽输出 */
  gpio_init_struct.Pull = GPIO_PULLUP;                     /* 上拉 */
  gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;           /* 快速 */
  HAL_GPIO_Init(LCD_RST_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_RST引脚 */
  
  /* LT758复位 */
  LCD_RST(1);	
  Delay_ms(10);
  LCD_RST(0);
  Delay_ms(100);				   
  LCD_RST(1);	
  Delay_ms(100);  
}

/*---------------------------------------------------------------------------------------*/
/* Function:    Check_IC_ready                                                           */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     LT758x Status (0x50)                                                     */
/* Description: Check if LT758x is ready to work by reading the status register bit 1	 */
/*              Operation mode status                                                    */
/*				0: Normal operation state   											 */
/*				1: Inhibit operation state												 */
/*				Inhibit operation state means (1) internal reset event keep running;     */
/*				(2) initial display still running; or (3)chip enters power saving state. */
/*---------------------------------------------------------------------------------------*/
unsigned char Check_IC_ready(void)
{
	unsigned char temp = 0;
	unsigned short i;
	while(1)
	{
		for (i = 0; i < 1000; i++)
		{
			temp = LCD_StatusRead();
			if (temp == 0x50)
				return temp;
		}
		delay_ms(100);
		LT758_HW_Reset();
		delay_ms(100);
	}
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_SDRAM_initial                                                      */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: LT758x SDRAM initialization                                              */
/*---------------------------------------------------------------------------------------*/
void LT758_SDRAM_initial(unsigned char mclk)
{
	unsigned char tempReg = 0;
	unsigned short sdram_itv;

	LCD_RegisterWrite(0xe0, 0x29);
	LCD_RegisterWrite(0xe1, 0x03); // CAS:2=0x02--768CAS:3=0x03--758
	sdram_itv = (64000 * mclk) / 4096;

	LCD_RegisterWrite(0xe2, sdram_itv);
	LCD_RegisterWrite(0xe3, sdram_itv >> 8);

	tempReg = LCD_RegisterRead(0xE4);

	tempReg |= (0x00 | 0x04);

	LCD_RegisterWrite(0xE4, tempReg); /* Enable SDRAM Timing Parameter Register */
	LCD_RegisterWrite(0xE0, 0x0);
	LCD_RegisterWrite(0xE1, (8 << 4) | 7);
	LCD_RegisterWrite(0xE2, (4 << 4) | 0);
	LCD_RegisterWrite(0xE3, (2 << 4) | 6);
	LCD_RegisterWrite(0xE4, tempReg & 0xFB); /* Disable SDRAM Timing Parameter Register */
											 // printf("0xE4:%x \r\n", LCD_RegisterRead(0xE4));

	tempReg = LCD_RegisterRead(0xE4);
	LCD_RegisterWrite(0xE4, tempReg | 0x01);
	Check_SDRAM_Ready();
	Delay_ms(2);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    Set_LCD_Panel                                                            */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set LCD panel related parameters                                         */
/*---------------------------------------------------------------------------------------*/
void Set_LCD_Panel(void)
{
#if Display_16bpp
	//**[01h]**//
	TFT_16bit();

	//**[02h]**//
	RGB_16b_16bpp();

	Memory_16bpp_Mode();

#else
	//**[01h]**//
	TFT_24bit();

	//**[02h]**//
	RGB_16b_24bpp_mode1();

	Memory_24bpp_Mode();

#endif

	MemWrite_Left_Right_Top_Down();
	// MemWrite_Down_Top_Left_Right();

#if STM32_FMC_16
	Host_Bus_16bit(); // Host Bus 16bit
#else
	Host_Bus_8bit();  // Host Bus 8bit
#endif

	//**[03h]**//
	Graphic_Mode();
	Memory_Select_SDRAM();

	PCLK_Falling();   // REG[12h]: Falling edge
	// PCLK_Rising();

	VSCAN_T_to_B();   // REG[12h]: Scan from top to bottom
	// VSCAN_B_to_T();   //From bottom to top

	PDATA_Set_RGB();  // REG[12h]:Select RGB output
	// PDATA_Set_RBG();
	// PDATA_Set_GRB();
	// PDATA_Set_GBR();
	// PDATA_Set_BRG();
	// PDATA_Set_BGR();

	HSYNC_Low_Active();  // REG[13h]:
	// HSYNC_High_Active();

	VSYNC_Low_Active();  // REG[13h]:
	// VSYNC_High_Active();

	DE_High_Active();    // REG[13h]:
	// DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
	LCD_Horizontal_Non_Display(LCD_HBPD);
	LCD_HSYNC_Start_Position(LCD_HFPD);
	LCD_HSYNC_Pulse_Width(LCD_HSPW);
	LCD_Vertical_Non_Display(LCD_VBPD);
	LCD_VSYNC_Start_Position(LCD_VFPD);
	LCD_VSYNC_Pulse_Width(LCD_VSPW);

	Memory_XY_Mode(); // Block mode (X-Y coordination addressing)
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_initial                                                            */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Initialize LT758 SDRAM and LCD panel related parameters                  */
/*---------------------------------------------------------------------------------------*/
void LT758_initial(void)
{
	LT758_SDRAM_initial(MCLK);
	Check_SDRAM_Ready();
	Set_LCD_Panel();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    PS758_PLL                                                                */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Initialize PLL Clock                                                     */
/*---------------------------------------------------------------------------------------*/
void PS758_PLL(void)
{
	unsigned int PCLK_M = 11;
	unsigned char PCLK_N = 1;
	unsigned char PCLK_OD = 2;
	unsigned char PCLK_Ext_ODR = 0;
	
	// 23-138MHZ,27-162MHZ,30-180MHZ,32-192MHZ;
	// unsigned int MCLK_M = 30; // only for nor flash
	unsigned int MCLK_M = 27; 	 // for nand flash
	unsigned char MCLK_N = 1;
	unsigned char MCLK_OD = 1;
	
	// 23-138MHZ,27-162MHZ,30-180MHZ,32-192MHZ;
	// unsigned int CCLK_M = 28; // only for nor flash
	unsigned int CCLK_M = 25; 	 // for nand flash
	unsigned char CCLK_N = 1;
	unsigned char CCLK_OD = 1;

	unsigned char tempReg;
	unsigned long temp = 0;
	unsigned int temp1 = 0;

	MCLK = MCLK_M * 6;
	
	temp = (LCD_HBPD + LCD_HFPD + LCD_HSPW + LCD_XSIZE_TFT) * (LCD_VBPD + LCD_VFPD + LCD_VSPW + LCD_YSIZE_TFT) * 60;
	temp1 = temp / 1000000;
	PCLK_M = temp1 / 3;
	if ((temp1 % 3) == 2)
		PCLK_M++;
	
	/* PCLK setting ,Maximum 100MHz */
	{
		tempReg = ((PCLK_OD & 0x03) << 6) | ((PCLK_Ext_ODR & 0x01) << 5) | ((PCLK_N & 0x0F) << 1);
		LCD_RegisterWrite(0x05, tempReg);
		LCD_RegisterWrite(0x06, PCLK_M & 0xFF);
	}

	/* MCLK setting ,Maximum 180MHz */
	{
		tempReg = ((MCLK_OD & 0x03) << 6) | ((MCLK_N & 0x0F) << 1);
		LCD_RegisterWrite(0x07, tempReg);
		LCD_RegisterWrite(0x08, MCLK_M & 0xFF);
	}

	/* CCLK setting ,Maximum 170MHz */
	{
		tempReg = ((CCLK_OD & 0x03) << 6) | ((CCLK_N & 0x0F) << 1);
		LCD_RegisterWrite(0x09, tempReg);
		LCD_RegisterWrite(0x0A, CCLK_M & 0xFF);
	}

	tempReg = LCD_RegisterRead(0x00);
	LCD_RegisterWrite(0x00, tempReg | 0x80);

	while ((LCD_RegisterRead(0x00) & 0x80) == 0)
		delay_us(1);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Init                                                               */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Initialize LT758x                                                        */
/*---------------------------------------------------------------------------------------*/
void LT758_Init(void)
{
	unsigned char status = 0;
  lcd_power_on();   // 打开LCD模块电源
	delay_ms(100);	  // Delay for LT758 power on
	LT758_HW_Reset(); // LT758 Hardware Reset
	status = Check_IC_ready(); //0x50
	printf("LCD_StatusRead():%x\r\n", status);
	PS758_PLL();
	delay_ms(100);
	LT758_initial();
	write_758_reg(0x01, read_758_reg(0x01) & 0xBF); // #WAIT Enable
	DMA_Read_3BH();									// Set read command as 3Bh (Flash Fast Read command)
}

//--------------------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU8_8bpp_Memory_Write                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 8bpp data to LT758x SDRAM by sending 8bits data at a time     */
/*---------------------------------------------------------------------------------------*/
void MPU8_8bpp_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned char *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
		}
	}

	Check_Mem_WR_FIFO_Empty();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU8_16bpp_Memory_Write                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 16bpp data to LT758x SDRAM by sending 8bits data at a time    */
/*---------------------------------------------------------------------------------------*/
void MPU8_16bpp_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned char *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
		}
	}
	Check_Mem_WR_FIFO_Empty();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU8_24bpp_Memory_Write                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 24bpp data to LT758x SDRAM by sending 8bits data at a time    */
/*---------------------------------------------------------------------------------------*/
void MPU8_24bpp_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned char *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*data);
			data++;
		}
	}
	Check_Mem_WR_FIFO_Empty();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU16_16bpp_Memory_Write                                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 16bpp data to LT758x SDRAM by sending 16bits data at a time   */
/*---------------------------------------------------------------------------------------*/
void MPU16_16bpp_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned short *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
		}
	}
	Check_Mem_WR_FIFO_Empty();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU16_24bpp_Mode1_Memory_Write                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 24bpp data to LT758x SDRAM by sending 16bits data at a time   */
/*              -> Mode1                                                                 */
/*---------------------------------------------------------------------------------------*/
void MPU16_24bpp_Mode1_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned short *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w / 2; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;

			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;

			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
		}
	}
	Check_Mem_WR_FIFO_Empty();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    MPU16_24bpp_Mode2_Memory_Write                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  x: x coordinate                                                      */
/*                  y: y coordinate                                                      */
/*                  w: data width                                                        */
/*                  h: data height                                                       */
/*              *data: the first data address                                            */
/* Returns:     None                                                                     */
/* Description: MCU writes 24bpp data to LT758x SDRAM by sending 16bits data at a time   */
/*              -> Mode2                                                                 */
/*---------------------------------------------------------------------------------------*/
void MPU16_24bpp_Mode2_Memory_Write(
	unsigned short x, 
	unsigned short y, 
	unsigned short w, 
	unsigned short h, 
	const unsigned short *data)
{
	unsigned short i, j;
	Graphic_Mode();
	Active_Window_XY(x, y);
	Active_Window_WH(w, h);
	Goto_Pixel_XY(x, y);
	LCD_CmdWrite(0x04);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
		}
	}
	Check_Mem_WR_FIFO_Empty();
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------- Draw Line -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLine                                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*                X1: X1 coordinate                                                      */
/*                Y1: Y1 coordinate                                                      */
/*                X2: X2 coordinate                                                      */
/*                Y2: Y2 coordinate                                                      */
/*              LineColor: Line color                                                    */
/* Returns:     None                                                                     */
/* Description: Draw a line to connect (X1, Y1) and (X2, Y2) in the designated color     */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawLine(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned long LineColor)
{
#if Display_16bpp
	Foreground_color_65k(LineColor);
#else
	Foreground_color_16M(LineColor);
#endif
	Line_Start_XY(X1, Y1);
	Line_End_XY(X2, Y2);
	Start_Line();
	Check_2D_Busy();
}

/*------------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLine_Width                                                        */
/*                                                                                          */
/* Parameters:                                                                              */
/*                X1: X1 coordinate                                                         */
/*                Y1: Y1 coordinate                                                         */
/*                X2: X2 coordinate                                                         */
/*                Y2: Y2 coordinate                                                         */
/*              LineColor: Line color                                                       */
/*                  Width: Line width                                                       */
/* Returns:     None                                                                        */
/* Description: Draw a line to connect (X1, Y1) and (X2, Y2) in the designated color and    */
/*              width  																		*/
/*------------------------------------------------------------------------------------------*/
void LT758_DrawLine_Width(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned long LineColor, 
	unsigned short Width)
{
	unsigned short i = 0;
	signed short x = 0, y = 0;
	double temp = 0;
	x = X2 - X1;
	y = Y2 - Y1;
	if (x == 0)
		temp = 2;
	else
		temp = -((double)y / (double)x);
	if (temp >= -1 && temp <= 1)
	{
		while (Width--)
		{
			LT758_DrawLine(X1, Y1 + i, X2, Y2 + i, LineColor);
			i++;
		}
	}

	else
	{
		while (Width--)
		{
			LT758_DrawLine(X1 + i, Y1, X2 + i, Y2, LineColor);
			i++;
		}
	}
}

//------------------------------------- Draw Circle -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircle                                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  XCenter: Circle center X coordinate                                  */
/*                  YCenter: Circle center Y coordinate                                  */
/*                        R: Radius                                                      */
/*              CircleColor: Circle color                                                */
/* Returns:     None                                                                     */
/* Description: Draw a circle at the center of (XCenter, YCenter) with R radius and the  */
/*              designated color                                                         */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircle(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short R, 
	unsigned long CircleColor)
{
#if Display_16bpp
	Foreground_color_65k(CircleColor);
#else
	Foreground_color_16M(CircleColor);
#endif
	Circle_Center_XY(XCenter, YCenter);
	Circle_Radius_R(R);
	Start_Circle_or_Ellipse();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircle_Fill                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  XCenter: Circle center X coordinate                                  */
/*                  YCenter: Circle center Y coordinate                                  */
/*                        R: Radius                                                      */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid circle at the center of (XCenter, YCenter) with R radius    */
/*              and the designated color                                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircle_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Circle_Center_XY(XCenter, YCenter);
	Circle_Radius_R(R);
	Start_Circle_or_Ellipse_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircle_Width                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  XCenter: Circle center X coordinate                                  */
/*                  YCenter: Circle center Y coordinate                                  */
/*                        R: Radius                                                      */
/*              CircleColor: Circle color                                                */
/*          ForegroundColor: Filled color                                                */
/*                    Width: Line width                                                  */
/* Returns:     None                                                                     */
/* Description: Draw a circle at the center of (XCenter, YCenter) with R radius, the     */
/*              designated line color, line width, and filled color                      */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircle_Width(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short R, 
	unsigned long CircleColor, 
	unsigned long ForegroundColor, 
	unsigned short Width)
{
	LT758_DrawCircle_Fill(XCenter, YCenter, R + Width, CircleColor);
	LT758_DrawCircle_Fill(XCenter, YCenter, R, ForegroundColor);
}

//------------------------------------- Draw Ellipse -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawEllipse                                                        */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  XCenter: Circle center X coordinate                                  */
/*                  YCenter: Circle center Y coordinate                                  */
/*                      X_R: x Radius                                                    */
/*                      Y_R: y Radius                                                    */
/*              EllipseColor: Ellipse line color                                         */
/* Returns:     None                                                                     */
/* Description: Draw an ellipse at the center of (XCenter,YCenter) with radius           */
/*              (X_R, Y_R) and the designated line color                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawEllipse(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long EllipseColor)
{
#if Display_16bpp
	Foreground_color_65k(EllipseColor);
#else
	Foreground_color_16M(EllipseColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Circle_or_Ellipse();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawEllipse_Fill                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Circle center X coordinate                              */
/*                      YCenter: Circle center Y coordinate                              */
/*                          X_R: x Radius                                                */
/*                          Y_R: y Radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid ellipse at the center of (XCenter,YCenter), with radius     */
/*              (X_R, Y_R) and the designated filled color                               */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawEllipse_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Circle_or_Ellipse_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawEllipse_Width                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Circle center X coordinate                              */
/*                      YCenter: Circle center Y coordinate                              */
/*                          X_R: x Radius                                                */
/*                          Y_R: y Radius                                                */
/*                 EllipseColor: Line color                                              */
/*              ForegroundColor: Filled color                                            */
/*                        Width: Line width                                              */
/* Returns:     None                                                                     */
/* Description: Draw a solid ellipse at the center of (XCenter,YCenter),with radius      */
/*              (X_R, Y_R), the designated filled color, line width, and line color      */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawEllipse_Width(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long EllipseColor, 
	unsigned long ForegroundColor, 
	unsigned short Width)
{
	LT758_DrawEllipse_Fill(XCenter, YCenter, X_R + Width, Y_R + Width, EllipseColor);
	LT758_DrawEllipse_Fill(XCenter, YCenter, X_R, Y_R, ForegroundColor);
}

//------------------------------------- Draw Square -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawSquare                                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                       X1: Left-top X1 coordinate                                      */
/*                       Y1: Left-top Y1 coordinate                                      */
/*                       X2: Right-bottom X2 coordinate                                  */
/*                       Y2: Right-bottom Y2 coordinate                                  */
/*              SquareColor: Line color                                                  */
/* Returns:     None                                                                     */
/* Description: Draw a hollow square from left-top (X1, Y1) to right-bottom (X2, Y2)     */
/*              in the designated line color                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawSquare(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned long SquareColor)
{
#if Display_16bpp
	Foreground_color_65k(SquareColor);
#else
	Foreground_color_16M(SquareColor);
#endif
	Square_Start_XY(X1, Y1);
	Square_End_XY(X2, Y2);
	Start_Square();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawSquare_Fill                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: Left-top X1 coordinate                                  */
/*                           Y1: Left-top Y1 coordinate                                  */
/*                           X2: Right-bottom X2 coordinate                              */
/*                           Y2: Right-bottom Y2 coordinate                              */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid square from left-top (X1, Y1) to right-bottom (X2, Y2)      */
/*              in the designated filled color                                           */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawSquare_Fill(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Square_Start_XY(X1, Y1);
	Square_End_XY(X2, Y2);
	Start_Square_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawSquare_Width                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: Left-top X1 coordinate                                  */
/*                           Y1: Left-top Y1 coordinate                                  */
/*                           X2: Right-bottom X2 coordinate                              */
/*                           Y2: Right-bottom Y2 coordinate                              */
/*                  SquareColor: Line color                                              */
/*              ForegroundColor: Filled color                                            */
/*                        Width: Line width                                              */
/* Returns:     None                                                                     */
/* Description: Draw a solid square with frame from left-top (X1, Y1) to right-bottom    */
/*              (X2, Y2) in the designated filled color, line color, and width           */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawSquare_Width(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned long SquareColor, 
	unsigned long ForegroundColor, 
	unsigned short Width)
{
	LT758_DrawSquare_Fill(X1 - Width, Y1 - Width, X2 + Width, Y2 + Width, SquareColor);
	LT758_DrawSquare_Fill(X1, Y1, X2, Y2, ForegroundColor);
}

//------------------------------------- Draw Rounded Square -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircleSquare                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: Left-top X1 coordinate                                  */
/*                           Y1: Left-top Y1 coordinate                                  */
/*                           X2: Right-bottom X2 coordinate                              */
/*                           Y2: Right-bottom Y2 coordinate                              */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              CircleSquareColor: Line color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a hollow rounded square from left-top (X1, Y1) to right-bottom      */
/*              (X2, Y2) in the designated line color                                    */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircleSquare(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CircleSquareColor)
{
#if Display_16bpp
	Foreground_color_65k(CircleSquareColor);
#else
	Foreground_color_16M(CircleSquareColor);
#endif
	Square_Start_XY(X1, Y1);
	Square_End_XY(X2, Y2);
	Circle_Square_Radius_RxRy(X_R, Y_R);
	Start_Circle_Square();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircleSquare_Fill                                              */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: Left-top X1 coordinate                                  */
/*                           Y1: Left-top Y1 coordinate                                  */
/*                           X2: Right-bottom X2 coordinate                              */
/*                           Y2: Right-bottom Y2 coordinate                              */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid rounded square from left-top (X1, Y1) to right-bottom       */
/*              (X2, Y2) in the designated filled color                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircleSquare_Fill(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Square_Start_XY(X1, Y1);
	Square_End_XY(X2, Y2);
	Circle_Square_Radius_RxRy(X_R, Y_R);
	Start_Circle_Square_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCircleSquare_Width                                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                             X1: Left-top X1 coordinate                                */
/*                             Y1: Left-top Y1 coordinate                                */
/*                             X2: Right-bottom X2 coordinate                            */
/*                             Y2: Right-bottom Y2 coordinate                            */
/*                            X_R: X radius                                              */
/*                            Y_R: Y radius                                              */
/*              CircleSquareColor: Line color                                            */
/*                ForegroundColor: Filled color                                          */
/*                          Width: Line width                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid rounded square with frame from left-top (X1, Y1) to         */
/*              right-bottom (X2, Y2) in the designated filled color, line color, and    */
/*              width 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawCircleSquare_Width(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CircleSquareColor, 
	unsigned long ForegroundColor, 
	unsigned short Width)
{
	LT758_DrawCircleSquare_Fill(X1 - Width, Y1 - Width, X2 + Width, Y2 + Width, X_R, Y_R, CircleSquareColor);
	LT758_DrawCircleSquare_Fill(X1, Y1, X2, Y2, X_R, Y_R, ForegroundColor);
}

//------------------------------------- Draw Triangle -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawTriangle                                                       */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                TriangleColor: Line color                                              */
/* Returns:     None                                                                     */
/* Description: Draw a hollow triangle with 3 designated points in the designated line   */
/*				color 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawTriangle(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned long TriangleColor)
{
#if Display_16bpp
	Foreground_color_65k(TriangleColor);
#else
	Foreground_color_16M(TriangleColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Start_Triangle();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawTriangle_Fill                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid triangle with 3 designated points in the designated filled  */
/*				color																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawTriangle_Fill(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Start_Triangle_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawTriangle_Frame                                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                TriangleColor: Line color                                              */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid triangle with frame by 3 designated points in the designated*/
/*              line color and filled color                                              */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawTriangle_Frame(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned long TriangleColor, 
	unsigned long ForegroundColor)
{
	LT758_DrawTriangle_Fill(X1, Y1, X2, Y2, X3, Y3, ForegroundColor);
	LT758_DrawTriangle(X1, Y1, X2, Y2, X3, Y3, TriangleColor);
}

//------------------------------------- Draw Curve -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLeftUpCurve                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/* Returns:     None                                                                     */
/* Description: Draw a hollow curve on the left-up side with the center point            */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated line     */
/*              color   																 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawLeftUpCurve(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor)
{
#if Display_16bpp
	Foreground_color_65k(CurveColor);
#else
	Foreground_color_16M(CurveColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Left_Up_Curve();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLeftDownCurve                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/* Returns:     None                                                                     */
/* Description: Draw a hollow curve on the left-down side with the center point          */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated line     */
/*              color							 										 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawLeftDownCurve(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor)
{
#if Display_16bpp
	Foreground_color_65k(CurveColor);
#else
	Foreground_color_16M(CurveColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Left_Down_Curve();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawRightUpCurve                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/* Returns:     None                                                                     */
/* Description: Draw a hollow curve on the right-up side with the center point           */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated line 	 */
/*				color																     */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawRightUpCurve(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor)
{
#if Display_16bpp
	Foreground_color_65k(CurveColor);
#else
	Foreground_color_16M(CurveColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Right_Up_Curve();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawRightDownCurve                                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/* Returns:     None                                                                     */
/* Description: Draw a hollow curve on the right-down side with the center point         */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated line 	 */
/* 				color																     */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawRightDownCurve(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor)
{
#if Display_16bpp
	Foreground_color_65k(CurveColor);
#else
	Foreground_color_16M(CurveColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Right_Down_Curve();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_SelectDrawCurve                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/*                     Dir: Curve direction                                              */
/* Returns:     None                                                                     */
/* Description: Draw a hollow curve of designated direction with the center point        */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated line     */
/*              color																     */
/*---------------------------------------------------------------------------------------*/
void LT758_SelectDrawCurve(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor, 
	unsigned short Dir)
{
	switch (Dir)
	{
	case 0:
		LT758_DrawLeftDownCurve(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 1:
		LT758_DrawLeftUpCurve(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 2:
		LT758_DrawRightUpCurve(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 3:
		LT758_DrawRightDownCurve(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	default:
		break;
	}
}

//------------------------------------- Draw Solid Curve -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLeftUpCurve_Fill                                               */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Curve center X coordinate                               */
/*                      YCenter: Curve center Y coordinate                               */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid curve on the left-up side with the center point             */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated filled   */
/*				color 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawLeftUpCurve_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Left_Up_Curve_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawLeftDownCurve_Fill                                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Curve center X coordinate                               */
/*                      YCenter: Curve center Y coordinate                               */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid curve on the left-down side with the center point           */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated filled   */
/*				color																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawLeftDownCurve_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Left_Down_Curve_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawRightUpCurve_Fill                                              */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Curve center X coordinate                               */
/*                      YCenter: Curve center Y coordinate                               */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid curve on the right-up side with the center point            */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated filled 	 */
/*				color 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawRightUpCurve_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Right_Up_Curve_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawRightDownCurve_Fill                                            */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Curve center X coordinate                               */
/*                      YCenter: Curve center Y coordinate                               */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid curve on the right-down side with the center point          */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated filled   */
/*				color 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawRightDownCurve_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Ellipse_Center_XY(XCenter, YCenter);
	Ellipse_Radius_RxRy(X_R, Y_R);
	Start_Right_Down_Curve_Fill();
	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_SelectDrawCurve_Fill                                               */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 XCenter: Curve center X coordinate                                    */
/*                 YCenter: Curve center Y coordinate                                    */
/*                     X_R: X radius                                                     */
/*                     Y_R: Y radius                                                     */
/*              CurveColor: Line color                                                   */
/*                     Dir: Curve direction                                              */
/* Returns:     None                                                                     */
/* Description: Draw a solid curve of designated direction with the center point         */
/*              (XCenter, YCenter) and radius of X_R and Y_R, in the designated filled   */
/*				color 																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_SelectDrawCurve_Fill(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned long CurveColor, 
	unsigned short Dir)
{
	switch (Dir)
	{
	case 0:
		LT758_DrawLeftDownCurve_Fill(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 1:
		LT758_DrawLeftUpCurve_Fill(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 2:
		LT758_DrawRightUpCurve_Fill(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	case 3:
		LT758_DrawRightDownCurve_Fill(XCenter, YCenter, X_R, Y_R, CurveColor);
		break;
	default:
		break;
	}
}

//------------------------------------- Draw Quadrilateral -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawQuadrilateral                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                           X4: 4th point, X4 coordinate                                */
/*                           Y4: 4th point, Y4 coordinate                                */
/*              ForegroundColor: Line color                                              */
/* Returns:     None                                                                     */
/* Description: Draw a hollow quadrilateral by 4 designated points in the designated     */
/*              line color                                                               */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawQuadrilateral(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned short X4, 
	unsigned short Y4, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Ellipse_Radius_RxRy(X4, Y4);
	LCD_CmdWrite(0x67);
	LCD_DataWrite(0x8d);
	Check_Busy_Draw();

	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawQuadrilateral_Fill                                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                           X4: 4th point, X4 coordinate                                */
/*                           Y4: 4th point, Y4 coordinate                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid quadrilateral by 4 designated points in the designated      */
/*              filled color                                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawQuadrilateral_Fill(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned short X4, 
	unsigned short Y4, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Ellipse_Radius_RxRy(X4, Y4);
	LCD_CmdWrite(0x67);
	LCD_DataWrite(0xa7);
	Check_Busy_Draw();

	Check_2D_Busy();
}

//------------------------------------- Draw Pentagon -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawPentagon                                                       */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                           X4: 4th point, X4 coordinate                                */
/*                           Y4: 4th point, Y4 coordinate                                */
/*                           X5: 5th point, X5 coordinate                                */
/*                           Y5: 5th point, Y5 coordinate                                */
/*              ForegroundColor: Line color                                              */
/* Returns:     None                                                                     */
/* Description: Draw a hollow pentagon by 5 designated points in the designated line     */
/*              color																     */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawPentagon(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned short X4, 
	unsigned short Y4, 
	unsigned short X5, 
	unsigned short Y5, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Ellipse_Radius_RxRy(X4, Y4);
	Ellipse_Center_XY(X5, Y5);
	LCD_CmdWrite(0x67);
	LCD_DataWrite(0x8f);
	Check_Busy_Draw();

	Check_2D_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawPentagon_Fill                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: 1st point, X1 coordinate                                */
/*                           Y1: 1st point, Y1 coordinate                                */
/*                           X2: 2nd point, X2 coordinate                                */
/*                           Y2: 2nd point, Y2 coordinate                                */
/*                           X3: 3rd point, X3 coordinate                                */
/*                           Y3: 3rd point, Y3 coordinate                                */
/*                           X4: 4th point, X4 coordinate                                */
/*                           Y4: 4th point, Y4 coordinate                                */
/*                           X5: 5th point, X5 coordinate                                */
/*                           Y5: 5th point, Y5 coordinate                                */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid pentagon by 5 designated points in the designated filled    */
/*  			color																	 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawPentagon_Fill(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned short X4, 
	unsigned short Y4, 
	unsigned short X5, 
	unsigned short Y5, 
	unsigned long ForegroundColor)
{
#if Display_16bpp
	Foreground_color_65k(ForegroundColor);
#else
	Foreground_color_16M(ForegroundColor);
#endif
	Triangle_Point1_XY(X1, Y1);
	Triangle_Point2_XY(X2, Y2);
	Triangle_Point3_XY(X3, Y3);
	Ellipse_Radius_RxRy(X4, Y4);
	Ellipse_Center_XY(X5, Y5);
	LCD_CmdWrite(0x67);
	LCD_DataWrite(0xa9);
	Check_Busy_Draw();

	Check_2D_Busy();
}

//------------------------------------- Draw Cylinder -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawCylinder                                                       */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      XCenter: Ellipse center X coordinate                             */
/*                      YCenter: Ellipse center Y coordinate                             */
/*                          X_R: X radius                                                */
/*                          Y_R: Y radius                                                */
/*                            H: Height                                                  */
/*                CylinderColor: Line color                                              */
/*              ForegroundColor: Filled color                                            */
/* Returns:     None                                                                     */
/* Description: Draw a solid framed cylinder with the ellipse center point			 	 */
/*				(XCenter, YCenter) and radius of X_R and Y_R, in the designated filled   */
/*				color and line color 													 */
/*---------------------------------------------------------------------------------------*/
unsigned char LT758_DrawCylinder(
	unsigned short XCenter, 
	unsigned short YCenter, 
	unsigned short X_R, 
	unsigned short Y_R, 
	unsigned short H, 
	unsigned long CylinderColor, 
	unsigned long ForegroundColor)
{
	if (YCenter < H)
		return 1;

	// Ellipse - bottom
	LT758_DrawEllipse_Fill(XCenter, YCenter, X_R, Y_R, ForegroundColor);
	LT758_DrawEllipse(XCenter, YCenter, X_R, Y_R, CylinderColor);

	// Rectangle - middle
	LT758_DrawSquare_Fill(XCenter - X_R, YCenter - H, XCenter + X_R, YCenter, ForegroundColor);

	// Ellipse - top
	LT758_DrawEllipse_Fill(XCenter, YCenter - H, X_R, Y_R, ForegroundColor);
	LT758_DrawEllipse(XCenter, YCenter - H, X_R, Y_R, CylinderColor);

	LT758_DrawLine(XCenter - X_R, YCenter, XCenter - X_R, YCenter - H, CylinderColor);
	LT758_DrawLine(XCenter + X_R, YCenter, XCenter + X_R, YCenter - H, CylinderColor);

	return 0;
}

//------------------------------------- Draw Cube -----------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DrawQuadrangular                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                             X1: 1st point, X1 coordinate                              */
/*                             Y1: 1st point, Y1 coordinate                              */
/*                             X2: 2nd point, X2 coordinate                              */
/*                             Y2: 2nd point, Y2 coordinate                              */
/*                             X3: 3rd point, X3 coordinate                              */
/*                             Y3: 3rd point, Y3 coordinate                              */
/*                             X4: 4th point, X4 coordinate                              */
/*                             Y4: 4th point, Y4 coordinate                              */
/*                             X5: 5th point, X5 coordinate                              */
/*                             Y5: 5th point, Y5 coordinate                              */
/*                             X6: 6th point, X6 coordinate                              */
/*                             Y6: 6th point, Y6 coordinate                              */
/*              QuadrangularColor: Line color                                            */
/*                ForegroundColor: Filled color                                          */
/* Returns:     None                                                                     */
/* Description: Draw a solid cube by 6 designated points in the designated filled color  */ 
/*              and line color 															 */
/*---------------------------------------------------------------------------------------*/
void LT758_DrawQuadrangular(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X2, 
	unsigned short Y2, 
	unsigned short X3, 
	unsigned short Y3, 
	unsigned short X4, 
	unsigned short Y4, 
	unsigned short X5, 
	unsigned short Y5, 
	unsigned short X6, 
	unsigned short Y6, 
	unsigned long QuadrangularColor, 
	unsigned long ForegroundColor)
{
	LT758_DrawSquare_Fill(X1, Y1, X5, Y5, ForegroundColor);
	LT758_DrawSquare(X1, Y1, X5, Y5, QuadrangularColor);

	LT758_DrawQuadrilateral_Fill(X1, Y1, X2, Y2, X3, Y3, X4, Y4, ForegroundColor);
	LT758_DrawQuadrilateral(X1, Y1, X2, Y2, X3, Y3, X4, Y4, QuadrangularColor);

	LT758_DrawQuadrilateral_Fill(X3, Y3, X4, Y4, X5, Y5, X6, Y6, ForegroundColor);
	LT758_DrawQuadrilateral(X3, Y3, X4, Y4, X5, Y5, X6, Y6, QuadrangularColor);
}

//------------------------------------------------- Make Table -------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_MakeTable                                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*                           X1: Starting point, X1 coordinate                           */
/*                           Y1: Starting point, Y1 coordinate                           */
/*                            W: Width                                                   */
/*                            H: Height                                                  */
/*                         Line: Number of column                                        */
/*                          Row: Number of row                                           */
/*                   TableColor: frame color                      C1                     */
/*                    ItemColor: Filled color for item column     C2                     */
/*              ForegroundColor: Filled color for content column  C3                     */
/*                       width1: Inner frame width                                       */
/*                       width2: Outer frame width                                       */
/*                         mode: 0: Vertical; 1: Horizontal                              */
/* Returns:     None                                                                     */
/* Description: Draw a table - refer to LT758x application note for more information.    */
/*---------------------------------------------------------------------------------------*/
void LT758_MakeTable(
	unsigned short X1,
	unsigned short Y1,
	unsigned short W,
	unsigned short H,
	unsigned short Line,
	unsigned short Row,
	unsigned long TableColor,
	unsigned long ItemColor,
	unsigned long ForegroundColor,
	unsigned short width1,
	unsigned short width2,
	unsigned char mode)
{
	unsigned short i = 0;
	unsigned short x2, y2;
	x2 = X1 + W * Row;
	y2 = Y1 + H * Line;

	LT758_DrawSquare_Width(X1, Y1, x2, y2, TableColor, ForegroundColor, width2);

	if (mode == 0)
		LT758_DrawSquare_Fill(X1, Y1, X1 + W, y2, ItemColor);
	else if (mode == 1)
		LT758_DrawSquare_Fill(X1, Y1, x2, Y1 + H, ItemColor);

	for (i = 0; i < Line; i++)
	{
		LT758_DrawLine_Width(X1, Y1 + i * H, x2, Y1 + i * H, TableColor, width1);
	}

	for (i = 0; i < Row; i++)
	{
		LT758_DrawLine_Width(X1 + i * W, Y1, X1 + i * W, y2, TableColor, width1);
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Color_Bar_ON                                                       */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Color bar display enable                                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_Color_Bar_ON(void)
{
	Color_Bar_ON();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Color_Bar_OFF                                                      */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Color bar display disable                                                */
/*---------------------------------------------------------------------------------------*/
void LT758_Color_Bar_OFF(void)
{
	Color_Bar_OFF();
}

//--------------------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_24bit_Linear                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      SCS: Select SPI Flash:  SCS: 0       SCS: 1                      */
/*                      Clk: SPI Clock = System Clock /{(Clk+1)*2}, Clk: 0 ~ 3           */
/*               flash_addr: Starting address in Flash (to retrieve the data)            */
/*              memory_addr: Starting address in SDRAM (to store the data)               */
/*                 data_num: Data amount                                                 */
/* Returns:     None                                                                     */
/* Description: To retrieve data from SPI Flash and transfer them to the designated      */
/*              SDRAM address, using linear mode, and 24bits address mode                */
/*---------------------------------------------------------------------------------------*/
void LT758_DMA_24bit_Linear(
	unsigned char SCS, 
	unsigned char Clk, 
	unsigned long flash_addr, 
	unsigned long memory_addr, 
	unsigned long data_num)
{
	Enable_SFlash_SPI(); // Enable SPI Flash interface
	if (SCS == 0)
		Select_SFI_0();  // Select SPI0
	if (SCS == 1)
		Select_SFI_1();  // Select SPI1

	Memory_Linear_Mode();
	SPI_Clock_Period(Clk);							// Set SPI clock
	SFI_DMA_Destination_Start_Address(memory_addr); // Set the starting address of the SDRAM
	SFI_DMA_Transfer_Number(data_num);				// Set the amount of data to be transmitted
	SFI_DMA_Source_Start_Address(flash_addr);		// Set the starting address of the flash
	Check_Busy_SFI_DMA();
	Start_SFI_DMA();
	Check_Busy_SFI_DMA();
	Memory_XY_Mode();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_32bit_Linear                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      SCS: Select SPI Flash:  SCS: 0       SCS: 1                      */
/*                      Clk: SPI Clock = System Clock /{(Clk+1)*2}, Clk: 0 ~ 3           */
/*               flash_addr: Starting address in Flash (to retrieve the data)            */
/*              memory_addr: Starting address in SDRAM (to save the data)                */
/*                 data_num: Data amount                                                 */
/* Returns:     None                                                                     */
/* Description: To retrieve data from SPI Flash and transfer them to the designated      */
/*              SDRAM address, using linear mode, and 32bit address mode                 */
/*---------------------------------------------------------------------------------------*/
void LT758_DMA_32bit_Linear(
	unsigned char SCS, 
	unsigned char Clk, 
	unsigned long flash_addr, 
	unsigned long memory_addr, 
	unsigned long data_num)
{
	Enable_SFlash_SPI(); // Enable SPI Flash interface
	if (SCS == 0)
		Select_SFI_0();  // Select SPI0
	if (SCS == 1)
		Select_SFI_1();  // Select SPI1

	Memory_Linear_Mode();
	Select_SFI_32bit_Address();
	SPI_Clock_Period(Clk);							// Set SPI clock
	SFI_DMA_Destination_Start_Address(memory_addr); // Set the starting address of the SDRAM
	SFI_DMA_Transfer_Number(data_num);				// Set the amount of data to be transmitted
	SFI_DMA_Source_Start_Address(flash_addr);		// Set the starting address of the Flash
	Check_Busy_SFI_DMA();
	Start_SFI_DMA();
	Check_Busy_SFI_DMA();
	Memory_XY_Mode();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_24bit_Block                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*               SCS: Select SPI Flash:  SCS: 0    SCS: 1                                */
/*               Clk: SPI Clock = System Clock /{(Clk+1)*2}, Clk: 0 ~ 3                  */
/*                X1: Starting X coordinate in SDRAM layer                               */
/*                Y1: Starting Y coordinate in SDRAM layer                               */
/*               X_W: Width of the data to be transmitted, e.g. picture width            */
/*               Y_H: Height of the data to be transmitted, e.g. picture height          */
/*               P_W: Picture width                                                      */
/*              Addr: Starting address in Flash (to retrieve the data)                   */
/* Returns:     None                                                                     */
/* Description: To retrieve data from SPI Flash and transfer them to the designated      */
/*              SDRAM address, using block mode, and 24bit address mode                  */
/*---------------------------------------------------------------------------------------*/
void LT758_DMA_24bit_Block(
	unsigned char SCS, 
	unsigned char Clk, 
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned short P_W, 
	unsigned long Addr)
{
	Enable_SFlash_SPI(); // Enable SPI Flash interface
	if (SCS == 0)
		Select_SFI_0();  // Select SPI0
	if (SCS == 1)
		Select_SFI_1();  // Select SPI1

	Memory_XY_Mode();
	SPI_Clock_Period(Clk);						   // Set SPI clock
	Goto_Pixel_XY(X1, Y1);						   // Set the starting point of SDRAM in Graphic mode
	SFI_DMA_Destination_Upper_Left_Corner(X1, Y1); // Set the DMA transmition destination (starting point in SDRAM)
	SFI_DMA_Transfer_Width_Height(X_W, Y_H);	   // Set the width and height of the transmitted data
	SFI_DMA_Source_Width(P_W);					   // Set the picture width
	SFI_DMA_Source_Start_Address(Addr);			   // Set the starting address in SPI Flash
	Start_SFI_DMA();							   // Start DMA transmition
	Check_Busy_SFI_DMA();						   // Check if the transmition is done
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_32bit_Block                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*               SCS: Select SPI Flash:  SCS: 0    SCS: 1                                */
/*               Clk: SPI Clock = System Clock /{(Clk+1)*2}, Clk: 0 ~ 3                  */
/*                X1: Starting X coordinate in SDRAM layer                               */
/*                Y1: Starting Y coordinate in SDRAM layer                               */
/*               X_W: Width of the data to be transmitted, e.g. picture width            */
/*               Y_H: Height of the data to be transmitted, e.g. picture height          */
/*               P_W: Picture width                                                      */
/*              Addr: Starting address in Flash (to retrieve the data)                   */
/* Returns:     None                                                                     */
/* Description: To retrieve data from SPI Flash and transmit them to the designated      */
/*              SDRAM address, using block mode, and 32bit address mode                  */
/*---------------------------------------------------------------------------------------*/
void LT758_DMA_32bit_Block(
	unsigned char SCS, 
	unsigned char Clk, 
	unsigned short X1, 
	unsigned short Y1, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned short P_W, 
	unsigned long Addr)
{
	Enable_SFlash_SPI();
	if (SCS == 0)
		Select_SFI_0();
	if (SCS == 1)
		Select_SFI_1();

	Memory_XY_Mode();
	SPI_Clock_Period(Clk);
	Select_SFI_32bit_Address();
	Goto_Pixel_XY(X1, Y1);
	SFI_DMA_Destination_Upper_Left_Corner(X1, Y1);
	SFI_DMA_Transfer_Width_Height(X_W, Y_H);
	SFI_DMA_Source_Width(P_W);
	SFI_DMA_Source_Start_Address(Addr);
	Start_SFI_DMA();
	Check_Busy_SFI_DMA();
	Select_SFI_24bit_Address();
}

//--------------------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Select_Internal_Font_Init                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*                   Size: Font size  16: 16*16   24: 24*24    32: 32*32                 */
/*                    XxN: Width magnification: 1~4                                      */
/*                    YxN: Height magnification: 1~4                                     */
/*              ChromaKey: 0: Transparent background; 1: Use background color            */
/*              Alignment: 0: Non-alignment; 1: Alignment                                */
/* Returns:     None                                                                     */
/* Description: Initialize the internal font                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_Select_Internal_Font_Init(
	unsigned char Size, 
	unsigned char XxN, 
	unsigned char YxN, 
	unsigned char ChromaKey, 
	unsigned char Alignment)
{
	if (Size == 16)
		Font_Select_8x16_16x16();
	if (Size == 24)
		Font_Select_12x24_24x24();
	if (Size == 32)
		Font_Select_16x32_32x32();

	//Width magnification
	if (XxN == 1)
		Font_Width_X1();
	if (XxN == 2)
		Font_Width_X2();
	if (XxN == 3)
		Font_Width_X3();
	if (XxN == 4)
		Font_Width_X4();

	//Height magnification
	if (YxN == 1)
		Font_Height_X1();
	if (YxN == 2)
		Font_Height_X2();
	if (YxN == 3)
		Font_Height_X3();
	if (YxN == 4)
		Font_Height_X4();

	//Transparency setting
	if (ChromaKey == 0)
		Font_Background_select_Color();
	if (ChromaKey == 1)
		Font_Background_select_Transparency();

	//Alignment setting
	if (Alignment == 0)
		Disable_Font_Alignment();
	if (Alignment == 1)
		Enable_Font_Alignment();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Print_Internal_Font_String                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                            x: Start position X                                        */
/*                            y: Start position Y                                        */
/*                    FontColor: Font color                                              */
/*              BackGroundColor: Background color of the font                            */
/*                           *c: Starting address of the string to be printed            */
/* Returns:     None                                                                     */
/* Description: Print out internal font                                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_Print_Internal_Font_String(
	unsigned short x, 
	unsigned short y, 
	unsigned long FontColor, 
	unsigned long BackGroundColor, 
	char *c)
{
	Text_Mode();
	CGROM_Select_Internal_CGROM();
	
#if Display_16bpp
	Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#else
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
	
	Goto_Text_XY(x, y);
	Show_String(c);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Select_Outside_Font_Init                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*                     SCS: Select SPI Flash:  SCS: 0    SCS: 1                          */
/*                     Clk: SPI Clock = System Clock /{(Clk+1)*2}                        */
/*               FlashAddr: Starting address in SPI Falsh                                */
/*              MemoryAddr: Starting address in SDRAM (destination)                      */
/*                     Num: Data amount of the font                                      */
/*                    Size: Font size  16: 16*16   24: 24*24    32: 32*32                */
/*                     XxN: Width magnification: 1~4                                     */
/*                     YxN: Height magnification: 1~4                                    */
/*               ChromaKey: 0: Transparent background; 1: Use background color           */
/*               Alignment: 0: Non-alignment; 1: Alignment                               */
/* Returns:     None                                                                     */
/* Description: Initialize the external font                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_Select_Outside_Font_Init(
	unsigned char SCS, 
	unsigned char Clk, 
	unsigned long FlashAddr, 
	unsigned long MemoryAddr, 
	unsigned long Num, 
	unsigned char Size, 
	unsigned char XxN, 
	unsigned char YxN, 
	unsigned char ChromaKey, 
	unsigned char Alignment)
{

	if (Size == 16)
		Font_Select_8x16_16x16();
	if (Size == 24)
		Font_Select_12x24_24x24();
	if (Size == 32)
		Font_Select_16x32_32x32();

	//Width magnification
	if (XxN == 1)
		Font_Width_X1();
	if (XxN == 2)
		Font_Width_X2();
	if (XxN == 3)
		Font_Width_X3();
	if (XxN == 4)
		Font_Width_X4();

	//Height magnification
	if (YxN == 1)
		Font_Height_X1();
	if (YxN == 2)
		Font_Height_X2();
	if (YxN == 3)
		Font_Height_X3();
	if (YxN == 4)
		Font_Height_X4();

	//Transparency setting
	if (ChromaKey == 0)
		Font_Background_select_Color();
	if (ChromaKey == 1)
		Font_Background_select_Transparency();

	//Alignment setting
	if (Alignment == 0)
		Disable_Font_Alignment();
	if (Alignment == 1)
		Enable_Font_Alignment();

	if (w25q256)
		LT758_DMA_32bit_Linear(SCS, Clk, FlashAddr, MemoryAddr, Num);
	else
		LT758_DMA_24bit_Linear(SCS, Clk, FlashAddr, MemoryAddr, Num);
	CGRAM_Start_address(MemoryAddr);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Print_Outside_Font_String                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*                            x: Start position X                                        */
/*                            y: Start position Y                                        */
/*                    FontColor: Font color                                              */
/*              BackGroundColor: Background color of the font                            */
/*                           *c: Starting address of the string to be printed            */
/* Returns:     None                                                                     */
/* Description: Print out internal and external font                                     */
/*---------------------------------------------------------------------------------------*/
void LT758_Print_Outside_Font_String(
	unsigned short x, 
	unsigned short y, 
	unsigned long FontColor, 
	unsigned long BackGroundColor, 
	unsigned char *c)
{
	unsigned short temp_H = 0;
	unsigned short temp_L = 0;
	unsigned short temp = 0;
	unsigned long i = 0;

	Text_Mode();
	Font_Select_UserDefine_Mode();

#if Display_16bpp
	Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#else
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif

	Goto_Text_XY(x, y);

	while (c[i] != '\0')
	{
		if (c[i] < 0xa1)
		{
			CGROM_Select_Internal_CGROM(); // using internal CGROM as the source of the character
			LCD_CmdWrite(0x04);
			LCD_DataWrite(c[i]);
			Check_Mem_WR_FIFO_not_Full();
			i += 1;
		}
		else
		{
			Font_Select_UserDefine_Mode(); // User-defined font
			temp_H = ((c[i] - 0xa1) & 0x00ff) * 94;
			temp_L = c[i + 1] - 0xa1;
			temp = temp_H + temp_L + 0x8000;
			LCD_CmdWrite(0x04);
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite((temp >> 8) & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(temp & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			i += 2;
		}
	}

	Check_2D_Busy();

	Graphic_Mode(); // back to graphic mode
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Print_Outside_Ascii_String                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                            x: Start position X                                        */
/*                            y: Start position Y                                        */
/*                    FontColor: Font color                                              */
/*              BackGroundColor: Background color of the font                            */
/*                           *c: Starting address of the string to be printed            */
/* Returns:     None                                                                     */
/* Description: Print out internal and external font                                     */
/*---------------------------------------------------------------------------------------*/
void LT758_Print_Outside_Ascii_String(
	unsigned short x, 
	unsigned short y, 
	unsigned long FontColor, 
	unsigned long BackGroundColor, 
	unsigned char *c)
{
	unsigned short temp_H = 0;
	unsigned short temp_L = 0;
	unsigned short temp = 0;
	unsigned long i = 0;

	Text_Mode();
	Font_Select_UserDefine_Mode();

#if Display_16bpp
	Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#else
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif

	Goto_Text_XY(x, y);

	while (c[i] != '\0')
	{
		if (c[i] < 0xa1)
		{
			Font_Select_UserDefine_Mode(); // User-defined font
			LCD_CmdWrite(0x04);
			temp = c[i] + 0x0000;
			LCD_DataWrite((temp >> 8) & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(temp & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			i += 1;
		}
		else
		{
			Font_Select_UserDefine_Mode(); // User-defined font
			LCD_CmdWrite(0x04);
			temp_H = ((c[i] - 0xa1) & 0x00ff) * 94;
			temp_L = c[i + 1] - 0xa1;
			temp = (temp_H + temp_L + 0x0000) / 2;
			LCD_DataWrite((temp >> 8) & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(temp & 0xff);
			Check_Mem_WR_FIFO_not_Full();
			i += 2;
		}
	}

	Check_2D_Busy();

	Graphic_Mode(); // back to graphic mode
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Text_cursor_Init                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*              On_Off_Blinking: Cursor blinking, 0: disable   1: enable                 */
/*                Blinking_Time: Set the cursor blinking time                            */
/*                          X_W: Cursor width                                            */
/*                          Y_W: Cursor height                                           */
/* Returns:     None                                                                     */
/* Description: Initialize text cursor -                                                 */
/*              refer to LT758x application note for more detail                         */
/*---------------------------------------------------------------------------------------*/
void LT758_Text_cursor_Init(
	unsigned char On_Off_Blinking, 
	unsigned short Blinking_Time, 
	unsigned short X_W, 
	unsigned short Y_W)
{
	if (On_Off_Blinking == 0)
		Disable_Text_Cursor_Blinking();
	if (On_Off_Blinking == 1)
		Enable_Text_Cursor_Blinking();

	Blinking_Time_Frames(Blinking_Time);

	//[3E][3Fh]
	Text_Cursor_H_V(X_W, Y_W);

	Enable_Text_Cursor();
	Text_Mode();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Enable_Text_cursor                                                 */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Enable text cursor                                                       */
/*---------------------------------------------------------------------------------------*/
void LT758_Enable_Text_Cursor(void)
{
	Enable_Text_Cursor();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Disable_Text_cursor                                                */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Disable text cursor                                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_Disable_Text_Cursor(void)
{
	Disable_Text_Cursor();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Graphic_cursor_Init                                                */
/*                                                                                       */
/* Parameters:                                                                           */
/*                 Cursor_N: Select cursor: 1 ~ 4                                        */
/*                   Color1: Color 1                                                     */
/*                   Color2: Color 2                                                     */
/*                    X_Pos: X coordinate of the cursor                                  */
/*                    Y_Pos: Y coordinate of the cursor                                  */
/*              *Cursor_Buf: First address of the cursor data                            */
/* Returns:     None                                                                     */
/* Description: Initialize text cursor -                                                 */
/*              refer to LT758x application note for more detail                         */
/*---------------------------------------------------------------------------------------*/
void LT758_Graphic_cursor_Init(
	unsigned char Cursor_N, 
	unsigned char Color1, 
	unsigned char Color2, 
	unsigned short X_Pos, 
	unsigned short Y_Pos, 
	unsigned char *Cursor_Buf)
{
	unsigned int i;

	Memory_Select_Graphic_Cursor_RAM();
	Graphic_Mode();

	switch (Cursor_N)
	{
	case 1:
		Select_Graphic_Cursor_1();
		break;
	case 2:
		Select_Graphic_Cursor_2();
		break;
	case 3:
		Select_Graphic_Cursor_3();
		break;
	case 4:
		Select_Graphic_Cursor_4();
		break;
	default:
		break;
	}

	LCD_CmdWrite(0x04);

	for (i = 0; i < 256; i++)
	{
		LCD_DataWrite(Cursor_Buf[i]);
	}

	Memory_Select_SDRAM();
	Set_Graphic_Cursor_Color_0(Color1);
	Set_Graphic_Cursor_Color_1(Color2);
	Graphic_Cursor_XY(X_Pos, Y_Pos);

	Enable_Graphic_Cursor();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Set_Graphic_cursor_Pos                                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*              Cursor_N: Select cursor: 1 ~ 4                                           */
/*                 X_Pos: X coordinate of the cursor                                     */
/*                 Y_Pos: Y coordinate of the cursor                                     */
/* Returns:     None                                                                     */
/* Description: Set graphic cursor position                                              */
/*---------------------------------------------------------------------------------------*/
void LT758_Set_Graphic_cursor_Pos(
	unsigned char Cursor_N, 
	unsigned short X_Pos, 
	unsigned short Y_Pos)
{
	Graphic_Cursor_XY(X_Pos, Y_Pos);
	switch (Cursor_N)
	{
	case 1:
		Select_Graphic_Cursor_1();
		break;
	case 2:
		Select_Graphic_Cursor_2();
		break;
	case 3:
		Select_Graphic_Cursor_3();
		break;
	case 4:
		Select_Graphic_Cursor_4();
		break;
	default:
		break;
	}
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Enable_Graphic_cursor                                              */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Enable graphic cursor                                                    */
/*---------------------------------------------------------------------------------------*/
void LT758_Enable_Graphic_Cursor(void)
{
	Enable_Graphic_Cursor();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Disable_Graphic_cursor                                             */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Disable graphic cursor                                                   */
/*---------------------------------------------------------------------------------------*/
void LT758_Disable_Graphic_Cursor(void)
{
	Disable_Graphic_Cursor();
}

//-----------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_PIP_Init                                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  On_Off: 0: Disable   1: Eanble   2: Keep state                       */
/*              Select_PIP: 1: Use PIP1  2: Use PIP2                                     */
/*                   PAddr: Starting address of the PIP                                  */
/*                      XP: X coordinate of the PIP, must be divisible by 4              */
/*                      YP: Y coordinate of the PIP, must be divisible by 4              */
/*              ImageWidth: Width of the background(main) image                          */
/*                   X_Dis: X coordinate of the display window                           */
/*                   Y_Dis: Y coordinate of the display window                           */
/*                     X_W: Width of the display window, must be divisible by 4          */
/*                     Y_H: Height of the display window, must be divisible by 4         */
/* Returns:     None                                                                     */
/* Description: Initialize Picture-In-Picture fucntion                                   */
/*---------------------------------------------------------------------------------------*/
void LT758_PIP_Init(
	unsigned char On_Off, 
	unsigned char Select_PIP, 
	unsigned long PAddr,
	unsigned short XP, 
	unsigned short YP,
	unsigned long ImageWidth, 
	unsigned short X_Dis, 
	unsigned short Y_Dis, 
	unsigned short X_W, 
	unsigned short Y_H)
{
#if Display_16bpp
	if (Select_PIP == 1)
	{
		Select_PIP1_Window_16bpp();
		Select_PIP1_Parameter();
	}
	if (Select_PIP == 2)
	{
		Select_PIP2_Window_16bpp();
		Select_PIP2_Parameter();
	}
#else
	if (Select_PIP == 1)
	{
		Select_PIP1_Window_24bpp();
		Select_PIP1_Parameter();
	}
	if (Select_PIP == 2)
	{
		Select_PIP2_Window_24bpp();
		Select_PIP2_Parameter();
	}
#endif
	PIP_Display_Start_XY(X_Dis, Y_Dis);
	PIP_Image_Start_Address(PAddr);
	PIP_Image_Width(ImageWidth);
	PIP_Window_Image_Start_XY(XP, YP);
	PIP_Window_Width_Height(X_W, Y_H);

	if (On_Off == 0)
	{
		if (Select_PIP == 1)
			Disable_PIP1();
		if (Select_PIP == 2)
			Disable_PIP2();
	}

	if (On_Off == 1)
	{
		if (Select_PIP == 1)
			Enable_PIP1();
		if (Select_PIP == 2)
			Enable_PIP2();
	}
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Set_DisWindowPos                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*                  On_Off: 0: Disable   1: Eanble   2: Keep state                       */
/*              Select_PIP: 1: Use PIP1  2: Use PIP2                                     */
/*                   X_Dis: X coordinate of the display window                           */
/*                   Y_Dis: Y coordinate of the display window                           */
/* Returns:     None                                                                     */
/* Description: Set the PIP display position                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_Set_DisWindowPos(
	unsigned char On_Off, 
	unsigned char Select_PIP, 
	unsigned short X_Dis, 
	unsigned short Y_Dis)
{
	if (Select_PIP == 1)
		Select_PIP1_Parameter();
	if (Select_PIP == 2)
		Select_PIP2_Parameter();

	if (On_Off == 0)
	{
		if (Select_PIP == 1)
			Disable_PIP1();
		if (Select_PIP == 2)
			Disable_PIP2();
	}

	if (On_Off == 1)
	{
		if (Select_PIP == 1)
			Enable_PIP1();
		if (Select_PIP == 2)
			Enable_PIP2();
	}

	PIP_Display_Start_XY(X_Dis, Y_Dis);
}

//-----------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    	LT758_BTE_Solid_Fill                                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT area (DT:Destination)   */
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT area                   */
/*                          YDes: Left-top Y coordinate of the DT area                   */
/*                         color: Filled color                                           */
/*                           X_W: Filled width                                           */
/*                           Y_H: Filled Height                                          */
/*                   Color_depth: Color depth 											 */
/* Returns:     None                                                                     */
/* Description: Fill the designated color to the rectangle area (DT) specified by BTE    */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Solid_Fill(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned long color, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (Color_depth == 16)
	{
		BTE_Destination_Color_16bpp();
		Foreground_color_65k(color);
	}
	else // Color_depth = 24
	{
		BTE_Destination_Color_24bpp();
		Foreground_color_16M(color);
	}

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);
	BTE_Window_Size(X_W, Y_H);

	BTE_Operation_Code(0x0c);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Memory_Copy_JPG                                                */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S0_Addr: Starting address (SDRAM) of S0 picture                         */
/*                  S0_W: Width of the S0 picture                                        */
/*                   XS0: Left-top X coordinate of the S0 picture                        */
/*                   YS0: Left-top Y coordinate of the S0 picture                        */
/*               S1_Addr: Starting address (SDRAM) of S1 picture                         */
/*                  S1_W: Width of the S1 picture                                        */
/*                   XS1: Left-top X coordinate of the S1 picture                        */
/*                   YS1: Left-top Y coordinate of the S1 picture                        */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)       */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*              ROP_Code:                                                                */
/*                        0000b		0(Blackness)                                         */
/*                        0001b		~S0.~S1 or ~(S0+S1)                                  */
/*                        0010b		~S0.S1                                               */
/*                        0011b		~S0                                                  */
/*                        0100b		S0.~S1                                               */
/*                        0101b		~S1                                                  */
/*                        0110b		S0^S1                                                */
/*                        0111b		~S0 + ~S1 or ~(S0 . S1)                              */
/*                        1000b		S0.S1                                                */
/*                        1001b		~(S0^S1)                                             */
/*                        1010b		S1                                                   */
/*                        1011b		~S0+S1                                               */
/*                        1100b		S0                                                   */
/*                        1101b		S0+~S1                                               */
/*                        1110b		S0+S1                                                */
/*                        1111b		1(whiteness)                                         */
/*                   X_W: Width of the active window                                     */
/*                   Y_H: Height of the active window                                    */
/*           Color_depth: Color depth   8: 8bpp   16: 16bpp   24: 24bpp                  */
/* Returns:     None                                                                     */
/* Description: BTE memory copy for JPG image, with ROP                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Memory_Copy_JPG(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	BTE_S0_Color_24bpp();
	BTE_S1_Color_24bpp();

	switch (Color_depth)
	{
	case 8:
		BTE_Destination_Color_8bpp();
		break;
	case 16:
		BTE_Destination_Color_16bpp();
		break;
	default:
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x02); // BTE Operation: Memory copy (move) with ROP.
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Memory_Copy                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S0_Addr: Starting address (SDRAM) of S0 picture                         */
/*                  S0_W: Width of the S0 picture                                        */
/*                   XS0: Left-top X coordinate of the S0 picture                        */
/*                   YS0: Left-top Y coordinate of the S0 picture                        */
/*               S1_Addr: Starting address (SDRAM) of S1 picture                         */
/*                  S1_W: Width of the S1 picture                                        */
/*                   XS1: Left-top X coordinate of the S1 picture                        */
/*                   YS1: Left-top Y coordinate of the S1 picture                        */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)       */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*              ROP_Code:                                                                */
/*                        0000b		0(Blackness)                                         */
/*                        0001b		~S0.~S1 or ~(S0+S1)                                  */
/*                        0010b		~S0.S1                                               */
/*                        0011b		~S0                                                  */
/*                        0100b		S0.~S1                                               */
/*                        0101b		~S1                                                  */
/*                        0110b		S0^S1                                                */
/*                        0111b		~S0 + ~S1 or ~(S0 . S1)                              */
/*                        1000b		S0.S1                                                */
/*                        1001b		~(S0^S1)                                             */
/*                        1010b		S1                                                   */
/*                        1011b		~S0+S1                                               */
/*                        1100b		S0                                                   */
/*                        1101b		S0+~S1                                               */
/*                        1110b		S0+S1                                                */
/*                        1111b		1(whiteness)                                         */
/*                   X_W: Width of the active window                                     */
/*                   Y_H: Height of the active window                                    */
/*           Color_depth: Color depth   16: 16bpp   24: 24bpp                            */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with ROP                                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Memory_Copy(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bpp();
		BTE_Destination_Color_16bpp();
	}
	else // Color_depth = 24
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_24bpp();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x02); // BTE Operation: Memory copy (move) with ROP
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Memory_Copy_Chroma_key                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                       S0_Addr: Starting address (SDRAM) of S0 picture                 */
/*                          S0_W: Width of the S0 picture                                */
/*                           XS0: Left-top X coordinate of the S0 picture                */
/*                           YS0: Left-top Y coordinate of the S0 picture                */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*              Background_color: Color (chroma key) to be taken as transparent          */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/* 				 	 Color_depth: Color depth	16: 16bpp   24: 24bpp  				     */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with Chroma Key (w/o ROP)                                */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Memory_Copy_Chroma_key(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned long Background_color, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (Color_depth == 16)
	{
		Background_color_65k(Background_color);
		BTE_S0_Color_16bpp();
		BTE_Destination_Color_16bpp();
	}
	else // Color_depth = 24
	{
		Background_color_16M(Background_color);
		BTE_S0_Color_24bpp();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Operation_Code(0x05); // BTE Operation: Memory copy (move) with chroma keying (w/o ROP)
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Pattern_Fill                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*             P_8x8_or_16x16: 0: use 8x8 icon   1: use 16x16 icon                       */
/*                    S0_Addr: Starting address (SDRAM) of S0 picture                    */
/*                       S0_W: Width of the S0 picture                                   */
/*                        XS0: Left-top X coordinate of the S0 picture                   */
/*                        YS0: Left-top Y coordinate of the S0 picture                   */
/*                   Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)  */
/*                      Des_W: Width of the DT picture                                   */
/*                      XDes: Left-top X coordinate of the DT picture                    */
/*                      YDes: Left-top Y coordinate of the DT picture                    */
/*                  ROP_Code:                                                            */
/*                            0000b		0(Blackness)                                     */
/*                            0001b		~S0.~S1 or ~(S0+S1)                              */
/*                            0010b		~S0.S1                                           */
/*                            0011b		~S0                                              */
/*                            0100b		S0.~S1                                           */
/*                            0101b		~S1                                              */
/*                            0110b		S0^S1                                            */
/*                            0111b		~S0 + ~S1 or ~(S0 . S1)                          */
/*                            1000b		S0.S1                                            */
/*                            1001b		~(S0^S1)                                         */
/*                            1010b		S1                                               */
/*                            1011b		~S0+S1                                           */
/*                            1100b		S0                                               */
/*                            1101b		S0+~S1                                           */
/*                            1110b		S0+S1                                            */
/*                            1111b		1(whiteness)                                     */
/*                       X_W: Width of the active window                                 */
/*                       Y_H: Height of the active window                                */
/*				 Color_depth: Color depth   16: 16bpp;  24: 24bpp						 */
/* Returns:     None                                                                     */
/* Description: BTE pattern fill operation with ROP                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Pattern_Fill(
	unsigned char P_8x8_or_16x16, 
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (P_8x8_or_16x16 == 0)
	{
		Pattern_Format_8X8();
	}
	else // P_8x8_or_16x16 == 1
	{
		Pattern_Format_16X16();
	}

	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_Destination_Color_16bpp();
	}
	else // Color_depth = 24
	{
		BTE_S0_Color_24bpp();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x06); // BTE Operation: Pattern Fill with ROP
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Pattern_Fill_With_Chroma_key                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*               P_8x8_or_16x16: 0: use 8x8 icon   1: use 16x16 icon                     */
/*                      S0_Addr: Starting address (SDRAM) of S0 picture                  */
/*                         S0_W: Width of the S0 picture                                 */
/*                          XS0: Left-top X coordinate of the S0 picture                 */
/*                          YS0: Left-top Y coordinate of the S0 picture                 */
/*                     Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination) */
/*                        Des_W: Width of the DT picture                                 */
/*                         XDes: Left-top X coordinate of the DT picture                 */
/*                         YDes: Left-top Y coordinate of the DT picture                 */
/*                     ROP_Code:                                                         */
/*                                0000b		0(Blackness)                                 */
/*                                0001b		~S0.~S1 or ~(S0+S1)                          */
/*                                0010b		~S0.S1                                       */
/*                                0011b		~S0                                          */
/*                                0100b		S0.~S1                                       */
/*                                0101b		~S1                                          */
/*                                0110b		S0^S1                                        */
/*                                0111b		~S0 + ~S1 or ~(S0 . S1)                      */
/*                                1000b		S0.S1                                        */
/*                                1001b		~(S0^S1)                                     */
/*                                1010b		S1                                           */
/*                                1011b		~S0+S1                                       */
/*                                1100b		S0                                           */
/*                                1101b		S0+~S1                                       */
/*                                1110b		S0+S1                                        */
/*                                1111b		1(whiteness)                                 */
/*              Background_color: Color (chroma key) to be taken as transparent          */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*                   Color_depth: Color depth                                            */
/* Returns:     None                                                                     */
/* Description: BTE pattern fill operation with chroma key and ROP                       */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Pattern_Fill_With_Chroma_key(
	unsigned char P_8x8_or_16x16, 
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned long Background_color, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (P_8x8_or_16x16 == 0)
	{
		Pattern_Format_8X8();
	}
	else // P_8x8_or_16x16 == 1
	{
		Pattern_Format_16X16();
	}

	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_Destination_Color_16bpp();
		Background_color_65k(Background_color);
	}
	else // Color_depth = 24
	{
		BTE_S0_Color_24bpp();
		BTE_Destination_Color_24bpp();
		Background_color_16M(Background_color);
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x07); // BTE Operation: Pattern Fill with Chroma key.
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_ColorExpansion_MCU8                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*              Background_color: Background color										 */
/*                         *data: Starting address of the written data                   */
/* 					 Color_depth: Color depth                                            */
/* Returns:     None                                                                     */
/* Description: MCU write data to DT with color expansion                                */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_ColorExpansion_MCU8(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color,	/*Foreground_color : The source (1bit mapping picture) mapping data 1 is translated to Foreground color by color expansion*/
	unsigned long Background_color,	/*Background_color : The source (1bit mapping picture) mapping data 0 is translated to Background color by color expansion*/
	unsigned char *data, 
	unsigned char Color_depth)
{
	unsigned short i, j;
	unsigned short temp;

	if (Color_depth == 16)
	{
		Foreground_color_65k(Foreground_color);
		Background_color_65k(Background_color);
		BTE_Destination_Color_16bpp();
	}
	else if (Color_depth == 24)
	{
		Foreground_color_16M(Foreground_color);
		Background_color_16M(Background_color);
		BTE_Destination_Color_24bpp();
	}

	BTE_ROP_Code(15);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x08); // BTE Operation: MPU Write with Color Expansion (w/o ROP)
	BTE_Enable();

	SDRAM_ReadWrite(); // Memory Data Read/Write Port
	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < X_W / 8; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
#if STM32_FMC_8 | STM32_SPI
      UNUSED(temp);
			LCD_DataWrite(*data++); // MCU 8bit interface, the start bit is bit7
#else
			temp = *data++;
			if (j + 1 < X_W / 8)
			{
				j++;
				temp = (temp << 8) | *data++;
			}
			else
			{
				temp = temp << 8;
			}
			LCD_DataWrite_Pixel(temp); // MCU 16bit interface, the start bit is bit15, therefore temp << 8
#endif
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU8                       */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*                         *data: Starting address of the written data                   */
/*                   Color_depth: Color depth                                            */
/* Returns:     None                                                                     */
/* Description: MCU write data to DT with color expansion and ignore background color    */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU8(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color, /*Foreground_color : The source (1bit mapping picture) mapping data 1 is translated to Foreground color by color expansion*/
	unsigned char *data, 
	unsigned char Color_depth)
{
	unsigned short i, j;
	unsigned short temp;

	if (Color_depth == 16)
	{
		Foreground_color_65k(Foreground_color);
		BTE_Destination_Color_16bpp();
	}
	else if (Color_depth == 24)
	{
		Foreground_color_16M(Foreground_color);
		BTE_Destination_Color_24bpp();
	}

	BTE_ROP_Code(15);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x09); // BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)
	BTE_Enable();

	SDRAM_ReadWrite(); // Memory Data Read/Write Port
	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < X_W / 8; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
#if STM32_FMC_8 | STM32_SPI
      UNUSED(temp);
			LCD_DataWrite(*data++);
#else
			temp = *data++;
			if (j + 1 < X_W / 8)
			{
				j++;
				temp = (temp << 8) | *data++;
			}
			else
			{
				temp = temp << 8;
			}
			LCD_DataWrite_Pixel(temp); // MCU 16bit interface, the start bit is bit15, therefore temp << 8
#endif
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Memory_Copy_ColorExpansion_8_ColorDepth                        */
/*                                                                                       */
/* Parameters:                                                                           */
/*                       S0_Addr: Starting address (SDRAM) of S0 picture                 */
/*                          S0_W: Width of the S0 picture                                */
/*                           XS0: Left-top X coordinate of the S0 picture                */
/*                           YS0: Left-top Y coordinate of the S0 picture                */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*              Background_color: Background color                                       */
/*                   Color_depth: Color depth   16: 16bpp;   24: 24bpp 				     */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with color expansion (w/o ROP)                           */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Memory_Copy_ColorExpansion_8_ColorDepth(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color, 
	unsigned long Background_color, 
	unsigned char Color_depth)
{
	BTE_ROP_Code(7);

	BTE_S0_Color_8bpp();
	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	if (Color_depth == 16)
	{
		BTE_Destination_Color_16bpp();
		Foreground_color_65k(Foreground_color);
		Background_color_65k(Background_color);
	}
	else
	{
		BTE_Destination_Color_24bpp();
		Foreground_color_16M(Foreground_color);
		Background_color_16M(Background_color);
	}

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Operation_Code(0x0e); // BTE Operation: Memory copy (move) with color expansion (w/o ROP)
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Memory_Copy_ColorExpansion_Chroma_key_8_ColorDepth             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                       S0_Addr: Starting address (SDRAM) of S0 picture                 */
/*                          S0_W: Width of the S0 picture                                */
/*                           XS0: Left-top X coordinate of the S0 picture                */
/*                           YS0: Left-top Y coordinate of the S0 picture                */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*                   Color_depth: Color depth 	16: 16dpp;   24: 24dpp					 */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with color expansion and chroma key (w/o ROP)            */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Memory_Copy_ColorExpansion_Chroma_key_8_ColorDepth(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color, 
	unsigned char Color_depth)
{
	BTE_ROP_Code(7);

	BTE_S0_Color_8bpp();
	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	if (Color_depth == 16)
	{
		BTE_Destination_Color_16bpp();
		Foreground_color_65k(Foreground_color);
	}
	else // Color_depth = 24
	{
		BTE_Destination_Color_24bpp();
		Foreground_color_16M(Foreground_color);
	}

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Operation_Code(0x0f); // BTE Operation: Memory copy (move) with chroma keying (w/o ROP)
	BTE_Window_Size(X_W, Y_H);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_Picture_Alpha_Blending                               */
/*                                                                                       */
/* Parameters:                                                                           */
/*                       S1_Addr: Starting address (SDRAM) of S1 picture                 */
/*                          S1_W: Width of the S1 picture                                */
/*                           XS1: Left-top X coordinate of the S1 picture                */
/*                           YS1: Left-top Y coordinate of the S1 picture                */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*                         *data: Starting address of the S0 picture                     */
/*						   alpha: Alpha blending level (0~31 levels)                     */
/*                   Color_depth: Color depth 											 */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with color expansion and chroma key (w/o ROP)            */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_Picture_Alpha_Blending(
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char *data, 
	unsigned char alpha, 
	unsigned char Color_depth)
{
	unsigned short i, j;

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x0B); // BTE Operation: MCU Write with opacity.
	BTE_Alpha_Blending_Effect(alpha);
	BTE_Enable();

	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bpp();
		BTE_Destination_Color_16bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit
		for (i = 0; i < Y_H; i++)
		{
			for (j = 0; j < (X_W); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
#if STM32_FMC_16
				LCD_DataWrite_Pixel(*(data++) | (*(data++) << 8));
#else
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
#endif
			}
		}
	}
	else if (Color_depth == 24)
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_24bpp();
		BTE_Destination_Color_24bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit
		for (i = 0; i < Y_H; i++)
		{
#if STM32_FMC_16
			for (j = 0; j < (X_W / 2); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite_Pixel(*(data++) | (*(data++) << 8));
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite_Pixel(*(data++) | (*(data++) << 8));
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite_Pixel(*(data++) | (*(data++) << 8));
			}
#else
			for (j = 0; j < (X_W); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
			}
#endif
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Picture_Alplha_Blending                                        */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S0_Addr: Starting address (SDRAM) of S0 picture                         */
/*                  S0_W: Width of the S0 picture                                        */
/*                   XS0: Left-top X coordinate of the S0 picture                        */
/*                   YS0: Left-top Y coordinate of the S0 picture                        */
/*               S1_Addr: Starting address (SDRAM) of S1 picture                         */
/*                  S1_W: Width of the S1 picture                                        */
/*                   XS1: Left-top X coordinate of the S1 picture                        */
/*                   YS1: Left-top Y coordinate of the S1 picture                        */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)       */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*                   X_W: Width of the active window                                     */
/*                   Y_H: Height of the active window                                    */
/*                 alpha: Alpha blending level (0~31 levels)                             */
/*           Color_depth: Color depth													 */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with opacity setting -                                   */
/*              refer to LT758x application note for more detail                         */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Picture_Alpha_Blending(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char alpha, 
	unsigned char Color_depth)
{
	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bpp();
		BTE_Destination_Color_16bpp();
	}
	else // Color depth = 24
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_24bpp();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x0A); // BTE Operation: Memory write with opacity (w/o ROP)
	BTE_Alpha_Blending_Effect(alpha);
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Pixel_Alplha_Blending                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S0_Addr: Starting address (SDRAM) of S0 picture                         */
/*                  S0_W: Width of the S0 picture                                        */
/*                   XS0: Left-top X coordinate of the S0 picture                        */
/*                   YS0: Left-top Y coordinate of the S0 picture                        */
/*               S1_Addr: Starting address (SDRAM) of S1 picture (PNG)                   */
/*                  S1_W: Width of the S1 picture  (PNG)                                 */
/*                   XS1: Left-top X coordinate of the S1 picture (PNG)                  */
/*                   YS1: Left-top Y coordinate of the S1 picture (PNG)                  */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)       */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*                   X_W: Display width                                                  */
/*                   Y_H: Display height                                                 */
/*             Alpha_bit: Alpha channel number                                           */
/* Returns:     None                                                                     */
/* Description: BTE memory copy with opacity setting -                                   */
/*              refer to "LT758x application note for more detail 					     */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Pixel_Alpha_Blending(
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Alpha_bit)
{
	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	switch (Alpha_bit)
	{
	case 0:
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_32bit_Alpha();
		BTE_Destination_Color_16bpp();
	}
	break;

	case 1:
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_16bit_Alpha();
		BTE_Destination_Color_24bpp();
	}
	break;

	case 2:
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_8bit_Alpha();
		BTE_Destination_Color_16bpp();
	}
	break;

	case 4:
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bit_Alpha();
		BTE_Destination_Color_16bpp();
	}
	break;

	case 8:
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_32bit_Alpha();
		BTE_Destination_Color_24bpp();
	}
	break;

	default:
		break;
	}

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);

	BTE_Operation_Code(0x0A); // BTE Operation: Memory write with opacity (w/o ROP)
	BTE_Enable();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_MCU_16bit                                            */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S1_Addr: Starting address (SDRAM) of S1 picture                         */
/*                  S1_W: Width of the S1 picture                                        */
/*                   XS1: Left-top X coordinate of the S1 picture                        */
/*                   YS1: Left-top Y coordinate of the S1 picture                        */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)        */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*              ROP_Code:                                                                */
/*                        0000b		0(Blackness)                                         */
/*                        0001b		~S0.~S1 or ~(S0+S1)                                  */
/*                        0010b		~S0.S1                                               */
/*                        0011b		~S0                                                  */
/*                        0100b		S0.~S1                                               */
/*                        0101b		~S1                                                  */
/*                        0110b		S0^S1                                                */
/*                        0111b		~S0 + ~S1 or ~(S0 . S1)                              */
/*                        1000b		S0.S1                                                */
/*                        1001b		~(S0^S1)                                             */
/*                        1010b		S1                                                   */
/*                        1011b		~S0+S1                                               */
/*                        1100b		S0                                                   */
/*                        1101b		S0+~S1                                               */
/*                        1110b		S0+S1                                                */
/*                        1111b		1(whiteness)                                         */
/*                  X_W: Width of the active window                                      */
/*                  Y_H: Height of the active window                                     */
/*                *data: Starting address of S0 picture                                  */
/* Returns:     None                                                                     */
/* Description: MCU write S0 data to DT with ROP                                         */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_MCU_16bit(
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	const unsigned short *data)
{
	unsigned short i, j;

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x00); // BTE Operation: MPU Write with ROP.
	BTE_Enable();

#if Display_16bpp
	BTE_S0_Color_16bpp();
	BTE_S1_Color_16bpp();
	BTE_Destination_Color_16bpp();

	LCD_CmdWrite(0x04); // Memory Data Read/Write Port

	// MCU_16bit_ColorDepth_16bpp
	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < (X_W); j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
		}
	}
#else
	BTE_S0_Color_24bpp();
	BTE_S1_Color_24bpp();
	BTE_Destination_Color_24bpp();

	LCD_CmdWrite(0x04); // Memory Data Read/Write Port

	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < (X_W); j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
		}
	}
#endif

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_MCU8                                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*               S1_Addr: Starting address (SDRAM) of S1 picture                         */
/*                  S1_W: Width of the S1 picture                                        */
/*                   XS1: Left-top X coordinate of the S1 picture                        */
/*                   YS1: Left-top Y coordinate of the S1 picture                        */
/*              Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)        */
/*                 Des_W: Width of the DT picture                                        */
/*                  XDes: Left-top X coordinate of the DT picture                        */
/*                  YDes: Left-top Y coordinate of the DT picture                        */
/*              ROP_Code:                                                                */
/*                        0000b		0(Blackness)                                         */
/*                        0001b		~S0.~S1 or ~(S0+S1)                                  */
/*                        0010b		~S0.S1                                               */
/*                        0011b		~S0                                                  */
/*                        0100b		S0.~S1                                               */
/*                        0101b		~S1                                                  */
/*                        0110b		S0^S1                                                */
/*                        0111b		~S0 + ~S1 or ~(S0 . S1)                              */
/*                        1000b		S0.S1                                                */
/*                        1001b		~(S0^S1)                                             */
/*                        1010b		S1                                                   */
/*                        1011b		~S0+S1                                               */
/*                        1100b		S0                                                   */
/*                        1101b		S0+~S1                                               */
/*                        1110b		S0+S1                                                */
/*                        1111b		1(whiteness)                                         */
/*                  X_W: Width of the active window                                      */
/*                  Y_H: Height of the active window                                     */
/*                *data: Starting address of S0 picture                                  */
/*			Color_depth: Color depth													 */
/* Returns:     None                                                                     */
/* Description: MCU write S0 data to DT with ROP                                         */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_MCU8(
	unsigned long S1_Addr, 
	unsigned short S1_W, 
	unsigned short XS1, 
	unsigned short YS1, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char *data, 
	unsigned char Color_depth)
{
	unsigned short i, j;
	unsigned int temp;

	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W);
	BTE_S1_Window_Start_XY(XS1, YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x00); // BTE Operation: MPU Write with ROP.
	BTE_Enable();

	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bpp();
		BTE_Destination_Color_16bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit_ColorDepth_16bpp
		for (i = 0; i < Y_H; i++)
		{
			for (j = 0; j < (X_W); j++)
			{

				Check_Mem_WR_FIFO_not_Full();
#if STM32_FMC_8 | STM32_SPI
        UNUSED(temp);
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
#else
				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);
#endif
			}
		}
	}
	else
	{

		BTE_S0_Color_24bpp();
		BTE_S1_Color_24bpp();
		BTE_Destination_Color_24bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit_ColorDepth_24bpp
		for (i = 0; i < Y_H; i++)
		{
#if STM32_FMC_8 | STM32_SPI
			for (j = 0; j < (X_W); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite(*(data++));
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite(*(data++));
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite(*(data++));
			}
#else
			for (j = 0; j < (X_W / 2); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);

				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);

				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);
			}
#endif
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_Chroma_key_MCU8                                      */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*              Background_color: Color (chroma key) to be taken as transparent          */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*                         *data: Starting address of S0 picture                         */
/*                   Color_depth: Color depth											 */
/* Returns:     None                                                                     */
/* Description: MCU write S0 data to DT with chroma key                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_Chroma_key_MCU8(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned long Background_color, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char *data, 
	unsigned char Color_depth)
{
	unsigned int i, j;
	unsigned short temp;

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x04); // BTE Operation: MPU Write with chroma keying (w/o ROP)
	BTE_Enable();

	if (Color_depth == 16)
	{
		Background_color_65k(Background_color);
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bpp();
		BTE_Destination_Color_16bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit_ColorDepth_16bpp
		for (i = 0; i < Y_H; i++)
		{
			for (j = 0; j < (X_W); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
#if STM32_FMC_8 | STM32_SPI
        UNUSED(temp);
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
#else
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);
#endif
			}
		}
	}
	else // Color depth = 24
	{
		Background_color_16M(Background_color);
		BTE_S0_Color_24bpp();
		BTE_S1_Color_24bpp();
		BTE_Destination_Color_24bpp();

		SDRAM_ReadWrite(); // Memory Data Read/Write Port

		// MCU_8bit_ColorDepth_24bpp
		for (i = 0; i < Y_H; i++)
		{
#if STM32_FMC_8 | STM32_SPI
			for (j = 0; j < (X_W); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
				LCD_DataWrite(*(data++));
			}
#else
			for (j = 0; j < (X_W / 2); j++)
			{
				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);

				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);

				Check_Mem_WR_FIFO_not_Full();
				temp = *(data++);
				temp |= (*(data++) << 8);
				LCD_DataWrite_Pixel(temp);
			}
#endif
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_Pattern_Fill                                                   */
/*                                                                                       */
/* Parameters:                                                                           */
/*             P_8x8_or_16x16: 0: use 8x8 icon   1: use 16x16 icon                       */
/*                    S0_Addr: Starting address (SDRAM) of S0 picture                    */
/*                       S0_W: Width of the S0 picture                                   */
/*                        XS0: Left-top X coordinate of the S0 picture                   */
/*                        YS0: Left-top Y coordinate of the S0 picture                   */
/*                   Des_Addr: Starting address (SDRAM) of DT picture (DT: Destination)  */
/*                      Des_W: Width of the DT picture                                   */
/*                      XDes: Left-top X coordinate of the DT picture                    */
/*                      YDes: Left-top Y coordinate of the DT picture                    */
/*                  ROP_Code:                                                            */
/*                            0000b		0(Blackness)                                     */
/*                            0001b		~S0.~S1 or ~(S0+S1)                              */
/*                            0010b		~S0.S1                                           */
/*                            0011b		~S0                                              */
/*                            0100b		S0.~S1                                           */
/*                            0101b		~S1                                              */
/*                            0110b		S0^S1                                            */
/*                            0111b		~S0 + ~S1 or ~(S0 . S1)                          */
/*                            1000b		S0.S1                                            */
/*                            1001b		~(S0^S1)                                         */
/*                            1010b		S1                                               */
/*                            1011b		~S0+S1                                           */
/*                            1100b		S0                                               */
/*                            1101b		S0+~S1                                           */
/*                            1110b		S0+S1                                            */
/*                            1111b		1(whiteness)                                     */
/*                       X_W: Width of the active window                                 */
/*                       Y_H: Height of the active window                                */
/* 				 Color_depth: Color depth												 */
/* Returns:     None                                                                     */
/* Description: BTE pattern fill operation with ROP                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_Pattern_Fill1(
	unsigned char P_8x8_or_16x16, 
	unsigned long S0_Addr, 
	unsigned short S0_W, 
	unsigned short XS0, 
	unsigned short YS0, 
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned int ROP_Code, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned char Color_depth)
{
	if (P_8x8_or_16x16 == 0)
	{
		Pattern_Format_8X8();
	}
	if (P_8x8_or_16x16 == 1)
	{
		Pattern_Format_16X16();
	}

	if (Color_depth == 16)
	{
		BTE_S0_Color_16bpp();
		BTE_Destination_Color_16bpp();
	}
	else
	{
		BTE_S0_Color_24bpp();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0, YS0);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_ROP_Code(ROP_Code);
	BTE_Operation_Code(0x06); // BTE Operation: Pattern Fill with ROP.
	BTE_Window_Size(X_W, Y_H);

	write_758_reg(0x0C, 0x04);
	write_758_reg(0x0D, 0x00);
	BTE_Enable();
	while ((LCD_RegisterRead(0x0C) & 0x04) == 0)
		;
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_Chroma_key_MCU_16bit                                 */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*              Background_color: Color (chroma key) to be taken as transparent          */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*                         *data: Starting address of S0 picture                         */
/* Returns:     None                                                                     */
/* Description: MCU write S0 data to DT with chroma key                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_Chroma_key_MCU_16bit(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned long Background_color, 
	unsigned short X_W, 
	unsigned short Y_H, 
	const unsigned short *data)
{
	unsigned int i, j;

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x04); // BTE Operation: MPU Write with chroma keying (w/o ROP)
	BTE_Enable();

	LCD_CmdWrite(0x04); // Memory Data Read/Write Port

#if Display_16bpp
	Background_color_65k(Background_color);
	BTE_S0_Color_16bpp();
	BTE_Destination_Color_16bpp();

	// MCU_16bit_ColorDepth_16bpp
	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < (X_W); j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
		}
	}
#else
	Background_color_16M(Background_color);
	BTE_S0_Color_24bpp();
	BTE_Destination_Color_24bpp();

	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < (X_W); j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
			LCD_DataWrite(*(data++));
		}
	}
#endif

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_ColorExpansion_MCU_16bit                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*              Background_color: Background color                                       */
/*                         *data: Starting address of the written data                   */
/* Returns:     None                                                                     */
/* Description: MCU write data to DT with color expansion                                */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_ColorExpansion_MCU_16bit(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color,	/*Foreground_color : The source (1bit mapping picture) mapping data 1 is translated to Foreground color by color expansion*/
	unsigned long Background_color, /*Background_color : The source (1bit mapping picture) mapping data 0 is translated to Background color by color expansion*/
	const unsigned short *data)
{
	unsigned short i, j;

	RGB_16b_16bpp();

#if Display_16bpp
	Foreground_color_65k(Foreground_color);
	Background_color_65k(Background_color);
	BTE_Destination_Color_16bpp();
#else
	Foreground_color_16M(Foreground_color);
	Background_color_16M(Background_color);
	BTE_Destination_Color_24bpp();
#endif

	BTE_ROP_Code(15);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x8); // BTE Operation: MPU Write with Color Expansion (w/o ROP)
	BTE_Enable();

	LCD_CmdWrite(0x04); // Memory Data Read/Write Port

	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < X_W / 16; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      Des_Addr: Starting address (SDRAM) of DT picture (DT:Destination)*/
/*                         Des_W: Width of the DT picture                                */
/*                          XDes: Left-top X coordinate of the DT picture                */
/*                          YDes: Left-top Y coordinate of the DT picture                */
/*                           X_W: Width of the active window                             */
/*                           Y_H: Height of the active window                            */
/*              Foreground_color: Foreground color                                       */
/*                         *data: Starting address of the written data                   */
/* Returns:     None                                                                     */
/* Description: MCU write data to DT with color expansion and ignore background color    */
/*---------------------------------------------------------------------------------------*/
void LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit(
	unsigned long Des_Addr, 
	unsigned short Des_W, 
	unsigned short XDes, 
	unsigned short YDes, 
	unsigned short X_W, 
	unsigned short Y_H, 
	unsigned long Foreground_color, 	/*Foreground_color : The source (1bit map picture) map data 1 is translated to Foreground color by color expansion*/
	const unsigned short *data)
{
	unsigned short i, j;

	RGB_16b_16bpp();

#if Display_16bpp
	Foreground_color_65k(Foreground_color);
	BTE_Destination_Color_16bpp();
#else
	Foreground_color_16M(Foreground_color);
	BTE_Destination_Color_24bpp();
#endif

	BTE_ROP_Code(15);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes, YDes);

	BTE_Window_Size(X_W, Y_H);
	BTE_Operation_Code(0x9); // BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)
	BTE_Enable();

	LCD_CmdWrite(0x04); // Memory Data Read/Write Port

	for (i = 0; i < Y_H; i++)
	{
		for (j = 0; j < X_W / 16; j++)
		{
			Check_Mem_WR_FIFO_not_Full();
			LCD_DataWrite_Pixel(*data);
			data++;
		}
	}

	Check_Mem_WR_FIFO_Empty();
	Check_BTE_Busy();
}

//----------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_PWM0_Init                                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      on_off: 0: disable PWM0    1: enable PWM0                        */
/*               Clock_Divided: PWM clock, setting range 0~3(1,1/2,1/4,1/8)              */
/*                   Prescalar: Clock scaling, setting range 1~256                       */
/*                Count_Buffer: PWM output cycle                                         */
/*              Compare_Buffer: Duty cycle                                               */
/* Returns:     None                                                                     */
/* Description: Initialize PWM0                                                          */
/*---------------------------------------------------------------------------------------*/
void LT758_PWM0_Init(
	unsigned char on_off, 
	unsigned char Clock_Divided, 
	unsigned char Prescalar, 
	unsigned short Count_Buffer, 
	unsigned short Compare_Buffer)
{
	Select_PWM0();
	Auto_Reload_PWM0();
	Set_PWM_Prescaler_1_to_256(Prescalar);

	switch (Clock_Divided)
	{
	case 0:
		Select_PWM0_Clock_Divided_By_1();
		break;
	case 1:
		Select_PWM0_Clock_Divided_By_2();
		break;
	case 2:
		Select_PWM0_Clock_Divided_By_4();
		break;
	case 3:
		Select_PWM0_Clock_Divided_By_8();
		break;
	default:
		Select_PWM0_Clock_Divided_By_1();
		break;
	}

	Set_Timer0_Count_Buffer(Count_Buffer);
	Set_Timer0_Compare_Buffer(Compare_Buffer);

	if (on_off == 1)
		Start_PWM0();
	if (on_off == 0)
		Stop_PWM0();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_PWM0_Duty                                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*              Compare_Buffer: Duty cycle                                               */
/* Returns:     None                                                                     */
/* Description: Set PWM0 duty cycle                                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_PWM0_Duty(unsigned short Compare_Buffer)
{
	Set_Timer0_Compare_Buffer(Compare_Buffer);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_PWM1_Init                                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      on_off: 0: disable PWM0    1: enable PWM0                        */
/*               Clock_Divided: PWM clock, setting range 0~3(1,1/2,1/4,1/8)              */
/*                   Prescalar: Clock scaling, setting range 1~256                       */
/*                Count_Buffer: PWM output cycle                                         */
/*              Compare_Buffer: Duty cycle                                               */
/* Returns:     None                                                                     */
/* Description: Initialize PWM1                                                          */
/*---------------------------------------------------------------------------------------*/
void LT758_PWM1_Init(
	unsigned char on_off, 
	unsigned char Clock_Divided, 
	unsigned char Prescalar, 
	unsigned short Count_Buffer, 
	unsigned short Compare_Buffer)
{
	Select_PWM1();
	Auto_Reload_PWM1();
	Set_PWM_Prescaler_1_to_256(Prescalar);

	switch (Clock_Divided)
	{
	case 0:
		Select_PWM1_Clock_Divided_By_1();
		break;
	case 1:
		Select_PWM1_Clock_Divided_By_2();
		break;
	case 2:
		Select_PWM1_Clock_Divided_By_4();
		break;
	case 3:
		Select_PWM1_Clock_Divided_By_8();
		break;
	default:
		Select_PWM1_Clock_Divided_By_1();
		break;
	}

	Set_Timer1_Count_Buffer(Count_Buffer);
	Set_Timer1_Compare_Buffer(Compare_Buffer);

	if (on_off == 1)
		Start_PWM1();
	if (on_off == 0)
		Stop_PWM1();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_PWM1_Duty                                                          */
/*                                                                                       */
/* Parameters:                                                                           */
/*              Compare_Buffer: Duty cycle                                               */
/* Returns:     None                                                                     */
/* Description: Set PWM1 duty cycle                                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_PWM1_Duty(unsigned short Compare_Buffer)
{
	Set_Timer1_Compare_Buffer(Compare_Buffer);
}

//----------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Standby                                                            */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set LT758x enter standby mode                                            */
/*---------------------------------------------------------------------------------------*/
void LT758_Standby(void)
{
	Power_Saving_Standby_Mode();
	Check_Power_is_Saving();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Wkup_Standby                                                       */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Wake up LT758x from standby mode                                         */
/*---------------------------------------------------------------------------------------*/
void LT758_Wkup_Standby(void)
{
	Power_Normal_Mode();
	Check_Power_is_Normal();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Suspend                                                            */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set LT758x enter suspend mode                                            */
/*---------------------------------------------------------------------------------------*/
void LT758_Suspend(void)
{
	LT758_SDRAM_initial(10);
	Power_Saving_Suspend_Mode();
	Check_Power_is_Saving();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Wkup_Suspend                                                       */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Wake up LT758x from suspend mode                                         */
/*---------------------------------------------------------------------------------------*/
void LT758_Wkup_Suspend(void)
{
	Power_Normal_Mode();
	Check_Power_is_Normal();
	LT758_SDRAM_initial(MCLK);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_SleepMode                                                          */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set LT758x enter sleep mode                                              */
/*---------------------------------------------------------------------------------------*/
void LT758_SleepMode(void)
{
	Power_Saving_Sleep_Mode();
	Check_Power_is_Saving();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_Wkup_Sleep                                                         */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Wake up LT758x from sleep mode                                           */
/*---------------------------------------------------------------------------------------*/
void LT758_Wkup_Sleep(void)
{
	Power_Normal_Mode();
	Check_Power_is_Normal();
}
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_SPI_Init                                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*               cs: 0: SPI0    1: SPI1                                                  */
/*              div: Set clock period                                                    */
/* Returns:     None                                                                     */
/* Description: Initialize LT758x SPI (e.g. to SPI Flash)                                */
/*---------------------------------------------------------------------------------------*/
void LT758_SPI_Init(unsigned char cs, unsigned char div)
{
	if (cs == 0)
	{
		Select_SFI_0();
		Select_nSS_drive_on_xnsfcs0();
	}
	else
	{
		Select_SFI_1();
		Select_nSS_drive_on_xnsfcs1();
	}
	SPI_Clock_Period(div);
	Enable_SFlash_SPI();
}

//--------------------------------------------------- MCU write Flash -------------------------------------------------------------
// Flash Instruction Table
#define W25X_WriteEnable		0x06       
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02
#define W25X_BlockErase32KB		0x52
#define W25X_BlockErase64KB		0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_ReadSR                                                      */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     Byte                                                                     */
/*              BIT 7   6   5    4    3    2    1    0                                   */
/*                 SPR  RV  TB  BP2  BP1  BP0  WEL  BUSY                                 */
/*              Default: 0x00                                                            */
/* Description: Read Status Register of the SPI Flash                                    */
/*---------------------------------------------------------------------------------------*/
unsigned char LT758_W25QXX_ReadSR(void)   //Read W25QXX status register
{
	unsigned char byte=0;   
	nSS_Active();                         		 	 //Select the device   
	SPI_Master_FIFO_Data_Put(W25X_ReadStatusReg); 	 //Send the Read Status command    
	byte=SPI_Master_FIFO_Data_Put(0Xff);             //Read one byte  
	nSS_Inactive();                                  //Disselect the device     
	return byte;
} 

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Write_Enable                                                */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Enable "Write to Flash"                                                  */
/*---------------------------------------------------------------------------------------*/ 
void LT758_W25QXX_Write_Enable(void)   
{
	nSS_Active();                      				//Select the device   
	SPI_Master_FIFO_Data_Put(W25X_WriteEnable); 	//Send the Write Enable command
	nSS_Inactive();									//Disselect the device
}
	
/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Write_Disable                                               */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Disable "Write to Flash"                                                 */
/*---------------------------------------------------------------------------------------*/   
void LT758_W25QXX_Write_Disable(void)   
{
	nSS_Active();                      				//Select the device   
	SPI_Master_FIFO_Data_Put(W25X_WriteDisable); 	//Send the Write Disalbe command    
	nSS_Inactive(); 								//Disselect the device 
} 		

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Wait_Busy                                                   */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Wait while the flash is busy                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_W25QXX_Wait_Busy(void)   		
{   
	while((LT758_W25QXX_ReadSR()&0x01)==0x01) ;  	//Wait while the flash is busy
} 

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Quad_Enable                                                 */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Enable the flash device                                                  */
/*---------------------------------------------------------------------------------------*/ 
void LT758_W25QXX_Quad_Enable(void)
{
	unsigned char byte=0;   
	nSS_Active();                            	//Select the device   
	SPI_Master_FIFO_Data_Put(0x35); 			//Send the Read Status command    
	byte=SPI_Master_FIFO_Data_Put(0Xff);        //Read one byte  
	nSS_Inactive();                            	//Disselect the device   

	byte |= 0x02;

	LT758_W25QXX_Write_Enable();
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0x31);
	SPI_Master_FIFO_Data_Put(byte);
	nSS_Inactive();
	LT758_W25QXX_Wait_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Enter_4Byte_AddressMode                                     */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set 4Byte addressing mode for Winbond flash                              */
/*---------------------------------------------------------------------------------------*/
void LT758_W25QXX_Enter_4Byte_AddressMode(void)
{
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0xB7);
	nSS_Inactive();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25QXX_Read                                                        */
/*                                                                                       */
/* Parameters:                                                                           */
/*                    pBuffer: Buffer for storing read data                              */
/*                   ReadAddr: Starting address of the Flash for reading data            */
/*              NumByteToRead: Amount of data to read                                    */
/* Returns:     None                                                                     */
/* Description: To retrieve data from Flash                                              */
/*---------------------------------------------------------------------------------------*/
void LT758_W25QXX_Read(
	unsigned char *pBuffer, 
	unsigned long ReadAddr, 
	unsigned int NumByteToRead)
{
	unsigned int i;
	nSS_Active();
	SPI_Master_FIFO_Data_Put(W25X_ReadData);
	SPI_Master_FIFO_Data_Put((unsigned char)((ReadAddr) >> 16));
	SPI_Master_FIFO_Data_Put((unsigned char)((ReadAddr) >> 8));
	SPI_Master_FIFO_Data_Put((unsigned char)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI_Master_FIFO_Data_Put(0XFF);
	}
	nSS_Inactive();
}

//--------------------------------------------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_ReadSR                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*              reg: register index                                                      */
/* Returns:                                                                              */
/*              register status                                                          */
/* Description: Read register value                                                      */
/*---------------------------------------------------------------------------------------*/
unsigned char LT758_W25NXX_ReadSR(unsigned char reg)
{
	unsigned char byte = 0;
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0x0f);
	SPI_Master_FIFO_Data_Put(reg);
	byte = SPI_Master_FIFO_Data_Put(0Xff);
	nSS_Inactive();
	return byte;
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_Write_SR                                                  */
/*                                                                                       */
/* Parameters:                                                                           */
/*              reg: register index                                                      */
/*              val: value to be written                                                 */
/* Returns:     None                                                                     */
/* Description: Write a value to the designated register                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_Write_SR(
	unsigned char reg, 
	unsigned char val)
{
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0x1f);
	SPI_Master_FIFO_Data_Put(reg);
	SPI_Master_FIFO_Data_Put(val);
	nSS_Inactive();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_Wait_Busy                                                 */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Wait until the read status process done                                  */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_Wait_Busy(void)
{
	while ((LT758_W25NXX_ReadSR(0xC0) & 0x01) == 0x01)
		;
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_ContinuousRead_Mode                                       */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: Set continuous read mode                                                 */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_ContinuousRead_Mode(void)
{
	unsigned char statut = 0;
	statut = LT758_W25NXX_ReadSR(0xb0);
	statut &= cClrb3;
	statut &= cClrb4;
	statut |= cSetb4;
	LT758_W25NXX_Write_SR(0xb0, statut);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_Write_Page                                                */
/*                                                                                       */
/* Parameters:                                                                           */
/*              page: page index                                                         */
/* Returns:     None                                                                     */
/* Description: Write to the designated page                                             */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_Write_Page(unsigned long page)
{
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0x13);
	SPI_Master_FIFO_Data_Put((unsigned char)(page >> 16));
	SPI_Master_FIFO_Data_Put((unsigned char)(page >> 8));
	SPI_Master_FIFO_Data_Put((unsigned char)page);
	nSS_Inactive();
	LT758_W25NXX_Wait_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_ReadPageAddr_Data                                         */
/*                                                                                       */
/* Parameters:                                                                           */
/*                    pBuffer: The buffer to store the read data                         */
/*                    PageNum: Page index                                                */
/*                   PageAddr: Starting address of the data in the designate page        */
/*              NumByteToRead: The amount of data to be read                             */
/* Returns:     None                                                                     */
/* Description: Retrieve data form flash, and store them to designated buffer            */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_ReadPageAddr_Data(unsigned char *pBuffer, unsigned long PageNum, unsigned long PageAddr, unsigned int NumByteToRead)
{
	unsigned int i;

	//------ Retrieve data from flash, and store the data to BUFF-----
	nSS_Active(); // Enable flash
	SPI_Master_FIFO_Data_Put(0x13);
	SPI_Master_FIFO_Data_Put((unsigned char)((PageNum) >> 16)); // Sending address
	SPI_Master_FIFO_Data_Put((unsigned char)((PageNum) >> 8));
	SPI_Master_FIFO_Data_Put((unsigned char)(PageNum));
	nSS_Inactive();
	LT758_W25NXX_Wait_Busy();
	LT758_W25NXX_Write_SR(0xb0, LT758_W25NXX_ReadSR(0xb0) | 0x08); // Set the starting address to read the data

	//------ Get the BUFF data and write to pBuffer ------
	nSS_Active();
	SPI_Master_FIFO_Data_Put(0x03);
	SPI_Master_FIFO_Data_Put((PageAddr) >> 8);
	SPI_Master_FIFO_Data_Put(PageAddr);
	SPI_Master_FIFO_Data_Put(0x00);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI_Master_FIFO_Data_Put(0XFF); // Read the data in loop
	}
	nSS_Active();
	LT758_W25NXX_Wait_Busy();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_BadBlockRemap                                             */
/*                                                                                       */
/* Parameters:                                                                           */
/*                    on_off: enable or disable bad block remap                          */
/*               logic_block: logic block number (bad block)                             */
/*            physical_block: physical block number (normal block)                       */
/*                  link_num: link reg nuber                                             */
/* Returns:     None                                                                     */
/* Description: Write Bad Block Table                                                    */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_BadBlockRemap(unsigned char on_off,unsigned int logic_block,unsigned int physical_block,unsigned char link_num)
{
	unsigned char tempReg;
	unsigned long remap_link0;
	LCD_CmdWrite(0x00);
	tempReg = LCD_DataRead();
	LCD_DataWrite(tempReg | 0x08);//Enable Register Group

	remap_link0 = (on_off << 31) | ((logic_block) << 16) | (physical_block);

	LCD_CmdWrite(0xB0+(link_num*4));
	LCD_DataWrite(remap_link0);
	LCD_CmdWrite(0xB1+(link_num*4));
	LCD_DataWrite(remap_link0 >> 8);
	LCD_CmdWrite(0xB2+(link_num*4));
	LCD_DataWrite(remap_link0 >> 16);
	LCD_CmdWrite(0xB3+(link_num*4));
	LCD_DataWrite(remap_link0 >> 24);

	LCD_CmdWrite(0x00);
	tempReg = LCD_DataRead();
	LCD_DataWrite(tempReg & 0xF7);//Disable Register Group
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_W25NXX_Read                                                      */
/*                                                                                       */
/* Parameters:                                                                           */
/*                    pBuffer: The buffer to store the read data                         */
/*                   PageAddr: Starting address of the data in the designate page        */
/*              NumByteToRead: The amount of data to be read                             */
/* Returns:     None                                                                     */
/* Description: Read designated amount of data from flash                                */
/*---------------------------------------------------------------------------------------*/
void LT758_W25NXX_Read(unsigned char *pBuffer, unsigned long ReadAddr, unsigned int NumByteToRead) // Read less than 2k of data at a time, starting from page 0
{
	unsigned long page, endpage, remainder;
	remainder = ReadAddr % 2048;				   // Starting address of the picture in a page
	page = (ReadAddr / 2048);					   // The page number of the starting address of the picture
	endpage = ((ReadAddr + NumByteToRead) / 2048); // The page number of the ending address of the picture
	if (page == endpage)
	{
		LT758_W25NXX_ReadPageAddr_Data(pBuffer, page, remainder, NumByteToRead);
	}
	else
	{
		LT758_W25NXX_ReadPageAddr_Data(pBuffer, page, remainder, 2048 - remainder);
		LT758_W25NXX_ReadPageAddr_Data(&pBuffer[2048 - remainder], endpage, 0, NumByteToRead - (2048 - remainder));
	}
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_JPG                                                            */
/*                                                                                       */
/* Parameters:                                                                           */
/*                      X1: X coordinate of the SDRAM destination (Canvas)               */
/*                      Y1: Y coordinate of the SDRAM destination (Canvas)               */
/*					  Addr:	Starting address (in the SPI Flash) of the JPG picture 		 */
/* Returns:     None                                                                     */
/* Description: To retrieve JPG picture from SPI Flash, decode the JPG data, and store   */
/*              the data to SDRAM, starting at the designated address.                   */
/*---------------------------------------------------------------------------------------*/
unsigned char LT758_DMA_JPG(
	unsigned short X1, 
	unsigned short Y1, 
	unsigned long Addr)
{
	Enable_SFlash_SPI(); // Enable SPI interface
	Graphic_Mode();		 // Set as Graphic Mode

	DMA_Read_6BH(); // Set as Fast Read Quard Output

	Reset_CPOL(); // Set SPI operation mode: Mode 0
	Reset_CPHA(); // Set SPI operation mode: Mode 0

	SPI_Clock_Period(0); // Set SPI clock cycle

	Memory_XY_Mode(); // Set as Block mode

	SFI_DMA_Destination_Upper_Left_Corner(X1, Y1);

	/******************* Key code for JPG **************************/
	SFI_DMA_Source_Start_Address(JPG_Key_Addr); // Starting address of the JPG key code
	Start_JPG_DMA();
	Check_Busy_JPG_DMA();
	/******************* Key code for JPG **************************/

	SFI_DMA_Source_Start_Address(Addr); // Starting address of the JPG picture
	Start_JPG_DMA();
	Check_Busy_JPG_DMA();

	LCD_RegisterWrite(0xB6, LCD_RegisterRead(0xB6) & 0xFD); // Set back to regular DMA
	
	DMA_Read_3BH();

	return ((LCD_RegisterRead(0xB6) & 0xF0) >> 4); // Return the decoding result
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_DMA_PNG                                                            */
/*                                                                                       */
/* Parameters:                                                                           */
/*             PNG_Color_depth: 0:8bit 1:16bit 2:32bit                                   */
/*              PNG_Flash_Addr: Starting address (in the SPI Flash) of the PNG picture   */
/*           PNG_Display_Layer: Address of the display layer                             */
/*                       PNG_X: String position, starting x coordinate                   */
/*                       PNG_Y: String position, starting y coordinat                    */
/*                       PNG_W: Width of the data to be transmitted, e.g. picture width  */
/*                       PNG_H: Height of the data to be transmitted, e.g. picture height*/
/*              PNG_Buff_Layer: Buff address in SDRAM                                    */
/*                     BG_Addr: Background start address                                 */
/*                        BG_X: Background x coordinate                                  */
/*                        BG_Y: Background y coordinate                                  */
/* Returns:     None                                                                     */
/* Description: To retrieve PNG picture from SPI Flash                                   */
/*---------------------------------------------------------------------------------------*/
void LT758_DMA_PNG(
	unsigned char PNG_Color_depth, 
	unsigned long PNG_Flash_Addr, 
	unsigned long PNG_Display_Layer, 
	unsigned short PNG_X, 
	unsigned short PNG_Y, 
	unsigned short PNG_W, 
	unsigned short PNG_H, 
	unsigned long PNG_Buff_Layer, 
	unsigned long BG_Layer, 
	unsigned short BG_X, 
	unsigned short BG_Y)
{
	unsigned char alpha_bit = 0;
	Canvas_Image_Start_address(PNG_Buff_Layer);
	Canvas_image_width(LCD_XSIZE_TFT);
	if(PNG_Color_depth == 0)
		Memory_8bpp_Mode();
	else if(PNG_Color_depth == 1)
		Memory_16bpp_Mode();
	else if(PNG_Color_depth == 2)
		Memory_32bpp_Mode();
	LT758_DMA_24bit_Block(1,0,0,0,PNG_W,PNG_H,PNG_W,PNG_Flash_Addr);
#if Display_16bpp
	Memory_16bpp_Mode();
	if(PNG_Color_depth == 0)
		alpha_bit = 2;
	else if(PNG_Color_depth == 1)
		alpha_bit = 4;
	else if(PNG_Color_depth == 2)
		alpha_bit = 0;
#else
	Memory_24bpp_Mode();
	if(PNG_Color_depth == 1)
		alpha_bit = 1;
	else if(PNG_Color_depth == 2)
		alpha_bit = 8;
#endif
	Canvas_Image_Start_address(PNG_Display_Layer);
	Canvas_image_width(LCD_XSIZE_TFT);
	LT758_BTE_Pixel_Alpha_Blending(BG_Layer,LCD_XSIZE_TFT,BG_X,BG_Y,PNG_Buff_Layer,LCD_XSIZE_TFT,0,0,PNG_Display_Layer,LCD_XSIZE_TFT,PNG_X,PNG_Y,PNG_W,PNG_H,alpha_bit);
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_I2C_Init                                                           */
/*                                                                                       */
/* Parameters:                                                                           */
/*              div: Set Clock Prescale                                                  */
/* Returns:     None                                                                     */
/* Description: Initialize LT758x I2C Master SDA pin & SCL pin                           */
/*---------------------------------------------------------------------------------------*/
void LT758_I2C_Init(unsigned char div)
{
	LT758_I2CM_Clock_Prescale(div);
	LT758_I2CM_Enable();
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_I2C_Write_NByte                                                    */
/*                                                                                       */
/* Parameters:                                                                           */
/*               slave_addr: slave device address                                        */
/*                      reg: slave device reg                                            */
/*					    num: The amount of data to be written                            */
/*                       *p: data buff                                                   */
/* Returns:     None                                                                     */
/* Description: I2C send data                                                            */
/*---------------------------------------------------------------------------------------*/
void LT758_I2C_Write_NByte(
	unsigned char slave_addr,
	unsigned char reg,
	unsigned int num, 
	unsigned char *p)
{
	unsigned int i=0;
	LT758_I2CM_Transmit_Data(slave_addr & 0xFE);
	LT758_I2CM_Write_With_Start();
	while(LT758_I2CM_Check_Slave_ACK());
	while(LT758_I2CM_transmit_Progress());
	
	LT758_I2CM_Transmit_Data(reg);
	LT758_I2CM_Write();
	while(LT758_I2CM_Check_Slave_ACK());
	while(LT758_I2CM_transmit_Progress());
	
	for(;i<num;i++)
	{
		LT758_I2CM_Transmit_Data(p[i]);
		LT758_I2CM_Write();
		while(LT758_I2CM_Check_Slave_ACK());
		while(LT758_I2CM_transmit_Progress());
	}

	LT758_I2CM_Stop();
	while(LT758_I2CM_Bus_Busy());		
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_I2C_Read_NByte                                                     */
/*                                                                                       */
/* Parameters:                                                                           */
/*               slave_addr: slave device address                                        */
/*                      reg: slave device reg                                            */
/*					    num: The amount of data to be read                               */
/*                       *p: data buff                                                   */
/* Returns:     None                                                                     */
/* Description: I2C send data                                                            */
/*---------------------------------------------------------------------------------------*/
void LT758_I2C_Read_NByte(
	unsigned char slave_addr,
	unsigned char reg,
	unsigned int num, 
	unsigned char *p)
{
	unsigned int i = 0;
	LT758_I2CM_Transmit_Data(slave_addr & 0xFE);
	LT758_I2CM_Write_With_Start();
	while(LT758_I2CM_Check_Slave_ACK());
	while(LT758_I2CM_transmit_Progress());
	
	LT758_I2CM_Transmit_Data(reg);
	LT758_I2CM_Write();
	while(LT758_I2CM_Check_Slave_ACK());
	while(LT758_I2CM_transmit_Progress());
	
	LT758_I2CM_Stop();
	while(LT758_I2CM_Bus_Busy());
	
	LT758_I2CM_Transmit_Data(slave_addr | 0x01);
	LT758_I2CM_Write_With_Start();
	while(LT758_I2CM_Check_Slave_ACK());
	while(LT758_I2CM_transmit_Progress());
	
	for(;i<num;i++)
	{
		if(i == num-1)
			LT758_I2CM_Read_With_Nack();
		else
			LT758_I2CM_Read_With_Ack();	
		while(LT758_I2CM_transmit_Progress());
		p[i]=LT758_I2CM_Receiver_Data();
	}

	LT758_I2CM_Stop();
	while(LT758_I2CM_Bus_Busy());
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_disp_V_Gray_888                                                    */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: display rgb888 gray                                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_disp_V_Gray_888(void)
{
	int i;
	int w0;
	int h0;

	unsigned long c, r, g, b;

	w0 = LCD_XSIZE_TFT / 4;
	
	if (LCD_YSIZE_TFT < 256)
	{
		h0 = 1;

		for (i = 0; i < LCD_YSIZE_TFT; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(0, i, w0 - 1, i + 1, (c << 16) & 0xFF0000);
		}

		for (i = 0; i < LCD_YSIZE_TFT; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(w0, i, w0 * 2 - 1, i + 1, (c << 8) & 0xFF00);
		}

		for (i = 0; i < LCD_YSIZE_TFT; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(w0 * 2, i, w0 * 3 - 1, i + 1, c);
		}

		for (i = 0; i < LCD_YSIZE_TFT; i++)
		{
			r = 255 - i;
			g = 255 - i;
			b = 255 - i;

			c = ((r << 16) & 0xFF0000) | ((g << 8) & 0xFF00) | (b & 0xFF);

			LT758_DrawSquare_Fill(w0 * 3, i, LCD_XSIZE_TFT - 1, i + 1, c);
		}
	}
	else
	{
		h0 = LCD_YSIZE_TFT / 256;

		for (i = 0; i < 255; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(0, h0 * i, w0 - 1, h0 * (i + 1), (c << 16) & 0xFF0000);
		}
		c = 0;
		LT758_DrawSquare_Fill(0, h0 * 255, w0 - 1, LCD_YSIZE_TFT - 1, c);

		for (i = 0; i < 255; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(w0, h0 * i, w0 * 2 - 1, h0 * (i + 1), (c << 8) & 0xFF00);
		}
		c = 0;
		LT758_DrawSquare_Fill(w0, h0 * 255, w0 * 2 - 1, LCD_YSIZE_TFT - 1, c);

		for (i = 0; i < 255; i++)
		{
			c = 255 - i;
			LT758_DrawSquare_Fill(w0 * 2, h0 * i, w0 * 3 - 1, h0 * (i + 1), c);
		}
		c = 0;
		LT758_DrawSquare_Fill(w0 * 2, h0 * 255, w0 * 3 - 1, LCD_YSIZE_TFT - 1, c);

		for (i = 0; i < 255; i++)
		{
			r = 255 - i;
			g = 255 - i;
			b = 255 - i;
			c = ((r << 16) & 0xFF0000) | ((g << 8) & 0xFF00) | (b & 0xFF);

			LT758_DrawSquare_Fill(w0 * 3, h0 * i, LCD_XSIZE_TFT - 1, h0 * (i + 1), c);
		}

		c = 0;
		LT758_DrawSquare_Fill(w0 * 3, h0 * 255, LCD_XSIZE_TFT - 1, LCD_YSIZE_TFT - 1, c);
	}
}

/*---------------------------------------------------------------------------------------*/
/* Function:    LT758_disp_V_Gray_565                                                    */
/*                                                                                       */
/* Parameters:  None                                                                     */
/* Returns:     None                                                                     */
/* Description: display rgb565 gray                                                      */
/*---------------------------------------------------------------------------------------*/
void LT758_disp_V_Gray_565(void)
{
      int i,w0,h0;
      unsigned long c;
      
      w0 = LCD_XSIZE_TFT/3;
      
      h0 = LCD_YSIZE_TFT/32;
      for(i=0;i<31;i++) 
      {
            c = 31 - i;
            LT758_DrawSquare_Fill(0,h0*i,w0-1,h0*(i+1),(c<<11)&0xF800);
      }
      c = 0;
      LT758_DrawSquare_Fill(0,h0*31,w0-1,LCD_YSIZE_TFT-1,c);
      
      
      h0 = LCD_YSIZE_TFT/64;              
      for(i=0;i<63;i++) 
      {
            c = 63 - i;
            LT758_DrawSquare_Fill(w0,h0*i,w0*2-1,h0*(i+1),(c<<5)&0x7E0);
      }
      c = 0;
      LT758_DrawSquare_Fill(w0,h0*63,w0*2-1,LCD_YSIZE_TFT-1,c);
         
      
      h0 = LCD_YSIZE_TFT/32;
      for(i=0;i<31;i++) 
      {
            c = 31 - i;
            LT758_DrawSquare_Fill(w0*2,h0*i,LCD_XSIZE_TFT-1,h0*(i+1),c&0x1F);
      }
      c = 0;
      LT758_DrawSquare_Fill(w0*2,h0*31,LCD_XSIZE_TFT-1,LCD_YSIZE_TFT-1,c);
}
