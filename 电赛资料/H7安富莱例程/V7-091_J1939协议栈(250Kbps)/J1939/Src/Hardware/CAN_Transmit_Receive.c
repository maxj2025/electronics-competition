/*
 * CAN_Transmit_Receive.c
 *
 *  Created on: 11 juli 2021
 *      Author: Daniel Mårtensson
 */

/* Layer */
#include "Hardware.h"
#include "bsp.h"
#include "Open_SAE_J1939/Open_SAE_J1939.h"
#include "ISO_11783/ISO_11783-7_Application_Layer/Application_Layer.h"

static J1939 *j1939_handler;
extern FDCAN_HandleTypeDef hfdcan2;
static __IO uint8_t s_ucTransState = 0;
static __IO uint8_t s_ucRectate = 0;

/* This is a call back function e.g listener, that will be called once SAE J1939 data is going to be sent */
static void (*Callback_Function_Send)(uint32_t, uint8_t, uint8_t[]) = NULL;
static void (*Callback_Function_Read)(uint32_t*, uint8_t[], bool*) = NULL;
static void (*Callback_Function_Traffic)(uint32_t, uint8_t, uint8_t[], bool) = NULL;

/* Platform independent library headers for CAN */
#if PROCESSOR_CHOICE == STM32
//#include "main.h"
#elif PROCESSOR_CHOICE == ARDUINO
#elif PROCESSOR_CHOICE == PIC
#elif PROCESSOR_CHOICE == AVR
#elif PROCESSOR_CHOICE == QT_USB
#include "CAN_to_USB/can_to_usb.h"
#elif PROCESSOR_CHOICE == INTERNAL_CALLBACK
/* Nothing here because else statement should not be running */
#else
/* Internal fields */
static bool internal_new_message[256] = {false};
static uint8_t internal_data[256*8] = {0};
static uint8_t internal_DLC[256] = {0};
static uint32_t internal_ID[256] = {0};
static uint8_t buffer_index_transmit = 0;
static uint8_t buffer_index_receive = 0;

/* Internal functions */
static ENUM_J1939_STATUS_CODES Internal_Transmit(uint32_t ID, uint8_t data[], uint8_t DLC) {
	internal_ID[buffer_index_transmit] = ID;
	internal_DLC[buffer_index_transmit] = DLC;
	uint8_t i;
	for(i = 0; i < 8; i++){
		if(i < DLC){
			internal_data[buffer_index_transmit*8 + i] = data[i];
		}else{
			internal_data[buffer_index_transmit*8 + i] = 0x0;
		}
	}
	internal_new_message[buffer_index_transmit] = true;
	buffer_index_transmit++;									/* When this is 256, then it will be come 0 again */
	return STATUS_SEND_OK;
}

static void Internal_Receive(uint32_t *ID, uint8_t data[], bool *is_new_message) {
	/* Do a quick check if we are going to read message that have no data */
	if(internal_new_message[buffer_index_receive] == false){
		*is_new_message = false;
		return;
	}

	*ID = internal_ID[buffer_index_receive];
	uint8_t i;
	for(i = 0; i < 8; i++){
		if(i < internal_DLC[buffer_index_receive]){
			data[i] = internal_data[buffer_index_receive*8 + i];
		}
	}
	*is_new_message = internal_new_message[buffer_index_receive];
	/* Reset */
	internal_new_message[buffer_index_receive] = false;
	internal_DLC[buffer_index_receive] = 0;
	buffer_index_receive++;										/* When this is 256, then it will be come 0 again */
}
#endif

/* 
 *  初始化CAN
 */
void STM32_Start_CAN(FDCAN_HandleTypeDef *hcan, J1939 *j1939) {
	j1939_handler = j1939;
	
	bsp_InitCan2();
}

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

/* 
 *  接收处理
 */
void J1939Pro(void)
{
	if(s_ucRectate == 1)
	{	
		DISABLE_INT();
		s_ucRectate = 0;
		ENABLE_INT();
		Open_SAE_J1939_Listen_For_Messages(j1939_handler);		
	}
}

/* 
 *  消息发送
 */
