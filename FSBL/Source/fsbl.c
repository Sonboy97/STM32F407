#include <stdio.h>
#include <string.h>
#include "fsbl.h"
#include "ymodem.h"
#include "main.h"
#include "usart.h"
#include "gpio.h"

// FSBL头
const header_t Header =
{
	.name = "FSBL",
	.version = "V1.0.5",
	.date = __DATE__,
	.time = __TIME__,
	.start_addr = 0x08000000,
};


ymodem_handle_t ymodem_handle;

/**
 * @brief FSBL boot
 */
void FSBL_Boot(void)
{
	printf("\r\n\r\n");
	printf("FSBL : STM32F407ZGT6 First Stage Boot Loader \r\n");
	printf("FSBL : %s (%s %s) \r\n", Header.version, Header.date, Header.time);

	if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
	{
		header_t *ssbl_header = (header_t *)SSBL_PARTITION_ADDR;
		
		printf("FSBL : Verify the integrity of SSBL \r\n");
		if(strcmp(ssbl_header->name, "SSBL") == 0)
		{
			printf("FSBL : Jump to SSBL(0x%08X)... \n\r", ssbl_header->start_addr);
			
			if(program_jump(ssbl_header->start_addr) == false)
			{
				printf("FSBL : SSBL jump failed ! \r\n");
			}
		}
		else
		{
			printf("FSBL : Verify SSBL failed !\r\n");
		}
	}
	else
	{
		printf("FSBL : Enter download mode \r\n");
	}
}

/**
 * @brief FSBL Download
 */
void FSBL_Download(void)
{
	ymodem_result_t result;

	/* 初始化Ymodem */
	result = ymodem_init(&ymodem_handle, SSBL_PARTITION_ADDR);
	if (result != YMODEM_OK)
	{
		printf("FSBL : Ymodem init failed: %d \r\n", result);
	}
	else
	{
		printf("FSBL : Ymodem receiver ready, waiting for file transfer... \r\n");

		HAL_Delay(100);
		
		/* 启动Ymodem接收 */
		result = ymodem_receive(&ymodem_handle);
		
		printf("\r\n");

		/* 处理传输结果 */
		switch (result)
		{
			case YMODEM_OK:
				printf("FSBL : File transfer successful! \r\n");
				printf("FSBL : Filename: %s \r\n", ymodem_handle.filename);
				printf("FSBL : File size: %u bytes \r\n", ymodem_handle.filesize);
				printf("FSBL : Packets received: %u \r\n", ymodem_handle.packets_received);
				printf("FSBL : Retransmissions: %u \r\n", ymodem_handle.retransmissions);
				break;
			case YMODEM_ERROR_TIMEOUT:
				printf("FSBL : Transfer failed: Timeout \r\n");
				break;
			case YMODEM_ERROR_CANCEL:
				printf("FSBL : Transfer failed: Cancelled \r\n");
				break;
			case YMODEM_ERROR_OVERSIZE:
				printf("FSBL : Transfer failed: File too large \r\n");
				break;
			case YMODEM_ERROR_FLASH:
				printf("FSBL : Transfer failed: Flash operation error \r\n");
				break;
			default:
				printf("FSBL : Transfer failed: Error code %d \r\n", result);
				break;
		}
	}
	
	if(result == YMODEM_OK)
	{
		printf("FSBL : System Reset \r\n");
		HAL_Delay(100);
		HAL_NVIC_SystemReset();
	}
}
