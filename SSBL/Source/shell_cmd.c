#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "ssbl.h"
#include "common.h"
#include "header.h"

int shellAppPartition(int argc, char *argv[])
{
	if(argc == 1)
	{
		if(AppPartitionID == 'S')
		{
			printf("APP Partition ID : S (single app partition)\r\n");
		}
		else if(AppPartitionID == 'A')
		{
			printf("APP Partition ID : A (app partition A)\r\n");
		}
		else if(AppPartitionID == 'B')
		{
			printf("APP Partition ID : B (app partition B)\r\n");
		}
		else
		{
			printf("APP Partition ID error !\r\n");
			return 1;
		}
	}
	else if(argc == 2)
	{
		if(strcmp(argv[1], "s") == 0 || strcmp(argv[1], "S") == 0)
		{
			AppPartitionID = 'S';
		}
		else if(strcmp(argv[1], "a") == 0 || strcmp(argv[1], "A") == 0)
		{
			AppPartitionID = 'A';
		}
		else if(strcmp(argv[1], "b") == 0 || strcmp(argv[1], "B") == 0)
		{
			AppPartitionID = 'B';
		}
		else
		{
			printf("Parameter error !\r\n");
			return 1;
		}

		printf("Set APP Partition ID : %c\r\n", AppPartitionID);
	}
	else
	{
		printf("Number of parameters error !\r\n");
		return 1;
	}

	return 0;
}
SHELL_EXPORT_CMD(partation, shellAppPartition, app partation id);


int shellBoot(int argc, char *argv[])
{
	header_t *app_header;
	char *app_name ;

	if(argc == 1)
	{
		if(AppPartitionID == 'S' || AppPartitionID == 'A' || AppPartitionID == 'B')
		{
			app_header = (header_t *)SAPP_PARTITION_ADDR;
			app_name = "SAPP";

			if(AppPartitionID == 'A' || AppPartitionID == 'B') // double partition
			{
				app_header = (header_t *)((AppPartitionID=='A') ? APP1_PARTITION_ADDR : APP2_PARTITION_ADDR);
				app_name = (AppPartitionID=='A') ? "APP1" : "APP2";
			}
		}
		else
		{
			printf("APP Partition ID error !\r\n");
			return 1;
		}
	}
	else if(argc == 2)
	{
		if(strcmp(argv[1], "s") == 0 || strcmp(argv[1], "S") == 0)
		{
			app_header = (header_t *)SAPP_PARTITION_ADDR;
			app_name = "SAPP";
		}
		else if(strcmp(argv[1], "a") == 0 || strcmp(argv[1], "A") == 0)
		{
			app_header = (header_t *)APP1_PARTITION_ADDR;
			app_name = "APP1";
		}
		else if(strcmp(argv[1], "b") == 0 || strcmp(argv[1], "B") == 0)
		{
			app_header = (header_t *)APP2_PARTITION_ADDR;
			app_name = "APP2";
		}
		else
		{
			printf("Parameter error !\r\n");
			return 1;
		}
	}
	else
	{
		printf("Number of parameters error !\r\n");
		return 1;
	}

	printf("Verify the integrity of APP \r\n");
	if(strcmp(app_header->name, app_name) == 0)
	{
		printf("Jump to APP(0x%08X)... \n\r", app_header->start_addr);

		if(program_jump(app_header->start_addr) == false)
		{
			printf("APP jump failed ! \r\n");
			return 1;
		}
	}
	else
	{
		printf("Verify APP failed !\r\n");
		return 1;
	}

	return 0;
}
SHELL_EXPORT_CMD(boot, shellBoot, boot app);

