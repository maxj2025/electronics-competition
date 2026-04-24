/**
 ****************************************************************************************************
 * @file        spb.h
 * @version     V1.0
 * @brief       SPB效果实现 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H743XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __SPB_H
#define __SPB_H

#include "./BSP/SPBLCD/spblcd.h"


/* 支持的语言种类 */
#define GUI_LANGUAGE_NUM    3


#define SPB_PAGE_NUM        3       /* SPB页数,每页最多放8个图标 */
#define SPB_ICOS_NUM        18      /* SPB区域总图标数 */

#define SPB_MOVE_WIN        2       /* SPB滑动检测窗口值 */
#define SPB_MOVE_MIN        8       /* SPB认为是滑动的最小值,超过会认为是滑动,但是不一定会翻页 */
#define SPB_MOVE_ACT        50      /* SPB滑动的启动值,超过这个值将会进行翻页 */

#define SPB_ALPHA_VAL       18      /* SPB选中透明度设置 */
#define SPB_ALPHA_COLOR     WHITE   /* SPB透明色 */
#define SPB_FONT_COLOR      WHITE   /* SPB字体颜色 */

#define SPB_MSG_COLOR       WHITE   /* 顶部信息条字体颜色 */
#define SPB_MSG_BKCOLOR     0X0949  /* 顶部信息栏背景色 0X1D16 */
#define SPB_MICO_COLOR      0X0D3D  /* 顶部mico图标颜色 0X6B4D  0X243C */

extern uint8_t *const spb_bkpic_path_tbl[9];
extern uint8_t *const spb_icos_path_tbl[4][SPB_ICOS_NUM];
extern uint8_t *const spb_micos_path_tbl[3][3];

/* SPB 图标控制结构体 */
__packed typedef struct _m_spb_icos
{
    uint16_t x;         /* 图标坐标及尺寸 */
    uint16_t y;
    uint8_t width;
    uint8_t height;
    uint8_t *path;      /* 图标路径指针 */
    uint8_t *name;      /* 图标名字指针 */
} m_spb_icos;


/* SPB 控制器 */
typedef struct _m_spb_dev
{
    uint16_t oldxpos;       /* 上一次TP的x坐标位置 */
    uint16_t curxpos;       /* 当前tp按下的x坐标 */
    uint16_t curypos;       /* 当前tp按下的y坐标 */
    uint16_t spbsta;        /**
                             * spb状态 
                             * [15]:第一次按下标志
                             * [14]:滑动标志;
                             * [13~0]:滑动点数
                             */

    uint16_t pos;           /* 当前帧的位置(x坐标) */
    uint16_t oldpos;        /* 触摸按下时的帧位置 */

    uint8_t stabarheight;   /* 状态栏高度 */
    uint16_t spbheight;     /* SPB滑动区域高度 */
    uint16_t spbwidth;      /* SPB滑动区域宽度,一般等于LCD分辨率宽度 */
    uint16_t spbahwidth;    /* SPB左右预留宽度,一般等于LCD宽度的1/4 */
    uint16_t spbfontsize;   /* SPB字体大小 */

    uint8_t selico;         /* 当前选中的图标: 0~8,被选中的图标编号; 其他,没有任何图标被选中 */
    m_spb_icos icos[SPB_ICOS_NUM];  /* 总图标数 */
    m_spb_icos micos[4];            /* 4个主图标 */
} m_spb_dev;

extern m_spb_dev spbdev;


uint8_t spb_init(uint8_t mode);
void spb_delete(void);
uint8_t spb_load_icos(void);
uint8_t spb_load_micos(void);
void spb_gsm_signal_show(uint16_t x, uint16_t y, uint8_t signal);
void spb_stabar_msg_show(uint8_t clr);
uint8_t spb_load_mui(void);
void spb_frame_move(uint8_t dir, uint8_t skips, uint16_t pos);
void spb_unsel_micos(uint8_t selx);
void spb_set_sel(uint8_t sel);
uint8_t spb_move_chk(void);

#endif







