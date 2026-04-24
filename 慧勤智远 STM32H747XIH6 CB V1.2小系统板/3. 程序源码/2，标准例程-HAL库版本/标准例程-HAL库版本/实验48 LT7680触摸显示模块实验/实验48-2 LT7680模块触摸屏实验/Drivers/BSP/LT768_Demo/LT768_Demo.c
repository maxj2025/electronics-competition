/**
 ****************************************************************************************************
 * @file        LT768_Demo.c
 * @version     V1.0
 * @brief       LT7680&LT7381模块 应用测试程序
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/LT768_Demo/LT768_Demo.h"
#include "./BSP/LT768_Lib/LT768_Lib.h"
#include "./BSP/LT768/LT768.h"


/* 显示图片 */
void Display_picture(
 unsigned short x		    	/* x坐标 */
,unsigned short y			    /* y坐标 */
,unsigned short w			    /* 宽度 注意：必须为4的倍数 */
,unsigned short h			    /* 高度 */
,const unsigned char *bmp	/* 图片数据 */
)	
{
	  unsigned short  i, j, color;
	  unsigned char a, b;
		int p;
	
    Graphic_Mode();
    Active_Window_XY(x, y);
    Active_Window_WH(w, h);
    Goto_Pixel_XY(x, y);
    LCD_CmdWrite(0x04);
  
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {  
            a = (bmp[p]);
            b = (bmp[p + 1]);
            color = ((a << 8) | b);			
            if ((j % 32) == 0)Check_Mem_WR_FIFO_not_Full();			
            LCD_DataWrite_Pixel(color);
            p = p + 2;
        }
    }
    
    Check_Mem_WR_FIFO_Empty();
    Active_Window_XY(0, 0);
    Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
}

/* 灰阶 */
void GRAY_32(void)
{
    int i;
  
    for (i = 0; i < 32; i++)
    {
        LT768_DrawSquare_Fill(0, (LCD_YSIZE_TFT / 24) * i, (LCD_XSIZE_TFT - 1), (i + 1) * (LCD_YSIZE_TFT / 24), (i << 11) | (i << 6) | (i << 0));
    } 
}

/* 棋盘格 */
void WB_block(void)
{ 
    int i, n, v, h;
    int x = LCD_XSIZE_TFT  / 8, y = LCD_YSIZE_TFT / 8;
    h = LCD_XSIZE_TFT / x;
    v = LCD_YSIZE_TFT / y;
  
    for (n = 0; n < v; n++)
    { 
        if (n % 2 == 0)
        { 
            for (i = 0; i < h; i++)
            {
                if (i % 2 == 0)
                { 
                    LT768_DrawSquare_Fill(i * x, n * y, (i + 1) * x - 1, (n + 1) * y, White);
                }
                else 
                { 
                    LT768_DrawSquare_Fill(i * x, n * y, (i + 1) * x - 1, (n + 1) * y, Black);
                }
            }
        }
        else
        {
            for (i = 0; i < h; i++)
            { 
                if (i % 2 == 0)
                {
                    LT768_DrawSquare_Fill(i * x, n * y, (i + 1) * x - 1, (n + 1) * y, Black);
                }
                else
                {
                    LT768_DrawSquare_Fill(i * x, n * y, (i + 1) * x - 1, (n + 1) * y, White);
                } 
            }
        }
    }
}