ENUM_J1939_STATUS_CODES CAN_Send_Message(uint32_t ID, uint8_t data[]) {
	int32_t time1;
	ENUM_J1939_STATUS_CODES status = STATUS_SEND_BUSY;
	
	#if PROCESSOR_CHOICE == STM32
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	TxHeader.Identifier = ID;             		         /* 设置接收帧消息的ID */
	TxHeader.IdType = FDCAN_EXTENDED_ID;     		     /* 标准ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* 数据帧 */
	TxHeader.DataLength = FDCAN_DLC_BYTES_8;      		 /* 发送数据长度 */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* 设置错误状态指示 */
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;              /* 关闭可变波特率 */
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;               /* 经典CAN */
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; 	 /* 用于发送事件FIFO控制, 不存储 */
	TxHeader.MessageMarker = 0;                          /* 用于复制到TX EVENT FIFO的消息Maker来识别消息状态，范围0到0xFF */
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, data); /* 根据需要，可以修改_DataBuf数值 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_FIFO_EMPTY, 0); /* 使能Tx FIFO发送空中断 */
	
	s_ucTransState = 0;
	
	time1 = bsp_GetRunTime();
	while(s_ucTransState == 0)
	{
		if (bsp_CheckRunTime(time1) >= 1000)
		{
			goto err;
		}
	}
	
	status = STATUS_SEND_OK;
	
	#elif PROCESSOR_CHOICE == ARDUINO
	/* Implement your CAN send 8 bytes message function for the Arduino platform */
	#elif PROCESSOR_CHOICE == PIC
	/* Implement your CAN send 8 bytes message function for the PIC platform */
	#elif PROCESSOR_CHOICE == AVR
	/* Implement your CAN send 8 bytes message function for the AVR platform */
    #elif PROCESSOR_CHOICE == QT_USB
    status = QT_USB_Transmit(ID, data, 8);
	#elif PROCESSOR_CHOICE == INTERNAL_CALLBACK
    /* Call our callback function */
    Callback_Function_Send(ID, 8, data);
    status = STATUS_SEND_OK;
	#else
	/* If no processor are used, use internal feedback for debugging */
	status = Internal_Transmit(ID, data, 8);
	#endif

	/* Display traffic */
	if (Callback_Function_Traffic != NULL) {
		Callback_Function_Traffic(ID, 8, data, true); /* ID, 8 bytes of data, data array, TX = true */
	}

err:
	return status;
}

/* Send a PGN request
 * PGN: 0x00EA00 (59904)
 */
ENUM_J1939_STATUS_CODES CAN_Send_Request(uint32_t ID, uint8_t PGN[]) {
	int32_t time1;
	
	ENUM_J1939_STATUS_CODES status = STATUS_SEND_BUSY;
	#if PROCESSOR_CHOICE == STM32
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	/* 第1步：配置发送参数 ===================================================================*/
	TxHeader.Identifier = ID;             		     /* 设置接收帧消息的ID */
	TxHeader.IdType = FDCAN_EXTENDED_ID;     		     /* 标准ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* 数据帧 */
	TxHeader.DataLength = FDCAN_DLC_BYTES_3;      		 /* 发送数据长度 */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* 设置错误状态指示 */
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;              /* 关闭可变波特率 */
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;               /* 经典CAN */
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;    /* 用于发送事件FIFO控制, 不存储 */
	TxHeader.MessageMarker = 0;                          /* 用于复制到TX EVENT FIFO的消息Maker来识别消息状态，范围0到0xFF */
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, PGN); /* 根据需要，可以修改_DataBuf数值 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_FIFO_EMPTY, 0); /* 使能Tx FIFO发送空中断 */
	
	s_ucTransState = 0;
	
	time1 = bsp_GetRunTime();
	while(s_ucTransState == 0)
	{
		if (bsp_CheckRunTime(time1) >= 1000)
		{
			goto err;
		}
	}
	
	status = STATUS_SEND_OK;
	
	#elif PROCESSOR_CHOICE == ARDUINO
	/* Implement your CAN send 3 bytes message function for the Arduino platform */
	#elif PROCESSOR_CHOICE == PIC
	/* Implement your CAN send 3 bytes message function for the PIC platform */
	#elif PROCESSOR_CHOICE == AVR
	/* Implement your CAN send 3 bytes message function for the AVR platform */
    #elif PROCESSOR_CHOICE == QT_USB
    status = QT_USB_Transmit(ID, PGN, 3);                       /* PGN is always 3 bytes */
	#elif PROCESSOR_CHOICE == INTERNAL_CALLBACK
    /* Call our callback function */
    Callback_Function_Send(ID, 3, PGN);
    status = STATUS_SEND_OK;
	#else
	/* If no processor are used, use internal feedback for debugging */
	status = Internal_Transmit(ID, PGN, 3);
	#endif

	/* Display traffic */
	if (Callback_Function_Traffic != NULL) {
		Callback_Function_Traffic(ID, 3, PGN, true); /* ID, 3 bytes of data, PGN array, TX = true */
	}

