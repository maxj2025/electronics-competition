/*
*********************************************************************************************************
*
*	模块名称 : CAN FD应用。
*	文件名称 : demo_canfd.c
*	版    本 : V1.0
*	说    明 : J1939应用
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2024-01-07   Eric2013    正式发布
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "can_network.h"
#include "Open_SAE_J1939/Open_SAE_J1939.h"
#include "ISO_11783/ISO_11783-7_Application_Layer/Application_Layer.h"


/*
	启用CANFD2，硬件无需跳线，以太网功能需要屏蔽（有引脚复用）。
*/
extern FDCAN_HandleTypeDef hfdcan2;
extern void STM32_Start_CAN(FDCAN_HandleTypeDef *hcan, J1939 *j1939);
extern void J1939Pro(void);
J1939 j1939_1 = {0};

/*
*********************************************************************************************************
*	函 数 名: DemoCANFD
*	功能说明: CAN测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoCANFD(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */

	bsp_StartAutoTimer(0, 500);	          /* 启动1个500ms的自动重装的定时器 */
	STM32_Start_CAN(&hfdcan2, &j1939_1);  /* 初始化CAN */ 
	Open_SAE_J1939_Startup_ECU(&j1939_1); /* 启动本机 */
	j1939_1.information_this_ECU.this_ECU_address = 0x80;  /* 设置本机地址，范围0-253, 而255用于广播地址，254用于错误地址 */
	
	while (1)
	{
		J1939Pro();  /* 消息处理 */
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
        /* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		
		}
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
