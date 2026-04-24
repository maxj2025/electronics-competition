/**
 ****************************************************************************************************
 * @file        settings.c
 * @version     V1.0
 * @brief       APP-设置 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "settings.h"
#include "calendar.h"
#include "spb.h"


_system_setings systemset;
#define SYS_MENU_SIZE           23      /* 条目个数 */

/* 系统设置主目录表 */
uint8_t *const sysset_mmenu_tbl[GUI_LANGUAGE_NUM][SYS_MENU_SIZE] =   /* 系统一级目录的个数 */
{
    {
        "1.时间设置",
        "2.日期设置",
        "3.闹钟时间设置",
        "4.闹钟开关设置",
        "5.闹钟铃声设置",
        "6.语言设置",
        "7.数码相框设置",
        "8.音乐播放器模式设置",
        "9.视频播放器模式设置",
        "10.板载喇叭开关设置",
        "11.ES8388音量&3D设置",
        "12.ES8388音效1设置",
        "13.ES8388音效2设置",
        "14.ES8388音效3设置",
        "15.ES8388音效4设置",
        "16.ES8388音效5设置",
        "17.背光设置",
        "18.屏幕校准",
        "19.系统文件更新",
        "20.恢复默认设置",
        "21.系统信息",
        "22.系统状态", 
        "23.关于"
    },
    {
        "1.時間設置",
        "2.日期設置",
        "3.鬧鐘時間設置",
        "4.鬧鐘開關設置",
        "5.鬧鐘鈴聲設置",
        "6.語言設置",
        "7.數碼相框設置",
        "8.音樂播放器模式設置",
        "9.視頻播放器模式設置", 
        "10.板載喇叭開關設置", 
        "11.ES8388音量&3D設置",
        "12.ES8388音效1設置",
        "13.ES8388音效2設置",
        "14.ES8388音效3設置",
        "15.ES8388音效4設置",
        "16.ES8388音效5設置",
        "17.背光設置",
        "18.屏幕校準",
        "19.系統文件更新",
        "20.恢復默認設置",
        "21.系統信息",
        "22.系統狀態", 
        "23.關於"
    },
    {
        "1.TIME SET",
        "2.DATE SET",
        "3.ALARM TIME SET",
        "4.ALARM ON/OFF SET",
        "5.ALARM RING SET",
        "6.LANGUAGE SET",
        "7.DIGITAL PHOTO FRAME SET",
        "8.AUDIO PLAYER MODE SET",
        "9.VIDEO PLAYER MODE SET",
        "10.ONBOARD SPEAKER SET",
        "11.ES8388 VOL&3D SET",
        "12.ES8388 EQ1 SET",
        "13.ES8388 EQ2 SET",
        "14.ES8388 EQ3 SET",
        "15.ES8388 EQ4 SET",
        "16.ES8388 EQ5 SET",
        "17.BACKLIGHT SET",
        "18.TOUCH SCREEN ADJUST", 
        "19.SYSTEM FILE UPDATE",
        "20.RESTORE DEFAULT SET",
        "21.SYSTEM INFORMATION",
        "22.SYSTEM STATUS", 
        "23.ABOUT"
    },
};

/* 语言设置列表 */
uint8_t *const sysset_language_tbl[GUI_LANGUAGE_NUM] =
{
    "简体中文", "繁體中文", "English",
};

/* 铃声设置列表 */
uint8_t *const sysset_ring_tbl[GUI_LANGUAGE_NUM][4] =
{
    {"铃声1", "铃声2", "铃声3", "铃声4",},
    {"鈴聲1", "鈴聲2", "鈴聲3", "鈴聲4",},
    {"RING 1", "RING 2", "RING 3", "RING 4",},
};

/* 音乐/视频播放模式设置列表 */
uint8_t *const sysset_avplaymode_tbl[GUI_LANGUAGE_NUM][3] =
{
    {"全部循环", "随机播放", "单曲循环"},
    {"全部循環", "隨機播放", "單曲循環"},
    {"LOOP PLAY", "SHUFFLE PLAY", "SINGLE PLAY"},
};

/* 板载喇叭开关设置列表 */
uint8_t *const sysset_speakermode_tbl[GUI_LANGUAGE_NUM][2] =
{
    {"关闭喇叭", "开启喇叭"},
    {"關閉喇叭", "開啟喇叭"},
    {"Speaker OFF", "Speaker ON"},
};

/* ES8388音量&3D設置 */
uint8_t *const sysset_es8388vol3d_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"主音量:", "主音量:", "Volume:",},
    {"3D效果:", "3D效果:", "3D Effect:"},
};

/* ES8388EQ设置 */
uint8_t *const sysset_es8388eq_tbl[2][GUI_LANGUAGE_NUM]=
{ 
    {"中心频率:","中心頻率:","Center Freq:",}, 
    {"增益:","增益:","Gain:"}, 
};

/* ES8388可设置频率表 */
uint8_t *const sysset_es8388freq_tbl[5][4]=
{
    {"80","105","135","175",},
    {"230","300","385","500",},
    {"650","850","1100","1400",},
    {"1800","2400","3200","4100",},
    {"5300","6900","9000","11700",},
};

/* 系统设置相关提示信息 */
uint8_t *const sysset_system_remaindmsg_tbl[3][GUI_LANGUAGE_NUM] =
{
    {"您确认更新系统文件?", "您確認更新系統文件?", "Are you sure to update?"},
    {"您确认恢复默认设置?", "您確認恢復默認設置?", "Are you sure to restore?"},
    {"恢复默认设置中...", "恢復默認設置中...", "Default set restoring...",},
};

/* 系统更新复制提示信息 */
uint8_t *const sysset_system_update_cpymsg_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"正在复制:", "正在複製:", " Copying:",},
    {"当前文件夹:", "當前文件夾:", "Cur Folder:",},
};

/* 系统更新提示 */
uint8_t *const sysset_system_update_msg_tbl[GUI_LANGUAGE_NUM] =
{"系统正在更新...", "系統正在更新...", "SYSTEM Updating..."};

/* 系统更新结果提示 */
uint8_t *const sysset_system_update_err_tbl[3][GUI_LANGUAGE_NUM] =
{
    {"系统文件更新成功!", "系統文件更新成功!", "SYSTEM file lost!",},
    {"系统文件丢失!", "系統文件丟失!", "SYSTEM file lost!",},
    {"用户终止更新!", "用戶終止更新!", "User stop update!",},
};

/* 系统信息标注表 */
uint8_t *const sysset_system_info_caption_tbl[11][GUI_LANGUAGE_NUM] =
{
    {"处理器:","處理器:","CPU:"},
    {"内存:","內存:","RAM:"},
    {"SD卡:","SD卡:","SD Card:"},
    {"FLASH盘:","FLASH盤:","FLASH Disk:"},
    {"SD NAND:","SD NAND:","SD NAND:"},
    {"U盘:","U盤:","U Disk:"},
    {"操作系统:","操作系統:","OS:"},
    {"图形界面:","圖形界面:","GUI:"},
    {"硬件平台:","硬件平臺:","Hardware:"},
    {"版权信息:","版權信息:","Copyright:"},
    {"技术支持:","技術支持:","Tech Support:"},
};

/* 系统提示信息表 */
uint8_t *const sysset_system_info_msg_tbl[11] =
{
    "STM32H747XIH6 @ 480Mhz",
    "1060KB+32MB",
    "MB",
    "KB",
    "MB",
    "MB",
    "uCOS II ",
    "WKS GUI ",
    "STM32H747 Core Board  ",
    "",
    "",
};

/* 系统状提示信息 */
uint8_t *const sysset_sysstatus_tbl[5][GUI_LANGUAGE_NUM] =
{
    {"CPU使用率:", "CPU使用率:", "CPU USAGE:",},
    {"内部内存使用率:", "内部内存使用率:", "IN MEMORY USAGE:",},
    {"TCM内存使用率:", "TCM内存使用率:", "TCM MEMORY USAGE:",},
    {"外部内存使用率:", "外部内存使用率:", "EX MEMORY USAGE:"},
    {"温度:", "溫度:", "TEMP:"},
};

/* 系统关于提示信息 */
uint8_t *const sysset_system_about_caption_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"STM32H747 小系统板","STM32H747 小系统板","STM32H747 Core Board",},
    {"产品序列号","產品序列號","Serial Number",},
};

/* 语言设置后的提示 */
uint8_t *const sysset_remindmsg_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"更新主界面", "更新主界面", "Updating",},
    {"系统正在更新主界面,请稍候...", "系統正在更新主界面,請稍候...", "System updating,Please wait...",},
};

/****************************************************************************************************/

#define SYSSET_BTN1_WIDTH           50          /* 一类按键宽度(加减按钮) */
#define SYSSET_BTN1_HEIGHT          40          /* 一类按键高度(加减按钮) */
#define SYSSET_BTN2_WIDTH           60          /* 二类按键宽度(确认/取消按钮) */
#define SYSSET_BTN2_HEIGHT          30          /* 二类按键高度(确认/取消按钮) */
#define SYSSET_BTN3_WIDTH           100         /* 三类按键宽度(单个按钮的宽度) */
#define SYSSET_EDIT_WIDTH           50          /* 文字编辑处的宽度 */
#define SYSSET_EDIT_HEIGHT          25          /* 文字编辑处的高度 */

#define SYSSET_INWIN_FONT_COLOR     0X736C      /* 0XAD53, 窗体内嵌字颜色 */
#define SYSSET_EDIT_BACK_COLOR      0XE79E      /* 显示内容区背景色 */

/****************************************************************************************************/


/**
 * @brief       时间/闹钟设置
 * @param       x,y             : 窗口坐标(窗口尺寸已经固定了的)
 * @param       hour,min        : 时, 分
 * @param       caption         : 窗口名字
 * @retval      0, OK;  其他, 取消/错误;
 */
