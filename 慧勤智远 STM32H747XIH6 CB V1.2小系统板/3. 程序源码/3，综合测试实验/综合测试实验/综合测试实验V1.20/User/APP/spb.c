/**
 ****************************************************************************************************
 * @file        spb.c
 * @version     V1.0
 * @brief       SPB效果实现 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H743XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./PICTURE/piclib.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./MALLOC/malloc.h"
#include "./TEXT/text.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/SPBLCD/spblcd.h"
#include "./BSP/LCD/ltdc.h"
//#include "./BSP/GSM/gsm.h"
#include "spb.h"
#include "string.h"
#include "common.h"
#include "calendar.h"
 
 
/* SPB 控制器 */
m_spb_dev spbdev;

uint8_t *const SPB_REMIND_MSG = "SPB Updating... Please Wait...";

/* 背景图路径,根据不同的lcd选择不同的路径 */
uint8_t*const spb_bkpic_path_tbl[9]=
{ 
    "1:/SYSTEM/SPB/BACKPIC/back_240320.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_272480.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_320480.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_480800.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_800456.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_1024570.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_8001250.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_1280770.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/back_7201250.jpg", 
};

/* 界面图标的路径表 */
uint8_t*const spb_icos_path_tbl[4][SPB_ICOS_NUM]=
{
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_56.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_56.bmp",
        "1:/SYSTEM/SPB/ICOS/music_56.bmp",
        "1:/SYSTEM/SPB/ICOS/video_56.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_56.bmp",
        "1:/SYSTEM/SPB/ICOS/set_56.bmp",
        "1:/SYSTEM/SPB/ICOS/game_56.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_56.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_56.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_56.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_56.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_56.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_56.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_56.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_56.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_56.bmp",
        "1:/SYSTEM/SPB/ICOS/app_56.bmp",
        "1:/SYSTEM/SPB/ICOS/v meter_56.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_70.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_70.bmp",
        "1:/SYSTEM/SPB/ICOS/music_70.bmp",
        "1:/SYSTEM/SPB/ICOS/video_70.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_70.bmp",
        "1:/SYSTEM/SPB/ICOS/set_70.bmp",
        "1:/SYSTEM/SPB/ICOS/game_70.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_70.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_70.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_70.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_70.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_70.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_70.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_70.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_70.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_70.bmp",
        "1:/SYSTEM/SPB/ICOS/app_70.bmp",
        "1:/SYSTEM/SPB/ICOS/v meter_70.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_80.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_80.bmp",
        "1:/SYSTEM/SPB/ICOS/music_80.bmp",
        "1:/SYSTEM/SPB/ICOS/video_80.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_80.bmp",
        "1:/SYSTEM/SPB/ICOS/set_80.bmp",
        "1:/SYSTEM/SPB/ICOS/game_80.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_80.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_80.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_80.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_80.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_80.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_80.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_80.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_80.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_80.bmp",
        "1:/SYSTEM/SPB/ICOS/app_80.bmp",
        "1:/SYSTEM/SPB/ICOS/v meter_80.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_110.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_110.bmp",
        "1:/SYSTEM/SPB/ICOS/music_110.bmp",
        "1:/SYSTEM/SPB/ICOS/video_110.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_110.bmp",
        "1:/SYSTEM/SPB/ICOS/set_110.bmp",
        "1:/SYSTEM/SPB/ICOS/game_110.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_110.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_110.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_110.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_110.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_110.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_110.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_110.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_110.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_110.bmp",
        "1:/SYSTEM/SPB/ICOS/app_110.bmp",
        "1:/SYSTEM/SPB/ICOS/v meter_110.bmp",
    },
};

/* 三个主图标的路径表 */
uint8_t*const spb_micos_path_tbl[3][3]=
{ 
    {
        "1:/SYSTEM/SPB/ICOS/phone_56.bmp",
        "1:/SYSTEM/SPB/ICOS/app_56.bmp",
        "1:/SYSTEM/SPB/ICOS/sms_56.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/phone_70.bmp",
        "1:/SYSTEM/SPB/ICOS/app_70.bmp",
        "1:/SYSTEM/SPB/ICOS/sms_70.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/phone_110.bmp",
        "1:/SYSTEM/SPB/ICOS/app_110.bmp",
        "1:/SYSTEM/SPB/ICOS/sms_110.bmp",
    },
};

