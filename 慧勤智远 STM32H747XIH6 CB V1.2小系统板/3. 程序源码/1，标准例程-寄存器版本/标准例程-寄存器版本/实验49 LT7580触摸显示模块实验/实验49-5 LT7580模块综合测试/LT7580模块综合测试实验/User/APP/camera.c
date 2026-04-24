/**
 ****************************************************************************************************
 * @file        camera.c
 * @version     V1.0
 * @brief       APP-照相机 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "camera.h"
#include "common.h"
#include "calendar.h"
//#include "audioplay.h"
#include "./BSP/OV2640/ov2640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/KEY/key.h"
//#include "qrplay.h"
//#include "facereco.h"
#include "calculator.h"
#include "./BSP/LED/led.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"


uint8_t bmp_request = 0;                /* bmp拍照请求:0,无bmp拍照请求;1,有bmp拍照请求,需要在帧中断里面,关闭DCMI接口. */
uint16_t curline = 0;                   /* 摄像头输出数据,当前行编号 */
uint16_t yoffset = 0;                   /* y方向的偏移量 */
volatile uint8_t hsync_int = 0;         /* 帧中断标志 */
volatile uint8_t jpeg_size;             /* jpeg图片分辨率 */
volatile uint8_t ov2640_mode = OV2640_RGB565_MODE;  /* 工作模式:0,RGB565模式;1,JPEG模式 */

#define jpeg_buf_size   4 * 1024 * 1024 /* 定义JPEG数据缓存jpeg_buf的大小(字节) */
#define jpeg_line_size  4 * 1024        /* 定义DMA接收数据时,一行数据的最大值 */

uint32_t *dcmi_line_buf[2];             /* JPEG数据 DMA双缓存buf指针 */
uint32_t *jpeg_data_buf;                /* JPEG数据缓存buf指针 */

volatile uint32_t jpeg_data_len = 0;    /* buf中的JPEG有效数据长度 */

/**
 * JPEG数据采集完成标志
 * 0,数据没有采集完;
 * 1,数据采集完了,但是还没处理;
 * 2,数据已经处理完成了,可以开始下一帧接收
 */
volatile uint8_t jpeg_data_ok = 0;

extern uint8_t g_framecnt;              /* 统一的帧计数器,在timer.c里面定义 */


/* JPEG尺寸支持列表 */
const uint16_t camera_jpeg_img_size_tbl[][2] =
{
    160, 120,   /* QQVGA */
    176, 144,   /* QCIF */
    320, 240,   /* QVGA */
    400, 240,   /* WQVGA */
    352, 288,   /* CIF */
    640, 480,   /* VGA */
    800, 600,   /* SVGA */
    1024, 768,  /* XGA */
    1280, 800,  /* WXGA */
    1280, 960,  /* XVGA */
    1440, 900,  /* WXGA+ */
    1280, 1024, /* SXGA */
    1600, 1200, /* UXGA */
};

/* 摄像头提示 */
uint8_t *const camera_remind_tbl[4][GUI_LANGUAGE_NUM] =
{
    {"初始化OV2640,请稍侯...", "初始化OV2640,請稍後...", "OV2640 Init,Please wait...",},
    {"未检测到OV2640,请检查...", "未檢測到OV2640,請檢查...", "No OV2640 find,Please check...",},
    {"保存为:", "保存為:", "SAVE AS:",},
    {"分辨率", "分辨率", "Resolution",},
};

/* 縮放提示 */
uint8_t *const camera_scalemsg_tbl[2][GUI_LANGUAGE_NUM] =
{
    "1:1显示(无缩放)！", "1:1顯示(無縮放)！", "1:1 Display(No Scale)！",
    "全尺寸缩放！", "全尺寸縮放！", "Full Scale！",
};

/* JPEG图片 13种尺寸 */
uint8_t *const camera_jpegimgsize_tbl[13] =
{"QQVGA", "QCIF", "QVGA", "WQVGA", "CIF", "VGA", "SVGA", "XGA", "WXGA", "XVGA", "WXGA+", "SXGA", "UXGA"};

/* 拍照成功提示框标题 */
uint8_t *const camera_saveok_caption[GUI_LANGUAGE_NUM] =
{
    "拍照成功！", "拍照成功！", "Take Photo OK！",
};

/* 拍照失败提示信息 */
uint8_t *const camera_failmsg_tbl[3][GUI_LANGUAGE_NUM] =
{
    "创建文件失败,请检查!", "創建文件失敗,請檢查!", "Creat File Failed,Please check!",
    "内存不足!", "內存不足!", "Out of memory!",
    "数据错误(图片尺寸太大)!", "數據錯誤(圖片尺寸太大)!", "Data Error(Picture is too big)!",
};

