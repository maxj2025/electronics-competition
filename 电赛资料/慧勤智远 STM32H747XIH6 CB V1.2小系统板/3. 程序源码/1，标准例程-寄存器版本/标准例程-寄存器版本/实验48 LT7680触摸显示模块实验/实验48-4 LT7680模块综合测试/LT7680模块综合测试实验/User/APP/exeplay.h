/**
 ****************************************************************************************************
 * @file        exeplay.h
 * @version     V1.0
 * @brief       APP-运行器 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __EXEPLAY_H
#define __EXEPLAY_H

#include "common.h"


/** 
    APP相关信息设置
    思路:把BIN文件先存放到外部SDRAM,然后设置标志位,产生一次软复位.软复位之后,系统判断标志位,
    如果需要运行app,则先把标志位清空,然后复制外部SDRAM的APP代码到内部sram,最后跳转到app的起
    始地址,开始运行app代码.
    注意:
    1,默认设置APP的尺寸最大为EXEPLAY_APP_SIZE字节.
    2,APP_SIZE必须小于EXEPLAY_APP_SIZE。
 */

#define EXEPLAY_APP_SIZE    200 * 1024      /* app代码的最大尺寸.这里为200K字节(还包括了SRAM，实际上不可能运行200K的代码) */
#define EXEPLAY_APP_BASE    0x24001000      /* app执行代码的目的地址,也就是将要运行的代码存放的地址 */
#define	EXEPLAY_SRC_BASE    0XD03E8000      /* app执行代码的源地址,也就是软复位之前,app代码存放的地址 */


typedef  void (*dummyfun)(void);            /* 定义一个函数类型 */
extern dummyfun jump2app;                   /* 假函数,让PC指针跑到新的main函数去 */
void exeplay_write_appmask(uint16_t val);
void exeplay_app_check(void);
uint8_t exe_play(void);

#endif