/* 界面图标icos的对应功能名字 */
uint8_t*const icos_name_tbl[GUI_LANGUAGE_NUM][SPB_ICOS_NUM]=
{
    { 
        "电子图书","数码相框","音乐播放","视频播放",
        "时钟","系统设置","FC游戏机","记事本",
        "运行器","手写画笔","照相机","录音机",
        "USB 连接","网络通信","无线传书","计算器", 
        "应用中心","电压表",
    },
    { 
        "電子圖書","數碼相框","音樂播放","視頻播放",
        "時鐘","系統設置","FC遊戲機","記事本",	 
        "運行器","手寫畫筆","照相机","錄音機 ",
        "USB 連接","網絡通信","無線傳書","計算器", 
        "應用中心","電壓表",
    },
    { 
        "EBOOK","PHOTOS","MUSIC","VIDEO",
        "TIME","SETTINGS","FC GAMES","NOTEPAD",
        "EXE","PAINT","CAMERA","RECODER",
        "USB","ETHERNET","WIRELESS","CALC", 
        "APPS","VOLT",
    },
};

/* 主图标对应的名字 */
uint8_t*const micos_name_tbl[GUI_LANGUAGE_NUM][3]=
{ 
    {
        "拨号","应用中心","短信",    
    },
    {
        "撥號","應用中心","短信",
    },
    {
        "PHONE","APPS","SMS",
    },
};

/**
 * @brief       初始化spb各个参数
 * @param       mode            : 0, 根据需要自动选择是否更新; 1,强制重新更新;
 * @retval      无
 */