/* 00级功能选项表标题 */
uint8_t *const camera_l00fun_caption[GUI_LANGUAGE_NUM] =
{
    "相机设置", "相機設置", "Camera Set",
};

/* 00级功能选项表 */
uint8_t *const camera_l00fun_table[GUI_LANGUAGE_NUM][6] =
{
    {"场景设置", "特效设置", "曝光等级", "亮度设置", "色度设置", "对比度设置",},
    {"場景設置", "特效設置", "曝光等級", "亮度設置", "色度設置", "對比度設置",},
    {"Scene", "Effects", "Exposure", "Brightness", "Saturation", "Contrast"},
};

/* 10级功能选项表 */
/* 场景模式 */
uint8_t *const camera_l10fun_table[GUI_LANGUAGE_NUM][5] =
{
    {"自动", "晴天", "办公室", "阴天", "家庭",},
    {"自動", "晴天", "辦公室", "陰天", "家庭",},
    {"Auto", "Sunny", "Office", "Cloudy", "Home"},
};

/* 11级功能选项表 */
/* 特效设置 */
uint8_t *const camera_l11fun_table[GUI_LANGUAGE_NUM][7] =
{
    {"普通", "负片", "黑白", "偏红色", "偏绿色", "偏蓝色", "复古"},
    {"普通", "負片", "黑白", "偏紅色", "偏綠色", "偏藍色", "復古"},
    {"Normal", "Negative", "B&W", "Redish", "Greenish", "Bluish", "Antique",},
};

/* 12~14级功能选项表 */
/* 都是-2~2的5個值 */
uint8_t *const camera_l125fun_table[GUI_LANGUAGE_NUM][5] =
{
    {"-2", "-1", "0", "+1", "+2",},
    {"-2", "-1", "0", "+1", "+2",},
    {"-2", "-1", "0", "+1", "+2",},
};

/**
 * @brief       处理JPEG数据
 * @note        在DCMI_IRQHandler中断服务函数里面被调用
 *              当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
 * @param       无
 * @retval      无
 */
void jpeg_data_process(void)
{
    uint16_t i;
    uint16_t rlen;          /* 剩余数据长度 */
    uint32_t *pbuf;

    curline = yoffset;      /* 行数复位 */
    g_framecnt++;

    if (ov2640_mode == OV2640_JPEG_MODE)                 /* 只有在JPEG格式下,才需要做处理 */
    {          
        if (jpeg_data_ok == 0)                           /* jpeg数据还未采集完? */
        {
            DMA1_Stream1->CR &= ~(1 << 0);               /* 停止当前传输 */

            while (DMA1_Stream1->CR & 0X01);             /* 等待DMA1_Stream1可配置 */

            rlen = jpeg_line_size - DMA1_Stream1->NDTR;  /* 得到剩余数据长度 */
            pbuf = jpeg_data_buf + jpeg_data_len;        /* 偏移到有效数据末尾,继续添加 */

            if (DMA1_Stream1->CR & (1 << 19))            /* 当前目标存储器为存储器1 */
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = dcmi_line_buf[1][i];       /* 读取buf1里面的剩余数据 */                  
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = dcmi_line_buf[0][i];       /* 读取buf0里面的剩余数据 */                  
                }
            }

            jpeg_data_len += rlen;      /* 加上剩余数据长度 */
            jpeg_data_ok = 1;           /* 标记JPEG数据采集完成,等待其他函数处理 */
        }
        
        if (jpeg_data_ok == 2)          /* 上一次的jpeg数据已经被处理了 */
        {
            DMA1_Stream1->NDTR = jpeg_line_size; /* 设置传输数据量为jpeg_line_size*4字节 */
            DMA1_Stream1->CR |= 1 << 0; /* 使能DMA1_Stream1，重新传输 */
          
            jpeg_data_ok = 0;           /* 标记数据未采集 */
            jpeg_data_len = 0;          /* 数据重新开始 */
        }
    }
    else
    {
        if (bmp_request == 1)           /* 有bmp拍照请求,关闭DCMI */
        {
            dcmi_stop();                /* 停止DCMI */
            bmp_request = 0;            /* 标记请求处理完成 */
        }
        
        if (lcdltdc.pwidth == 0)
        {
            lcd_set_cursor(0, 0);       /* 设置写入位置 */
            lcd_write_ram_prepare();    /* 开始写入GRAM */
        }
     
        hsync_int = 1;
    }
}

