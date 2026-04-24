/**
 ****************************************************************************************************
 * @file        usbplay.c
 * @version     V1.0
 * @brief       APP-USB连接 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "usbplay.h"
#include "usb_app.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"


/* 在nesplay.c里面定义 */
extern uint8_t *const nes_remindmsg_tbl[5][GUI_LANGUAGE_NUM];


/**
 * @brief       USB连接
 * @param       无
 * @retval      0, 正常退出; 其他, 错误代码;
 */
uint8_t usb_play(void)
{
    uint8_t rval = 0;       /* 返回值 */
    uint8_t offline_cnt = 0;
    uint8_t tct = 0;
    uint8_t USB_STA = 0;
    uint8_t busycnt = 0;    /* USB忙计数器 */
    uint8_t errcnt = 0;     /* USB错误计数器 */

    if (gui_phy.memdevflag & (1 << 3))
    {
        window_msg_box((lcddev.width - 220) / 2, (lcddev.height - 100) / 2, 220, 100, (uint8_t*)nes_remindmsg_tbl[0][gui_phy.language], (uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        while (gui_phy.memdevflag & (1 << 3))
        {
            delay_ms(5);    /* 死循环等待U盘被拔出 */
        }
    }

    g_back_color = LGRAY;
    lcd_clear(g_back_color);
    
    app_filebrower((uint8_t *)APP_MFUNS_CAPTION_TBL[12][gui_phy.language], 0X05); /* 显示标题 */
    usbapp_mode_set(USBD_MSC_MODE); /* DEVICE MSC */
    lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB DisConnected ", BLUE); /* 首先提示USB无连接 */

    while (rval == 0)
    {
        delay_ms(1000 / OS_TICKS_PER_SEC); /* 延时一个时钟节拍 */

        
        if (system_task_return)
        {
            if (busycnt) /* USB正在读写 */
            {
                lcd_show_string(60 + (lcddev.width - 240) / 2, 110 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB BUSY!!!", RED); /* 提示USB繁忙 */
                system_task_return = 0;	/* 取消 */
            }
            else break;  /* USB空闲,则退出USB */
        }

        tct++;

        if (tct == 40)   /* 每40ms进入一次 */
        {
            tct = 0;

            if (busycnt)busycnt--;
            else gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 110 + (lcddev.height - 320) / 2, 100, 16, g_back_color); /* 清除显示 */

            if (errcnt)errcnt--;
            else gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, 128, 16, g_back_color); /* 清除显示 */

            if (usbx.bDeviceState & 0x10)       /* 有轮询操作 */
            {
                offline_cnt = 0;                /* USB连接了,则清除offline计数器 */
                usbx.bDeviceState |= 0X80;      /* 标记USB连接正常 */
                usbx.bDeviceState &= ~(1 << 4); /* 清除轮询标志位 */
            }
            else
            {
                offline_cnt++;

                if (offline_cnt > 50)usbx.bDeviceState = 0; /* 2s内没收到在线标记,代表USB被拔出了 */
            }
        }

        if (USB_STA != usbx.bDeviceState) /* 状态改变了 */
        {
            gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, 120, 16, g_back_color); /* 清除显示 */

            if (usbx.bDeviceState & 0x01) /* 正在写 */
            {
                if (busycnt < 5)busycnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Writing...", BLUE); /* 提示USB正在写入数据 */
            }

            if (usbx.bDeviceState & 0x02) /* 正在读 */
            {
                if (busycnt < 5)busycnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Reading...", BLUE); /* 提示USB正在读出数据 */
            }

            if (usbx.bDeviceState & 0x04)
            {
                if (errcnt < 5)errcnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Write Error", RED); /* 提示写入错误 */
            }

            if (usbx.bDeviceState & 0x08)
            {
                if (errcnt < 5)errcnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Read  Error", RED); /* 提示读出错误 */
            }
            
            if (usbx.bDeviceState & 0X80)lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Connected   ", BLUE);   /* 提示USB连接已经建立 */
            else lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, 320, 16, "USB DisConnected", BLUE);  /* 提示USB被拔出了 */

            usbx.bDeviceState &= 0X90;      /* 清除除连接状态&轮询标志外的其他所有位 */
            USB_STA = usbx.bDeviceState;    /* 记录最后的状态 */
        }
    }

    g_back_color = BLACK;
    usbapp_mode_set(USBH_MSC_MODE);         /* HOST MSC */
    return rval;
}







