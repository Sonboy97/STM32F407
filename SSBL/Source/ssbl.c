#include <stdio.h>
#include <string.h>
#include "ssbl.h"
#include "usart.h"

// SSBL头
const header_t Header __attribute__((at(SSBL_PARTITION_ADDR))) =
{
	.name = "SSBL",
	.version = "V1.0.6",
	.date = __DATE__,
	.time = __TIME__,
	.start_addr = SSBL_PARTITION_ADDR + 0x200,
};

// APP分区标识
char AppPartitionID = 'S';  // app partition identifier, S:single partition, A:partition A, B:partition B


/**
 * @brief SSBL boot
 */
void SSBL_Boot(void)
{
	printf("SSBL : STM32F407ZGT6 Second Stage Boot Loader \r\n");
	printf("SSBL : %s (%s %s) \r\n", Header.version, Header.date, Header.time);

	uint8_t data, wait = 4;
	do {
		printf("\rSSBL : Hit any key to stop autoboot : %d ", --wait);
	}while (wait != 0 && HAL_UART_Receive(&huart1, &data, 1, 1000) != HAL_OK);
	
	printf("\r\n");
	
	if(wait == 0)
	{
		if(AppPartitionID == 'S' || AppPartitionID == 'A' || AppPartitionID == 'B')
		{
			header_t *app_header = (header_t *)SAPP_PARTITION_ADDR;
			char *app_name = "SAPP";

			if(AppPartitionID == 'A' || AppPartitionID == 'B') // double partition
			{
				app_header = (header_t *)((AppPartitionID=='A') ? APP1_PARTITION_ADDR : APP2_PARTITION_ADDR);
				app_name = (AppPartitionID=='A') ? "APP1" : "APP2";
			}

			printf("SSBL : Verify the integrity of APP \r\n");
			if(strcmp(app_header->name, app_name) == 0)
			{
				printf("SSBL : Jump to APP(0x%08X)... \n\r", app_header->start_addr);
				
				if(program_jump(app_header->start_addr) == false)
				{
					printf("SSBL : APP jump failed ! \r\n");
				}
			}
			else
			{
				printf("SSBL : Verify APP failed !\r\n");
			}
		}
		else
		{
			printf("SSBL : APP Partition ID error !\r\n");
		}
	}
}