/**
 * @brief       JPEG数据接收回调函数
 * @note        在DMA1_Stream1_IRQHandler中断服务函数里面被调用
 * @param       无
 * @retval      无
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    uint32_t *pbuf;
  
    pbuf = jpeg_data_buf + jpeg_data_len;            /* 偏移到有效数据末尾 */

    if (DMA1_Stream1->CR & (1 << 19))                /* buf0已满,正常处理buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = dcmi_line_buf[0][i];           /* 读取buf0里面的数据 */
        }
        
        jpeg_data_len += jpeg_line_size;             /* 存储数据位置偏移 */
    }
    else                                             /* buf1已满,正常处理buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = dcmi_line_buf[1][i];           /* 读取buf1里面的数据 */
        }
        
        jpeg_data_len += jpeg_line_size;             /* 存储数据位置偏移 */
    }
}
 
/**
 * @brief       RGB/MIPI屏数据接收回调函数
 * @param       无
 * @retval      无
 */
void rgblcd_dcmi_rx_callback(void)
{
    uint16_t *pbuf;

    if (DMA1_Stream1->CR & (1 << 19))                /* DMA使用buf1,读取buf0 */
    { 
        pbuf = (uint16_t *)dcmi_line_buf[0]; 
    }
    else                                             /* DMA使用buf0,读取buf1 */
    {
        pbuf = (uint16_t *)dcmi_line_buf[1]; 
    }     

    if (lcdltdc.pwidth != 0)   /* RGB/MIPI屏 */
    {
        ltdc_color_fill(0, curline, lcddev.width - 1, curline, pbuf); /* DMA2D填充 */
    }
    else
    {
        lcd_color_fill(0, curline, lcddev.width - 1, curline, pbuf);  /* 颜色填充 */ 
    }

    if (curline < lcddev.height) 
    {
        curline++;
    }
}

/**
 * @brief       切换为OV2640模式
 * @note        切换PC8/PC9/PC11为DCMI复用功能(AF13)
 * @param       无
 * @retval      无
 */
void sw_ov2640_mode(void)
{    
    OV2640_PWDN(0);                             /* OV2640 power up */
  
    /* GPIOC8/9/11切换为 DCMI接口 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8, AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9, AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13  DCMI_D4 */
}

/**
 * @brief       切换为SD卡模式
 * @note        切换PC8/PC9/PC11为SDMMC复用功能(AF12)
 * @param       无
 * @retval      无
 */
void sw_sdcard_mode(void)
{
    OV2640_PWDN(1);                             /* OV2640 power down */
  
    /* GPIOC8/9/11切换为 SDMMC接口 */ 
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8, AF12  SDMMC_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9, AF12  SDMMC_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11,AF12  SDMMC_D3 */
}

/**
 * @brief       得到文件名,按照日期时间命名
 * @note        bmp组合成:形如"0:PHOTO/PIC20120321210633.bmp"/"2:PHOTO/PIC20120321210633.bmp"的文件名
 *              jpg组合成:形如"0:PHOTO/PIC20120321210633.jpg"/"2:PHOTO/PIC20120321210633.jpg"的文件名
 * @param       pname           : 带路径的名字
 * @param       mode            : 0,创建.bmp文件;    1,创建.jpg文件;
 * @retval      无
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    calendar_get_time(&calendar);
    calendar_get_date(&calendar);

    if (mode == 0)
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);        /* 首选保存在SD卡 */
        else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);   /* SD卡不存在,则保存在U盘 */
    }
    else
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);        /* 首选保存在SD卡 */
        else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);   /* SD卡不存在,则保存在U盘 */
    }
}

/**
 * @brief       OV2640拍照jpg图片
 * @param       pname : 带路径的jpg文件名
 * @retval      0, 成功; 其他, 失败;
 */
