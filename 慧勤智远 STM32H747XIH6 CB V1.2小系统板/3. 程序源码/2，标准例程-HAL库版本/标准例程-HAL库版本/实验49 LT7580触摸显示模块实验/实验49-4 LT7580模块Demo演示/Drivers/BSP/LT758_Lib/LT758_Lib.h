/******************************************************************************
  * Copyright(c) 2024 Levetop Semiconductor Co., Ltd. All rights reserved
  * @file      LT758_Lib.h
  * @author    Levetop TFT Controller Application Team
  * @version   V1.0.0
  * @date      2024-12-30
  * @brief     LT758x Function Libraries 
*******************************************************************************/

#ifndef _LT758_Lib_H
#define _LT758_Lib_H

#include "./BSP/IF_PORT/if_port.h"


extern unsigned char csc;
extern unsigned char clk;
extern unsigned char Bypass_Mode;
extern unsigned int Flash_ID;
extern unsigned char w25q256;
extern unsigned char Flash_Type;

#define Display_16bpp    0	  /* Color depth  1:16bpp  0:24bpp */

/* 选择显示分辨率 */
#define TFT800480 	     0		/* 选择分辨率800×480 */
#define TFT1024600       1		/* 选择分辨率1024×600 */

//External Oscillator
#define XI_4M            0
#define XI_8M            0
#define XI_10M    	     0
#define XI_12M           1

#if TFT800480    
//LCD Resolution & Parameters
#define LCD_XSIZE_TFT    800	
#define LCD_YSIZE_TFT    480
#define LCD_VBPD		     23
#define LCD_VFPD	 	     22	
#define LCD_VSPW		     3
#define LCD_HBPD		     46
#define LCD_HFPD		     210
#define LCD_HSPW	     	 20
#endif

#if TFT1024600      
//LCD Resolution & Parameters
#define LCD_XSIZE_TFT    1024	
#define LCD_YSIZE_TFT    600
#define LCD_VBPD		     20
#define LCD_VFPD	    	 12	
#define LCD_VSPW		     3
#define LCD_HBPD		     140
#define LCD_HFPD		     160
#define LCD_HSPW	     	 20
#endif

#if Display_16bpp
/* 图层在显存的位置 */ 
#define layer1_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 0	     
#define layer2_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2	   
#define layer3_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2 * 2 
#define layer4_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2 * 3 
#define layer5_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2 * 4 
#define layer6_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2 * 5 
#define layer7_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2 * 6    

#else
/* 图层在显存的位置 */ 
#define layer1_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 0	    
#define layer2_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3	   
#define layer3_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3 * 2 
#define layer4_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3 * 3 
#define layer5_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3 * 4 
#define layer6_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3 * 5 
#define layer7_start_addr LCD_XSIZE_TFT * LCD_YSIZE_TFT * 3 * 6     
#endif

#define color256_black   0x00
#define color256_white   0xff
#define color256_red     0xe0
#define color256_green   0x1c
#define color256_blue    0x03
#define color256_yellow  color256_red|color256_green
#define color256_cyan    color256_green|color256_blue
#define color256_purple  color256_red|color256_blue
 
#define color65k_black   0x0000
#define color65k_white   0xffff
#define color65k_red     0xf800
#define color65k_green   0x07e0
#define color65k_blue    0x001f
#define color65k_yellow  color65k_red|color65k_green
#define color65k_cyan    color65k_green|color65k_blue
#define color65k_purple  color65k_red|color65k_blue

#define color65k_grayscale1    2113
#define color65k_grayscale2    2113*2
#define color65k_grayscale3    2113*3
#define color65k_grayscale4    2113*4
#define color65k_grayscale5    2113*5
#define color65k_grayscale6    2113*6
#define color65k_grayscale7    2113*7
#define color65k_grayscale8    2113*8
#define color65k_grayscale9    2113*9
#define color65k_grayscale10   2113*10
#define color65k_grayscale11   2113*11
#define color65k_grayscale12   2113*12
#define color65k_grayscale13   2113*13
#define color65k_grayscale14   2113*14
#define color65k_grayscale15   2113*15
#define color65k_grayscale16   2113*16
#define color65k_grayscale17   2113*17
#define color65k_grayscale18   2113*18
#define color65k_grayscale19   2113*19
#define color65k_grayscale20   2113*20
#define color65k_grayscale21   2113*21
#define color65k_grayscale22   2113*22
#define color65k_grayscale23   2113*23
#define color65k_grayscale24   2113*24
#define color65k_grayscale25   2113*25
#define color65k_grayscale26   2113*26
#define color65k_grayscale27   2113*27
#define color65k_grayscale28   2113*28
#define color65k_grayscale29   2113*29
#define color65k_grayscale30   2113*30
 
