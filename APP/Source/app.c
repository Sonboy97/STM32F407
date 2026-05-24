#include <stdio.h>
#include <string.h>
#include "app.h"
#include "main.h"
#include "usart.h"
#include "gpio.h"

// APP头
const header_t Header __attribute__((at(APP_PARTITION_ADDR))) =
{
#if defined(APP)
	.name = "SAPP",
#elif defined(APP1)
	.name = "APP1",
#elif defined(APP2)
	.name = "APP2",
#endif
	.version = "V1.0.3",
	.date = __DATE__,
	.time = __TIME__,
	.start_addr = APP_PARTITION_ADDR + 0x200,
};




