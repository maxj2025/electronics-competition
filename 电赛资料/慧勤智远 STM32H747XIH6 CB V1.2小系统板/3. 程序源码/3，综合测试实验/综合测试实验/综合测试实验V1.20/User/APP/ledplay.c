 /**
 ****************************************************************************************************
 * @file        ledplay.c
 * @version     V1.0
 * @brief       APP-LED测试 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "ledplay.h"
#include "./BSP/LED/led.h"


/* LED0按钮标题 */
uint8_t *const led0_btncaption_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"LED0亮", "LED0亮", "LED0 ON",},
    {"LED0灭", "LED0滅", "LED0 OFF",},
};

/* LED1按钮标题 */
uint8_t *const led1_btncaption_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"LED1亮", "LED1亮", "LED1 ON",},
    {"LED1灭", "LED1滅", "LED1 OFF",},
};

extern volatile uint8_t ledplay_led0_sta;    /* ledplay任务,LED0(绿灯)的控制状态 */

/**
 * @brief       LED测试
 * @param       caption         : 窗口名字
 * @retval      未用到
 */
uint8_t led_play(uint8_t *caption)
{

    uint8_t res, rval = 0;
    uint8_t led0sta = 1, led1sta = 1;

    _btn_obj *led0btn = 0;       /* 控制按钮 */
    _btn_obj *led1btn = 0;       /* 控制按钮 */

    uint16_t btnw, btnh;         /* 按钮参数 */
    uint16_t btnled0x, btnled0y, btnled1x, btnled1y;    /* 按钮坐标参数 */

    uint16_t cled0x, cled0y, cled1x, cled1y, cr;        /* 圆坐标参数 */

    uint8_t btnfsize;            /* 字体大小 */

    if (lcddev.width >= 480)
    {
        btnfsize = 24;          
    }
    else if (lcddev.width >= 320)
    {
        btnfsize = 24;
    }
    else if (lcddev.width >= 240)
    {
        btnfsize = 16;
    }

    if (lcddev.width > lcddev.height)
    {
        btnw = lcddev.width / 5;
        btnh = btnw / 4;

        cr = btnw / 2;
        cled0x = lcddev.width / 5 + cr;
        cled1x = cled0x + cr * 2 + lcddev.width / 5;

        cled0y = (lcddev.height - cr * 2 - 2 * btnh) / 2 + cr;
        cled1y = cled0y;

        btnled0x = lcddev.width / 5;
        btnled0y = (lcddev.height - cr * 2 - 2 * btnh) / 2 + 2 * cr + btnh;

        btnled1x = btnled0x + lcddev.width / 5 + btnw;
        btnled1y = btnled0y;
    }
    else
    {
        btnw = lcddev.width * 2 / 5;
        btnh = btnw / 4;

        cr = btnw / 2;
        cled0x = lcddev.width / 20 + cr;
        cled1x = cled0x + cr * 2 + lcddev.width / 10;

        cled0y = (lcddev.height - cr * 2 - 2 * btnh) / 2 + cr;
        cled1y = cled0y;

        btnled0x = lcddev.width / 20;
        btnled0y = (lcddev.height - cr * 2 - 2 * btnh) / 2 + 2 * cr + btnh;

        btnled1x = btnled0x + lcddev.width / 10 + btnw;
        btnled1y = btnled0y;
    }
    
    led0btn = btn_creat(btnled0x, btnled0y, btnw, btnh, 0, 0);
    led1btn = btn_creat(btnled1x, btnled1y, btnw, btnh, 0, 0);

    if (led0btn && led1btn)
    {
        lcd_clear(LGRAY);
        app_gui_tcbar(0, 0, lcddev.width, gui_phy.tbheight, 0x02);  /* 下分界线 */
        gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, WHITE, gui_phy.tbfsize, caption); /* 显示标题 */

        led0btn->caption = led0_btncaption_tbl[0][gui_phy.language];
        led0btn->font = btnfsize;
        led1btn->caption = led1_btncaption_tbl[0][gui_phy.language];
        led1btn->font = btnfsize;


        btn_draw(led0btn);   /* 画按钮 */
        btn_draw(led1btn);   /* 画按钮 */

        led0btn->caption = led0_btncaption_tbl[1][gui_phy.language];
        led1btn->caption = led1_btncaption_tbl[1][gui_phy.language];
        gui_fill_circle(cled0x, cled0y, cr, WHITE);
        gui_fill_circle(cled1x, cled1y, cr, WHITE);
        system_task_return = 0;

        while (1)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            res = btn_check(led0btn, &in_obj);

            if (res && ((led0btn->sta & (1 << 7)) == 0) && (led0btn->sta & (1 << 6))) /* 有输入,有按键按下且松开,并且TP松开了 */
            {
                led0sta = !led0sta;
                led0btn->caption = led0_btncaption_tbl[led0sta][gui_phy.language];

                if (led0sta)
                {
                    gui_fill_circle(cled0x, cled0y, cr, WHITE);
                }
                else
                {
                    gui_fill_circle(cled0x, cled0y, cr, GREEN);
                }

                LED0(led0sta);
                ledplay_led0_sta = !led0sta;
            }

            res = btn_check(led1btn, &in_obj);

            if (res && ((led1btn->sta & (1 << 7)) == 0) && (led1btn->sta & (1 << 6))) /* 有输入,有按键按下且松开,并且TP松开了 */
            {
                led1sta = !led1sta;
                led1btn->caption = led1_btncaption_tbl[led1sta][gui_phy.language];

                if (led1sta)
                {
                    gui_fill_circle(cled1x, cled1y, cr, WHITE);
                }
                else
                {
                    gui_fill_circle(cled1x, cled1y, cr, BLUE);
                }

                LED1(led1sta);
            }

            if (system_task_return)
            {
                break;  /* 返回 */
            }

            delay_ms(10);
        }
    }

    ledplay_led0_sta = 0;
    LED0(1);
    LED1(1);             /* 关闭LED */
    btn_delete(led0btn); /* 删除按钮 */
    btn_delete(led1btn); /* 删除按钮 */
    return rval;
}