uint8_t sysset_time_set(uint16_t x, uint16_t y, uint8_t *hour, uint8_t *min, uint8_t *caption)
{
    uint8_t rval = 0, res;
    uint8_t i;
    _window_obj *twin = 0;  /* 窗体 */
    _btn_obj *tbtn[6];      /* 总共六个按钮:0,时钟加按钮;1,时钟减按钮;2,分钟加按钮;3,分钟减按钮;4,确认按钮;5,取消按钮 */
    twin = window_creat(x, y, 150, 199, 0, 1 | 1 << 5, 16); /* 创建窗口 */
    tbtn[0] = btn_creat(x + 20, y + 42, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);            /* 创建按钮 */
    tbtn[1] = btn_creat(x + 20, y + 42 + 67, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);       /* 创建按钮 */
    tbtn[2] = btn_creat(x + 20 + 60, y + 42, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);       /* 创建按钮 */
    tbtn[3] = btn_creat(x + 20 + 60, y + 42 + 67, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);  /* 创建按钮 */
    tbtn[4] = btn_creat(x + 10, y + 42 + 117, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);      /* 创建按钮 */
    tbtn[5] = btn_creat(x + 10 + 70, y + 42 + 117, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02); /* 创建按钮 */

    for (i = 0; i < 6; i++)
    {
        if (tbtn[i] == NULL)
        {
            rval = 1;
            break;
        }

        if (i < 4)   /* 加减按键 */
        {
            tbtn[i]->bcfucolor = BLACK;     /* 松开时为黑色 */
            tbtn[i]->bcfdcolor = WHITE;     /* 按下时为白色 */
            tbtn[i]->bkctbl[0] = 0X453A;    /* 边框颜色 */
            tbtn[i]->bkctbl[1] = 0X5DDC;    /* 第一行的颜色 */
            tbtn[i]->bkctbl[2] = 0X5DDC;    /* 上半部分颜色 */
            tbtn[i]->bkctbl[3] = 0X453A;    /* 下半部分颜色 */
        }
        else     /* 确认和取消按键 */
        {
            tbtn[i]->bkctbl[0] = 0X8452;    /* 边框颜色 */
            tbtn[i]->bkctbl[1] = 0XAD97;    /* 第一行的颜色 */
            tbtn[i]->bkctbl[2] = 0XAD97;    /* 上半部分颜色 */
            tbtn[i]->bkctbl[3] = 0X8452;    /* 下半部分颜色 */
        }

        if (i == 0 || i == 2)tbtn[i]->caption = "＋";

        if (i == 1 || i == 3)tbtn[i]->caption = "－";

        if (i == 4)tbtn[i]->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];

        if (i == 5)tbtn[i]->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];
    }

    if (twin == NULL)rval = 1;
    else
    {
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
    }

    if (rval == 0)   /* 无错误 */
    {
        window_draw(twin);  /* 画出窗体 */

        for (i = 0; i < 6; i++)btn_draw(tbtn[i]);   /* 画按钮 */

        gui_fill_rectangle(x + 20 + 1, y + 42 + 41, SYSSET_EDIT_WIDTH - 2, SYSSET_EDIT_HEIGHT, SYSSET_EDIT_BACK_COLOR);         /* 填充时钟背景 */
        gui_fill_rectangle(x + 20 + 60 + 1, y + 42 + 41, SYSSET_EDIT_WIDTH - 2, SYSSET_EDIT_HEIGHT, SYSSET_EDIT_BACK_COLOR);    /* 填充分钟背景 */
        app_show_nummid(x + 20, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *hour, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
        app_show_nummid(x + 20 + 60, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *min, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            if (system_task_return)
            {
                rval = 1;   /* 取消设置 */
                break;      /* 返回 */
            }

            for (i = 0; i < 6; i++)
            {
                res = btn_check(tbtn[i], &in_obj); /* 确认按钮检测 */

                if (res)
                {
                    if ((tbtn[i]->sta & 0X80) == 0)   /* 有有效操作 */
                    {
                        switch (i)
                        {
                            case 0:/* 时钟增加按钮按下了 */
                                (*hour)++;

                                if (*hour > 23)*hour = 0;

                                break;

                            case 1:/* 时钟减少按钮按下了 */
                                if (*hour)(*hour)--;
                                else *hour = 23;

                                break;

                            case 2:/* 分钟增加按钮按下了 */
                                (*min)++;

                                if (*min > 59)(*min) = 0;

                                break;

                            case 3:/* 分钟减少按钮按下了 */
                                if (*min)(*min)--;
                                else *min = 59;

                                break;

                            case 4:/* 确认按钮按下 */
                                rval = 0XFF;
                                break;

                            case 5:/* 取消按钮按下 */
                                rval = 1;
                                break;
                        }
                    }

                    app_show_nummid(x + 20, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *hour, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
                    app_show_nummid(x + 20 + 60, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *min, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);

                }
            }

        }
    }

    window_delete(twin);    /* 删除窗口 */

    for (i = 0; i < 6; i++)btn_delete(tbtn[i]); /* 删除按钮 */

    system_task_return = 0; /* 清除退出标志 */

    if (rval == 0XFF)return 0;

    return rval;
}

/**
 * @brief       判断是否是闰年函数
 *  @note       月份   1  2  3  4  5  6  7  8  9  10 11 12
 *              闰年   31 29 31 30 31 30 31 31 30 31 30 31
 *              非闰年 31 28 31 30 31 30 31 31 30 31 30 31
 * @param       year            : 年份
 * @retval      0, 不是闰年;  1, 是闰年;
 */
uint8_t sysset_is_leap_year(uint16_t year)
{
    if (year % 4 == 0)   /* 必须能被4整除 */
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0)return 1; /* 如果以00结尾,还要能被400整除 */
            else return 0;
        }
        else return 1;
    }
    else return 0;
}

/**
 * @brief       日期设置
 * @param       x,y             : 窗口坐标(窗口尺寸已经固定了的)
 * @param       year,month,date : 年,月,日
 * @param       caption         : 窗口名字
 * @retval      0, OK;  其他, 取消/错误;
 */