uint8_t ov2640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;

    uint16_t datasize = 0;          /* 单次写入数据量 */
    uint32_t datalen = 0;           /* 总写入数据量 */
    uint8_t  *databuf;              /* 数据缓存，避免直接写外部SRAM数据到SD卡，导致写入下溢错误 */
   
    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* 开辟FIL字节的内存区域 */
    databuf = mymalloc(SRAMIN, 4096);               /* 申请4K内存 */
    
    if (databuf == NULL)
    {
        myfree(SRAMIN, f_jpg);      /* 释放f_jpg内存 */
        return 0XFF;                /* 内存申请失败 */
    }

    ov2640_mode = OV2640_JPEG_MODE;           
    jpeg_data_ok = 0;
    
    sw_ov2640_mode();               /* 切换为OV2640模式 */
    ov2640_jpeg_mode();             /* JPEG模式 */
    
    dcmi_rx_callback = jpeg_dcmi_rx_callback;   /* JPEG接收数据回调函数 */
    dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], jpeg_line_size, 2, 1); /* DCMI DMA配置(双缓冲模式) */
    
    ov2640_image_win_set(0, 0, 1600, 1200);
    ov2640_outsize_set(camera_jpeg_img_size_tbl[jpeg_size][0], camera_jpeg_img_size_tbl[jpeg_size][1]); /* 设置输出尺寸 */
    dcmi_start();                   /* 启动传输 */
    
    while (jpeg_data_ok != 1);      /* 等待第一帧图片采集完 */

    jpeg_data_ok = 2;               /* 忽略本帧图片,启动下一帧采集 */

    while (jpeg_data_ok != 1);      /* 等待第二帧图片采集完 */

    jpeg_data_ok = 2;               /* 忽略本帧图片,启动下一帧采集 */

    while (jpeg_data_ok != 1);      /* 等待第三帧图片采集完,第三帧,才保存到SD卡去 */

    dcmi_stop();                    /* 停止DMA搬运 */
    ov2640_mode = OV2640_RGB565_MODE; 
    sw_sdcard_mode();               /* 切换为SD卡模式 */
    
    printf("jpeg data size:%d\r\n", jpeg_data_len * 4); /* 串口打印JPEG文件大小 */
    pbuf = (uint8_t *)jpeg_data_buf;
    jpglen = 0;                     /* 设置jpg文件大小为0 */
    headok = 0;                     /* 清除jpg头标记 */
    
    for (i = 0; i < jpeg_data_len * 4; i++)   /* 查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小 */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8)) /* 找到FF D8 */
        {
            jpgstart = i;
            headok = 1;             /* 标记找到jpg头(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* 找到头以后,再找FF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }
    
    if (jpglen)                     /* 正常的jpeg数据 */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* 模式0,或者尝试打开失败,则创建新文件 */

        if (res == 0)
        {
            pbuf += jpgstart;       /* 偏移到0XFF,0XD8处 */
            
            while (datalen < jpglen)/* 循环写入！不能直接写外部SRAM数据到SDMMC，否则可能引起FIFO下溢错误 */
            {
                if ((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }
                else
                {
                    datasize = jpglen - datalen;    /* 最后的数据 */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* 写入内容 */
                pbuf += datasize;
                jpglen -= datasize;

                if (res)break;
            }
        }

        f_close(f_jpg);
    }
    else
    {
        res = 0XFD;
    }     

    jpeg_data_len = 0;
    sw_ov2640_mode();          /* 切换为OV2640模式 */
    ov2640_rgb565_mode();      /* RGB565模式 */
    
    if (lcdltdc.pwidth != 0)   /* RGB/MIPI屏 */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB/MIPI屏接收数据回调函数 */
        dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
    }
    else                                           
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB接收数据回调函数 */
        dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
    }
    
    myfree(SRAMIN, f_jpg);
    myfree(SRAMIN, databuf);
    
    return res;
}

/**
 * @brief       摄像头功能
 * @note        所有照片文件,均保存在SD卡PHOTO文件夹内.
 * @param       无
 * @retval      未用到
 */
