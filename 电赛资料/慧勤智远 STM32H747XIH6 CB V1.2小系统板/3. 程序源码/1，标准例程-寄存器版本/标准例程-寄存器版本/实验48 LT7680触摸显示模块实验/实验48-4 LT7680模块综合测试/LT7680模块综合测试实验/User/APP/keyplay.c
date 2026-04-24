/**
 ****************************************************************************************************
 * @file        keyplay.c
 * @version     V1.0
 * @brief       APP-按键测试 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#include "keyplay.h"
#include "./BSP/KEY/key.h"


/**
 * @brief       显示圆形提示信息
 * @param       x,y             : 要显示的圆中心坐标
 * @param       r               : 半径
 * @param       fsize           : 字体大小
 * @param       color           : 圆的颜色
 * @param       str             : 显示的字符串
 * @retval      无
 */
void key_show_circle(uint16_t x, uint16_t y, uint16_t r, uint8_t fsize, uint16_t color, uint8_t *str)
{
    gui_fill_circle(x, y, r, color);
    gui_show_strmid(x - r, y - fsize / 2, 2 * r, fsize, BLUE, fsize, str); /* 显示标题 */
}

/**
 * @brief       按键测试
 * @param       caption         : 窗口名字
 * @retval      未用到
 */
uint8_t key_play(uint8_t *caption)
{
    uint8_t key;
    uint16_t k0x, kux;
    uint16_t ky;
    uint16_t kcr;
    uint8_t fsize = 0;      /* key字体大小 */

    uint8_t keyold = 0XFF;  /* 按键和之前的按键值 */
    uint8_t keyflag = (1 << 0);

    kcr = lcddev.width / 10;
    k0x = 2 * kcr;
    kux = k0x + 6 * kcr;
    ky = (lcddev.height - gui_phy.tbheight) / 2 + gui_phy.tbheight;

    if (lcddev.width <= 272)
    {
        fsize = 12;
    }
    else if (lcddev.width == 320)
    {
        fsize = 16;
    }
    else if (lcddev.width >= 480)
    {
        fsize = 24;
    }

    lcd_clear(LGRAY);
    app_gui_tcbar(0, 0, lcddev.width, gui_phy.tbheight, 0x02);  /* 下分界线 */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, WHITE, gui_phy.tbfsize, caption); /* 显示标题 */

    while (1)
    {
        key = key_scan(1);

        if (key != keyold)
        {
            keyold = key;

            if (key == KEY0_PRES)key_show_circle(k0x, ky, kcr, fsize, RED, (uint8_t *)"KEY0");
            else key_show_circle(k0x, ky, kcr, fsize, YELLOW, (uint8_t *)"KEY0");

            if (key == WKUP_PRES)key_show_circle(kux, ky, kcr, fsize, RED, (uint8_t *)"WKUP");
            else key_show_circle(kux, ky, kcr, fsize, YELLOW, (uint8_t *)"WKUP");
        }

        keyflag |= (1 << key);
        
        if (keyflag == ((1 << 0) | (1 << KEY0_PRES) | (1 << WKUP_PRES)))
        {
            if (key == 0)
            {
                break;
            }
        }

        delay_ms(10);
    }

    return 0;
}