uint8_t sysset_date_set(uint16_t x, uint16_t y, uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *caption)
{
    uint8_t rval = 0, res;
    uint8_t i;
    uint8_t maxdate = 31;   /* 2月份最大的天数 */
    _window_obj *twin = 0;  /* 窗体 */
    _btn_obj *tbtn[8];      /* 总共八个按钮:0,年份加按钮;1,年份减按钮;2,月份加按钮;3月份减按钮;4,日期加按钮;5,日期减按钮;6,确认按钮;7,取消按钮 */
    twin = window_creat(x, y, 190, 199, 0, 1 | 1 << 5, 16); /* 创建窗口 */
    tbtn[0] = btn_creat(x + 10, y + 42, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);            /* 创建按钮 */
    tbtn[1] = btn_creat(x + 10, y + 42 + 67, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);       /* 创建按钮 */
    tbtn[2] = btn_creat(x + 10 + 60, y + 42, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);       /* 创建按钮 */
    tbtn[3] = btn_creat(x + 10 + 60, y + 42 + 67, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);  /* 创建按钮 */
    tbtn[4] = btn_creat(x + 10 + 120, y + 42, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02);      /* 创建按钮 */
    tbtn[5] = btn_creat(x + 10 + 120, y + 42 + 67, SYSSET_BTN1_WIDTH, SYSSET_BTN1_HEIGHT, 0, 0x02); /* 创建按钮 */

    tbtn[6] = btn_creat(x + 20, y + 42 + 117, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);      /* 创建按钮 */
    tbtn[7] = btn_creat(x + 20 + 90, y + 42 + 117, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02); /* 创建按钮 */

    for (i = 0; i < 8; i++)
    {
        if (tbtn[i] == NULL)
        {
            rval = 1;
            break;
        }

        if (i < 6)  /* 加减按键 */
        {
            tbtn[i]->bcfucolor = BLACK;     /* 松开时为黑色 */
            tbtn[i]->bcfdcolor = WHITE;     /* 按下时为白色 */
            tbtn[i]->bkctbl[0] = 0X453A;    /* 边框颜色 */
            tbtn[i]->bkctbl[1] = 0X5DDC;    /* 第一行的颜色 */
            tbtn[i]->bkctbl[2] = 0X5DDC;    /* 上半部分颜色 */
            tbtn[i]->bkctbl[3] = 0X453A;    /* 下半部分颜色 */
        }
        else    /* 确认和取消按键 */
        {
            tbtn[i]->bkctbl[0] = 0X8452;    /* 边框颜色 */
            tbtn[i]->bkctbl[1] = 0XAD97;    /* 第一行的颜色 */
            tbtn[i]->bkctbl[2] = 0XAD97;    /* 上半部分颜色 */
            tbtn[i]->bkctbl[3] = 0X8452;    /* 下半部分颜色 */
        }

        if (i == 0 || i == 2 || i == 4)tbtn[i]->caption = "＋";

        if (i == 1 || i == 3 || i == 5)tbtn[i]->caption = "－";

        if (i == 6)tbtn[i]->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];;

        if (i == 7)tbtn[i]->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];;
    }

    if (twin == NULL)rval = 1;
    else
    {
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
    }

    if (rval == 0)   /* 无错误 */
    {
        window_draw(twin);  /* 画出窗体 */

        for (i = 0; i < 8; i++)btn_draw(tbtn[i]);   /* 画按钮 */

        gui_fill_rectangle(x + 10 + 1, y + 42 + 41, SYSSET_EDIT_WIDTH - 2, SYSSET_EDIT_HEIGHT, SYSSET_EDIT_BACK_COLOR);         /* 填充年份背景 */
        gui_fill_rectangle(x + 10 + 60 + 1, y + 42 + 41, SYSSET_EDIT_WIDTH - 2, SYSSET_EDIT_HEIGHT, SYSSET_EDIT_BACK_COLOR);    /* 填充月份背景 */
        gui_fill_rectangle(x + 10 + 120 + 1, y + 42 + 41, SYSSET_EDIT_WIDTH - 2, SYSSET_EDIT_HEIGHT, SYSSET_EDIT_BACK_COLOR);   /* 填充日期背景 */

        app_show_nummid(x + 10, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *year, 4, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
        app_show_nummid(x + 10 + 60, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *month, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
        app_show_nummid(x + 10 + 120, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *date, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            if (system_task_return)
            {
                rval = 1;   /* 取消设置 */
                break;      /* 按键返回 */
            }

            for (i = 0; i < 8; i++)
            {
                res = btn_check(tbtn[i], &in_obj); /* 确认按钮检测 */

                if (res)
                {
                    if ((tbtn[i]->sta & 0X80) == 0)   /* 有有效操作 */
                    {
                        if (*month == 2)
                        {
                            if (sysset_is_leap_year(*year))maxdate = 29; /* 是闰年的2月份 */
                            else maxdate = 28;
                        }
                        else maxdate = 31;

                        switch (i)
                        {
                            case 0:/* 年份增加按钮按下了 */
                                (*year)++;

                                if (*year > 2100)*year = 2000;

                                break;

                            case 1:/* 年份减少按钮按下了 */
                                if (*year > 2000)(*year)--;
                                else *year = 2100;

                                break;

                            case 2:/* 月份增加按钮按下了 */
                                (*month)++;

                                if (*month > 12)(*month) = 1;

                                break;

                            case 3:/* 月份减少按钮按下了 */
                                if (*month > 1)(*month)--;
                                else *month = 12;

                                break;

                            case 4:/* 日期增加按钮按下了 */
                                (*date)++;

                                if (*date > maxdate)(*date) = 1;

                                break;

                            case 5:/* 日期减少按钮按下了 */
                                if (*date > 1)(*date)--;
                                else *date = maxdate;

                                break;

                            case 6:/* 确认按钮按下 */
                                rval = 0XFF;
                                break;

                            case 7:/* 取消按钮按下 */
                                rval = 1;
                                break;
                        }
                    }

                    app_show_nummid(x + 10, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *year, 4, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
                    app_show_nummid(x + 10 + 60, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *month, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
                    app_show_nummid(x + 10 + 120, y + 42 + 41, SYSSET_EDIT_WIDTH, SYSSET_EDIT_HEIGHT, *date, 2, 16, BLACK, SYSSET_EDIT_BACK_COLOR);
                }
            }

        }
    }

    window_delete(twin);        /* 删除窗口 */

    for (i = 0; i < 8; i++)btn_delete(tbtn[i]); /* 删除按钮 */

    system_task_return = 0;     /* 清除退出标志 */

    if (rval == 0XFF)return 0;

    return rval;
}

/**
 * @brief       ES8388音量与3D显示
 * @param       x,y             : 坐标
 * @param       mask            : 显示的类型
 *                                [0]:音量;0~100%
 *                                [1]:3D效果值:0~15
 *                                [7]:更新单位
 * @param       val             : 具体的值
 * @retval      无
 */
void sysset_es8388_vol3d_show(uint16_t x, uint16_t y, uint8_t mask, uint8_t val)
{
    if (mask & (1 << 7))
    {
        gui_show_string("%", x + 200 - 10 - 6, y + 32 + 12, 20, 12, 12, SYSSET_INWIN_FONT_COLOR);	//显示百分比
    }

    gui_phy.back_color = APP_WIN_BACK_COLOR; //设置背景色

    if (mask & (1 << 0)) //显示音量
    {
        gui_fill_rectangle(x + 200 - 10 - 24, y + 32 + 12, 18, 12, APP_WIN_BACK_COLOR); //清空之前的显示

        if (val >= 100)
        {
            val = 100;
            gui_show_num(x + 200 - 10 - 24, y + 32 + 12, 3, SYSSET_INWIN_FONT_COLOR, 12, val, 0); //显示音量百分比
        }
        else
        {
            gui_show_num(x + 200 - 10 - 18, y + 32 + 12, 2, SYSSET_INWIN_FONT_COLOR, 12, val, 0); //显示音量百分比
        }
    }

    if (mask & (1 << 1)) //显示3d设置值
    {
        gui_fill_rectangle(x + 200 - 10 - 12, y + 32 + 12 + 50, 12, 12, APP_WIN_BACK_COLOR); //清空之前的显示
        gui_show_num(x + 200 - 10 - 12, y + 32 + 12 + 50, 2, SYSSET_INWIN_FONT_COLOR, 12, val, 0); //显示数值
    }
}

///**
// * @brief       ES8388音量和3D设置
// *  @note       宽度:200,高度:212.
// * @param       x,y             : 坐标
// * @param       caption         : 名称
// * @param       wmset           : ES8388的设置
// * @retval      0,OK; 其他, 取消/错误.
// */
//uint8_t sysset_es8388_vol3d_set(uint16_t x, uint16_t y, _es8388_obj *wmset, uint8_t *caption)
//{
//    uint8_t rval = 0, res;
//    uint8_t i = 0;
//    _window_obj *twin = 0;  /* 窗体 */
//    _btn_obj *rbtn = 0;     /* 取消按钮 */
//    _btn_obj *okbtn = 0;    /* 确定按钮 */
//    _progressbar_obj *es8388prgb[2];    /* 进度条 */
//    uint8_t voltemp[2];
//    
//    twin = window_creat(x, y, 200, 212, 0, 1 | 1 << 5, 16); /* 创建窗口 */
//    es8388prgb[0] = progressbar_creat(x + 10, y + 32 + 30 + 50 * 0, 180, 20, 0X60); /* 创建进度条,显示进度柱 */
//    es8388prgb[1] = progressbar_creat(x + 10, y + 32 + 30 + 50 * 1, 180, 20, 0X60); /* 创建进度条,显示进度柱 */

//    if (es8388prgb[0] == NULL || es8388prgb[1] == NULL)rval = 1;
//    else
//    {
//        es8388prgb[0]->totallen = 30;		/* 主音量:0~30 */
//        es8388prgb[1]->totallen = 15;		/* 3d范围:0~15 */
//    }

//    okbtn = btn_creat(x + 30, y + 32 + 50 * 2 + 35, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);            /* 创建按钮 */
//    rbtn = btn_creat(x + 30 + 60 + 20, y + 32 + 50 * 2 + 35, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);   /* 创建按钮 */

//    if (twin == NULL || rbtn == NULL || okbtn == NULL || rval)rval = 1;
//    else
//    {
//        /* 窗口的名字和背景色 */
//        twin->caption = caption;
//        twin->windowbkc = APP_WIN_BACK_COLOR;
//        
//        /* 返回按钮的颜色 */
//        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
//        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
//        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
//        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
//        okbtn->bkctbl[0] = 0X8452;  /* 边框颜色 */
//        okbtn->bkctbl[1] = 0XAD97;  /* 第一行的颜色 */
//        okbtn->bkctbl[2] = 0XAD97;  /* 上半部分颜色 */
//        okbtn->bkctbl[3] = 0X8452;  /* 下半部分颜色 */

//        rbtn->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];    /* 标题为取消 */
//        okbtn->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];       /* 标题为返回 */

//        es8388prgb[0]->curpos = wmset->mvol; /* mvol:0~63 */
//        es8388prgb[1]->curpos = wmset->d3;
//        window_draw(twin);  /* 画出窗体 */
//        btn_draw(rbtn);     /* 画按钮 */
//        btn_draw(okbtn);    /* 画按钮 */
//        
//        sysset_es8388_vol3d_show(x, y, 1 << 7, 0);  /* 显示单位 */

//        for (i = 0; i < 2; i++)
//        {
//            voltemp[i] = es8388prgb[i]->curpos; /* 保存开始的状态 */
//            es8388prgb[i]->btncolor = BLUE;     /* 蓝色 */
//            es8388prgb[i]->prgbarlen = 5;       /* 滚条长度设置为5 */
//            progressbar_draw_progressbar(es8388prgb[i]);    /* 显示进度条 */
//            gui_show_string((uint8_t *)sysset_es8388vol3d_tbl[i][gui_phy.language], x + 10, y + 32 + 12 + 50 * i, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);    /* 显示各項的名字 */

//            if (i == 0)sysset_es8388_vol3d_show(x, y, 1 << 0, (es8388prgb[i]->curpos * 100) / es8388prgb[i]->totallen); /* 显示音量百分比 */
//            else sysset_es8388_vol3d_show(x, y, 1 << i, voltemp[i]); /* 显示所有数据 */
//        }
//    }

//    while (rval == 0)
//    {
//        tp_dev.scan(0);
//        in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
//        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

//        if (system_task_return)
//        {
//            rval = 1;   /* 退出,不保存 */
//            break;      /* 按键返回 */
//        }

//        res = btn_check(rbtn, &in_obj);         /* 取消按钮检测 */

//        if (res && ((rbtn->sta & 0X80) == 0))   /* 有有效操作 */
//        {
//            rval = 1;   /* 退出,不保存 */
//            break;      /* 退出 */
//        }

//        res = btn_check(okbtn, &in_obj);        /* 确定按钮检测 */

//        if (res && ((okbtn->sta & 0X80) == 0))  /* 有有效操作 */
//        {
//            rval = 0XFF;
//            break;      /* 退出 */
//        }

//        for (i = 0; i < 2; i++)
//        {
//            res = progressbar_check(es8388prgb[i], &in_obj);

//            if (res && (voltemp[i] != es8388prgb[i]->curpos))   /* 进度条改动了 */
//            {
//                voltemp[i] = es8388prgb[i]->curpos;             /* 保存最新的结果 */

//                if (i == 0)
//                {
//                    wmset->mvol = es8388prgb[i]->curpos;        /* 保存最新的结果 */
//                    app_es8388_volset(wmset->mvol);
//                    sysset_es8388_vol3d_show(x, y, 1 << 0, (es8388prgb[i]->curpos * 100) / es8388prgb[i]->totallen); /* 显示音量百分比 */
//                }
//                else
//                {
//                    wmset->d3 = es8388prgb[1]->curpos;
//                    es8388_3d_set(es8388prgb[1]->curpos);       /* 设置3D效果 */
//                    sysset_es8388_vol3d_show(x, y, 1 << 1, voltemp[1]); /* 更新数据 */
//                }
//            }
//        }
//    }

//    window_delete(twin);                /* 删除窗口 */
//    btn_delete(rbtn);                   /* 删除按钮 */
//    progressbar_delete(es8388prgb[0]);  /* 删除进度条 */
//    progressbar_delete(es8388prgb[1]);  /* 删除进度条 */
//    system_task_return = 0;             /* 清除退出标志 */

//    if (rval == 0XFF)return 0;

//    return rval;
//}

/**
 * @brief       ES8388 EQ设置显示
 * @param       x,y:sysset_wm8978_voleq_set的设置坐标
 * @param       eqx:当前设置的eq编号,0~4
 * @param       mask:
                [0]:中心频率标志;0~3 
                [1]:频率增益:0~24
                [7]:更新单位
 * @retval      无
 */
void sysset_wm8978_eq_show(uint16_t x,uint16_t y,uint8_t mask,uint8_t eqx,uint8_t val)
{ 
    signed char tdbval;
    uint8_t bits = 0;
    
    if (mask & (1 << 7))
    {
        gui_show_string("Hz",x + 200 - 10 - 12,y + 32 + 12,20,12,12,SYSSET_INWIN_FONT_COLOR);       /* 显示Hz */
        gui_show_string("dB",x + 200 - 10 - 12,y + 32 + 12 + 50,20,12,12,SYSSET_INWIN_FONT_COLOR);  /* 显示dB */
    }
    
    gui_phy.back_color = APP_WIN_BACK_COLOR;    /* 设置背景色 */
    
    if (mask & (1 << 0))                        /* 显示当前设置的中心频率 */
    { 
        gui_fill_rectangle(x + 200 - 10 - 12 - 6 * 5,y + 32 + 12,6 * 5,12,APP_WIN_BACK_COLOR);      /* 清空之前的显示 */
        
        switch(eqx)
        {
            case 0:
                if (val == 0) bits = 2;
                else bits = 3;
                break;
            case 1:
                bits = 3;
                break;
            case 2: 
                if (val < 2) bits = 3;
                else bits = 4;
                break;
            case 3:
                bits = 4;
                break;
            case 4:
                if (val == 3) bits = 5;
                else bits = 4;
                break;
        }
        gui_show_string((uint8_t*)sysset_es8388freq_tbl[eqx][val],x + 200 - 10 - 12 - 6 * bits,y + 32 + 12,6 * bits,12,12,SYSSET_INWIN_FONT_COLOR); /* 显示频率值 */
    }
    
    if (mask & (1 << 1))    /* 显示频率提升 */
    {  
        tdbval = (signed char)val - 12;
        app_show_float(x + 200 - 10 - 12,y + 32 + 12 + 50,tdbval,0,3,12,SYSSET_INWIN_FONT_COLOR,APP_WIN_BACK_COLOR);    /* 显示数值 */
    }  
}

/**
 * @brief       ES8388 EQ设置显示
 * @param       x,y:显示的坐标(宽度:200,高度:212)
 * @param       caption:名称
 * @param       wmset:WM8978的设置
 * @param       eqx:选择的频段 
 * @retval      返回值:0,ok;其他,取消或者错误.
 */
//uint8_t sysset_es8388_eq_set(uint16_t x,uint16_t y,_es8388_obj *wmset,uint16_t eqx,uint8_t* caption)
//{
//    uint8_t rval = 0,res;
//    uint8_t i = 0;
//    _window_obj* twin = 0;  /* 窗体 */
//    _btn_obj * rbtn = 0;    /* 取消按钮 */
//    _btn_obj * okbtn = 0;   /* 确定按钮 */
//    _progressbar_obj * es8388prgb[2];   /* 进度条 */
//    uint8_t voltemp[2];  
//    twin = window_creat(x,y,200,212,0,1 | 1 << 5,16);   /* 创建窗口 */
//    es8388prgb[0] = progressbar_creat(x + 10,y + 32 + 30 + 50 * 0,180,20,0X40); /* 创建进度条,不显示进度柱 */
//    es8388prgb[1] = progressbar_creat(x + 10,y + 32 + 30 + 50 * 1,180,20,0X60); /* 创建进度条,显示进度柱 */
//    
//    if (es8388prgb[0] == NULL || es8388prgb[1] == NULL) rval = 1;
//    else
//    {
//        es8388prgb[0]->totallen = 3;    /* 中心频率标志:0~3 */
//        es8388prgb[1]->totallen = 24;   /* 频率增益:0~24 */
//    }
//    
//    okbtn = btn_creat(x + 30,y + 32 + 50 * 2 + 35,SYSSET_BTN2_WIDTH,SYSSET_BTN2_HEIGHT,0,0x02);             /* 创建按钮 */
//    rbtn = btn_creat(x + 30 + 60 + 20,y + 32 + 50 * 2 + 35,SYSSET_BTN2_WIDTH,SYSSET_BTN2_HEIGHT,0,0x02);    /* 创建按钮 */
//    if (twin == NULL || rbtn == NULL || okbtn == NULL || rval)  rval = 1;
//    else
//    {
//        /* 窗口的名字和背景色 */
//        twin->caption = caption;
//        twin->windowbkc = APP_WIN_BACK_COLOR;
//        /* 返回按钮的颜色 */
//        rbtn->bkctbl[0] = 0X8452;       /* 边框颜色 */
//        rbtn->bkctbl[1] = 0XAD97;       /* 第一行的颜色 */
//        rbtn->bkctbl[2] = 0XAD97;       /* 上半部分颜色 */
//        rbtn->bkctbl[3] = 0X8452;       /* 下半部分颜色 */
//        okbtn->bkctbl[0] = 0X8452;      /* 边框颜色 */
//        okbtn->bkctbl[1] = 0XAD97;      /* 第一行的颜色 */
//        okbtn->bkctbl[2] = 0XAD97;      /* 上半部分颜色 */
//        okbtn->bkctbl[3] = 0X8452;      /* 下半部分颜色 */

//        rbtn->caption = (uint8_t*)GUI_CANCEL_CAPTION_TBL[gui_phy.language];     /* 标题为取消 */
//        okbtn->caption = (uint8_t*)GUI_OK_CAPTION_TBL[gui_phy.language];        /* 标题为返回 */
// 
//        es8388prgb[0]->curpos = wmset->cfreq[eqx];
//        es8388prgb[1]->curpos = wmset->freqval[eqx];
//        window_draw(twin);      /* 画出窗体 */
//        btn_draw(rbtn);         /* 画按钮 */
//        btn_draw(okbtn);        /* 画按钮 */
//        sysset_wm8978_eq_show(x,y,1 << 7,eqx,0);    /* 显示单位 */
//        
//        for (i = 0;i < 2;i ++)
//        {
//            voltemp[i] = es8388prgb[i]->curpos; /* 保存开始的状态 */
//            es8388prgb[i]->btncolor = BLUE;     /* 蓝色 */
//            es8388prgb[i]->prgbarlen = 5;       /* 滚条长度设置为5 */
//            progressbar_draw_progressbar(es8388prgb[i]);    /* 显示进度条 */
//            gui_show_string((uint8_t*)sysset_es8388eq_tbl[i][gui_phy.language],x + 10,y + 32 + 12 + 50 * i,190,12,12,SYSSET_INWIN_FONT_COLOR);  /* 显示各項的名字 */
//            sysset_wm8978_eq_show(x,y,1 << i,eqx,voltemp[i]);//显示所有数据 */
//        }
//    }
//    
//    while (rval == 0)
//    {
//        tp_dev.scan(0);    
//        in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);  /* 得到按键键值 */
//        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */
//        
//        if(system_task_return)
//        {
//            rval = 1;                           /* 退出,不保存 */
//            break;                              /* 按键返回 */
//        }
//        res = btn_check(rbtn,&in_obj);          /* 取消按钮检测 */
//        
//        if (res && ((rbtn->sta & 0X80) == 0))   /* 有有效操作 */
//        {
//            rval = 1;                           /* 退出,不保存 */
//            break;                              /* 退出 */
//        }
//        res = btn_check(okbtn,&in_obj);         /* 确定按钮检测 */
//        
//        if (res && ((okbtn->sta & 0X80) == 0))  /* 有有效操作 */
//        { 
//            rval = 0XFF;
//            break;                              /* 退出 */
//        }
//        
//        for (i = 0;i < 2;i++)
//        {
//            res = progressbar_check(es8388prgb[i],&in_obj);
//            
//            if (res && (voltemp[i] != es8388prgb[i]->curpos))   /* 进度条改动了 */
//            {
//                voltemp[i] = es8388prgb[i]->curpos;             /* 保存最新的结果 */
//                
//                if (i == 0)
//                {
//                    wmset->cfreq[eqx] = es8388prgb[0]->curpos;  /* 更新cfreq值 */
//                }
//                else
//                {
//                    wmset->freqval[eqx] = es8388prgb[1]->curpos; 
//                }
//                
//                app_es8388_eqset(wmset,eqx);
//                sysset_wm8978_eq_show(x,y,1 << i,eqx,voltemp[i]);   /* 更新数据 */
//            }
//        }
//    }
//    
//    window_delete(twin);                /* 删除窗口 */
//    btn_delete(rbtn);                   /* 删除按钮 */
//    progressbar_delete(es8388prgb[0]);  /* 删除进度条 */
//    progressbar_delete(es8388prgb[1]);  /* 删除进度条 */
//    system_task_return = 0;             /* 清除退出标志 */
//    
//    if (rval == 0XFF) return 0;
//    return rval;
//}

/**
 * @brief       背光亮度设置
 * @param       x,y             : 窗口坐标(宽度180,高度180)
 * @param       caption         : 窗口名字
 * @param       bkval           : 背光亮度
 * @retval      0, OK;  其他, 取消/错误;
 */ 
uint8_t sysset_bklight_set(uint16_t x, uint16_t y, uint8_t *caption, uint8_t *bkval)
{
    uint8_t rval = 0, res;
    _window_obj *twin = 0;      /* 窗体 */
    _btn_obj *rbtn = 0;         /* 取消按钮 */
    _btn_obj *okbtn = 0;        /* 确定按钮 */
    _progressbar_obj *bkprgb;   /* 背光设置滚动条 */
    uint8_t *buf;

    twin = window_creat(x, y, 180, 150, 0, 1 | 1 << 5, 16);         /* 创建窗口 */
    bkprgb = progressbar_creat(x + 15, y + 32 + 35, 150, 20, 0X20); /* 创建进度条,不显示浮标 */
    buf = gui_memin_malloc(100);

    if (bkprgb == NULL || buf == NULL)rval = 1;

    okbtn = btn_creat(x + 20, y + 110, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);             /* 创建按钮 */
    rbtn = btn_creat(x + 20 + 60 + 20, y + 110, SYSSET_BTN2_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);    /* 创建按钮 */

    if (twin == NULL || rbtn == NULL || okbtn == NULL || rval)rval = 1;
    else
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        /* 返回按钮的颜色 */
        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
        okbtn->bkctbl[0] = 0X8452;  /* 边框颜色 */
        okbtn->bkctbl[1] = 0XAD97;  /* 第一行的颜色 */
        okbtn->bkctbl[2] = 0XAD97;  /* 上半部分颜色 */
        okbtn->bkctbl[3] = 0X8452;  /* 下半部分颜色 */

        rbtn->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];    /* 标题为取消 */
        okbtn->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];       /* 标题为返回 */
        bkprgb->curpos = *bkval;    /* 取得当前的设置百分比 */
        bkprgb->totallen = 100;     /* 总长度为100 */

        window_draw(twin);          /* 画出窗体 */
        btn_draw(rbtn);             /* 画按钮 */
        btn_draw(okbtn);            /* 画按钮 */
        progressbar_draw_progressbar(bkprgb);       /* 显示进度条 */
        gui_phy.back_color = APP_WIN_BACK_COLOR;    /* 设置背景色 */

        if (bkprgb->curpos)sprintf((char *)buf, "%d%%", bkprgb->curpos); /* 百分比 */
        else sprintf((char *)buf, "auto");          /* auto */

        gui_show_string(buf, x + 15, y + 32 + 22, 24, 12, 12, SYSSET_INWIN_FONT_COLOR); /* 显示百分比/auto */
    }

    while (rval == 0)
    {
        tp_dev.scan(0);
        in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

        if (system_task_return)
        {
            rval = 1;   /* 取消设置 */
            break;      /* 按键返回 */
        }

        res = btn_check(rbtn, &in_obj); /* 取消按钮检测 */

        if (res && ((rbtn->sta & 0X80) == 0))   /* 有有效操作 */
        {
            rval = 1;
            break;/* 退出 */
        }

        res = btn_check(okbtn, &in_obj);    /* 确定按钮检测 */

        if (res && ((okbtn->sta & 0X80) == 0))  /* 有有效操作 */
        {
            rval = 0XFF;
            break;  /* 退出 */
        }

        res = progressbar_check(bkprgb, &in_obj);

        if (res && (*bkval != bkprgb->curpos))  /* 进度条改动了 */
        {
            *bkval = bkprgb->curpos;            /* 保存最新的结果 */
            gui_fill_rectangle(x + 15, y + 32 + 22, 24, 12, APP_WIN_BACK_COLOR);    /* 填充底色 */

            if (bkprgb->curpos)
            {
                sprintf((char *)buf, "%d%%", bkprgb->curpos); /* 百分比 */

                if (STM32_FMC_16)
                {
                    LCD_BLPWM_VAL = *bkval;     /* 设置背光亮度 */
                }
                else
                { 
                    LT768_PWM1_Init(1, 0, 100, 100, *bkval);
                }                          
            }
            else
            {
                sprintf((char *)buf, "auto");   /* auto */
                app_lcd_auto_bklight();         /* 自动背光控制 */
            }

            gui_show_string(buf, x + 15, y + 32 + 22, 24, 12, 12, SYSSET_INWIN_FONT_COLOR); /* 显示百分比/auto */
        }
    }

    gui_memin_free(buf);            /* 释放内存 */
    window_delete(twin);            /* 删除窗口 */
    btn_delete(rbtn);               /* 删除按钮 */
    progressbar_delete(bkprgb);     /* 删除进度条 */
    system_task_return = 0;         /* 清除退出标志 */

    if (rval == 0XFF)return 0;

    return rval;
}

