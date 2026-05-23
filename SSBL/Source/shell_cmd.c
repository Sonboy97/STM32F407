#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "main.h"
#include "common.h"
#include "header.h"

void shellBoot(int argc, char *argv[])
{
	header_t *app_header = (header_t *)SAPP_PARTITION_ADDR;

	printf("Verify the integrity of APP \r\n");
	if(strcmp(app_header->name, "APP") == 0)
	{
		printf("Jump to APP(0x%08X)... \n\r", app_header->start_addr);

		if(program_jump(app_header->start_addr) == false)
		{
			printf("APP jump failed ! \r\n");
		}
	}
	else
	{
		printf("Verify APP failed !\r\n");
	}
}
SHELL_EXPORT_CMD(boot, shellBoot, boot app);

