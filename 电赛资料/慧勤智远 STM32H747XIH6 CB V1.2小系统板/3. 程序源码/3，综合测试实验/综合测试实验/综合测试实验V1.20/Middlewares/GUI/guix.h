/**
 ****************************************************************************************************
 * @file        guix.h
 * @version     V1.0
 * @brief       GUI-底层实现 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */ 
 
#ifndef __GUIX_H
#define __GUIX_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/LCD/lcd.h"
#include "string.h"


/* 用到ucos的滴答时钟 */
typedef unsigned int        INT32U;
extern volatile  INT32U     OSTime;

/**
 * gui时间计数器,需要在外部实现
 * gui定时器,每10ms增加1次
 */
#define GUI_TIMER_10MS      OSTime/10       /* ucos的调度时间设置为1ms */

#define GUI_VERSION         130             /* gui版本(扩大了100倍),100表示V1.00 */
/****************************************************************************************************/

/* GUI支持的语言种类数目 */
#define GUI_LANGUAGE_NUM    3               /* 语言种类数: 0, 简体中文; 1, 繁体中文; 2, 英文; */

extern uint8_t *const GUI_OK_CAPTION_TBL[GUI_LANGUAGE_NUM];     /* 确认按钮 */
extern uint8_t *const GUI_OPTION_CAPTION_TBL[GUI_LANGUAGE_NUM]; /* 选项按钮 */
extern uint8_t *const GUI_BACK_CAPTION_TBL[GUI_LANGUAGE_NUM];   /* 取消按钮 */
extern uint8_t *const GUI_CANCEL_CAPTION_TBL[GUI_LANGUAGE_NUM]; /* 取消按钮 */
extern uint8_t *const GUI_QUIT_CAPTION_TBL[GUI_LANGUAGE_NUM];   /* 退出按钮 */
/****************************************************************************************************/

/* gui颜色定义 */
#define GUI_COLOR_BLACK     0X0000
#define GUI_COLOR_WHITE     0XFFFF


/* 输入事件类型 */
#define	IN_TYPE_TOUCH       0X00
#define IN_TYPE_KEY         0X01
#define IN_TYPE_JOYPAD      0X02
#define IN_TYPE_MOUSE       0X03
#define IN_TYPE_ERR         0XFF

/* gui输入接口 */
typedef __PACKED_STRUCT
{
    void (*get_key)(void *, uint8_t);   /* 获取键值参数 */
    uint16_t x;         /* 坐标 */
    uint16_t y;
    uint32_t keyval;    /* 按键键值 */
    uint8_t  intype;    /* 输入类型 */
    uint8_t  ksta;      /* 输入设备(按键)的状态 [7:1],保留;[0],0,松开;1,按下 */
} _in_obj;
extern _in_obj in_obj;

/**
 * gui底层接口
 * 也就是外部需要实现的函数!
 * 这里我们需要外部提供4个函数
 * 1,读点函数;
 * 2,画点函数;
 * 3,单色填充函数;
 * 4,彩色填充函数;
 */
typedef __PACKED_STRUCT
{
    uint8_t language;       /* GUI当前语言,取值范围0~GUI_LANGUAGE_NUM-1; */
    uint16_t memdevflag;    /**
                             * 存储设备状态;
                             * bit7:0,存储设备7~0在位状态;0,不在位;1,在位;
                             * bit15:8,存储设备7~0使用状态;0,不在使用中;1,正在使用中
                             * 本例中,存储设备0表示:SD卡
                             *                1表示:SPI FLASH
                             *                2表示:SD NAND  
                             *                3表示:U盘
                             * 其他,未用到
                             */
    /* 这四个参数,会控制listbox/filelistbox等控件的具体效果 */
    uint8_t tbfsize;        /* filelistbox/listbox时,顶部/底部横条字体大小 */
    uint8_t tbheight;       /* filelistbox/listbox时,顶部/底部横条高度 */
    uint8_t listfsize;      /* list字体大小 */
    uint8_t listheight;     /* list条目高度 */

    uint16_t back_color;    /* 背景色 */
    uint32_t(*read_point)(uint16_t, uint16_t);                              /* 读点函数 */
    void(*draw_point)(uint16_t, uint16_t, uint32_t);                        /* 画点函数 */
    void(*fill)(uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);          /* 单色填充函数 */
    void(*colorfill)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t *);   /* 彩色填充函数 */
    uint16_t lcdwidth;      /* LCD的宽度 */
    uint16_t lcdheight;     /* LCD的高度 */
} _gui_phy;
extern _gui_phy gui_phy;

