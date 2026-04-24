/**
 ****************************************************************************************************
 * @file        iap.h
 * @version     V1.0
 * @brief       IAP 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __IAP_H
#define __IAP_H

#include "./SYSTEM/sys/sys.h"


typedef void (*iapfun)(void);                   /* 定义一个函数类型的参数 */

#define FLASH_APP1_ADDR         0x08020000      /* FLASH应用程序起始地址(存放在内部FLASH)
                                                 * 保留 0X08000000~0X0801FFFF(128KB) 的空间为 Bootloader 使用(共128KB)
                                                 */

#define SRAM_APP_ADDR           0x24002000      /* SRAM应用程序起始地址(存放在内部SRAM) */


void iap_load_app(uint32_t appxaddr);                                         /* 跳转到APP程序执行 */
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t applen);   /* 在指定地址写入APP的bin数据 */

#endif