uint8_t spb_init(uint8_t mode)
{
    uint16_t i;
    uint8_t res = 0;
    uint8_t lcdtype = 0;    /* LCD类型 */
    uint8_t icoindex = 0;   /* IOC索引编号 */
    uint16_t icowidth;      /* 图标的宽度 */
    uint16_t icoxpit;       /* x方向图标之间的间距 */
  
    memset((void *)&spbdev, 0, sizeof(spbdev));
    spbdev.selico = 0xff;

    if (lcddev.width == 240)        /* 对于240*320的LCD屏幕 */
    {
        icowidth = 56;
        lcdtype = 0;
        icoindex = 0;
        icoxpit = (lcddev.width - icowidth * 4) / 4;
        spbdev.stabarheight = 0;
        spbdev.spbheight = 320;
        spbdev.spbwidth = 240;
        spbdev.spbfontsize = 12;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 8 + (i / 4) * 78;
        }
    }
    else if(lcddev.width == 272)    /* 对于272*480的LCD屏幕 */
    { 
        icowidth = 56;
        lcdtype = 1;
        icoindex = 0;
        icoxpit = (lcddev.width - icowidth * 4) / 4;
        spbdev.stabarheight = 0;
        spbdev.spbheight = 480;
        spbdev.spbwidth = 272;
        spbdev.spbfontsize = 12;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 40 + (i / 4) * 110;
        }
    }
    else if (lcddev.width == 320)   /* 对于320*480的LCD屏幕 */
    {
        icowidth = 70;
        lcdtype = 2;
        icoindex = 1;
        icoxpit = (lcddev.width - icowidth * 4) / 4;
        spbdev.stabarheight = 0;
        spbdev.spbheight = 480;
        spbdev.spbwidth = 320;
        spbdev.spbfontsize = 12;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 28 + (i / 4) * 113;
        }
    }
    else if (lcddev.width == 480)   /* 对于480*800的LCD屏幕 */
    {
        icowidth = 110;
        lcdtype = 3;
        icoindex = 3;
        icoxpit = (lcddev.width - icowidth * 4) / 4;
        spbdev.stabarheight = 0;
        spbdev.spbheight = 800;
        spbdev.spbwidth = 480;
        spbdev.spbfontsize = 16;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 56 + (i / 4) * 186;
        }
    }
    else if(lcddev.width == 800 && lcddev.height == 480)      /* 对于800*480的LCD屏幕 */
    { 
        icowidth = 80;
        lcdtype = 4;
        icoindex = 2;
        icoxpit = 48;
        spbdev.stabarheight = 24;
        spbdev.spbheight = 456;
        spbdev.spbwidth = 800;
        spbdev.spbfontsize = 16;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = 40 + (i % 6) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 39 + (i / 6) * 139;
        }
    }
    else if(lcddev.width == 1024)     /* 对于1024*600的LCD屏幕 */
    { 
        icowidth = 110;
        lcdtype = 5;
        icoindex = 3;
        icoxpit = 50;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 570;
        spbdev.spbwidth = 1024;
        spbdev.spbfontsize = 16;

        for (i = 0; i < SPB_ICOS_NUM; i++)
        {
            spbdev.icos[i].x = 57 + (i % 6) * (icowidth + icoxpit);
            spbdev.icos[i].y = spbdev.stabarheight + 45 + (i / 6) * 175;
        }
    }
    else if(lcddev.width == 800 && lcddev.height == 1280)     /* 对于800*1280的LCD屏幕 */
    { 
        icowidth = 110;
        lcdtype = 6;
        icoindex = 3;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 1250;
        spbdev.spbwidth = 800;
        spbdev.spbfontsize = 16;

        icoxpit = (lcddev.width - icowidth * 4) / 4;
      
        for (i = 0; i < SPB_ICOS_NUM; i++)
        {   
            spbdev.icos[i].width = icowidth;  /* 必须 等于图片的宽度尺寸 */
            spbdev.icos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;          
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y =  spbdev.stabarheight + 4 + icoxpit / 2 + (i / 4) * (spbdev.icos[i].height + icoxpit);
        }
    }
    else if(lcddev.width == 1280)     /* 对于1280*800的LCD屏幕 */
    { 
        icowidth = 110;
        lcdtype = 7;
        icoindex = 3;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 770;
        spbdev.spbwidth = 1280;
        spbdev.spbfontsize = 16;

        icoxpit = 90;
        spbdev.spbahwidth = 0;
      
        for (i = 0; i < SPB_ICOS_NUM; i++)
        {   
            spbdev.icos[i].width = icowidth;  /* 必须 等于图片的宽度尺寸 */
            spbdev.icos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;          
            spbdev.icos[i].x = 85 + (i % 6) * (icowidth + icoxpit);
            spbdev.icos[i].y =  spbdev.stabarheight + 95 + (i / 6) * 225;
        }
    }
    else if(lcddev.width == 720)     /* 对于720*1280的LCD屏幕 */
    { 
        icowidth = 110;
        lcdtype = 8;
        icoindex = 3;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 1250;
        spbdev.spbwidth = 720;
        spbdev.spbfontsize = 16;

        icoxpit = (lcddev.width - icowidth * 4) / 4;
        spbdev.spbahwidth = 0;
      
        for (i = 0; i < SPB_ICOS_NUM; i++)
        {   
            spbdev.icos[i].width = icowidth;  /* 必须 等于图片的宽度尺寸 */
            spbdev.icos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;          
            spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit);
            spbdev.icos[i].y =  spbdev.stabarheight + 4 + icoxpit / 2 + (i / 4) * (spbdev.icos[i].height + icoxpit);
        }
    }
    
    spbdev.spbahwidth = 0;

    for (i = 0; i < SPB_ICOS_NUM; i++)
    {
        spbdev.icos[i].width = icowidth;  /* 必须 等于图片的宽度尺寸 */
        spbdev.icos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;
        spbdev.icos[i].path = (uint8_t *)spb_icos_path_tbl[icoindex][i];
        spbdev.icos[i].name = (uint8_t *)icos_name_tbl[gui_phy.language][i];
    }
    
    if (lcddev.width <= 480)
    {
        spbdev.icos[13].path = (uint8_t *)spb_icos_path_tbl[icoindex][16];
        spbdev.icos[13].name = (uint8_t *)icos_name_tbl[gui_phy.language][16];    
        spbdev.icos[14].path = (uint8_t *)spb_icos_path_tbl[icoindex][17];
        spbdev.icos[14].name = (uint8_t *)icos_name_tbl[gui_phy.language][17];  
    }

    /* 指向EX SRAM LCD BUF */
    pic_phy.read_point = (uint32_t(*)(uint16_t,uint16_t))slcd_read_point;
    pic_phy.draw_point = (void(*)(uint16_t,uint16_t,uint32_t))slcd_draw_point;
    pic_phy.fillcolor = slcd_fill_color; 
    gui_phy.read_point = (uint32_t(*)(uint16_t,uint16_t))slcd_read_point;
    gui_phy.draw_point = (void(*)(uint16_t,uint16_t,uint32_t))slcd_draw_point;
    
    g_sramlcdbuf = gui_memex_malloc((uint32_t)spbdev.spbheight * spbdev.spbwidth * 2);/* 申请帧缓存大小 */
    
    if (g_sramlcdbuf == NULL)
    {
        return 1;/* 错误 */
    }
    
    res = piclib_ai_load_picfile((char *)spb_bkpic_path_tbl[lcdtype], 0, 0, spbdev.spbwidth, spbdev.spbheight, 0);/* 加载背景图片 */

    if (res == 0)
    {
        res = spb_load_icos();
    }
    
    /* 指向LCD */
    pic_phy.read_point = lcd_read_point;
    pic_phy.draw_point = lcd_draw_point;
    pic_phy.fillcolor = piclib_fill_color;
    gui_phy.read_point = lcd_read_point;
    gui_phy.draw_point = lcd_draw_point;
    spbdev.pos = spbdev.spbahwidth; /* 默认是第1页开始位置 */
    
    return 0;    
}

