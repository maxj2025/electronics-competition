/**
 ****************************************************************************************************
 * @file        usbh_hid_mouse.h
 * @version     V1.0
 * @brief       USB鼠标 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __USBH_HID_MOUSE_H
#define __USBH_HID_MOUSE_H

#include "usbh_hid.h"


/* 鼠标信息结构体 */
typedef struct _HID_MOUSE_Info
{
  uint8_t              x;           /* x轴增量（强制转换成signed char后使用）*/
  uint8_t              y;           /* y轴增量（强制转换成signed char后使用）*/
  uint8_t              z;           /* z轴增量（强制转换成signed char后使用）*/
  uint8_t              button;      /* 将buttons修改为button,存储按键状态 */
}
HID_MOUSE_Info_TypeDef;


USBH_StatusTypeDef USBH_HID_MouseInit(USBH_HandleTypeDef *phost);
HID_MOUSE_Info_TypeDef *USBH_HID_GetMouseInfo(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_HID_MouseDecode(USBH_HandleTypeDef *phost);

#endif




