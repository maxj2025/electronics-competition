#include "blue_app.h"

__attribute__((section (".RAM_D3"))) uint8_t blue_en = 1;
__attribute__((section (".RAM_D3"))) uint16_t blue_PIN = 1234;
void blue_Init(void)
{
	//查询版本号
	UART3_Printf("AT+VERSION\r\n");
	HAL_Delay(5);
	//查询模块的MAC 地址
	UART3_Printf("AT+LADDR\r\n");
	HAL_Delay(5);
//	//设置波特率为115200
//	UART3_Printf("AT+BAUD8\r\n");
//	HAL_Delay(5);
	//查询波特率
	UART3_Printf("AT+BAUD\r\n");
	HAL_Delay(5);
	//设置密码
	UART3_Printf("AT+PIN%d%d%d%d\r\n",blue_PIN/1000%10,blue_PIN/100%10,blue_PIN/10%10,blue_PIN%10);
	HAL_Delay(5);
	//查询密码
	UART3_Printf("AT+PIN\r\n");
	HAL_Delay(5);
	
	//查询name
	UART3_Printf("AT+NAME\r\n");
	HAL_Delay(5);
	//串口状态输出使能
	UART3_Printf("AT+ENLOG%d\r\n",blue_en);
	HAL_Delay(5);
	//查询串口状态输出使能
	UART3_Printf("AT+ENLOG\r\n");
	HAL_Delay(5);
}

//断开连接（连接状态下有效）
void blue_disc(void)
{
	UART3_Printf("AT+DISC\r\n");
	HAL_Delay(5);
}

//软复位
void blue_reset(void)
{
	UART3_Printf("AT+RESET\r\n");
	HAL_Delay(5);
}

void blue_send_data(uint16_t *data,uint16_t len)
{
	for(int i=0;i<len;i++)
	{
		UART3_Printf("%d\r\n",data[i]);
	}
}


void blue_send_str(char *str)
{

		UART3_Printf("%s\r\n",str);

}
