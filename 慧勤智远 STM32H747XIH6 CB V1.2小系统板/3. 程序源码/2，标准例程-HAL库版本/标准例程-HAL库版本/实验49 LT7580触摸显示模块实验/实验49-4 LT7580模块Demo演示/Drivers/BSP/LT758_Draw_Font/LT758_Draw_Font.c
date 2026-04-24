/******************************************************************************
  * Copyright(c) 2024 Levetop Semiconductor Co., Ltd. All rights reserved
  * @file      LT758_Draw_Font.c
  * @author    Levetop TFT Controller Application Team
  * @version   V1.0.0
  * @date      2024-12-30
  * @brief     LT758x Draw Font function
*******************************************************************************/
#include "./BSP/LT758_Draw_Font/LT758_Draw_Font.h"
#include "./BSP/LT758/LT758.h"

uint8_t Screen_DIR = 0;
uint8_t Ascii_w[95] = {0};
uint8_t TFT_bitcolor = !Display_16bpp;
uint8_t cSetb[8] = {cSetb0, cSetb1, cSetb2, cSetb3, cSetb4, cSetb5, cSetb6, cSetb7};
uint8_t cSetb_2bit[4] = {0xc0, 0x30, 0x0c, 0x03};
uint8_t cSetb_4bit[2] = {0xF0, 0x0F};
uint8_t fdata[8 * 128 * 2];

void Flash_Read_UI(unsigned char* pBuffer,unsigned long ReadAddr,unsigned int NumByteToRead)
{
	if(Flash_Type == 0)
	{
		LT758_W25QXX_Read(pBuffer,ReadAddr,NumByteToRead);
	}
	else if(Flash_Type == 1)
	{
		LT758_W25NXX_Read(pBuffer,ReadAddr,NumByteToRead);
	}
}


/*---------------------------------------------------------------------------------------*/
/* Function:    BTE_Pixel_16bbp_or_24bpp_Alpha_Blending_ZK                               */
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
/* Returns:     None                                                                     */
/* Description: BTE memory copy with opacity setting                                     */
/*---------------------------------------------------------------------------------------*/
void BTE_Pixel_16bbp_or_24bpp_Alpha_Blending_ZK
(
 uint32_t S0_Addr      
 ,uint16_t S0_W       
 ,uint16_t XS0        
 ,uint16_t YS0        
 ,uint32_t S1_Addr     
 ,uint16_t S1_W       
 ,uint16_t XS1        
 ,uint16_t YS1        
 ,uint32_t Des_Addr    
 ,uint16_t Des_W      
 ,uint16_t XDes       
 ,uint16_t YDes       
 ,uint16_t X_W        
 ,uint16_t Y_H        
)
{	

	if (TFT_bitcolor == 0)
	{
		BTE_S0_Color_16bpp();
		BTE_S1_Color_16bit_Alpha();
		BTE_Destination_Color_16bpp();
	}
	else if (TFT_bitcolor == 1)
	{
		BTE_S0_Color_24bpp();
		BTE_S1_Color_16bit_Alpha();
		BTE_Destination_Color_24bpp();
	}

	BTE_S0_Memory_Start_Address(S0_Addr);
	BTE_S0_Image_Width(S0_W);
	BTE_S0_Window_Start_XY(XS0,YS0);


	BTE_S1_Memory_Start_Address(S1_Addr);
	BTE_S1_Image_Width(S1_W); 
	BTE_S1_Window_Start_XY(XS1,YS1);

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);

	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x0A);		//BTE Operation: Memory write with opacity (w/o ROP)
	BTE_Enable();
	Check_BTE_Busy();
}