/* 用于外部设置坐标 */
uint16_t cpymsg_x;
uint16_t cpymsg_y;

/**
 * @brief       系统启动的时候,用于显示更新进度
 * @param       pname           : 更新文件名字
 * @param       pct             : 百分比
 * @param       mode            : 模式
 *                                [0]:更新文件名
 *                                [1]:更新百分比pct
 *                                [2]:更新文件夹
 *                                [3~7]:保留
 * @retval      0, OK;  1, 结束复制;
 */
uint8_t sysset_system_update_cpymsg(uint8_t *pname, uint8_t pct, uint8_t mode)
{
    uint16_t filled;

    if (system_task_return)return 1; /* 返回 */

    if (mode & 0X01)   /* 显示文件名 */
    {
        gui_phy.back_color = APP_WIN_BACK_COLOR;
        gui_fill_rectangle(cpymsg_x + 10 + 9 * 6, cpymsg_y + 32 + 15, 126, 12, APP_WIN_BACK_COLOR);     /* 填充底色  0XE73C */
        gui_show_string(pname, cpymsg_x + 10 + 9 * 6, cpymsg_y + 32 + 15, 126, 12, 12, BLACK);          /* 显示新的文件名 */
        gui_show_string((uint8_t *)sysset_system_update_cpymsg_tbl[1][gui_phy.language], cpymsg_x + 10, cpymsg_y + 32 + 15 + 21, 66, 12, 12, BLACK);
    }

    if (mode & 0X04)   /* 显示文件夹名 */
    {
        gui_phy.back_color = APP_WIN_BACK_COLOR;
        gui_fill_rectangle(cpymsg_x + 10 + 11 * 6, cpymsg_y + 32 + 15 + 21, 114, 12, APP_WIN_BACK_COLOR);   /* 填充底色  0XE73C */
        gui_show_string(pname, cpymsg_x + 10 + 11 * 6, cpymsg_y + 32 + 15 + 21, 114, 12, 12, BLACK);        /* 显示新的文件夹名 */
    }

    if (mode & 0X02)   /* 更新百分比 */
    {
        if (pct > 100)pct = 100;

        filled = pct;
        filled = (filled * 9) / 5;
        gui_fill_rectangle(cpymsg_x + 10, cpymsg_y + 32 + 15 + 21 * 2, filled, 16, 0X071F);                 /* 填充占用了的百分比部分 */
        gui_fill_rectangle(cpymsg_x + 10 + filled, cpymsg_y + 32 + 15 + 21 * 2, 180 - filled, 16, WHITE);   /* 填充未占用的百分比部分 */
        gui_show_num(cpymsg_x + 10 + 72, cpymsg_y + 32 + 15 + 21 * 2 + 2, 3, BLACK, 12, pct, 1);            /* 显示更新进度 */
        gui_show_string("%", cpymsg_x + 10 + 72 + 18, cpymsg_y + 32 + 15 + 21 * 2 + 2, 6, 12, 12, BLACK);   /* 显示百分号 */
    }

    delay_ms(1000 / OS_TICKS_PER_SEC); /* 延时一个时钟节拍 */
    return 0;
}