#define color16M_black   0x00000000
#define color16M_white   0x00ffffff
#define color16M_red     0x00ff0000
#define color16M_green   0x0000ff00
#define color16M_blue    0x000000ff
#define color16M_yellow  color16M_red|color16M_green
#define color16M_cyan    color16M_green|color16M_blue
#define color16M_purple  color16M_red|color16M_blue

#if Display_16bpp
/* LCD color */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

#else
/* LCD color */
#define White          0x00ffffff
#define Black          0x00000000
#define Blue           0x000000FF
#define Blue2          0x0000BFFF
#define Red            0x00ff0000
#define Magenta        0x00FF00FF
#define Green          0x0000ff00
#define Cyan           Green|Blue
#define Yellow         Red|Green
#define Grey         	 Red|Blue
#define Purple         Red|Blue
#endif

#define Line0          0
#define Line1          24
#define Line2          48
#define Line3          72
#define Line4          96
#define Line5          120
#define Line6          144
#define Line7          168
#define Line8          192
#define Line9          216
#define Line10         240
#define Line11         264
#define Line12         288
#define Line13         312
#define Line14         336
#define Line15         360
#define Line16         384
#define Line17         408
#define Line18         432
#define Line19         456
#define Line20         480
#define Line21         504
#define Line22         528
#define Line23         552
#define Line24         576
#define Line25         600


#if !defined(UNUSED)
#define UNUSED(x) ((void)(x))  
#endif 

#define JPG_Key_Addr   0

unsigned char Check_IC_ready(void);
void LT758_HW_Reset(void);
void LT758_Init(void);

/* MCU writes data to SDRAM */
void MPU8_8bpp_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned char *data);
void MPU8_16bpp_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned char *data);
void MPU8_24bpp_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned char *data); 
void MPU16_16bpp_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned short *data);
void MPU16_24bpp_Mode1_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned short *data); 
void MPU16_24bpp_Mode2_Memory_Write(unsigned short x,unsigned short y,unsigned short w,unsigned short h,const unsigned short *data);

/* Draw Line */
void LT758_DrawLine(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned long LineColor);
void LT758_DrawLine_Width(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned long LineColor,unsigned short Width);

/* Draw Circle */
void LT758_DrawCircle(unsigned short XCenter,unsigned short YCenter,unsigned short R,unsigned long CircleColor);
void LT758_DrawCircle_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short R,unsigned long ForegroundColor);
void LT758_DrawCircle_Width(unsigned short XCenter,unsigned short YCenter,unsigned short R,unsigned long CircleColor,unsigned long ForegroundColor,unsigned short Width);

/* Draw Ellipse */
void LT758_DrawEllipse(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long EllipseColor);
void LT758_DrawEllipse_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_DrawEllipse_Width(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long EllipseColor,unsigned long ForegroundColor,unsigned short Width);   

/* Draw Square */
void LT758_DrawSquare(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned long SquareColor);
void LT758_DrawSquare_Fill(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned long ForegroundColor);
void LT758_DrawSquare_Width(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned long SquareColor,unsigned long ForegroundColor,unsigned short Width);

/* Draw Rounded Square */
void LT758_DrawCircleSquare(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X_R,unsigned short Y_R,unsigned long CircleSquareColor);
void LT758_DrawCircleSquare_Fill(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_DrawCircleSquare_Width(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X_R,unsigned short Y_R,unsigned long CircleSquareColor,unsigned long ForegroundColor,unsigned short Width);

/* Draw Triangle */
void LT758_DrawTriangle(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned long TriangleColor);
void LT758_DrawTriangle_Fill(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned long ForegroundColor);
void LT758_DrawTriangle_Frame(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned long TriangleColor,unsigned long ForegroundColor);

/* Draw Quadrilateral */
void LT758_DrawQuadrilateral(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned short X4,unsigned short Y4,unsigned long ForegroundColor);
void LT758_DrawQuadrilateral_Fill(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned short X4,unsigned short Y4,unsigned long ForegroundColor);
void LT758_DrawTriangle_Frame(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned long TriangleColor ,unsigned long ForegroundColor);

/* Draw Pentagon */
void LT758_DrawPentagon(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned short X4,unsigned short Y4,unsigned short X5,unsigned short Y5,unsigned long ForegroundColor);
void LT758_DrawPentagon_Fill(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned short X4,unsigned short Y4,unsigned short X5,unsigned short Y5,unsigned long ForegroundColor);

/* Draw Curve */
void LT758_DrawLeftUpCurve(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor);
void LT758_DrawLeftDownCurve(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor);
void LT758_DrawRightUpCurve(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor);
void LT758_DrawRightDownCurve(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor);
void LT758_SelectDrawCurve(unsigned short XCenter ,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor,unsigned short  Dir);

/* Draw Solid Curve */
void LT758_DrawLeftUpCurve_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_DrawLeftDownCurve_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_DrawRightUpCurve_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_DrawRightDownCurve_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long ForegroundColor);
void LT758_SelectDrawCurve_Fill(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned long CurveColor,unsigned short  Dir);

