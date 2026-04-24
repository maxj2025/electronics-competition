/**
 ****************************************************************************************************
 * @file        usbh_diskio.h
 * @version     V1.0
 * @brief       usbh diskio 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H

#include "usbh_core.h"
#include "usbh_msc.h"
#include "./FATFS/source/diskio.h"


#define USB_DEFAULT_BLOCK_SIZE      512


/* 函数声明 */
DSTATUS USBH_initialize(void);                                   /* 初始化USB磁盘 */
DSTATUS USBH_status(void);                                       /* 获取USB磁盘状态 */
DRESULT USBH_read(BYTE *buff, DWORD sector, UINT count);         /* 读取USB磁盘 */
DRESULT USBH_write(const BYTE *buff, DWORD sector, UINT count);  /* 写入USB磁盘 */
DRESULT USBH_ioctl(BYTE cmd, void *buff);                        /* 获取USB磁盘控制参数 */

#endif 




