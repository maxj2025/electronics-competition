/**
 ****************************************************************************************************
 * @file        fileupd.h
 * @version     V1.0
 * @brief       文件更新 代码 
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __FILEUPD_H
#define __FILEUPD_H

#include "./SYSTEM/sys/sys.h"


/* 文件信息结构体定义
 * 用来保存文件基本信息，地址，大小等
 */
__packed typedef struct 
{
    uint32_t file1addr;          /* file1的地址 */
    uint32_t file1size;          /* file1的大小 */
    uint32_t file2addr;          /* file2的地址 */
    uint32_t file2size;          /* file2的大小 */
    uint32_t file3addr;          /* file3的地址 */
    uint32_t file3size;          /* file3的大小 */
    uint32_t file4addr;          /* file4的地址 */
    uint32_t file4size;          /* file4的大小 */
    uint32_t file5addr;          /* file5的地址 */
    uint32_t file5size;          /* file5的大小 */
} _file_info;

/* 文件信息结构体 */
extern _file_info f_info;

/******************************************************************************************/

uint8_t files_update(uint16_t x, uint16_t y, uint8_t size, uint8_t *src, uint32_t color);  /* 更新文件 */

#endif