void LT758_Draw_16bit
(
	uint16_t X1 		// Location transferred to memory X1
	,
	uint16_t Y1 		// Location transferred to memory Y1
	,
	uint16_t X_W	 	// Width of DMA transmission data
	,
	uint16_t Y_H 		// Height of DMA transmission data
	,
	uint16_t P_W 		// Width of picture
	,
	uint16_t pixel_format // Gray scale
	,
	uint32_t F_Color 	// Font color
	,
	uint32_t Addr 		// Flash address
	,
	uint32_t lay0 		// Address of layer
	,
	uint16_t canvas_w 	// Layer width
)
{
	uint16_t a, b = 0, c;
	uint16_t color_temp;
	uint16_t temp;
	uint16_t r, g;

	Flash_Read_UI(fdata, Addr, X_W * Y_H);

	if (TFT_bitcolor == 0)
	{
		if (pixel_format > 0)
		{
			r = (F_Color >> 12) & 0x0f;
			g = (F_Color >> 7) & 0x0f;
			b = (F_Color >> 1) & 0x0f;
			F_Color = (((r << 8) + (g << 4) + b)) & 0x0FFF;
		}
	}
	else if (TFT_bitcolor == 1)
	{
		if (pixel_format > 0)
		{
			r = (F_Color >> 20) & 0x0f;
			g = (F_Color >> 12) & 0x0f;
			b = (F_Color >> 4) & 0x0f;
			F_Color = (((r << 8) + (g << 4) + b)) & 0x0FFF;
		}
	}
	if (F_Color == 0)
		F_Color += 1;

	Graphic_Mode();
	Canvas_Image_Start_address(lay0);
	Canvas_image_width(canvas_w);
	Active_Window_XY(X1, Y1);
	if (Screen_DIR == 0 || Screen_DIR == 2)
		Active_Window_WH(P_W, Y_H);
	else if (Screen_DIR == 1 || Screen_DIR == 3)
		Active_Window_WH(Y_H, P_W);


	Goto_Pixel_XY(X1, Y1);

	LCD_CmdWrite(0x04);

	if (Screen_DIR == 0) // 0 degrees
	{
		if (pixel_format == 0)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 0; b < 8; b++)
					{
						if (fdata[X_W * a + c] & cSetb[7 - b])
						{
							LCD_DataWrite(F_Color);
							LCD_DataWrite(F_Color>>8);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(F_Color>>16);


						}
						else
						{
							LCD_DataWrite(0);
							LCD_DataWrite(0);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(0);

						}
					}
				}
			}
		}
		else if (pixel_format == 1)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 0; b < 4; b++)
					{
						temp = (fdata[X_W * a + c] & cSetb_2bit[b]);
						if (b == 0)
							temp = temp >> 6;
						else if (b == 1)
							temp = temp >> 4;
						else if (b == 2)
							temp = temp >> 2;
						if (temp > 0)
						{
							if (temp == 1)
								color_temp = F_Color | (0x04 << 12);
							else if (temp == 2)
								color_temp = F_Color | (0x08 << 12);
							else if (temp == 3)
								color_temp = F_Color | (0x0F << 12);
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);


						}
						else
						{
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 2)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 0; b < 2; b++)
					{
						temp = (fdata[X_W * a + c] & cSetb_4bit[b]);
						if (b == 0)
							temp = temp >> 4;
						if (temp > 0)
						{
							color_temp = F_Color | (temp << 12);
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
	}
	else if (Screen_DIR == 1) // 90 degrees
	{
		if (pixel_format == 0)
		{
			for (c = 0; c < X_W; c++)
			{
				for (b = 0; b < 8; b++)
				{
					for (a = 0; a < Y_H; a++)
					{
						if (fdata[X_W * (Y_H - 1 - a) + c] & cSetb[7 - b])
						{
							
							LCD_DataWrite(F_Color);
							LCD_DataWrite(F_Color>>8);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(F_Color>>16);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 1)
		{

			for (c = 0; c < X_W; c++)
			{
				for (b = 0; b < 4; b++)
				{
					for (a = 0; a < Y_H; a++)
					{
						temp = (fdata[X_W * (Y_H - 1 - a) + c] & cSetb_2bit[b]);

						if (b == 0)
							temp = temp >> 6;
						else if (b == 1)
							temp = temp >> 4;
						else if (b == 2)
							temp = temp >> 2;
						if (temp > 0)
						{
							switch (temp)
							{
							case 1:
								color_temp = F_Color | (0x04 << 12);
								break;
							case 2:
								color_temp = F_Color | (0x08 << 12);
								break;
							case 3:
								color_temp = F_Color | (0x0F << 12);
								break;
							default:
								break;
							}
							
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}

		}
		else if (pixel_format == 2)
		{

			for (c = 0; c < X_W; c++)
			{
				for (b = 0; b < 2; b++)
				{
					for (a = 0; a < Y_H ; a++)
					{
						temp = (fdata[X_W * (Y_H - 1 - a) + c] & cSetb_4bit[b]);

						if (b == 0)
							temp = temp >> 4;
						if (temp > 0)
						{

							color_temp = F_Color | (temp << 12);
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}

		}
	}
	else if (Screen_DIR == 2) // 180 degrees
	{
		if (pixel_format == 0)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 0; b < 8; b++)
					{
						if (fdata[X_W * (Y_H - 1 - a) + (X_W - 1 - c)] & cSetb[b])
						{
							
							LCD_DataWrite(F_Color);
							LCD_DataWrite(F_Color>>8);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(F_Color>>16);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 1)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 4; b > 0; b--)
					{
						temp = fdata[X_W * (Y_H - 1 - a) + (X_W - 1 - c)] & cSetb_2bit[b - 1];
						if (b == 1)
							temp = temp >> 6;
						else if (b == 2)
							temp = temp >> 4;
						else if (b == 3)
							temp = temp >> 2;
						if (temp > 0)
						{
							switch (temp)
							{
							case 1:
								color_temp = F_Color | (0x04 << 12);
								break;
							case 2:
								color_temp = F_Color | (0x08 << 12);
								break;
							case 3:
								color_temp = F_Color | (0x0F << 12);
								break;
							default:
								break;
							}
							
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 2)
		{
			for (a = 0; a < Y_H; a++)
			{
				for (c = 0; c < X_W; c++)
				{
					for (b = 2; b > 0; b--)
					{
						temp = fdata[X_W * (Y_H - 1 - a) + (X_W - 1 - c)] & cSetb_4bit[b - 1];
						if (b == 1)
							temp = temp >> 4;
						if (temp > 0)
						{
							color_temp = F_Color | (temp << 12);
							
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
	}
	else if (Screen_DIR == 3) // 270 degrees
	{
		if (pixel_format == 0)
		{
			for (c = 0; c < X_W; c++)
			{
				for (b = 0; b < 8; b++)
				{
					for (a = 0; a < Y_H; a++)
					{
						if (fdata[X_W * a + (X_W - 1 - c)] & cSetb[b])
						{
							
							LCD_DataWrite(F_Color);
							LCD_DataWrite(F_Color>>8);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(F_Color>>16);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
							if (TFT_bitcolor == 1)
								LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 1)
		{
			for (c = 0; c < X_W; c++)
			{
				for (b = 4; b > 0; b--)
				{
					for (a = 0; a < Y_H; a++)
					{
						temp = fdata[X_W * a + (X_W - 1 - c)] & cSetb_2bit[b - 1];
						if (b == 1)
							temp = temp >> 6;
						else if (b == 2)
							temp = temp >> 4;
						else if (b == 3)
							temp = temp >> 2;
						if (temp > 0)
						{
							switch (temp)
							{
							case 1:
								color_temp = F_Color | (0x04 << 12);
								break;
							case 2:
								color_temp = F_Color | (0x08 << 12);
								break;
							case 3:
								color_temp = F_Color | (0x0F << 12);
								break;
							default:
								break;
							}
							
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
		else if (pixel_format == 2)
		{
			for (c = 0; c < X_W; c++)
			{
				for (b = 2; b > 0; b--)
				{
					for (a = 0; a < Y_H; a++)
					{
						temp = fdata[X_W * a + (X_W - 1 - c)] & cSetb_4bit[b - 1];
						if (b == 1)
							temp = temp >> 4;
						if (temp > 0)
						{
							color_temp = F_Color | (temp << 12);
							
							LCD_DataWrite(color_temp);
							LCD_DataWrite(color_temp>>8);
						}
						else
						{
							
							LCD_DataWrite(0);
							LCD_DataWrite(0);
						}
					}
				}
			}
		}
	}
}

/*******************************************************************************
 * Function Name	: LT758_Draw_Font
 * Description		: Display Font Library.
 * Input			:
 * Output       	: None
 * Return       	: None
 ******************************************************************************/
void LT758_Draw_Font
(
	uint16_t encode		// Encoding type, 0:GB2312  1:GBK  2:BIG5  3:UNICODE  4:ASCII 6:UNICODE
	,
	uint32_t FlashAddr	// Font source address(exSpiFlash)
	,
	uint32_t ShowAddr	// Displays the address of the layer
	,
	uint32_t BuffAddr	// Buff address of the layer
	,
	uint16_t width		// Display the width of the layer
	,
	uint8_t W_Size		// Font width
	,
	uint8_t H_Size		// Font height
	,
	uint8_t Alignment	// Alignment 0:Left	1:Mid	2:Right
	,
	uint32_t FontColor	// The foreground color of the font
	,
	uint8_t star		// Text display mode: 0:txt  1:encryption use '*' (0x2A)
	,
	uint16_t Xs			// Text box start x position
	,
	uint16_t Ys			// Text box start Y position
	,
	uint16_t Xe			// Text box end x position
	,
	uint16_t Ye			// Text box end y position
	,
	uint8_t dis_x		// The interval between two adjacent fonts on the x-axis
	,
	uint8_t dis_y		// The interval between two adjacent fonts on the y-axis
	,
	uint8_t *c			// Address of text
	,
	uint16_t len_max	// max lenght of text
)
{
	uint16_t temp_H = 0, temp_L = 0, temp = 0;
	uint32_t i = 0;
	uint16_t width_Hor = 0;
	uint8_t pixel_format = 0, temp_pixel = 0, row = 0, Font_W;
	uint16_t row_w[20] = {0};
	uint16_t x_src = 0, y_src = 0, x_des = 0, y_des = 0, w_real = 0, h_real = 0;
	uint16_t len_disp = 0, offset_disp = 0, W_Len = 0, H_Len = 0;

	uint16_t unicode = 0;
	uint16_t start_code = 0, end_code = 0;
	uint8_t buff_code[4];
	uint8_t star_flag = 0;
	uint16_t str_num = 0, k;
#if STM32_FMC_16
	Host_Bus_8bit();
#endif
	Flash_Read_UI(Ascii_w, FlashAddr + 9, 95);
	Flash_Read_UI(&pixel_format, FlashAddr + 8, 1);
	if (pixel_format == 0)
		temp_pixel = 8;
	else if (pixel_format == 1)
		temp_pixel = 4;
	else if (pixel_format == 2)
		temp_pixel = 2;
	
	if (W_Size % temp_pixel != 0)
		width_Hor = W_Size / temp_pixel + 1;
	else
		width_Hor = W_Size / temp_pixel;

	if (TFT_bitcolor == 1 && pixel_format != 0)
		Memory_16bpp_Mode();

	if (Screen_DIR == 0 || Screen_DIR == 2)
	{
		W_Len = Xe - Xs + 1;
		H_Len = Ye - Ys + 1;
	}
	else if (Screen_DIR == 1 || Screen_DIR == 3)
	{
		W_Len = Ye - Ys + 1;
		H_Len = Xe - Xs + 1;
	}
	if (Screen_DIR == 0)
	{
		Canvas_Image_Start_address(BuffAddr);
		Canvas_image_width(LCD_XSIZE_TFT);
		LT758_DrawSquare_Fill(0, 0, Xe - Xs + 1, Ye - Ys + 1, Black);
	}
	else if (Screen_DIR == 1)
	{
		Canvas_Image_Start_address(BuffAddr);
		Canvas_image_width(LCD_XSIZE_TFT);
		LT758_DrawSquare_Fill(LCD_XSIZE_TFT - (Xe - Xs + 1), 0, LCD_XSIZE_TFT - 1, Ye - Ys + 1, Black);
	}
	else if (Screen_DIR == 2)
	{
		Canvas_Image_Start_address(BuffAddr);
		Canvas_image_width(LCD_XSIZE_TFT);
		LT758_DrawSquare_Fill(LCD_XSIZE_TFT - (Xe - Xs + 1), LCD_YSIZE_TFT - (Ye - Ys + 1), LCD_XSIZE_TFT - 1, LCD_YSIZE_TFT - 1, Black);
	}
	else if (Screen_DIR == 3)
	{
		Canvas_Image_Start_address(BuffAddr);
		Canvas_image_width(LCD_XSIZE_TFT);
		LT758_DrawSquare_Fill(0, LCD_YSIZE_TFT - (Ye - Ys + 1), Xe - Xs + 1, LCD_YSIZE_TFT - 1, Black);
	}
	
	if (encode == 4) // ASCII
	{
		while ((c[i] != '\0')) // Automatic exit when encountering non-Chinese character code
		{
			//WDT_FeedDog(); //Clear watchdog
			if (c[i] < 0x80 && c[i] > 0x00)
			{
				if (star == 1)
					temp = '*';
				else
					temp = c[i];
				if (((len_disp + Ascii_w[temp - 0x20]) > W_Len) || c[i] == 0x0A) // Line feed
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				if (c[i] != 0x0A)
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (temp - 0x20) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + Ascii_w[temp - 0x20] + 2 + dis_x;
				}

				i++;
				if (i >= len_max)
					break;
			}
			else
			{
				break;
			}
		}
	}
	else if (encode == 0) // GB2312
	{
		while ((c[i] != '\0')) // Automatic exit when encountering non-Chinese character code
		{
			//WDT_FeedDog(); //Clear watchdog
			
			if (star == 1 && c[i] >= 0xa1 && c[i] != 0xFF)
			{
				//c[i + 1] = '*';
				star_flag = 1;
				i++;
			}
			
			if ((c[i] < 128 && c[i] > 0x00) || star_flag == 1)
			{
				if (star == 1)
				{
					temp = '*';
					star_flag = 0;
				}
				else
					temp = c[i];
				if (((len_disp + Ascii_w[temp - 0x20]) > W_Len) || c[i] == 0x0A)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				if (c[i] != 0x0A)
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (temp - 0x20) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + Ascii_w[temp - 0x20] + 2 + dis_x;
				}

				i++;
				if (i >= len_max)
					break;
			}
			else if (c[i] >= 0xa1 && c[i] != 0xFF)
			{
				temp_H = ((c[i] - 0xa1) & 0x00ff) * 94;
				temp_L = c[i + 1] - 0xa1;
				temp = temp_H + temp_L;

				if ((len_disp + W_Size) > W_Len)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				if (c[i] != 0x0A)
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (95 + temp) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + W_Size + dis_x;
				}

				i += 2;
				if (i >= len_max)
					break;
			}
			else
			{
				break;
			}
		}
	}
	else if (encode == 1) // GBK
	{
		while ((c[i] != '\0'))
		{
			//WDT_FeedDog(); //Clear watchdog

			if (star == 1 && c[i] >= 0x81 && c[i + 1] >= 0x40 && c[i] != 0xFF)
			{
				star_flag = 1;
				i++;
			}

			if ((c[i] < 128 && c[i] > 0x00) || star_flag == 1)
			{

				if (star == 1)
				{
					star_flag = 0;
					temp = '*';
				}
				else
					temp = c[i];
				if (((len_disp + Ascii_w[temp - 0x20]) > W_Len) || c[i] == 0x0A)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;

					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				if (c[i] != 0x0A) // New line
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (temp - 0x20) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + Ascii_w[temp - 0x20] + 2 + dis_x;
				}

				i++;
				if (i >= len_max)
					break;
			}
			else if (c[i] >= 0x81 && c[i + 1] >= 0x40 && c[i] != 0xFF)
			{

				temp_H = (c[i] - 0x81) * 190;
				if (c[i + 1] <= 0x7F)
				{
					temp_L = c[i + 1] - 0x40;
				}
				else if (c[i + 1] > 0x7F)
				{
					temp_L = c[i + 1] - 0x40 - 1;
				}
				temp = temp_H + temp_L;

				if ((len_disp + W_Size + dis_x) > W_Len)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				if (Screen_DIR == 0)
				{
					x_src = len_disp;
					y_src = offset_disp;
				}
				else if (Screen_DIR == 1)
				{
					x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
					y_src = len_disp;
				}
				else if (Screen_DIR == 2)
				{
					x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
					y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
				}
				else if (Screen_DIR == 3)
				{
					x_src = offset_disp;
					y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
				}
				LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (95 + temp) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
				len_disp = len_disp + W_Size + dis_x;
				i += 2;
				if (i >= len_max)
					break;
			}
			else
			{
				break;
			}
		}
	}
	else if (encode == 2) // BIG5
	{
		while ((c[i] != '\0'))
		{
			//WDT_FeedDog(); //Clear watchdog
			if (star == 1 && c[i] >= 0xa1)
			{
				star_flag  = 1;
				i++;
			}
			if ((c[i] < 128 && c[i] > 0x00) || star_flag == 1)
			{
				if (star == 1)
				{
					star_flag = 0;
					temp = '*';
				}
				else
					temp = c[i];
				if (((len_disp + Ascii_w[temp - 0x20]) > W_Len) || c[i] == 0x0A)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}

				if (c[i] != 0x0A)
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (temp - 0x20) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + Ascii_w[temp - 0x20] + 2 + dis_x;
				}

				i++;
				if (i >= len_max)
					break;
			}
			else if (c[i] >= 0xa1)
			{
				temp_H = ((c[i] - 0xa1) & 0x00ff) * 160;
				if (c[i + 1] >= 0x40 && c[i + 1] <= 0x7F)
					temp_L = c[i + 1] - 0x40;
				else if (c[i + 1] >= 0xA0 && c[i + 1] <= 0xFF)
					temp_L = c[i + 1] - 0xA0 + 64;

				temp = temp_H + temp_L;

				if ((len_disp + W_Size) > W_Len)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}

				if (Screen_DIR == 0)
				{
					x_src = len_disp;
					y_src = offset_disp;
				}
				else if (Screen_DIR == 1)
				{
					x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
					y_src = len_disp;
				}
				else if (Screen_DIR == 2)
				{
					x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
					y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
				}
				else if (Screen_DIR == 3)
				{
					x_src = offset_disp;
					y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
				}
				LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (95 + temp) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
				len_disp += W_Size + dis_x;

				i += 2;
				if (i >= len_max)
					break;
			}
			else
				break;
		}
	}
	else if (encode == 6) // Unicode
	{
		Flash_Read_UI(buff_code, FlashAddr + 4, 4);
		start_code = buff_code[0] + (buff_code[1] << 8);
		end_code = buff_code[2] + (buff_code[3] << 8);

	
		while (((c[i] << 8) + c[i + 1]) > 0)
		{
			//WDT_FeedDog(); //Clear watchdog
			unicode = (c[i] << 8) + c[i + 1];

			if (star == 1 && unicode >= start_code && unicode <= end_code)
			{
				unicode = '*';
			}
			if (unicode < 128 && unicode > 0x00)
			{
				if (star == 1)
					unicode = '*';
				if (((len_disp + Ascii_w[unicode - 0x20]) > W_Len) || unicode == 0x0A)
				{

					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}

				if (unicode != 0x0A)
				{
					if (Screen_DIR == 0)
					{
						x_src = len_disp;
						y_src = offset_disp;
					}
					else if (Screen_DIR == 1)
					{
						x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
						y_src = len_disp;
					}
					else if (Screen_DIR == 2)
					{
						x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
						y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
					}
					else if (Screen_DIR == 3)
					{
						x_src = offset_disp;
						y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
					}
					
					LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + (unicode - 0x20) * width_Hor * H_Size, BuffAddr, LCD_XSIZE_TFT);
					len_disp = len_disp + Ascii_w[unicode - 0x20] + 2 + dis_x;

				}

				i += 2;
				if (i >= len_max)
					break;
			}
			else if (unicode >= start_code && unicode <= end_code)
			{
				Flash_Read_UI(&Font_W, FlashAddr + 104 + 95 * width_Hor * H_Size + (unicode - start_code) * (width_Hor * H_Size + 1), 1);
				
				if ((len_disp + Font_W) > W_Len)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}

				if (Screen_DIR == 0)
				{
					x_src = len_disp;
					y_src = offset_disp;
				}
				else if (Screen_DIR == 1)
				{
					x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
					y_src = len_disp;
				}
				else if (Screen_DIR == 2)
				{
					x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
					y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
				}
				else if (Screen_DIR == 3)
				{
					x_src = offset_disp;
					y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
				}
				LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 104 + 95 * width_Hor * H_Size + (unicode - start_code) * (width_Hor * H_Size + 1) + 1, BuffAddr, LCD_XSIZE_TFT);
				len_disp = len_disp + Font_W + 1 + dis_x;
				
	
				i += 2;
				if (i >= len_max)
					break;
			}
			else
				break;
		}
	}
	else if (encode == 9) // custom
	{
		uint8_t strbuf[2048];
		Flash_Read_UI(buff_code, FlashAddr + 4, 2);
		str_num = buff_code[0] + (buff_code[1] << 8);
	
		Flash_Read_UI(strbuf, FlashAddr + 9, str_num * 2);
		
		while (((c[i] << 8) + c[i + 1]) > 0)
		{
			//WDT_FeedDog(); //Clear watchdog

			unicode = (c[i] << 8) + c[i + 1];

			
			for(k = 0; k < str_num; k++)
			{
				if (unicode == (strbuf[k*2] + (strbuf[k*2 + 1] <<  8)))
				{
					break;
				}
			}
			
			if (k == str_num && unicode != 0x0A)
			{
				len_max = i; // coding error or max len (Exit after entering the display)
				break;
			}
			else if (k != str_num)
			{
				Flash_Read_UI(&Font_W, FlashAddr + 9 + str_num * 2 + k * (width_Hor * H_Size + 1) , 1);

	
				if ((len_disp + Font_W) > W_Len)
				{
					row_w[row] = len_disp - dis_x;
					row++;
					offset_disp += (H_Size + dis_y);
					len_disp = 0;
					if ((offset_disp + H_Size) > H_Len)
						break;
				}
				temp = unicode - start_code;

				if (Screen_DIR == 0)
				{
					x_src = len_disp;
					y_src = offset_disp;
				}
				else if (Screen_DIR == 1)
				{
					x_src = LCD_XSIZE_TFT - H_Size - offset_disp;
					y_src = len_disp;
				}
				else if (Screen_DIR == 2)
				{
					x_src = LCD_XSIZE_TFT - width_Hor * temp_pixel - len_disp;
					y_src = LCD_YSIZE_TFT - H_Size - offset_disp;
				}
				else if (Screen_DIR == 3)
				{
					x_src = offset_disp;
					y_src = LCD_YSIZE_TFT - width_Hor * temp_pixel - len_disp;
				}
				LT758_Draw_16bit(x_src, y_src, width_Hor, H_Size, width_Hor * temp_pixel, pixel_format, FontColor, FlashAddr + 9 + str_num * 2 + k * (width_Hor * H_Size + 1) + 1, BuffAddr, LCD_XSIZE_TFT);
				len_disp = len_disp + Font_W + 2 + dis_x;
			
				i += 2;
				if (i >= len_max)
					break;
			}
			else
				break;
		}
	}

	if (TFT_bitcolor == 1 && pixel_format != 0)
		Memory_24bpp_Mode();

	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
	Goto_Pixel_XY(0, 0);

	//data_w = len_disp; // For input method

	if (len_disp != 0)
	{
		row_w[row] = len_disp - 2 - dis_x;

		row++;
	}

	// Alignment
	if (Alignment == 0 || Alignment == 1 || Alignment == 2) // Mid
		offset_disp = (H_Len - row * (H_Size + dis_y) + dis_y) / 2;
	else if (Alignment == 3 || Alignment == 4 || Alignment == 5) // Upper
		offset_disp = 0;
	else if (Alignment == 6 || Alignment == 7 || Alignment == 8) // Below
		offset_disp = H_Len - row * (H_Size + dis_y) + dis_y;

	for (i = 0; i < row; i++)
	{
		if (Alignment == 0 || Alignment == 3 || Alignment == 6) // Left alignment 
		{
			if (Screen_DIR == 0)
			{
				x_src = 0;
				y_src = i * (H_Size + dis_y);
				x_des = Xs;
				y_des = Ys + offset_disp + i * (H_Size + dis_y);
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 1)
			{
				x_src = LCD_XSIZE_TFT - (1 + i) * (H_Size + dis_y);
				y_src = 0;
				x_des = Xe - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				y_des = Ys;
				w_real = H_Size;
				h_real = row_w[i];
			}
			else if (Screen_DIR == 2)
			{
				x_src = LCD_XSIZE_TFT - row_w[i];
				y_src = LCD_YSIZE_TFT - (1 + i) * (H_Size + dis_y);
				x_des = Xe - row_w[i] + 1;
				y_des = Ye - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 3)
			{
				x_src = i * (H_Size + dis_y);
				y_src = LCD_YSIZE_TFT - row_w[i];
				x_des = Xs + offset_disp + i * (H_Size + dis_y);
				y_des = Ye - row_w[i] + 1;
				w_real = H_Size ;
				h_real = row_w[i];
			}
		}
		else if (Alignment == 1 || Alignment == 4 || Alignment == 7) // Mid alignment 
		{
			if (Screen_DIR == 0)
			{
				x_src = 0;
				y_src = i * (H_Size + dis_y);
				x_des = Xs + (W_Len - row_w[i]) / 2;
				y_des = Ys + offset_disp + i * (H_Size + dis_y);
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 1)
			{
				x_src = LCD_XSIZE_TFT - (1 + i) * (H_Size + dis_y);
				y_src = 0;
				x_des = Xe - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				y_des = Ys + (W_Len - row_w[i]) / 2;
				w_real = H_Size;
				h_real = row_w[i];
			}
			else if (Screen_DIR == 2)
			{
				x_src = LCD_XSIZE_TFT - row_w[i];
				y_src = LCD_YSIZE_TFT - (1 + i) * (H_Size + dis_y);
				x_des = Xs + (W_Len - row_w[i]) / 2;
				y_des = Ye - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 3)
			{
				x_src = i * (H_Size + dis_y);
				y_src = LCD_YSIZE_TFT - row_w[i];
				x_des = Xs + offset_disp + i * (H_Size + dis_y);
				y_des = Ys + (W_Len - row_w[i]) / 2;
				w_real = H_Size;
				h_real = row_w[i];
			}
		}
		else if (Alignment == 2 || Alignment == 5 || Alignment == 8) // Right alignment 
		{
			if (Screen_DIR == 0)
			{
				x_src = 0;
				y_src = i * (H_Size + dis_y);
				x_des = Xe - row_w[i] + 1;
				y_des = Ys + offset_disp + i * (H_Size + dis_y);
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 1)
			{
				x_src = LCD_XSIZE_TFT - (1 + i) * (H_Size + dis_y);
				y_src = 0;
				x_des = Xe - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				y_des = Ye - row_w[i] + 1;
				w_real = H_Size;
				h_real = row_w[i];
			}
			else if (Screen_DIR == 2)
			{
				x_src = LCD_XSIZE_TFT - row_w[i];
				y_src = LCD_YSIZE_TFT - (1 + i) * (H_Size + dis_y);
				x_des = Xs;
				y_des = Ye - offset_disp - (1 + i) * (H_Size + dis_y) + 1;
				w_real = row_w[i];
				h_real = H_Size;
			}
			else if (Screen_DIR == 3)
			{
				x_src = i * (H_Size + dis_y);
				y_src = LCD_YSIZE_TFT - row_w[i];
				x_des = Xs + offset_disp + i * (H_Size + dis_y);
				y_des = Ys;
				w_real = H_Size;
				h_real = row_w[i];
			}
		}

		if (pixel_format == 0)
		{
			LT758_BTE_Memory_Copy_Chroma_key(BuffAddr, LCD_XSIZE_TFT, x_src, y_src,
											 ShowAddr, width, x_des, y_des,
											 Black, w_real, h_real,24);
		}
		else if (pixel_format == 1 || pixel_format == 2)
		{
			
	
			BTE_Pixel_16bbp_or_24bpp_Alpha_Blending_ZK(ShowAddr, LCD_XSIZE_TFT, x_des, y_des,
													   BuffAddr, LCD_XSIZE_TFT, x_src, y_src,
													   ShowAddr, LCD_XSIZE_TFT, x_des, y_des,
													   w_real, h_real);

		}
		Canvas_Image_Start_address(ShowAddr);
		Canvas_image_width(LCD_XSIZE_TFT);
		Active_Window_XY(0,0);
		Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);
#if STM32_FMC_16
		Host_Bus_16bit();
#endif
	}
}
