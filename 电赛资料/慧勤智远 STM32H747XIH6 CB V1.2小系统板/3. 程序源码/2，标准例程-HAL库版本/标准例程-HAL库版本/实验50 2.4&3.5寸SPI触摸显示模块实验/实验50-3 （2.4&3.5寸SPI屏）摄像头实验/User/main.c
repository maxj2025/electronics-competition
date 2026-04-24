/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       摄像头 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/LCD/lcd.h"
#include "./USMART/usmart.h"
#include "./BSP/KEY/key.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/OV2640/ov2640.h"
#include "./BSP/TIMER/timer.h"
#include "./BSP/LCD/spi_lcd.h"


uint8_t g_ov_mode = 0;                                      /* bit0: 0,RGB565模式;  1,JPEG模式 */
uint16_t g_curline = 0;                                     /* 摄像头输出数据,当前行编号 */
uint16_t g_yoffset = 0;                                     /* y方向的偏移量 */

#define jpeg_buf_size   1 * 1024 * 1024                     /* 定义JPEG数据缓存jpeg_buf的大小(*4字节) */
#define jpeg_line_size  4 * 1024                            /* 定义DMA接收数据时,一行数据的最大值 */

__ALIGNED(4) uint32_t g_dcmi_line_buf[2][jpeg_line_size];   /* DMA双缓存buf */

/* JPEG数据缓存buf,定义在外部SDRAM */
#if !(__ARMCC_VERSION >= 6010050)   /* 不是AC6编译器，即使用AC5编译器时 */

uint32_t g_jpeg_data_buf[jpeg_buf_size]  __attribute__((at(0XD0000000 + 1280 * 800 * 4)));      /* JPEG数据缓存buf,定义在LCD帧缓存之后 */

#else

uint32_t g_jpeg_data_buf[jpeg_buf_size]  __attribute__((section(".bss.ARM.__at_0XD03E8000")));  /* JPEG数据缓存buf,定义在LCD帧缓存之后 */

#endif

volatile uint32_t g_jpeg_data_len = 0;                      /* buf中的JPEG有效数据长度 */

/**
 * JPEG数据采集完成标志
 * 0,数据没有采集完;
 * 1,数据采集完了,但是还没处理;
 * 2,数据已经处理完成了,可以开始下一帧接收
 */
volatile uint8_t g_jpeg_data_ok = 0;


/* JPEG尺寸支持列表 */
const uint16_t jpeg_img_size_tbl[][2] =
{
    160, 120,       /* QQVGA */
    176, 144,       /* QCIF */
    320, 240,       /* QVGA */
    400, 240,       /* WGVGA */
    352, 288,       /* CIF */
    640, 480,       /* VGA */
    800, 600,       /* SVGA */
    1024, 768,      /* XGA */
    1280, 800,      /* WXGA */
    1280, 960,      /* XVGA */
    1440, 900,      /* WXGA+ */
    1280, 1024,     /* SXGA */
    1600, 1200,     /* UXGA */
};

const char *EFFECTS_TBL[7] = {"Normal", "Negative", "B&W", "Redish", "Greenish", "Bluish", "Antique"};     /* 7种特效 */
const char *JPEG_SIZE_TBL[13] = {"QQVGA", "QCIF", "QVGA", "WGVGA", "CIF", "VGA", "SVGA", "XGA", "WXGA", "SVGA", "WXGA+", "SXGA", "UXGA"}; /* JPEG图片 13种尺寸 */

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

    g_curline = g_yoffset;  /* 行数复位 */
  
    if (g_ov_mode & 0X01)   /* 只有在JPEG格式下,才需要做处理. */
    {
        if (g_jpeg_data_ok == 0)                        /* jpeg数据还未采集完? */
        {
            __HAL_DMA_DISABLE(&g_dma_dcmi_handle);      /* 停止当前传输 */
    
            while (DMA1_Stream1->CR & 0X01);            /* 等待DMA1_Stream1可配置 */

            rlen = jpeg_line_size - __HAL_DMA_GET_COUNTER(&g_dma_dcmi_handle);   /* 得到剩余数据长度 */
            pbuf = g_jpeg_data_buf + g_jpeg_data_len;                            /* 偏移到有效数据末尾,继续添加 */

            if (DMA1_Stream1->CR & (1 << 19))           /* 当前目标存储器为存储器1 */
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = g_dcmi_line_buf[1][i];    /* 读取buf1里面的剩余数据 */
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = g_dcmi_line_buf[0][i];    /* 读取buf0里面的剩余数据 */
                }
            }

            g_jpeg_data_len += rlen;    /* 加上剩余数据长度 */
            g_jpeg_data_ok = 1;         /* 标记JPEG数据采集完成,等待其他函数处理 */
        }

        if (g_jpeg_data_ok == 2)        /* 上一次的jpeg数据已经被处理了 */
        {
            __HAL_DMA_SET_COUNTER(&g_dma_dcmi_handle, jpeg_line_size);   /* 设置传输数据量为jpeg_line_size*4字节 */
            __HAL_DMA_ENABLE(&g_dma_dcmi_handle);                        /* 使能DMA，重新传输 */   
          
            g_jpeg_data_ok = 0;         /* 标记数据未采集 */
            g_jpeg_data_len = 0;        /* 数据重新开始 */
        }
    }
    else
    {
        lcd_set_cursor(0, 0);           /* 设置写入位置 */
        lcd_write_ram_prepare();        /* 开始写入GRAM */
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
  
    pbuf = g_jpeg_data_buf + g_jpeg_data_len;            /* 偏移到有效数据末尾 */

    if (DMA1_Stream1->CR & (1 << 19))                    /* buf0已满,正常处理buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = g_dcmi_line_buf[0][i];             /* 读取buf0里面的数据 */
        }
        
        g_jpeg_data_len += jpeg_line_size;               /* 存储数据位置偏移 */
    }
    else                                                 /* buf1已满,正常处理buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = g_dcmi_line_buf[1][i];             /* 读取buf1里面的数据 */
        }
        
        g_jpeg_data_len += jpeg_line_size;               /* 存储数据位置偏移 */
    }
}