/**
 * @brief       更新系统文件夹
 * @param       caption         : 窗口名字
 * @param       sx,sy           : 起始坐标
 * @retval      无
 */
void sysset_system_update(uint8_t *caption, uint16_t sx, uint16_t sy)
{
    _window_obj *twin = 0;  /* 窗体 */
    uint8_t res;
    uint8_t *p;
    res = app_system_file_check("0");   /* 先检查SD卡 */

    if (res)res = app_system_file_check("2");   /* SD卡有问题，再检查U盘 */

    if (res)
    {
        p = (uint8_t *)sysset_system_update_err_tbl[1][gui_phy.language];   /* 系统文件丢失 */
        window_msg_box(sx, sy + 20, 200, 80, p, APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(1500);
        system_task_return = 0; /* 取消返回 */
        return;
    }

    twin = window_creat(sx, sy, 200, 120, 0, 1, 16); /* 创建窗口 */

    if (twin != NULL)
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        window_draw(twin);  /* 画出窗体 */
        gui_phy.back_color = APP_WIN_BACK_COLOR;
        gui_show_string((uint8_t *)sysset_system_update_cpymsg_tbl[0][gui_phy.language], sx + 10, sy + 32 + 15, 54, 12, 12, BLACK);         /* 正在复制: */
        gui_show_string((uint8_t *)sysset_system_update_cpymsg_tbl[1][gui_phy.language], sx + 10, sy + 32 + 15 + 21, 66, 12, 12, BLACK);    /* 当前文件夹: */
        gui_draw_rectangle(sx + 10 - 1, sy + 32 + 15 + 21 * 2 - 1, 182, 18, 0X4A49);    /* 画边框 */
        gui_fill_rectangle(sx + 10, sy + 32 + 15 + 21 * 2, 180, 16, WHITE);             /* 填充底色  0XE73C */
        gui_show_num(sx + 10 + 72, sy + 32 + 15 + 21 * 2 + 2, 3, BLACK, 12, 0, 1);      /* 显示更新进度 */
        gui_show_string("%", sx + 10 + 72 + 18, sy + 32 + 15 + 21 * 2 + 2, 6, 12, 12, BLACK);   /* 显示百分号 */
        cpymsg_x = sx;
        cpymsg_y = sy;

        res = app_system_update(sysset_system_update_cpymsg, "0:");             /* 先从SD卡更新 */

        if (res != 0XFF)   /* 更新有误 */
        {
            res = app_system_update(sysset_system_update_cpymsg, "2:");         /* 尝试U盘更新 */
        }

        p = (uint8_t *)sysset_system_update_err_tbl[0][gui_phy.language];       /* 更新成功提示 */

        if (res == 0XFF)
        {
            p = (uint8_t *)sysset_system_update_err_tbl[2][gui_phy.language];   /* 强制退出 */
        }
        else if (res)
        {
            p = (uint8_t *)sysset_system_update_err_tbl[1][gui_phy.language];   /* 系统文件丢失 */
        }
        
        window_msg_box(sx, sy + 20, 200, 80, p, caption, 12, 0, 0, 0);
        delay_ms(1500);
    }

    window_delete(twin);
    system_task_return = 0; /* 取消返回 */
}