/**
 * @brief       删除SPB
 * @param       无
 * @retval      无
 */
void spb_delete(void)
{
    memset((void *)&spbdev, 0, sizeof(spbdev));
    gui_memex_free(g_sramlcdbuf);
}

/**
 * @brief       装载icos
 * @param       无
 * @retval      0,成功; 其他,错误代码;
 */
uint8_t spb_load_icos(void)
{
    uint8_t j;
    uint8_t res = 0;
    uint8_t icos_num; 
  
    if (lcdltdc.pwidth != 0)      /* 如果是RGB屏 */
    {
        icos_num = SPB_ICOS_NUM;
    }
    else
    {
        icos_num = 16;
    }
      
    for (j = 0; j < icos_num; j++)
    {
        res = minibmp_decode(spbdev.icos[j].path, spbdev.icos[j].x + spbdev.spbahwidth, spbdev.icos[j].y - spbdev.stabarheight, spbdev.icos[j].width, spbdev.icos[j].width, 0, 0);

        if (res)return 1;

        gui_show_strmid(spbdev.icos[j].x + spbdev.spbahwidth, spbdev.icos[j].y + spbdev.icos[j].width - spbdev.stabarheight, spbdev.icos[j].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.icos[j].name); /* 显示名字 */
    }

    return 0;
}

/**
 * @brief       装载主界面icos
 * @param       frame           : 帧编号
 * @retval      0,成功; 其他,错误代码;
 */
uint8_t spb_load_micos(void)
{
    uint8_t j;
    uint8_t res = 0;
    gui_fill_rectangle(0, spbdev.stabarheight + spbdev.spbheight, lcddev.width, lcddev.height - spbdev.stabarheight - spbdev.spbheight, SPB_MSG_BKCOLOR);

    for (j = 0; j < 3; j++)
    {
        res = minibmp_decode(spbdev.micos[j].path, spbdev.micos[j].x, spbdev.micos[j].y, spbdev.micos[j].width, spbdev.micos[j].width, 0, 0);

        if (res)return 1;

        gui_show_strmid(spbdev.micos[j].x, spbdev.micos[j].y + spbdev.micos[j].width, spbdev.micos[j].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.micos[j].name); /* 显示名字 */
    }

    return 0;
}

/* SD卡图标 */
/* PCtoLCD2002取模方式:阴码,逐行式,顺向 */
const uint8_t SPB_SD_ICO[60] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F,
    0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFC,
    0x00, 0x0F, 0xFE, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFC, 0x00, 0x0A, 0xAC, 0x00,
    0x0A, 0xAC, 0x00, 0x0A, 0xAC, 0x00, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00,
};