/**
 * @brief       LCD屏RGB565数据接收回调函数
 * @param       无
 * @retval      无
 */
void rgblcd_dcmi_rx_callback(void)
{  
    uint16_t *pbuf;

    if (DMA1_Stream1->CR & (1 << 19))                    /* DMA使用buf1,读取buf0 */
    { 
        pbuf = (uint16_t *)g_dcmi_line_buf[0]; 
    }
    else                                                 /* DMA使用buf0,读取buf1 */
    {
        pbuf = (uint16_t *)g_dcmi_line_buf[1]; 
    }

    spi_dma_lcd_color_fill(0, g_curline, lcddev.width - 1, g_curline, pbuf); /* SPI DMA颜色填充 */ 

    if (g_curline < lcddev.height)
    {
        g_curline++;
    }
}

/**
 * @brief       JPEG测试
 * @note        JPEG数据,通过串口1发送给电脑.
 * @param       无
 * @retval      无
 */
void jpeg_test(void)
{
    uint32_t i, jpgstart, jpglen;
    uint8_t *p;
    uint8_t key, headok = 0;
    uint8_t contrast = 2;
//    uint8_t effect = 0, saturation = 2;
    uint8_t msgbuf[15];                      /* 消息缓存区 */
    uint8_t size = 2;                        /* 默认是QVGA 320*240尺寸 */

    usart_init(921600);                      /* 设置串口波特率为921600 */
  
    lcd_clear(WHITE);
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "OV2640 JPEG Mode", RED);
    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Contrast", RED);                        /* 对比度 */