/* Draw Cylinder */
unsigned char LT758_DrawCylinder(unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R,unsigned short H,unsigned long CylinderColor,unsigned long ForegroundColor);

/* Draw Quadrangular */
void LT758_DrawQuadrangular(unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3,unsigned short X4,unsigned short Y4,unsigned short X5,unsigned short Y5,unsigned short X6,unsigned short Y6,unsigned long QuadrangularColor,unsigned long ForegroundColor);

/* Make a Table */
void LT758_MakeTable(unsigned short X1,unsigned short Y1,unsigned short W,unsigned short H,unsigned short Line,unsigned short Row,unsigned long  TableColor,unsigned long  ItemColor,unsigned long  ForegroundColor,unsigned short width1,unsigned short width2,unsigned char  mode);

/* DMA - Linear mode, to retrieve data from SPI Flash and store the data to SDRAM  */
void LT758_DMA_24bit_Linear(unsigned char SCS,unsigned char Clk,unsigned long flash_addr,unsigned long memory_ad,unsigned long data_num);
void LT758_DMA_32bit_Linear(unsigned char SCS,unsigned char Clk,unsigned long flash_addr,unsigned long memory_ad,unsigned long data_num);

/* DMA - Block mode, to retrieve data from SPI Flash and store the data to SDRAM */
void LT758_DMA_24bit_Block(unsigned char SCS,unsigned char Clk,unsigned short X1,unsigned short Y1 ,unsigned short X_W,unsigned short Y_H,unsigned short P_W,unsigned long Addr);
void LT758_DMA_32bit_Block(unsigned char SCS,unsigned char Clk,unsigned short X1,unsigned short Y1 ,unsigned short X_W,unsigned short Y_H,unsigned short P_W,unsigned long Addr);

/* Using internal Font */
void LT758_Select_Internal_Font_Init(unsigned char Size,unsigned char XxN,unsigned char YxN,unsigned char ChromaKey,unsigned char Alignment);
void LT758_Print_Internal_Font_String(unsigned short x,unsigned short y,unsigned long FontColor,unsigned long BackGroundColor ,char *c);

/* Using external Font */
void LT758_Select_Outside_Font_Init(unsigned char SCS,unsigned char Clk,unsigned long FlashAddr,unsigned long MemoryAddr,unsigned long Num,unsigned char Size,unsigned char XxN,unsigned char YxN,unsigned char ChromaKey,unsigned char Alignment);
void LT758_Print_Outside_Font_String(unsigned short x,unsigned short y,unsigned long FontColor,unsigned long BackGroundColor,unsigned char *c);
void LT758_Print_Outside_Ascii_String(unsigned short x,unsigned short y,unsigned long FontColor,unsigned long BackGroundColor,unsigned char *c);

/* Text cursor */
void LT758_Text_cursor_Init(unsigned char On_Off_Blinking,unsigned short Blinking_Time,unsigned short X_W,unsigned short Y_W);
void LT758_Enable_Text_Cursor(void);
void LT758_Disable_Text_Cursor(void);

/* Graphic cursor */
void LT758_Graphic_cursor_Init(unsigned char Cursor_N,unsigned char Color1,unsigned char Color2,unsigned short X_Pos,unsigned short Y_Pos,unsigned char *Cursor_Buf);
void LT758_Set_Graphic_cursor_Pos(unsigned char Cursor_N,unsigned short X_Pos,unsigned short Y_Pos);
void LT758_Enable_Graphic_Cursor(void);
void LT758_Disable_Graphic_Cursor(void);