/* USB 图标, 取模方式如上 */
const uint8_t SPB_USB_ICO[60] =
{
    0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xF0, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x60, 0x00, 0x00,
    0x67, 0x00, 0x04, 0x67, 0x00, 0x0E, 0x62, 0x00, 0x0E, 0x62, 0x00, 0x04, 0x62, 0x00, 0x04, 0x7C,
    0x00, 0x06, 0x60, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
};

/**
 * @brief       显示gsm信号强度,占用20*20像素大小
 * @param       x,y             : 起始坐标
 * @param       signal          : 信号强度,范围:0~30
 * @retval      无
 */
void spb_gsm_signal_show(uint16_t x, uint16_t y, uint8_t signal)
{
    uint8_t t;
    uint16_t color;
    signal /= 5;

    if (signal > 5)signal = 5;

    for (t = 0; t < 5; t++)
    {
        if (signal)   /* 有信号 */
        {
            signal--;
            color = SPB_MICO_COLOR;
        }
        else color = 0X6B4D;   /* 无信号 */

        gui_fill_rectangle(x + 1 + t * 4, y + 20 - 6 - t * 3, 3, (t + 1) * 3, color);
    }
}

/* GSM模块提示信息 */
uint8_t *const spb_gsma_msg[GUI_LANGUAGE_NUM][3] =
{
    "无移动网", "中国移动", "中国联通",
    "無移動網", "中國移動", "中國聯通",
    " NO NET ", "CHN Mobi", "CHN Unic",
};

/**
 * @brief       更新顶部信息条信息数据
 * @param       clr             : 0,不清除背景; 1,清楚背景;
 * @retval      无
 */
void spb_stabar_msg_show(uint8_t clr)
{
    uint8_t temp;

    if (clr)
    {
        gui_fill_rectangle(0, 0, lcddev.width, spbdev.stabarheight, SPB_MSG_BKCOLOR);
        gui_show_string("CPU:  %", 24 + 64 + 20 + 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, SPB_MSG_COLOR);   /* 显示CPU数据 */
        gui_show_string(":", lcddev.width - 42 + 16, (spbdev.stabarheight - 16) / 2, 8, 16, 16, SPB_MSG_COLOR);             /* 显示':' */
    }

    /* GSM信息显示 */
    spb_gsm_signal_show(2, (spbdev.stabarheight - 20) / 2, 0); /* 显示信号质量 */
    gui_fill_rectangle(2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, SPB_MSG_BKCOLOR);

//    if (gsmdev.status & (1 << 5))
//    {
//        if (gsmdev.status & (1 << 4))temp = 2;
//        else temp = 1;

//        gui_show_string(spb_gsma_msg[gui_phy.language][temp], 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, WHITE); /* 显示运营商名字 */
//    }
//    else 
    gui_show_string(spb_gsma_msg[gui_phy.language][0], 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, WHITE);   /* 显示运营商名字 */

    /* 更新SD卡信息 */
    if (gui_phy.memdevflag & (1 << 0))app_show_mono_icos(24 + 64 + 2, (spbdev.stabarheight - 20) / 2, 20, 20, (uint8_t *)SPB_SD_ICO, SPB_MICO_COLOR, SPB_MSG_BKCOLOR);
    else gui_fill_rectangle(24 + 64 + 2, 0, 20, spbdev.stabarheight, SPB_MSG_BKCOLOR);

    /* 更新U盘信息 */
    if (gui_phy.memdevflag & (1 << 3))app_show_mono_icos(24 + 64 + 2 + 20, (spbdev.stabarheight - 20) / 2, 20, 20, (uint8_t *)SPB_USB_ICO, SPB_MICO_COLOR, SPB_MSG_BKCOLOR);
    else gui_fill_rectangle(24 + 64 + 2 + 20, 0, 20, spbdev.stabarheight, SPB_MSG_BKCOLOR);

    /* 显示CPU使用率 */
    gui_phy.back_color = SPB_MSG_BKCOLOR;
    temp = OSCPUUsage;

    if (OSCPUUsage > 99)temp = 99; /* 最多显示到99% */

    gui_show_num(24 + 64 + 2 + 20 + 20 + 2 + 32, (spbdev.stabarheight - 16) / 2, 2, SPB_MSG_COLOR, 16, temp, 0);    /* 显示CPU使用率 */
    
    /* 显示时间 */      
    calendar_get_time(&calendar);
    gui_show_num(lcddev.width - 42, (spbdev.stabarheight - 16) / 2, 2, SPB_MSG_COLOR, 16, calendar.hour, 0X80);     /* 显示时钟 */
    gui_show_num(lcddev.width - 2 - 16, (spbdev.stabarheight - 16) / 2, 2, SPB_MSG_COLOR, 16, calendar.min, 0X80);  /* 显示分钟 */
}