//    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Saturation", RED);                    /* 色彩饱和度 */
//    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Effect", RED);                        /* 特效 */
    lcd_show_string(30, 120, 200, 16, 16, "WK_UP:Size", RED);                           /* 分辨率设置 */
    sprintf((char *)msgbuf, "JPEG Size:%s", JPEG_SIZE_TBL[size]);
    lcd_show_string(30, 140, 200, 16, 16, (char *)msgbuf, RED);                         /* 显示当前JPEG分辨率 */

    ov2640_jpeg_mode();                                                                 /* JPEG模式 */
    dcmi_init();                                                                        /* DCMI配置 */
    dcmi_rx_callback = jpeg_dcmi_rx_callback;                                           /* JPEG接收数据回调函数 */
    dcmi_dma_init((uint32_t)&g_dcmi_line_buf[0], (uint32_t)&g_dcmi_line_buf[1], jpeg_line_size, DMA_MDATAALIGN_WORD, DMA_MINC_ENABLE);  /* DCMI DMA配置 */
    ov2640_outsize_set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);         /* 设置输出尺寸 */
    dcmi_start();                                                                       /* 启动传输 */

    while (1)
    { 
        if (g_jpeg_data_ok == 1)                                                        /* 已经采集完一帧图像了 */
        {  
            p = (uint8_t *)g_jpeg_data_buf;
            lcd_show_string(30, 210, 210, 16, 16, "Sending JPEG data...", RED);         /* 提示正在传输数据 */
            jpglen = 0;                                                                 /* 设置jpg文件大小为0 */
            headok = 0;                                                                 /* 清除jpg头标记 */
          
            for (i = 0; i < g_jpeg_data_len * 4; i++)                                   /* 查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小 */
            {
                if ((p[i] == 0XFF) && (p[i + 1] == 0XD8))                               /* 找到FF D8 */
                {
                    jpgstart = i;
                    headok = 1;                                                         /* 标记找到jpg头(FF D8) */
                }
                
                if ((p[i] == 0XFF) && (p[i + 1] == 0XD9) && headok)                     /* 找到头以后,再找FF D9 */
                {
                    jpglen = i - jpgstart + 2;
                    break;
                }
            }
            
            if (jpglen)                                                                 /* 正常的jpeg数据 */
            {
                p += jpgstart;                                                          /* 偏移到0XFF,0XD8处 */
              
                for (i = 0; i < jpglen; i++)                                            /* 发送整个jpg文件 */
                {
                    USART_UX->TDR = p[i];
                  
                    while ((USART_UX->ISR & 0X40) == 0);                                /* 等待上一个数据发送完成 */ 
                    
                    key = key_scan(0); 
                  
                    if (key)break;
                }  
            }
            
            if (key != 0)                                                               /* 有按键按下,需要处理 */
            {  
                lcd_show_string(30, 210, 210, 16, 16, "Quit Sending data   ", RED);     /* 提示退出数据传输 */
                switch (key)
                {
                    case KEY0_PRES:                                                     /* 对比度设置 */
                        contrast++;
                        if( contrast > 4)contrast = 0;
                        ov2640_contrast(contrast);
                        sprintf((char*)msgbuf, "Constrast:%d", (signed char)contrast - 2);
                        break;
                    
//                    case KEY0_PRES:                                                   /* 饱和度设置 */
//                        saturation++;
//                        if (saturation > 4)saturation = 0;
//                        ov2640_color_saturation(saturation);
//                        sprintf((char *)msgbuf, "Saturation:%d", saturation);
//                        break;
                    
//                    case KEY0_PRES:                                                   /* 特效设置 */ 
//                        effect++;
//                        if (effect > 6)effect = 0;
//                        ov2640_special_effects(effect);                                  
//                        sprintf((char *)msgbuf, "Effect:%s", EFFECTS_TBL[effect]);
//                        break;
                    
                    case WKUP_PRES:                                                     /* JPEG输出尺寸设置 */
                        size++;
                        if (size > 12)size = 0;  /* 循环 */

                        ov2640_outsize_set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);   /* 设置输出尺寸 */
                        sprintf((char *)msgbuf, "JPEG Size:%s", JPEG_SIZE_TBL[size]);
                        break;
                    
                    default : break;
                }
                
                key = 0;                                                            /* 避免重复进入 */
                lcd_fill(30, 140, 239, 140 + 16, WHITE);
                lcd_show_string(30, 140, 210, 16, 16, (char *)msgbuf, RED);         /* 显示提示内容 */
                delay_ms(800);
            }
            else 
            {
                lcd_show_string(30, 210, 210, 16, 16, "Send data complete!!", RED); /* 提示传输完成 */
            }
            
            g_jpeg_data_ok = 2; /* 标记jpeg数据处理完了,可以让DMA去采集下一帧了. */
        }
    }
}

/**
 * @brief       RGB565测试
 * @note        RGB数据直接显示在LCD上面
 * @param       无
 * @retval      无
 */
void rgb565_test(void)
{
    uint8_t key;
    uint8_t contrast = 2;
//    uint8_t effect = 0, saturation = 3;
    uint8_t scale = 1;                                                      /* 默认是全尺寸缩放 */
    uint8_t msgbuf[15];                                                     /* 消息缓存区 */
    uint16_t outputheight = 0;

    lcd_clear(WHITE);
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "OV2640 RGB565 Mode", RED);
    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Contrast", RED);            /* 对比度 */