err:
	return status;
}

/* Returns true if the message data is new */
void STM32_CAN_Get_ID_Data(uint32_t* ID, uint8_t data[], bool* is_new_message) {
	FDCAN_RxHeaderTypeDef RxHeader = {0};
	uint8_t RxData[8] = {0};
	
	HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RxData);
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	
	/* Check the length of the data */
	if(RxHeader.DataLength == 0){
		*is_new_message = false;
		return;
	}
	
	*ID = RxHeader.Identifier;

	/* Read data */
	memcpy(data, RxData, 8);

	/* Leave the function by saying that the message is new */
	*is_new_message = true;
}

/* Read the current CAN-bus message. Returning false if the message has been read before, else true */
bool CAN_Read_Message(uint32_t *ID, uint8_t data[]) {
	bool is_new_message = false;
	#if PROCESSOR_CHOICE == STM32
	
	STM32_CAN_Get_ID_Data(ID, data, &is_new_message);
	#elif PROCESSOR_CHOICE == ARDUINO
	/* Implement your CAN function to get ID, data[] and the flag is_new_message here for the Arduino platform */
	#elif PROCESSOR_CHOICE == PIC
	/* Implement your CAN function to get ID, data[] and the flag is_new_message here for the PIC platform */
	#elif PROCESSOR_CHOICE == AVR
	/* Implement your CAN function to get ID, data[] and the flag is_new_message here for the AVR platform */
    #elif PROCESSOR_CHOICE == QT_USB
    QT_USB_Get_ID_Data(ID, data, &is_new_message);
	#elif PROCESSOR_CHOICE == INTERNAL_CALLBACK
    Callback_Function_Read(ID, data, &is_new_message);
	#else
	/* If no processor are used, use internal feedback for debugging */
	Internal_Receive(ID, data, &is_new_message);
	#endif

	/* Display traffic */
	if (Callback_Function_Traffic != NULL && is_new_message) {
		Callback_Function_Traffic(*ID, 8, data, false); /* ID, 8 bytes of data, data array, TX = false */
	}

	return is_new_message;
}

void CAN_Set_Callback_Functions(void (*Callback_Function_Send_)(uint32_t, uint8_t, uint8_t[]), void (*Callback_Function_Read_)(uint32_t*, uint8_t[], bool*), void (*Callback_Function_Traffic_)(uint32_t, uint8_t, uint8_t[], bool)){
	Callback_Function_Send = Callback_Function_Send_;
	Callback_Function_Read = Callback_Function_Read_;
	Callback_Function_Traffic = Callback_Function_Traffic_;
}

void CAN_Delay(uint8_t milliseconds) {
	#if PROCESSOR_CHOICE == STM32
	HAL_Delay(milliseconds);
	#elif PROCESSOR_CHOICE == ARDUINO

	#elif PROCESSOR_CHOICE == PIC

	#elif PROCESSOR_CHOICE == AVR

	#elif PROCESSOR_CHOICE == QT_USB

	#elif PROCESSOR_CHOICE == INTERNAL_CALLBACK
	/* Get start time */
	clock_t start_time = clock();

	/* Compute desired duration */
	clock_t desired_duration = (clock_t)(CLOCKS_PER_SEC * milliseconds / 1000);

	/* Wait */
	while ((clock() - start_time) < desired_duration) {
		;
	}
	#else
	/* Nothing */
	#endif
}
