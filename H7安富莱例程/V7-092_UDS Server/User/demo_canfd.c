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
#include "uds/uds_types.h"
#include "uds/uds.h"
#include <stdbool.h>

/*
	启用CANFD2，硬件无需跳线，以太网功能需要屏蔽（有引脚复用）。
*/
extern FDCAN_HandleTypeDef hfdcan2;

// required, this must send a single CAN message with the given arbitration
// ID (i.e. the CAN message ID) and data. The size will never be more than 8
// bytes.
bool send_can(const uint32_t arbitration_id, const uint8_t* data,
        const uint8_t size) {
    
}

// optional, provide to receive debugging log messages
void debug(const char* format, ...) {

}


// not used in the current version
void set_timer(uint16_t time_ms, void (*callback)) {
 
}

// Optional: This is your callback that will be called the response to your
// diagnostic request is received.
void response_received_handler(const DiagnosticResponse* response) {
    // You received a response! Do something with it.
}

/*
*********************************************************************************************************
*	函 数 名: DemoCANFD
*	功能说明: CAN测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
//DiagnosticShims shims;
void DemoCANFD(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */

	bsp_StartAutoTimer(0, 500);	          /* 启动1个500ms的自动重装的定时器 */

//	shims = diagnostic_init_shims(debug, send_can, set_timer);
//	
//	DiagnosticRequestHandle handle = diagnostic_request_pid(&shims,
//        DIAGNOSTIC_STANDARD_PID, // this is a standard PID request, not an extended or enhanced one
//        OBD2_FUNCTIONAL_BROADCAST_ID, // the request is going out to the broadcast arbitration ID
//        0x2, // we want PID 0x2
//        response_received_handler); // our callback (optional, use NULL if you don't have one)
	
	while (1)
	{
		
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
