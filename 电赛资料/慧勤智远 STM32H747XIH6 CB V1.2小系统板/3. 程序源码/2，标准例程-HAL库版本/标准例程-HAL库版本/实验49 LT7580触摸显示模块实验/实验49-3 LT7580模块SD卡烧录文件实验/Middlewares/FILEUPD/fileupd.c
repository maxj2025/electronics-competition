/**
 ****************************************************************************************************
 * @file        fileupd.c
 * @version     V1.0
 * @brief       文件更新 代码 
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "string.h"
#include "./BSP/LCD/lcd.h"
#include "./FILEUPD/fileupd.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/source/ff.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LT758_FLASH/LT758_w25qxx.h"


/* 文件区域占用的总扇区数大小(一个扇区4K字节) */
#define FILESECSIZE         0


/**
 * 文件存放起始地址
 */
#define FILEADDR            0

/* 最大更新文件数量 */
#define FILE_NUM            5      
 
/* 用来保存文件基本信息，地址，大小等 */
_file_info f_info;

/* 文件存放在磁盘中的路径 */
char *const FILE_PATH[FILE_NUM] =
{
    "/file1.bin",                  /* file1的存放位置 */
    "/file2.bin",                  /* file2的存放位置 */
    "/file3.bin",                  /* file3的存放位置 */
    "/file4.bin",                  /* file4的存放位置 */
    "/file5.bin",                  /* file5的存放位置 */
};

/* 更新时的提示信息 */
char *const FILE_UPDATE_REMIND_TBL[FILE_NUM] =
{
    "Updating file1.bin",          /* 提示正在更新file1.bin */
    "Updating file2.bin",          /* 提示正在更新file2.bin */
    "Updating file3.bin",          /* 提示正在更新file3.bin */
    "Updating file4.bin",          /* 提示正在更新file4.bin */
    "Updating file5.bin",          /* 提示正在更新file5.bin */

};

/**
 * @brief       显示当前文件更新进度
 * @param       x, y    : 坐标
 * @param       size    : 字体大小
 * @param       totsize : 整个文件大小
 * @param       pos     : 当前文件指针位置
 * @param       color   : 字体颜色
 * @retval      无
 */
static void files_progress_show(uint16_t x, uint16_t y, uint8_t size, uint32_t totsize, uint32_t pos, uint32_t color)
{
    float prog;
    uint8_t t = 0XFF;
    prog = (float)pos / totsize;
    prog *= 100;

    if (t != prog)
    {
        lcd_show_string(x + 3 * size / 2, y, 240, 320, size, "%", color);
        t = prog;

        if (t > 100)
        {
            t = 100;
        }
        
        lcd_show_num(x, y, t, 3, size, color);  /* 显示数值 */
    }
}

/**
 * @brief       更新某一个文件
 * @param       x, y    : 提示信息的显示位置
 * @param       size    : 提示信息字体大小
 * @param       fpath   : 文件路径
 * @param       fx      : 更新的内容
 *   @arg                 0, file1;
 *   @arg                 1, file2;
 *   @arg                 2, file3;
 *   @arg                 3, file4;
 *   @arg                 4, file5;
 * @param       color   : 字体颜色
 * @retval      0, 成功; 其他, 错误代码;
 */