uint8_t camera_play(void)
{
    uint8_t rval = 0;
    uint8_t res;
    uint8_t *caption = 0;
    uint8_t *pname;
    uint8_t selx = 0;
    uint8_t l00sel = 0, l10sel = 0, l11sel = 0; /* 默认选择项 */
    uint8_t l2345sel[4];
    uint8_t *psn;
    uint8_t key;
    uint8_t scale = 1;      /* 默认是全尺寸缩放 */
    uint16_t outputheight = 0;

    /* 提示开始检测OV2640 */
    window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);

    if (gui_phy.memdevflag & (1 << 0))f_mkdir("0:PHOTO"); /* 强制创建文件夹,给照相机用 */

    if (gui_phy.memdevflag & (1 << 3))f_mkdir("3:PHOTO"); /* 强制创建文件夹,给照相机用 */

    if (ov2640_init())   /* 初始化OV2640 */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(500);
        rval = 1;
    }

    dcmi_line_buf[0] = gui_memin_malloc(jpeg_line_size * 4);    /* 为jpeg dma接收申请内存 */
    dcmi_line_buf[1] = gui_memin_malloc(jpeg_line_size * 4);    /* 为jpeg dma接收申请内存 */
    jpeg_data_buf = gui_memex_malloc(jpeg_buf_size);            /* 为jpeg文件申请内存 */
    pname = gui_memin_malloc(40);   /* 申请40个字节内存,用于存放文件名 */
    psn = gui_memin_malloc(50);     /* 申请50个字节内存,用于存放类似：“保存为:0:PHOTO/PIC20120321210633.bmp”的提示语 */

    if (!dcmi_line_buf[1] || !jpeg_data_buf || !pname || !psn)rval = 1; /* 申请失败 */

    if (rval == 0)   /* OV2640正常 */
    {
        l2345sel[0] = 2;    /* 曝光设置2,实际值0 */
        l2345sel[1] = 2;    /* 亮度默认为2,实际值0 */
        l2345sel[2] = 2;    /* 色度默认为2,实际值0 */
        l2345sel[3] = 2;    /* 对比度默认为2,实际值0 */

        ov2640_auto_exposure(l2345sel[0]);      /* 自动曝光设置 */
        ov2640_brightness(l2345sel[1]);         /* 亮度设置 */
        ov2640_color_saturation(l2345sel[2]);   /* 色度设置 */
        ov2640_contrast(l2345sel[3]);           /* 对比度设置 */

        ov2640_rgb565_mode();      /* RGB565模式 */
        dcmi_init();               /* DCMI配置 */

        if (lcdltdc.pwidth != 0)   /* RGB/MIPI屏 */
        {
            dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB/MIPI屏接收数据回调函数 */
            dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
        }
        else                                           
        {
            dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB接收数据回调函数 */
            dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
        }      
 
        if (lcddev.height == 1024 || lcddev.height == 1280)
        {
            yoffset = (lcddev.height - 800) / 2;
            outputheight = 800;
        }
        else if (lcddev.width == 1280)
        {
            yoffset = (lcddev.height - 600) / 2;
            outputheight = 600;
        }
        else 
        {
            yoffset = 0;
            outputheight = lcddev.height;
        }
 
        curline = yoffset;                              /* 行数复位 */    
        ov2640_outsize_set(lcddev.width, outputheight); /* 满屏缩放显示 */
        dcmi_start();                                   /* 启动传输 */
        lcd_clear(0x0000);
        delay_ms(200);        
        
        jpeg_size = 12;         /* JPEG拍照,默认设置为1600*1200分辨率(UXGA) */

        while (1)
        {
            tp_dev.scan(0);

            if (tp_dev.sta & TP_PRES_DOWN)
            {
                dcmi_stop();
                sw_sdcard_mode();   /* 切换为SD卡模式 */
                caption = (uint8_t *)camera_l00fun_caption[gui_phy.language];
                res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 6) / 2, 160, 72 + 32 * 6, (uint8_t **)camera_l00fun_table[gui_phy.language], 6, (uint8_t *)&l00sel, 0X90, caption); /* 单选 */
                sw_ov2640_mode();   /* 切换为OV2640模式 */

                if (res == 0)
                {
                    dcmi_start();
                    delay_ms(200);
                    dcmi_stop();
                    sw_sdcard_mode();   /* 切换为SD卡模式 */
                    caption = (uint8_t *)camera_l00fun_table[gui_phy.language][l00sel];

                    switch (l00sel)
                    {
                        case 0:/* 场景设置 */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 5) / 2, 160, 72 + 32 * 5, (uint8_t **)camera_l10fun_table[gui_phy.language], 5, (uint8_t *)&l10sel, 0X90, caption); /* 单选 */
                            sw_ov2640_mode();/* 切换为OV2640模式 */

                            if (res == 0)
                            {
                                ov2640_light_mode(l10sel);
                            }

                            break;

                        case 1:/* 特效设置 */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 7) / 2, 160, 72 + 32 * 7, (uint8_t **)camera_l11fun_table[gui_phy.language], 7, (uint8_t *)&l11sel, 0X90, caption); /* 单选 */
                            sw_ov2640_mode();/* 切换为OV2640模式 */                     

                            if (res == 0)
                            {
                                ov2640_special_effects(l11sel);
                            }

                            break;

                        case 2:/* 曝光设置 */
                        case 3:/* 亮度设置 */
                        case 4:/* 色度设置 */
                        case 5:/* 对比度设置 */
                            selx = l2345sel[l00sel - 2]; /* 得到之前的选择 */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 5) / 2, 160, 72 + 32 * 5, (uint8_t **)camera_l125fun_table[gui_phy.language], 5, (uint8_t *)&selx, 0X90, caption); /* 单选 */
                            sw_ov2640_mode();/* 切换为OV2640模式 */                      

                            if (res == 0)
                            {
                                l2345sel[l00sel - 2] = selx; /* 记录新值 */

                                if (l00sel == 2)ov2640_auto_exposure(selx);     /* 自动曝光设置 */

                                if (l00sel == 3)ov2640_brightness(selx);        /* 亮度设置 */

                                if (l00sel == 4)ov2640_color_saturation(selx);  /* 色度设置 */

                                if (l00sel == 5)ov2640_contrast(selx);          /* 对比度设置 */
                            }

                            break;
                    }
                }
          
                sw_ov2640_mode();   /* 切换为OV2640模式 */ 
                dcmi_start();              
            }

            if (system_task_return)break; /* 按键返回 */

            key = key_scan(0);            /* 按键扫描 */

            if (key)
            {
                if (key == KEY0_PRES)     /* 如果是BMP拍照,则等待300ms,去抖动,以获得稳定的bmp照片 */
                {
                    delay_ms(300);
                    bmp_request = 1;      /* 请求关闭DCMI */
                    while (bmp_request);  /* 等待请求处理完成 */
                }
                else
                {
                    dcmi_stop();           
                }

                while (key_scan(1)); /* 等待按键松开 */

                switch (key)
                {
                    case KEY0_PRES:  /* KEY0按下,BMP拍照 */
                        LED1(0);     /* LED1(蓝灯)亮,提示拍照中 */
                        sw_sdcard_mode();       /* 切换为SD卡模式 */
                    
                        filesave_flag = 1;
                        camera_new_pathname(pname, OV2640_RGB565_MODE); /* 得到bmp文件名 */
                        res = bmp_encode(pname, 0, yoffset, lcddev.width, outputheight, 0); /* bmp拍照 */
//                        camera_new_pathname(pname, OV2640_JPEG_MODE);   /* 得到jpg文件名 */                    
//                        res = ov2640_jpg_photo(pname); /* jpeg拍照 */
                        filesave_flag = 0;
                    
                        if (res)     /* 拍照失败了 */
                        {
                            if (res == 0XFF)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* 内存错误 */
                            else if (res == 0XFD)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[2][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* 数据错误 */
                            else window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* SD卡读写操作错误 */
                        }
                        else
                        {
                            strcpy((char *)psn, (const char *)camera_remind_tbl[2][gui_phy.language]);
                            strcat((char *)psn, (const char *)pname);
                            window_msg_box((lcddev.width - 180) / 2, (lcddev.height - 80) / 2, 180, 80, psn, (uint8_t *)camera_saveok_caption[gui_phy.language], 12, 0, 0, 0);
                            delay_ms(100);
                        }

                        LED1(1);            /* LED1(蓝灯)灭,提示保存完成 */
                        delay_ms(2000);
                        sw_ov2640_mode();   /* 切换为OV2640模式 */

                        if (scale == 0)
                        {
                            ov2640_image_win_set((1600 - lcddev.width) / 2, (1200 - outputheight) / 2, lcddev.width, outputheight); /* 1:1真实尺寸 */
                            ov2640_outsize_set(lcddev.width, outputheight);
                        }
                        else
                        {
                            ov2640_image_win_set(0, 0, 1600, 1200); /* 全尺寸缩放 */
                            ov2640_outsize_set(lcddev.width, outputheight);
                        }

                        break;
                }

                sw_ov2640_mode();   /* 切换为OV2640模式 */ 
                dcmi_start();       /* 这里先使能DCMI,然后立即关闭DCMI,后面再开启DCMI,可以防止RGB屏的侧移问题 */
                dcmi_stop();
                dcmi_start();   
            }

            if (hsync_int)          /* 刚刚产生帧中断,可以延时 */
            {
                delay_ms(10);
                hsync_int = 0;
            }
        }
    }

    dcmi_stop();        /* 停止摄像头工作 */
    sw_sdcard_mode();   /* 切换为SD卡模式 */
    gui_memin_free(dcmi_line_buf[0]);
    gui_memin_free(dcmi_line_buf[1]);
    gui_memex_free(jpeg_data_buf);
    gui_memin_free(pname);
    gui_memin_free(psn);
	  LED1(1);            /* 关闭LED1(蓝灯) */
    
    return 0;
}








