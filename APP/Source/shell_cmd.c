#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "log.h"
#include "common.h"
#include "header.h"
#include "main.h"
#include "rtc.h"

int shellDate(int argc, char *argv[])
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK && HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK)
	{
		printf("%04d-%02d-%02d %02d:%02d:%02d\r\n", 2000+sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
	}

	return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN, date, shellDate, show date);

