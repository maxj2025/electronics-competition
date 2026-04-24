/**
 ****************************************************************************************************
 * @file        phoneplay.h
 * @version     V1.0
 * @brief       APP-拨号器 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __PHONEPLAY_H
#define __PHONEPLAY_H

#include "common.h"
#include "./BSP/GSM/gsm.h"


#define PHONE_MAX_INPUT     200     /* 最大输入200个号码长度 */


extern uint8_t *const PHONE_CALL_PIC[3];
extern uint8_t *const PHONE_HANGUP_PIC[3];
extern uint8_t *const PHONE_DEL_PIC[3];

typedef __PACKED_STRUCT
{
    uint16_t xoff;      /* x方向偏移 */
    uint16_t yoff;      /* y方向偏移 */
    uint16_t width;     /* 宽度 */
    uint16_t height;    /* 高度 */

    uint16_t time;      /* 通话时间 */

    uint8_t *inbuf;     /* 输入数字缓冲区 */
    uint8_t inlen;      /* 输入数字长度 */
    uint8_t *phonebuf;  /* 号码缓冲区 */
    uint8_t plen;       /* 号码长度 */
    uint8_t psize;      /* 号码字体大小:28,36,54 */
    uint8_t tsize;      /* 时间字体大小:12,16,24 */
} _phonedis_obj;
extern _phonedis_obj *pdis;


void phone_show_clear(_phonedis_obj *pdis);
void phone_show_time(_phonedis_obj *pdis);
void phone_show_calling(_phonedis_obj *pdis, __gsmdev *gsmx);
void phone_show_phone(_phonedis_obj *pdis, __gsmdev *gsmx);
void phone_show_input(_phonedis_obj *pdis, __gsmdev *gsmx);
void phone_ring(void);
void phone_incall_task(void *pdata);
uint8_t phone_incall_task_creat(void);
void phone_incall_task_delete(void);
uint8_t phone_play(void);

#endif