//    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Saturation", RED);        /* 色彩饱和度 */
//    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Effects", RED);           /* 特效 */
    lcd_show_string(30, 120, 200, 16, 16, "WK_UP:FullSize/Scale", RED);     /* 1:1尺寸(显示真实尺寸)/全尺寸缩放 */

    ov2640_rgb565_mode();        /* RGB565模式 */
  
    dcmi_init();                 /* DCMI配置 */
  
    dcmi_rx_callback = rgblcd_dcmi_rx_callback;                             /* LCD屏RGB565接收数据回调函数 */
    dcmi_dma_init((uint32_t)g_dcmi_line_buf[0], (uint32_t)g_dcmi_line_buf[1], lcddev.width / 2, DMA_MDATAALIGN_HALFWORD, DMA_MINC_ENABLE);  /* DCMI DMA配置 */
    
    TIMX_INT->CR1 &= ~(0x01);    /* 关闭定时器,关闭帧率统计，打开的话，LCD屏在串口打印的时候会抖 */

    if (lcddev.height == 1024 || lcddev.height == 1280)
    {
        g_yoffset = (lcddev.height - 800) / 2;
        outputheight = 800;
    }
    else if (lcddev.width == 1280)
    {
        g_yoffset = (lcddev.height - 600) / 2;
        outputheight = 600;
    }
    else 
    {
        g_yoffset = 0;
        outputheight = lcddev.height;
    }

    g_curline = g_yoffset;  /* 行数复位 */    
    ov2640_outsize_set(lcddev.width, outputheight);   /* 满屏缩放显示 */
    dcmi_start();           /* 启动传输 */

    while (1)
    { 
        key = key_scan(0); 

        if (key)
        { 
            dcmi_stop();          /* 停止显示 */
          
            switch (key)
            {
                case KEY0_PRES:   /* 对比度设置 */
                    contrast++;
                    if (contrast > 4)contrast = 0;
                    ov2640_contrast(contrast);                /* 设置对比度 */       
                    sprintf((char*)msgbuf, "Contrast:%d", (signed char)contrast - 2);
                    break;

//                case KEY0_PRES: /* 饱和度设置 */
//                    saturation++;
//                    if (saturation > 4)saturation = 0;
//                    ov2640_color_saturation(saturation);    /* 设置色彩饱和度 */
//                    sprintf((char *)msgbuf, "Saturation:%d", (signed char)saturation - 2);
//                    break;

//                case KEY0_PRES: /* 特效设置 */
//                    effect++;
//                    if (effect > 6)effect = 0;
//                    ov5640_special_effects(effect);         /* 设置特效 */
//                    sprintf((char *)msgbuf, "%s", EFFECTS_TBL[effect]);
//                    break;

                case WKUP_PRES:   /* 1:1尺寸(显示真实尺寸)/缩放 */   
                    scale =! scale;
                
                    if (scale == 0)
                    {
                        ov2640_write_reg(0XFF, 0X01);
                        ov2640_write_reg(0x11, 0x82);         /* 设置PCLK=24MHz,此时帧率为10帧 */
                      
                        if (lcddev.id == 0x7796)
                        {
                            ov2640_image_win_set((1600 - lcddev.width) / 2, (1200 - outputheight) / 2, lcddev.width, outputheight);
                        }
                        else if (lcddev.id == 0X7789)
                        {
                            ov2640_image_win_set((1600 - lcddev.width) / 2, (1200 - outputheight) / 2, lcddev.width, outputheight + 100);                        
                        }
                        
                        ov2640_outsize_set(lcddev.width, outputheight);
                        sprintf((char *)msgbuf, "Full Size 1:1");                    
                    }
                    else
                    {
                        ov2640_write_reg(0XFF, 0X01);
                        ov2640_write_reg(0x11, 0x00);         /* 恢复帧率为15帧 */                      
                        ov2640_image_win_set(0, 0, 1600, 1200);
                        ov2640_outsize_set(lcddev.width, outputheight);
                        sprintf((char *)msgbuf, "Scale");
                    }      
                    
                    break;
                    
                default : break;
            }
            
            lcd_show_string(30, 50, 210, 16, 16, (char *)msgbuf, RED);  /* 显示提示内容  */
            delay_ms(800); 
            dcmi_start();                                               /* 重新开始传输 */
        } 
        
        delay_ms(10);
    }    
}

int main(void)
{
    uint8_t key = 0;
    uint16_t t = 0;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    usmart_init(240);	                      /* 初始化USMART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    timx_int_init(10000 - 1, 24000 - 1);    /* 10Khz计数, 1秒钟中断一次, 用于统计帧率 */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "OV2640 TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    
    while (ov2640_init())      /* 初始化OV2640 */
    {
        lcd_show_string(30, 130, 240, 16, 16, "OV2640 ERROR", RED);
        delay_ms(200);
        lcd_fill(30, 130, 239, 170, WHITE);
        delay_ms(200);
        LED0_TOGGLE();
    }

    lcd_show_string(30, 130, 200, 16, 16, "OV2640 OK", RED);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)
        {
            g_ov_mode = 0;     /* RGB565模式 */
            break;
        }
        else if (key == WKUP_PRES)
        {
            g_ov_mode = 1;     /* JPEG模式 */
            break;
        }

        t++;

        if (t == 100)
        {
            lcd_show_string(30, 150, 230, 16, 16, "KEY0:RGB565  WK_UP:JPEG", RED);  /* 闪烁显示提示信息 */
        }

        if (t == 200)
        {
            lcd_fill(30, 150, 230, 150 + 16, WHITE);
            t = 0;
            LED0_TOGGLE();     /* LED0(绿灯)闪烁 */
        }

        delay_ms(5);
    }
    
    if (g_ov_mode == 1)
    {
        jpeg_test();           /* JPEG测试 */
    }
    else 
    {
        rgb565_test();         /* RGB565测试 */
    }
}




