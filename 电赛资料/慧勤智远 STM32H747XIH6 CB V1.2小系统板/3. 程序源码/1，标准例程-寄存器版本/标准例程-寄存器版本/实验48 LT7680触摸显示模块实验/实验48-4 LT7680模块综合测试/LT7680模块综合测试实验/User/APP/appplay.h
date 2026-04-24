/**
 ****************************************************************************************************
 * @file        appplay.h
 * @version     V1.0
 * @brief       APP-应用中心 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __APPPLAY_H
#define __APPPLAY_H

#include "common.h"


#define APPPLAY_EX_BACKCOLOR    0X0000          /* 窗体外部背景色 */
#define APPPLAY_IN_BACKCOLOR    0X8C51          /* 窗体内部背景色 */
#define APPPLAY_NAME_COLOR      0X001F          /* 程序名颜色 */

#define APPPLAY_ALPHA_VAL       18              /* APP选中透明度设置 */
#define APPPLAY_ALPHA_COLOR     WHITE           /* APP透明色 */

/* 各图标/图片路径 */
extern uint8_t *const appplay_icospath_tbl[3][2];  /* icos的路径表 */
extern uint8_t *const appplay_appname_tbl[3][2];   /* icos名字 */

/* APP图标参数管理 */
typedef __PACKED_STRUCT _m_app_icos
{
    uint16_t x;         /* 图标坐标及尺寸 */
    uint16_t y;
    uint8_t width;
    uint8_t height;
    uint8_t *path;      /* 图标路径 */
    uint8_t *name;      /* 图标名字 */
} m_app_icos;

/* APP控制器 */
typedef struct _m_app_dev
{
    uint8_t selico; /**
                     * 当前选中的图标
                     * 0~1,被选中的图标编号
                     * 其他,没有任何图标被选中
                     */
    
    m_app_icos icos[2];    /* 总共2个图标 */
} m_app_dev;


uint8_t app_play(void);

#endif




