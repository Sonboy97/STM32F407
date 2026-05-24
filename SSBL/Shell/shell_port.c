/**
 * @file shell_port.c
 * @version 0.1
 * @date 2026-04-13
 */

#include <stdbool.h>
#include "shell.h"
#include "shell_port.h"
#include "stm32f4xx_hal.h"
#include "usart.h"

SHELL_TypeDef Shell;

/**
 * @brief 用户shell写
 */
signed char ShellRead(char *data)
{
	if(HAL_UART_Receive_DMA(&huart1, (uint8_t *)data, 1) == HAL_OK)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


/**
 * @brief 用户shell读
 */
void ShellWrite(const char data)
{
	while(HAL_OK != HAL_UART_Transmit_DMA(&huart1, (uint8_t *)&data, 1));
}


/**
 * @brief 用户shell初始化
 * 
 */
void ShellInit(void)
{
	Shell.read = ShellRead;
	Shell.write = ShellWrite;
	shellInit(&Shell);
}

/**
 * @brief 用户shell任务
 * 
 */
void ShellPoll(void)
{
	shellTask(&Shell);
}
