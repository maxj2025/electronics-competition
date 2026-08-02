#include "usart_app.h"

volatile uint8_t uart_flag;
volatile uint8_t uart_rx_len;

volatile uint8_t uart3_flag;
volatile uint8_t uart3_rx_len;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance==USART1)
	{
		if(uart_flag==0)
		{
			uart_flag = 1;
			uart_rx_len = Size;
		}
	}
	
	if(huart->Instance==USART3)
	{
		if(uart3_flag==0)
		{
			uart3_flag = 1;
			uart3_rx_len = Size;
		}
	}
}

void UART1_Printf(const char *format,...)
{
	char tmp[128];
	va_list argptr;
	va_start(argptr,format);
	vsprintf((char* )tmp,format,argptr);
	va_end(argptr);
	HAL_UART_Transmit(&huart1,(const uint8_t *)&tmp,strlen(tmp),HAL_MAX_DELAY);	
}

void UART3_Printf(const char *format,...)
{
	char tmp[128];
	va_list argptr;
	va_start(argptr,format);
	vsprintf((char* )tmp,format,argptr);
	va_end(argptr);
	HAL_UART_Transmit(&huart3,(const uint8_t *)&tmp,strlen(tmp),HAL_MAX_DELAY);	
}

void uart_proc(void)
{
	if(uart_flag==1)
	{
		SCB_InvalidateDCache_by_Addr((uint32_t *)&uart_rx_dma_buffer, sizeof(uart_rx_dma_buffer));
		memcpy(uart_rx_temp_buffer,uart_rx_dma_buffer,uart_rx_len);
		memset(uart_rx_dma_buffer,0,sizeof(uart_rx_dma_buffer));
		
		UART1_Printf("Rx:%s\r\n",uart_rx_temp_buffer);
		
		memset(uart_rx_temp_buffer,0,sizeof(uart_rx_temp_buffer));
		uart_flag = 0;
	}
	
	if(uart3_flag==1)
	{
		SCB_InvalidateDCache_by_Addr((uint32_t *)&uart3_rx_dma_buffer, sizeof(uart3_rx_dma_buffer));
		memcpy(uart3_rx_temp_buffer,uart3_rx_dma_buffer,uart3_rx_len);
		memset(uart3_rx_dma_buffer,0,sizeof(uart3_rx_dma_buffer));
		

		if(strcmp(uart3_rx_temp_buffer,"led_on")==0)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
		}

		else if(strcmp(uart3_rx_temp_buffer,"led_off")==0)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
		}
		memset(uart3_rx_temp_buffer,0,sizeof(uart3_rx_temp_buffer));
		uart3_flag = 0;
	}
	
}