/**
 * @brief       显示系统信息
 * @param       x,y             : 起始坐标(宽度:200, 高度: 312)
 * @param       caption         : 窗口名字
 * @retval      无
 */
void sysset_system_info(uint16_t x, uint16_t y, uint8_t *caption)
{
    _window_obj *twin = 0;  /* 窗体 */
    _btn_obj *rbtn = 0;     /* 返回按钮 */
    uint8_t rval = 0;
    uint8_t *msgbuf;
    uint8_t numstrbuf[6];
    uint8_t i;
    uint32_t dtsize, dfsize;
    msgbuf = mymalloc(SRAMIN, 31);  /* 申请31个字节 */
    twin = window_creat(x, y, 200, 312, 0, 1 | 1 << 5, 16); /* 创建窗口 */
    rbtn = btn_creat(x + 50, y + 32 + 10 + 20 * 11 + 10, SYSSET_BTN3_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02);   /* 创建按钮 */

    if (twin != NULL && rbtn != NULL)
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;

        /* 返回按钮的颜色 */
        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
        rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language];      /* 标题为返回 */

        window_draw(twin);          /* 画出窗体 */
        btn_draw(rbtn);             /* 画按键 */
        gui_draw_rectangle(x + 10 - 4, y + 32 + 14 - 4, 188, 20 * 11, 0X4A49);  /* 画边框 */
        gui_phy.back_color = APP_WIN_BACK_COLOR;
        rval = strlen((const char *)sysset_system_info_caption_tbl[0][gui_phy.language]);

        for (i = 0; i < 9; i++)
        {
            strcpy((char *)msgbuf, (const char *)sysset_system_info_caption_tbl[i][gui_phy.language]);

            if (i == 2 || i == 3 || i == 4 || i == 5)   /* SD卡/FLASH盘/SD NAND/U盘 容量 */
            {
                dtsize = 0;
                dfsize = 0;

                if (i == 2 || i == 4 || i == 5)
                {
                    if (i == 2) 
                    {
                        exfuns_get_free("0:", &dtsize, &dfsize);    /* 得到SD卡剩余容量和总容量 */ 
                    }
                    else if (i == 4)
                    {
                        exfuns_get_free("2:", &dtsize, &dfsize);    /* 得到SD NAND剩余容量和总容量 */ 
                    }
                    else
                    {
                        exfuns_get_free("3:",&dtsize,&dfsize);      /* 得到U盘剩余容量和总容量 */
                    }

                    dtsize >>= 10;
                    dfsize >>= 10;
                }
                else exfuns_get_free("1:", &dtsize, &dfsize);       /* 得到FLASH盘剩余容量和总容量 */

                gui_num2str(numstrbuf, dfsize);
                strcat((char *)msgbuf, (const char *)numstrbuf);    /* 增加空闲容量 */

                if (i == 2 || i == 4 || i == 5) strcat((char *)msgbuf, (const char *)"MB/");
                else strcat((char *)msgbuf, (const char *)"KB/");

                gui_num2str(numstrbuf, dtsize);
                strcat((char *)msgbuf, (const char *)numstrbuf);    /* 增加总容量 */
            }

            strcat((char *)msgbuf, (const char *)sysset_system_info_msg_tbl[i]);

            if (i == 6 || i == 7 || i == 8)
            {
                if (i == 6) app_get_version(numstrbuf, OS_VERSION / 100, 3);    /* OS版本 */
                else if (i == 7) app_get_version(numstrbuf, GUI_VERSION, 3);    /* GUI版本 */
                else app_get_version(numstrbuf, HARDWARE_VERSION, 2);           /* 硬件版本 */

                strcat((char *)msgbuf, (const char *)numstrbuf);
            }

            gui_show_string(msgbuf, x + 10, y + 32 + 14 + 20 * i, 180, 12, 12, BLACK); /* 显示信息 */
        }

        while (1)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            if (system_task_return)
            {
                break;  /* 按键返回 */
            }

            rval = btn_check(rbtn, &in_obj);        /* 返回按钮检测 */

            if (rval && (rbtn->sta & 0X80) == 0)break;  /* 有有效操作 */

        }
    }

    myfree(SRAMIN, msgbuf);
    window_delete(twin);
    btn_delete(rbtn);
    system_task_return = 0; /* 取消返回 */
}

/**
 * @brief       显示系统状态
 * @param       x,y             : 起始坐标(宽度:200, 高度: 242)
 * @param       caption         : 窗口名字
 * @retval      无
 */
void sysset_system_status(uint16_t x, uint16_t y, uint8_t *caption)
{
    uint8_t rval = 0, res;
    uint16_t i = 0;
    uint16_t temperate = 0;
    _window_obj *twin = 0;              /* 窗体 */
    _btn_obj *rbtn = 0;                 /* 返回按钮 */
    _progressbar_obj *cpuprgb = 0;      /* CPU使用率进度条 */
    _progressbar_obj *meminprgb = 0;    /* 内部内存使用进度条 */
    _progressbar_obj *memtcmprgb = 0;   /* 内部TCM内存使用进度条 */
    _progressbar_obj *memexprgb = 0;    /* 外部内存使用进度条 */
 
    adc3_init();                        /* 初始化ADC3(使能内部温度传感器) */
      
    twin = window_creat(x, y, 200, 242, 0, 1 | 1 << 5, 16);                     /* 创建窗口 */
    cpuprgb = progressbar_creat(x + 10, y + 32 + 19, 180, 18, 0X61);            /* 创建进度条 */
    meminprgb = progressbar_creat(x + 10, y + 32 + 19 + 35, 180, 18, 0X61);     /* 创建进度条 */
    memtcmprgb = progressbar_creat(x + 10, y + 32 + 19 + 35 * 2, 180, 18, 0X61);/* 创建进度条 */
    memexprgb = progressbar_creat(x + 10, y + 32 + 19 + 35 * 3, 180, 18, 0X61); /* 创建进度条 */
    rbtn = btn_creat(x + 50, y + 32 + 19 + 35 * 3 + 36, SYSSET_BTN3_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02); /* 创建按钮 */

    if (twin == NULL || cpuprgb == NULL || meminprgb == NULL || memexprgb == NULL || rbtn == NULL) rval = 1;
    else
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        /* 返回按钮的颜色 */
        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
        rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language]; /* 标题为返回 */
        cpuprgb->totallen = 100;    /* 最大是100 */
        meminprgb->totallen = 100;  /* 最大是100 */
        memtcmprgb->totallen = 100; /* 最大是100 */ 
        memexprgb->totallen = 100;  /* 最大是100 */ 
        window_draw(twin);          /* 画出窗体 */
        btn_draw(rbtn);             /* 画按钮 */
        gui_show_string((uint8_t *)sysset_sysstatus_tbl[0][gui_phy.language], x + 10, y + 32 + 5, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);            /* 显示CPU使用率 */
        gui_show_string((uint8_t *)sysset_sysstatus_tbl[1][gui_phy.language], x + 10, y + 32 + 5 + 35, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);       /* 显示内部内存使用率 */
        gui_show_string((uint8_t *)sysset_sysstatus_tbl[2][gui_phy.language], x + 10, y + 32 + 5 + 35 * 2, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);   /* 显示TCM内存使用率 */
        gui_show_string((uint8_t *)sysset_sysstatus_tbl[3][gui_phy.language], x + 10, y + 32 + 5 + 35 * 3, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);   /* 显示外部内存使用率 */
        gui_show_string((uint8_t *)sysset_sysstatus_tbl[4][gui_phy.language], x + 200 - 10 - 36 - 36, y + 32 + 5, 190, 12, 12, SYSSET_INWIN_FONT_COLOR); /* 显示温度 */
        
        gui_show_string("℃", x + 200 - 10 - 12, y + 32 + 5, 190, 12, 12, SYSSET_INWIN_FONT_COLOR);  /* 显示CPU当前温度 */
    }

    while (rval == 0)
    {
        tp_dev.scan(0);
        in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

        if (system_task_return)
        {
            break;  /* 按键返回 */
        }

        res = btn_check(rbtn, &in_obj);         /* 返回按钮检测 */

        if (res)
        {
            if ((rbtn->sta & 0X80) == 0)        /* 有有效操作 */
            {
                break;/* 退出 */
            }
        }

        if (i == 0)   /* 时间到了,更新内存,CPU等的状态 */
        {
            cpuprgb->curpos = OSCPUUsage; 
            meminprgb->curpos = my_mem_perused(SRAMIN) / 10;     /* 内部内存使用率 */
            memtcmprgb->curpos = my_mem_perused(SRAMDTCM) / 10;  /* SRAMDTCM内存使用率 */
            memexprgb->curpos = my_mem_perused(SRAMEX) / 10;     /* 外部内存使用率 */

            progressbar_draw_progressbar(cpuprgb);      /* 显示进度条 */
            progressbar_draw_progressbar(meminprgb);    /* 显示进度条 */
            progressbar_draw_progressbar(memtcmprgb);   /* 显示进度条 */
            progressbar_draw_progressbar(memexprgb);    /* 显示进度条 */ 
            /* 显示温度值 */
            temperate = adc3_get_temperature() / 10;    /* 得到内部温度 */
            app_show_float(x + 200 - 10 - 12, y + 32 + 5, temperate, 1, 5, 12, SYSSET_INWIN_FONT_COLOR, twin->windowbkc); /* 显示温度 */
            i = OS_TICKS_PER_SEC;
        }

        i--;
    }

    window_delete(twin);            /* 删除窗口 */
    btn_delete(rbtn);               /* 删除按钮 */
    progressbar_delete(cpuprgb);    /* 删除进度条 */
    progressbar_delete(meminprgb);  /* 删除进度条 */
    progressbar_delete(memtcmprgb); /* 删除进度条 */
    progressbar_delete(memexprgb);  /* 删除进度条 */
    system_task_return = 0;         /* 清除退出标志 */
}