/* BitBLT - Graphics Process Functions */
void LT758_BTE_Solid_Fill(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned long color,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_Memory_Copy_JPG(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_Memory_Copy(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H,unsigned char  Color_depth);
void LT758_BTE_Memory_Copy_Chroma_key(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_Pattern_Fill(unsigned char P_8x8_or_16x16,unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code ,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_Pattern_Fill_With_Chroma_key(unsigned char P_8x8_or_16x16,unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_MCU_Write_MCU8(unsigned long S1_Addr,unsigned short S1_W,unsigned short XS,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H ,unsigned char *data,unsigned char Color_depth);
void LT758_BTE_MCU_Write_Chroma_key_MCU8(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,unsigned char *data,unsigned char Color_depth);
void LT758_BTE_Pattern_Fill1(unsigned char P_8x8_or_16x16,unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H,unsigned char Color_depth);
void LT758_BTE_MCU_Write_ColorExpansion_MCU8(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color,unsigned char *data,unsigned char Color_depth);
void LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU8(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned char*data,unsigned char Color_depth);
void LT758_BTE_Memory_Copy_ColorExpansion_8_ColorDepth(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color,unsigned char Color_depth);
void LT758_BTE_Memory_Copy_ColorExpansion_Chroma_key_8_ColorDepth(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned char Color_depth);
void LT758_BTE_MCU_Write_Picture_Alpha_Blending(unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned char *data,unsigned char alpha,unsigned char Color_depth);
void LT758_BTE_Picture_Alpha_Blending(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned char alpha,unsigned char Color_depth);
void LT758_BTE_Pixel_Alpha_Blending(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes ,unsigned short X_W,unsigned short Y_H,unsigned char  Alpha_bit);
void LT758_BTE_MCU_Write_MCU_16bit(unsigned long S1_Addr,unsigned short S1_W,unsigned short XS,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H ,const unsigned short *data);
void LT758_BTE_MCU_Write_Chroma_key_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,const unsigned short *data);
void LT758_BTE_MCU_Write_ColorExpansion_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color ,unsigned long Background_color ,const unsigned short *data);
void LT758_BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W,unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,const unsigned short *data);

/*  PIP */
void LT758_PIP_Init(unsigned char On_Off,unsigned char Select_PIP,unsigned long PAddr,unsigned short XP,unsigned short YP,unsigned long ImageWidth,unsigned short X_Dis,unsigned short Y_Dis,unsigned short X_W,unsigned short Y_H);
void LT758_Set_DisWindowPos(unsigned char On_Off,unsigned char Select_PIP,unsigned short X_Dis,unsigned short Y_Dis);

/*  PWM */
void LT758_PWM0_Init(unsigned char on_off,unsigned char Clock_Divided,unsigned char Prescalar,unsigned short Count_Buffer,unsigned short Compare_Buffer);
void LT758_PWM1_Init(unsigned char on_off,unsigned char Clock_Divided,unsigned char Prescalar,unsigned short Count_Buffer,unsigned short Compare_Buffer);
void LT758_PWM0_Duty(unsigned short Compare_Buffer);
void LT758_PWM1_Duty(unsigned short Compare_Buffer);

/* Standby Mode */
void LT758_Standby(void);
void LT758_Wkup_Standby(void);

/* Suspend Mode */
void LT758_Suspend(void);
void LT758_Wkup_Suspend(void);

/* Sleep Mode */
void LT758_SleepMode(void);
void LT758_Wkup_Sleep(void);

/* SPI Master */
void LT758_SPI_Init(unsigned char cs,unsigned char div);

/* nor flash W25QXX */
unsigned char LT758_W25QXX_ReadSR(void);
void LT758_W25QXX_Write_Enable(void);
void LT758_W25QXX_Write_Disable(void);
void LT758_W25QXX_Wait_Busy(void);
void LT758_W25QXX_Quad_Enable(void);
void LT758_W25QXX_Enter_4Byte_AddressMode(void);
void LT758_W25QXX_Read(unsigned char* pBuffer,unsigned long ReadAddr,unsigned int NumByteToRead);

/* nand flash W25NXX */
unsigned char LT758_W25NXX_ReadSR(unsigned char reg);
void LT758_W25NXX_Write_SR(unsigned char reg,unsigned char val);
void LT758_W25NXX_Wait_Busy(void);
void LT758_W25NXX_ContinuousRead_Mode(void);
void LT758_W25NXX_Write_Page(unsigned long page);
void LT758_W25NXX_ReadPageAddr_Data(unsigned char *pBuffer, unsigned long PageNum, unsigned long PageAddr, unsigned int NumByteToRead);
void LT758_W25NXX_BadBlockRemap(unsigned char on_off,unsigned int logic_block,unsigned int physical_block,unsigned char link_num);
void LT758_W25NXX_Read(unsigned char* pBuffer,unsigned long ReadAddr,unsigned int NumByteToRead);

/* JPG */
unsigned char LT758_DMA_JPG(unsigned short X1,unsigned short Y1,unsigned long Addr);

/* PNG */
void LT758_DMA_PNG(unsigned char PNG_Color_depth, unsigned long PNG_Flash_Addr, unsigned long PNG_Display_Layer, unsigned short PNG_X, unsigned short PNG_Y, unsigned short PNG_W, unsigned short PNG_H, unsigned long PNG_Buff_Layer, unsigned long BG_Layer, unsigned short BG_X, unsigned short BG_Y);

/* I2CM */
void LT758_I2C_Init(unsigned char div);
void LT758_I2C_Write_NByte(unsigned char slave_addr,unsigned char reg,unsigned int num, unsigned char *p);
void LT758_I2C_Read_NByte(unsigned char slave_addr,unsigned char reg,unsigned int num, unsigned char *p);

/* V_Gray */
void LT758_disp_V_Gray_888(void);
void LT758_disp_V_Gray_565(void);

#endif