uint16_t gui_alpha_blend565(uint16_t src, uint16_t dst, uint8_t alpha);     /* alpha blending */
uint16_t gui_color_chg(uint32_t rgb);           /* 颜色转换.将32位色转为16位色 */
uint16_t gui_rgb332torgb565(uint16_t rgb332);   /* 将rgb332转为rgb565 */
long long gui_pow(uint8_t m, uint8_t n);        /* 得到m^n */
void gui_swap_u16(uint16_t *p1,uint16_t *p2);

uint8_t *gui_path_name(uint8_t *pname, uint8_t *path, uint8_t *name);

void gui_memset(void *p, uint8_t c, uint32_t len);
void *gui_memin_malloc(uint32_t size);
void gui_memin_free(void *ptr);
void *gui_memex_malloc(uint32_t size);
void gui_memex_free(void *ptr);
void *gui_memin_realloc(void *ptr, uint32_t size);

void gui_init(void);
void gui_get_key(void *obj, uint8_t type);
uint32_t gui_disabs(uint32_t x1, uint32_t x2);  /* 差的绝对值 */
void gui_alphablend_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t aval);
void gui_draw_bigpoint(uint16_t x0, uint16_t y0, uint16_t color);   /* 画大点 */
void gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color); /* 画任意线 */
void gui_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color);  /* 画一条粗线 */
void gui_draw_bline1(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t size, uint16_t color); /* 画一条粗线,方法1 */
void gui_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);     /* 画矩形 */
void gui_draw_arcrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t mode, uint16_t upcolor, uint16_t downcolor); /* 画圆角矩形 */
void gui_draw_vline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color);        /* 画垂直线 */
void gui_draw_hline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color);        /* 画水平线 */
void gui_fill_colorblock(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t *ctbl, uint8_t mode);  /* 填充颜色块 */
void gui_smooth_color(uint32_t srgb, uint32_t ergb, uint16_t *cbuf, uint16_t len);  /* 颜色平滑过渡 */
void gui_draw_smooth_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t srgb, uint32_t ergb);  /* 画颜色平滑过渡的矩形/线条 */
void gui_fill_rectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color); /* 填充区域 */
void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void gui_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void gui_draw_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color);
void gui_fill_ellipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint16_t color);
void gui_draw_argrec(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);
void gui_show_strmid(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t size, uint8_t *str);
void gui_show_ptchar(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t chr, uint8_t mode);
void gui_show_ptfont(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t size, uint8_t *chr, uint8_t mode);
void gui_show_ptstr(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint8_t size, uint8_t *str, uint8_t mode);
void gui_show_ptstrwhiterim(uint16_t x, uint16_t y, uint16_t xend, uint16_t yend, uint16_t offset, uint16_t color, uint16_t rimcolor, uint8_t size, uint8_t *str);  /* 特效显示字符串 */
void gui_draw_icos(uint16_t x, uint16_t y, uint8_t size, uint8_t index);
void gui_draw_icosalpha(uint16_t x, uint16_t y, uint8_t size, uint8_t index);
void gui_show_num(uint16_t x, uint16_t y, uint8_t len, uint16_t color, uint8_t size, long long num, uint8_t mode);
uint8_t *gui_num2str(uint8_t *str, uint32_t num);   /* 将数字转为字符串 */
void gui_draw_arc(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t rx, uint16_t ry, uint16_t r, uint16_t color, uint8_t mode);

uint32_t gui_get_stringline(uint8_t *str, uint16_t linelenth, uint8_t font);
void gui_show_string(uint8_t *str, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t fcolor);
uint16_t gui_string_forwardgbk_count(uint8_t *str, uint16_t pos);


#endif



