static uint8_t files_update_filex(uint16_t x, uint16_t y, uint8_t size, uint8_t *fpath, uint8_t fx, uint32_t color)
{
    uint32_t flashaddr = 0;
    FIL *fftemp;
    uint8_t *tempbuf;
    uint8_t res;
    uint16_t bread;
    uint32_t offx = 0;
    uint8_t rval = 0;
    fftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));  /* 分配内存 */
  
    if (fftemp == NULL)
    {
        rval = 1;
    }

    tempbuf = mymalloc(SRAMIN, 4096);               /* 分配4096个字节空间 */

    if (tempbuf == NULL)
    {
        rval = 1;
    }
    
    res = f_open(fftemp, (const TCHAR *)fpath, FA_READ);

    if (res)
    {
        rval = 2;   /* 打开文件失败 */
    }

    if (rval == 0)
    {
        switch (fx)
        {
            case 0: /* 更新 file1 */
                f_info.file1addr = FILEADDR ;                               
                f_info.file1size = fftemp->obj.objsize;                  /* file1文件大小 */
                flashaddr = f_info.file1addr;                            /* file1的起始地址 */
                break;

            case 1: /* 更新 file2 */
                f_info.file2addr = f_info.file1addr + f_info.file1size;  /* file1之后，紧跟file2文件 */
                f_info.file2size = fftemp->obj.objsize;                  /* file2文件大小 */
                flashaddr = f_info.file2addr;                            /* file2的起始地址 */
                break;

            case 2: /* 更新 file3 */
                f_info.file3addr = f_info.file2addr + f_info.file2size;  /* file2之后，紧跟file3文件 */
                f_info.file3size = fftemp->obj.objsize;                  /* file3文件大小 */
                flashaddr = f_info.file3addr;                            /* file3的起始地址 */
                break;

            case 3: /* 更新 file4 */
                f_info.file4addr = f_info.file3addr + f_info.file3size;  /* file3之后，紧跟file4文件 */
                f_info.file4size = fftemp->obj.objsize;                  /* file4文件大小 */
                flashaddr = f_info.file4addr;                            /* file4的起始地址 */
                break;
            
            case 4: /* 更新 file5 */
                f_info.file5addr = f_info.file4addr + f_info.file4size;  /* file4之后，紧跟file5文件 */
                f_info.file5size = fftemp->obj.objsize;                  /* file5文件大小 */
                flashaddr = f_info.file5addr;                            /* file5的起始地址 */
                break;
        }
        
        while (res == FR_OK)   /* 死循环执行 */
        {
            res = f_read(fftemp, tempbuf, 4096, (UINT *)&bread);         /* 读取数据 */

            if (res != FR_OK)
            {
                break;         /* 执行错误 */
            }

            spi_flash_write_nocheck(tempbuf, offx + flashaddr, bread);   /* 写入bread个数据 */
            offx += bread;
            files_progress_show(x, y, size, fftemp->obj.objsize, offx, color);  /* 进度显示 */

            if (bread != 4096)
            {
                break;         /* 读完了 */
            }
        }
        
        f_close(fftemp);
    }

    myfree(SRAMIN, fftemp);    /* 释放内存 */
    myfree(SRAMIN, tempbuf);   /* 释放内存 */
    
    return res;
}

uint8_t file_nums = 1;

/**
 * @brief       更新文件
 * @note        所有查找到的文件一起更新
 * @param       x, y    : 提示信息的显示位置
 * @param       size    : 提示信息字体大小
 * @param       src     : 字库来源磁盘
 *   @arg                 "0:", SD卡;
 *   @arg                 "1:", FLASH盘
 * @param       color   : 字体颜色
 * @retval      0, 成功; 其他, 错误代码;
 */
uint8_t files_update(uint16_t x, uint16_t y, uint8_t size, uint8_t *src, uint32_t color)
{
    uint8_t *pname;
    uint32_t *buf;
    uint8_t res = 0;
    uint16_t i;
    FIL *fftemp;
    uint8_t rval = 0;
   
    res = 0XFF;
    pname = mymalloc(SRAMIN, 100);                  /* 申请100字节内存 */
    buf = mymalloc(SRAMIN, 4096);                   /* 申请4K字节内存 */
    fftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));  /* 分配内存 */

    if (buf == NULL || pname == NULL || fftemp == NULL)
    {
        myfree(SRAMIN, fftemp);
        myfree(SRAMIN, pname);
        myfree(SRAMIN, buf);
        return 5;                  /* 内存申请失败 */
    }

    for (i = 0; i < FILE_NUM; i++) /* 先查找文件是否正常 */
    {
        strcpy((char *)pname, (char *)src);                 /* copy src内容到pname */
        strcat((char *)pname, (char *)FILE_PATH[i]);        /* 追加具体文件路径 */
        res = f_open(fftemp, (const TCHAR *)pname, FA_READ);/* 尝试打开 */

        if (res)
        {
            if (i == 0)            /* 第一个文件file1打开失败 */
            {
                rval |= 1 << 7;    /* 标记打开文件失败 */
            }
              
            break;                 /* 出错了,直接退出 */
        }
    }
   
    file_nums = i;                 /* 记录查找到的文件数量 */
    
    myfree(SRAMIN, fftemp);        /* 释放内存 */

    if (rval == 0)                 /* 存在要更新的文件 */
    {
        lcd_show_string(x, y, 240, 320, size, "Erasing chip... ", color);   /* 提示正在全片擦除 */
        spi_flash_erase_chip();    /* 先擦除整个芯片,提高写入速度 */
        
        for (i = 0; i < file_nums; i++) /* 依次更新文件 */
        {
            lcd_show_string(x, y, 240, 320, size, FILE_UPDATE_REMIND_TBL[i], color);
            strcpy((char *)pname, (char *)src);             /* copy src内容到pname */
            strcat((char *)pname, (char *)FILE_PATH[i]);    /* 追加具体文件路径 */
            res = files_update_filex(x + 20 * size / 2, y, size, pname, i, color);   /* 更新字库 */

            if (res)
            {
                myfree(SRAMIN, buf);
                myfree(SRAMIN, pname);
                return 1 + i;
            }
        }
    }

    myfree(SRAMIN, pname);  /* 释放内存 */
    myfree(SRAMIN, buf);
    
    return rval;            /* 无错误 */
}