extern uint8_t *const sysset_remindmsg_tbl[2][GUI_LANGUAGE_NUM];

/**
 * @brief       加载SPB主界面UI
 * @param       无
 * @retval      0,成功; 其他,错误代码;
 */
uint8_t spb_load_mui(void)
{
    uint8_t res = 0;
    
    if (spbdev.spbheight == 0 && spbdev.spbwidth == 0)
    {
        lcd_clear(BLACK);   /* 黑屏 */
        window_msg_box((lcddev.width - 220) / 2, (lcddev.height - 100) / 2, 220, 100, (uint8_t *)sysset_remindmsg_tbl[1][gui_phy.language], (uint8_t *)sysset_remindmsg_tbl[0][gui_phy.language], 12, 0, 0, 0); /* 显示提示信息 */
        res = spb_init(0);  /* 重新初始化 */
    }
    
    if(res == 0)
    {
        if (lcddev.width > 480)
        {
            spb_stabar_msg_show(1);         /* 显示状态栏信息,清除所有后显示 */
        }
 
        slcd_frame_show(spbdev.pos);        /* 显示主界面icos */
        //res = spb_load_micos();           /* 加载主图标 */
    }
    
    return res;
}

/**
 * @brief       移动屏幕
 * @param       dir             : 方向,0:左移;1,右移
 * @param       skips           : 每次跳跃列数
 * @param       pos             : 起始位置
 * @retval      无
 */
void spb_frame_move(uint8_t dir, uint8_t skips, uint16_t pos)
{
    uint8_t i;
    uint16_t x;
    uint16_t endpos = spbdev.spbahwidth;

    for (i = 0; i < SPB_PAGE_NUM; i++)  /* 得到终点位置 */
    {
        if (dir == 0)   /* 左移 */
        {
            if (pos <= (spbdev.spbwidth * i + spbdev.spbahwidth))
            {
                endpos = spbdev.spbwidth * i + spbdev.spbahwidth;
                break;
            }
        }
        else    /* 右移 */
        {
            if (pos >= (spbdev.spbwidth * (SPB_PAGE_NUM - i - 1) + spbdev.spbahwidth))
            {
                endpos = spbdev.spbwidth * (SPB_PAGE_NUM - i - 1) + spbdev.spbahwidth;
                break;
            }
        }
    }

    if (dir)    /* 屏幕右移 */
    {
        for (x = pos; x > endpos;)
        {
            if ((x - endpos) > skips)x -= skips;
            else x = endpos;

            slcd_frame_show(x);
        }
    }
    else        /* 屏幕左移 */
    {
        for (x = pos; x < endpos;)
        {
            x += skips;

            if (x > endpos)x = endpos;

            slcd_frame_show(x);
        }
    }

    spbdev.pos = endpos;
}

/**
 * @brief       清除某个mico图标的选中状态
 * @param       selx            : SPB_ICOS_NUM~SPB_ICOS_NUM+2,表示将要清除选中状态的mico编号
 * @retval      无
 */
void spb_unsel_micos(uint8_t selx)
{
    if(selx >= SPB_ICOS_NUM && selx < (SPB_ICOS_NUM + 3))   /* 合法的编号 */
    {
        selx -= SPB_ICOS_NUM;
        gui_fill_rectangle(spbdev.micos[selx].x,spbdev.micos[selx].y,spbdev.micos[selx].width,spbdev.micos[selx].height,SPB_MSG_BKCOLOR); 
        minibmp_decode(spbdev.micos[selx].path,spbdev.micos[selx].x,spbdev.micos[selx].y,spbdev.micos[selx].width,spbdev.micos[selx].width,0,0);
        gui_show_strmid(spbdev.micos[selx].x,spbdev.micos[selx].y+spbdev.micos[selx].width,spbdev.micos[selx].width,spbdev.spbfontsize,SPB_FONT_COLOR,spbdev.spbfontsize,spbdev.micos[selx].name);  /* 显示名字 */
    }
}

