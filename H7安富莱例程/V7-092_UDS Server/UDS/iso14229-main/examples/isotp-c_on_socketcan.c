#include "iso14229.h"
#include "isotp-c/isotp.h"
//#include <linux/can.h>
//#include <linux/can/raw.h>
//#include <net/if.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
//#include <sys/ioctl.h>
//#include <unistd.h>
#include <errno.h>

static int sockfd = 0;
static bool HasSetup = false;

extern FDCAN_HandleTypeDef hfdcan2;
static __IO uint8_t s_ucTransState = 0;
static __IO uint8_t s_ucRectate = 0;

/* 
 *  接收回调处理
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			s_ucRectate = 1;
		}
	}
}

/* 
 *  发送空回调处理
 */
void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
	if (hfdcan == &hfdcan2)
	{
		s_ucTransState = 1;
	}
}

static void SetupOnce() {
	
    HasSetup = true;
}

uint32_t isotp_user_get_us() { return UDSMillis() * 1000; }

void isotp_user_debug(const char *message, ...) {}

int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t *data, const uint8_t size) {
    SetupOnce();
	
	int32_t time1;

	FDCAN_TxHeaderTypeDef TxHeader = {0};	
	/* 第1步：配置发送参数 ===================================================================*/
	TxHeader.Identifier = arbitration_id;             		     /* 设置接收帧消息的ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		     /* 标准ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* 数据帧 */
	TxHeader.DataLength = size <<16;      		         /* 发送数据长度 */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* 设置错误状态指示 */
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;              /* 关闭可变波特率 */
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;               /* 经典CAN */
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;    /* 用于发送事件FIFO控制, 不存储 */
	TxHeader.MessageMarker = 0;                          /* 用于复制到TX EVENT FIFO的消息Maker来识别消息状态，范围0到0xFF */
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, (uint8_t *)data); /* 根据需要，可以修改_DataBuf数值 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_FIFO_EMPTY, 0); /* 使能Tx FIFO发送空中断 */
	
	s_ucTransState = 0;
	
	time1 = bsp_GetRunTime();
	while(s_ucTransState == 0)
	{
		if (bsp_CheckRunTime(time1) >= 1000)
		{
			return ISOTP_RET_ERROR;
		}
	}				
    return ISOTP_RET_OK;
}

static void printhex(const uint8_t *addr, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x,", addr[i]);
    }
    printf("\n");
}

void SocketCANRecv(UDSTpIsoTpC_t *tp, int phys_recv_id, int func_recv_id) {
	FDCAN_RxHeaderTypeDef RxHeader = {0};
	uint8_t RxData[8] = {0};
	
	if(s_ucRectate == 1)
	{	
		DISABLE_INT();
		s_ucRectate = 0;
		ENABLE_INT();
		
		HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RxData);
		HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
		
		 if (RxHeader.Identifier == phys_recv_id) {
                //UDS_DBG_PRINT("phys recvd can\n");
                //UDS_DBG_PRINTHEX(frame.data, frame.can_dlc);
                isotp_on_can_message(&tp->phys_link, RxData, RxHeader.DataLength>>16);
            } else if (RxHeader.Identifier == func_recv_id) {
               // UDS_DBG_PRINT("func recvd can\n");
               // UDS_DBG_PRINTHEX(frame.data, frame.can_dlc);
                isotp_on_can_message(&tp->func_link, RxData, RxHeader.DataLength>>16);
            }
	}
}