/**
 * @brief       关于系统
 * @param       x,y             : 起始坐标
 * @param       caption         : 窗口名字
 * @retval      无
 */
void sysset_system_about(uint16_t x, uint16_t y, uint8_t *caption)
{
    _window_obj *twin = 0;      /* 窗体 */
    _btn_obj *rbtn = 0;         /* 返回按钮 */
    uint8_t rval = 0;
    uint32_t sn0, sn1, sn2;
    uint8_t *tbuf;
    uint8_t verbuf[6];

    tbuf = mymalloc(SRAMIN, 31);    /* 申请31个字节 */
    twin = window_creat(x, y, 200, 262, 0, 1 | 1 << 5, 16);	/* 创建窗口 */
    rbtn = btn_creat(x + 50, y + 32 + 180 + 10, SYSSET_BTN3_WIDTH, SYSSET_BTN2_HEIGHT, 0, 0x02); /* 创建按钮 */

    if (twin != NULL && rbtn != NULL)
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        /* 返回按钮的颜色 */
        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
        rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language];  /* 标题为返回 */

        window_draw(twin);          /* 画出窗体 */
        btn_draw(rbtn);             /* 画按键 */
        gui_draw_rectangle(x + 10 - 4, y + 32 + 10 + 8, 188, 102, 0X4A49);  /* 画边框 */
        gui_draw_rectangle(x + 10 - 4, y + 32 + 120 + 18, 188, 42, 0X4A49); /* 画边框 */

        gui_phy.back_color = APP_WIN_BACK_COLOR;
        gui_show_ptstr(x + 10, y + 32 + 10, x + 10 + 180, y + 32 + 10 + 16, 0, BLACK, 16, (uint8_t *)sysset_system_about_caption_tbl[0][gui_phy.language], 0); 
        strcpy((char *)tbuf, "HARDWARE:");
        app_get_version(verbuf, HARDWARE_VERSION, 2);
        strcat((char *)tbuf, (const char *)verbuf);
        strcat((char *)tbuf, ", SOFTWARE:");
        app_get_version(verbuf, SOFTWARE_VERSION, 3);
        strcat((char *)tbuf, (const char *)verbuf);
        gui_show_string(tbuf, x + 10, y + 32 + 34, 180, 12, 12, BLACK); /* 显示信息 */
        gui_show_string("Copyright (C) 2022~2032", x + 10, y + 32 + 34 + 20 * 1, 180, 12, 12, BLACK); /* 显示信息 */
        gui_show_string("东莞慧勤智远科技有限公司", x + 10, y + 32 + 34 + 20 * 2, 180, 12, 12, BLACK); /* 显示信息 */

        gui_show_ptstr(x + 10, y + 32 + 120 + 10, x + 10 + 180, y + 32 + 120 + 10 + 16, 0, BLACK, 16, (uint8_t *)sysset_system_about_caption_tbl[1][gui_phy.language], 0); /* 产品序列号 */
        app_getstm32_sn(&sn0, &sn1, &sn2); /* 得到序列号 */
        sprintf((char *)tbuf, "SN:%X%X%X", sn0, sn1, sn2);
        gui_show_string(tbuf, x + 10, y + 32 + 120 + 10 + 24, 180, 12, 12, BLACK); /* 显示信息 */

        while (1)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            if (system_task_return)
            {
                break;  /* 按键返回 */
            }

            rval = btn_check(rbtn, &in_obj);    /* 返回按钮检测 */

            if (rval && (rbtn->sta & 0X80) == 0)break;  /* 有有效操作 */

        }
    }

    myfree(SRAMIN, tbuf);
    window_delete(twin);
    btn_delete(rbtn);
    system_task_return = 0; /* 取消返回 */
}

/**
 * @brief       查找条目名字
 * @param       mcaption        : 主目录下的目录名(一定要有'.'字符在里面)
 * @retval      条目位置
 */
uint8_t *set_search_caption(const uint8_t *mcaption)
{
    while (*mcaption != '.')mcaption++;

    return (uint8_t *)(++mcaption);
}

/**
 * @brief       读取系统设置信息
 *  @note       系统设置数据保存在:SYSTEM_PARA_SAVE_BASE
 * @param       sysset          : 系统信息
 * @retval      无
 */
void sysset_read_para(_system_setings *sysset)
{
    bl24cxx_read(SYSTEM_PARA_SAVE_BASE, (uint8_t *)sysset, sizeof(_system_setings));
}

/**
 * @brief       写入系统设置信息
 *  @note       系统设置数据保存在:SYSTEM_PARA_SAVE_BASE
 * @param       sysset          : 系统信息
 * @retval      无
 */
void sysset_save_para(_system_setings *sysset)
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();    /* 进入临界区(无法被中断打断) */
    bl24cxx_write(SYSTEM_PARA_SAVE_BASE, (uint8_t *)sysset, sizeof(_system_setings));
    OS_EXIT_CRITICAL();     /* 退出临界区(可以被中断打断) */
}

/**
 * @brief       系统设置
 * @param       无
 * @retval      无
 */