/**
 * @brief       设置选中哪个图标
 * @param       sel             : 0~SPB_ICOS_NUM代表当前页的选中ico
 * @retval      无
 */
void spb_set_sel(uint8_t sel)
{
    uint16_t temp = 0;
  
    slcd_frame_show(spbdev.pos);      /* 刷新背景 */
    //spb_unsel_micos(spbdev.selico); /* 清除主图标选中状态 */    
    spbdev.selico = sel; 
    
    if (sel < SPB_ICOS_NUM)
    { 
        temp = spbdev.icos[sel].x;       
        gui_alphablend_area(temp, spbdev.icos[sel].y, spbdev.icos[sel].width, spbdev.icos[sel].height, SPB_ALPHA_COLOR, SPB_ALPHA_VAL);       
        minibmp_decode(spbdev.icos[sel].path, temp, spbdev.icos[sel].y, spbdev.icos[sel].width, spbdev.icos[sel].width, 0, 0);
        gui_show_strmid(temp, spbdev.icos[sel].y + spbdev.icos[sel].width, spbdev.icos[sel].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.icos[sel].name);/* 显示名字 */
    }
    else
    {
        sel -= SPB_ICOS_NUM;
        gui_alphablend_area(spbdev.micos[sel].x, spbdev.micos[sel].y, spbdev.micos[sel].width, spbdev.micos[sel].height, SPB_ALPHA_COLOR, SPB_ALPHA_VAL);
        minibmp_decode(spbdev.micos[sel].path, spbdev.micos[sel].x, spbdev.micos[sel].y, spbdev.micos[sel].width, spbdev.micos[sel].width, 0, 0);
        gui_show_strmid(spbdev.micos[sel].x, spbdev.micos[sel].y + spbdev.micos[sel].width, spbdev.micos[sel].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.micos[sel].name); /* 显示名字 */
    }
}

/**
 * @brief       屏幕滑动及按键检测
 * @param       无
 * @retval      0~15,被双击的图标编号
 *              0xff,没有任何图标被双击或者按下
 */
uint8_t spb_move_chk(void)
{
    uint8_t i = 0xff;
    
    tp_dev.scan(0);                             /* 扫描 */
    
    if (tp_dev.sta & TP_PRES_DOWN)              /* 有按键被按下 */
    {
        spbdev.spbsta |= 1 << 15;               /* 标记按下 */        
        spbdev.curxpos = tp_dev.x[0];           /* 记录当前坐标 */
        spbdev.curypos = tp_dev.y[0];           /* 记录当前坐标 */
    }
    else if(spbdev.spbsta & 0x8000)             /* 之前有按下 */
    {
        for (i = 0; i < SPB_ICOS_NUM; i++)      /* 检查按下的图标编号 */
        {
            if (i < SPB_ICOS_NUM)
            {                
                if ((spbdev.curxpos > spbdev.icos[i].x) && (spbdev.curxpos < spbdev.icos[i].x + spbdev.icos[i].width) && (spbdev.curypos > spbdev.icos[i].y)&&
                    (spbdev.curypos < spbdev.icos[i].y + spbdev.icos[i].height))
                {   
                    break;/* 得到选中的编号 */
                }
            }
        }
        if (i < (SPB_ICOS_NUM))    /* 有效 */
        { 
            if (i != spbdev.selico)/* 选中了不同的图标,切换图标 */
            {
                spb_set_sel(i);
                i = 0xff;
            }
            else
            {   
                spbdev.selico = 0XFF; /* 发生了双击,重新复位selico */
            }
        }
        else
        {
            i = 0XFF;/* 无效的点按 */
        }
        
        spbdev.spbsta = 0;    /* 清空标志 */
        tp_dev.sta = 0;       /* 清零状态 */
    }
    
    return i;
}