uint8_t sysset_play(void)
{
    uint8_t tlanguage;      /* 进入时语言的类型 */
    uint8_t res;
    uint8_t selitem = 0;
    uint8_t topitem = 0;
    uint8_t **items;
    uint8_t *caption;       /* 标题列表 */
    uint8_t *scaption;      /* 子标题 */
    uint16_t temp1;
    uint8_t temp2, temp3;   /* 时间/日期暂存 */
    uint8_t savemask = 0;   /* 0,不需要保存任何数据 */
                            /* [0]:systemset数据改动标记 */
                            /* [1]:es8388set数据改动标记 */
                            /* [2]:alarm数据改动标记 */
//    _es8388_obj tesset;

    
    caption = (uint8_t *)APP_MFUNS_CAPTION_TBL[5][gui_phy.language];
    items = (uint8_t **)sysset_mmenu_tbl[gui_phy.language];
    tlanguage = gui_phy.language;       /* 保存刚进入是语言的种类 */

    while (1)
    {
        res = app_listbox_select(&selitem, &topitem, caption, items, SYS_MENU_SIZE);

        if (system_task_return)
        {
            break;  /* 按键返回 */
        }

        if (res & 0X80)
        {
            scaption = set_search_caption(items[selitem]);

            switch (selitem)   /* 判断选择的条目 */
            {
                case 0:/* 时间设置 */
                    calendar_get_time(&calendar);   /* 更新到最新的时间 */
                    temp1 = calendar.hour;
                    temp2 = calendar.min;
                    res = sysset_time_set((lcddev.width - 150) / 2, (lcddev.height - 200) / 2, (uint8_t *)&temp1, (uint8_t *)&temp2, scaption);

                    if (res == 0)   /* 成功 */
                    {
                        rtc_set_time(temp1, temp2, 0, 0);   /* 设置时间 */
                    }

                    break;

                case 1:/* 日期设置 */
                    calendar_get_date(&calendar);   /* 更新到最新的日期 */
                    temp1 = calendar.year;
                    temp2 = calendar.month;
                    temp3 = calendar.date;
                    res = sysset_date_set((lcddev.width - 190) / 2, (lcddev.height - 200) / 2, &temp1, (uint8_t *)&temp2, (uint8_t *)&temp3, scaption);

                    if (res == 0)   /* 成功 */
                    {
                        rtc_set_date(temp1 - 2000, temp2, temp3, rtc_get_week(temp1, temp2, temp3));    /* 设置日期 */
                    }

                    break;

                case 2:/* 闹钟时间设置 */
                    temp1 = alarm.hour;
                    temp2 = alarm.min;
                    res = sysset_time_set((lcddev.width - 150) / 2, (lcddev.height - 200) / 2, (uint8_t *)&temp1, (uint8_t *)&temp2, scaption);

                    if (res == 0)   /* 成功,点击了确认按钮 */
                    {
                        alarm.hour = temp1;
                        alarm.min = temp2;
                        calendar_alarm_init(&alarm, &calendar); /* 重新初始化闹钟 */
                        savemask |= 1 << 2; /* 标记闹钟数据改动了 */
                    }

                    break;

                case 3:/* 闹钟开关设置 */
                    temp1 = alarm.weekmask;
                    res = app_items_sel((lcddev.width - 200) / 2, (lcddev.height - 302) / 2, 200, 302, (uint8_t **)calendar_week_table[gui_phy.language], 7, (uint8_t *)&temp1, 0XB0, scaption); /* 多选 */

                    if (res == 0)   /* 设置成功 */
                    {
                        alarm.weekmask = temp1;
                        calendar_alarm_init(&alarm, &calendar); /* 重新初始化闹钟 */
                        savemask |= 1 << 2; /* 标记闹钟数据改动了 */
                    }

                    break;

                case 4:/* 闹钟铃声类型设置 */
                    temp1 = alarm.ringsta & 0X03;
                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 232) / 2, 180, 72 + 40 * 4, (uint8_t **)sysset_ring_tbl[gui_phy.language], 4, (uint8_t *)&temp1, 0X90, scaption); /* 单选 */

                    if (res == 0)   /* 设置成功 */
                    {
                        alarm.ringsta &= 0XFC;      /* 取消最后两位原来的设置 */
                        alarm.ringsta |= temp1;     /* 重新设置铃声 */
                        savemask |= 1 << 2;         /* 标记闹钟数据改动了 */
                    }

                    break;

                case 5:/* 语言设置 */
                    temp1 = gui_phy.language; /* 得到之前的设置 */
                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 192) / 2, 180, 72 + 40 * 3, (uint8_t **)sysset_language_tbl, 3, (uint8_t *)&temp1, 0X90, scaption); /* 单选 */

                    if (res == 0)   /* 设置成功 */
                    {
                        gui_phy.language = temp1;       /* 重新设置语言 */
                        systemset.syslanguage = temp1;  /* 重新设置默认语言 */
                        caption = (uint8_t *)APP_MFUNS_CAPTION_TBL[5][gui_phy.language];
                        items = (uint8_t **)sysset_mmenu_tbl[gui_phy.language];
                        savemask |= 1 << 0;             /* 标记系统设置数据改动了 */
                    }

                    break;

                case 6:/* 图片浏览模式设置 */
                    temp1 = systemset.picmode; /* 得到之前的设置 */
                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 152) / 2, 180, 72 + 40 * 2, (uint8_t **)sysset_avplaymode_tbl[gui_phy.language], 2, (uint8_t *)&temp1, 0X90, scaption); /* 2个模式 */

                    if (res == 0)   /* 设置成功 */
                    {
                        systemset.picmode = temp1; /* 重设图片浏览模式 */
                        savemask |= 1 << 0; /* 标记系统设置数据改动了 */
                    }

                    break;

                case 7:/* 音乐播放模式设置 */
                    temp1 = systemset.audiomode; /* 得到之前的设置 */
                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 192) / 2, 180, 72 + 40 * 3, (uint8_t **)sysset_avplaymode_tbl[gui_phy.language], 3, (uint8_t *)&temp1, 0X90, scaption); /* 3个模式 */

                    if (res == 0)   /* 设置成功 */
                    {
                        systemset.audiomode = temp1; /* 重设音乐播放模式 */
                        savemask |= 1 << 0; /* 标记系统设置数据改动了 */
                    }

                    break;

                case 8:/* 视频播放模式设置 */
                    temp1 = systemset.videomode; /* 得到之前的设置 */
                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 192) / 2, 180, 72 + 40 * 3, (uint8_t **)sysset_avplaymode_tbl[gui_phy.language], 3, (uint8_t *)&temp1, 0X90, scaption); /* 3个模式 */

                    if (res == 0) /* 设置成功 */
                    {
                        systemset.videomode = temp1; /* 重设视频播放模式 */
                        savemask |= 1 << 0; /* 标记系统设置数据改动了 */
                    }

                    break;

                case 9:/* 板载喇叭开关设置 */
//                    temp1 = es8388set.speakersw;    /* 得到之前的设置 */
//                    res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 152) / 2, 180, 72 + 40 * 2, (uint8_t **)sysset_speakermode_tbl[gui_phy.language], 2, (uint8_t *)&temp1, 0X90, scaption); /* 2个模式 */

//                    if (res == 0)   /* 设置成功 */
//                    {
//                        es8388set.speakersw = temp1;        /* 重设喇叭模式 */
//                        app_es8388_volset(es8388set.mvol);
//                        savemask |= 1 << 0;                     /* 标记系统设置数据改动了 */
//                    }

                    break;

                case 10:/* ES8388音量&3D设置 */
//                    tesset = es8388set;
//                    res = sysset_es8388_vol3d_set((lcddev.width - 200) / 2, (lcddev.height - 212) / 2, &tesset, scaption);

//                    if (res == 0)   /* 设置成功 */
//                    {
//                        es8388set = tesset;     /* 保存到es8388set */
//                        savemask |= 1 << 1;     /* 标记es8388set数据改动了 */
//                    }
//                    else            /* 恢复之前的设置 */
//                    {
//                        app_es8388_volset(es8388set.mvol);
//                        es8388_3d_set(es8388set.d3);
//                    }

                    break;
                case 11://EQ1~5设置
                case 12:
                case 13:
                case 14:
                case 15:
//                    tesset = es8388set;
//                    res = sysset_es8388_eq_set((lcddev.width - 200) / 2,(lcddev.height - 212) / 2,&tesset,selitem - 11,scaption);
//                
//                    if (res == 0)               /* 设置成功 */
//                    {
//                        es8388set = tesset;     /* 保存到es8388set */
//                        savemask |= 1 << 1;     /* 标记vsset数据改动了 */
//                    }
//                    else app_es8388_setall();   /* 恢复之前的设置  */
                    break; 
                case 16:/* 背光设置 */
                    temp1 = systemset.lcdbklight; /* 得到之前的背光 */
                    res = sysset_bklight_set((lcddev.width - 180) / 2, (lcddev.height - 150) / 2, scaption, (uint8_t *)&systemset.lcdbklight); /* 背光设置 */

                    if (res == 0)   /* 设置成功 */
                    {
                        savemask |= 1 << 0; /* 标记系统设置数据改动了 */
                    }
                    else    /* 取消设置了 */
                    {
                        systemset.lcdbklight = temp1; /* 恢复原来的设置 */

                        if (systemset.lcdbklight)     /* 恢复之前的设置 */
                        {
                            if (STM32_FMC_16)
                            {
                                LCD_BLPWM_VAL = systemset.lcdbklight;
                            }
                            else
                            { 
                                LT768_PWM1_Init(1, 0, 100, 100, systemset.lcdbklight);
                            }                           
                        }
                        else 
                        {
                            app_lcd_auto_bklight();   /* 自动背光控制 */
                        }
                    }

                    break;

                case 17:/* 屏幕校准 */
                    if ((tp_dev.touchtype & 0X80) == 0)
                    {
                        tp_adjust();        /* 电阻屏才需要校准 */
                    }
                    
                    system_task_return = 0; /* 清除退出标志 */
                    break;

                case 18:/* 系统更新 */
                    res = window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, "", (uint8_t *)sysset_system_remaindmsg_tbl[0][gui_phy.language], 12, 0, 0X03, 0);

                    if (res == 1)   /* 需要更新 */
                    {
                        sysset_system_update((uint8_t *)sysset_system_update_msg_tbl[gui_phy.language], (lcddev.width - 200) / 2, (lcddev.height - 120) / 2 - 10);
                    }

                    system_task_return = 0; /* 取消返回 */
                    break;

                case 19:/* 恢复默认设置 */
                    res = window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, "", (uint8_t *)sysset_system_remaindmsg_tbl[1][gui_phy.language], 12, 0, 0X03, 0);

                    if (res == 1)   /* 需要恢复默认设置 */
                    {
                        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)sysset_system_remaindmsg_tbl[2][gui_phy.language], scaption, 12, 0, 0, 0);
                        app_set_default();
                        delay_ms(1000);
                    }

                    break;

                case 20:/* 系统信息 */
                    sysset_system_info((lcddev.width - 200) / 2, (lcddev.height - 312) / 2, scaption);
                    break;

                case 21:/* 系统状态 显示CPU使用率/内存状态等 */
                    sysset_system_status((lcddev.width - 200) / 2, (lcddev.height - 242) / 2, scaption);
                    break;

                case 22:/* 关于 */
                    sysset_system_about((lcddev.width - 200) / 2, (lcddev.height - 262) / 2, scaption);
                    break;
            }
        }
        else break;  /* 有错误发生了 */
    }

    if (savemask & 1 << 0)   /* 系统设置数据改动了 */
    {
        printf("sysset save!\r\n");
        sysset_save_para(&systemset);   /* 保存系统设置信息 */
    }

//    if (savemask & 1 << 1)  /* es8388数据改动了 */
//    {
//        printf("ES8388 save!\r\n");
//        es8388_save_para(&es8388set);   /* 保存ES8388设置信息 */
//    }

    if (savemask & 1 << 2)  /* 闹钟数据改动了 */
    {
        printf("alarm save!\r\n");
        calendar_save_para(&alarm);     /* 保存闹钟设置信息 */
    }

    if (tlanguage != gui_phy.language)  /* 语言发生了改变 */
    {
        lcd_clear(BLACK);
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)sysset_remindmsg_tbl[1][gui_phy.language], (uint8_t *)sysset_remindmsg_tbl[0][gui_phy.language], 12, 0, 0, 0);
        spb_delete();                   /* 删除之前的SPB界面 */
        spb_init(1);                    /* 更新主界面 */
    }

    return res;
}





