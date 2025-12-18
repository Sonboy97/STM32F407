#include "stm32f4xx_hal.h"
#include "common.h"

typedef void (*jump_callback)(void);


_Bool program_jump(uint32_t program_addr)
{
	uint32_t jump_addr;
	jump_callback callback;

	if(((*(__IO uint32_t*)program_addr) & 0x2FFE0000 ) == 0x20000000)
	{
		__disable_irq();
		HAL_DeInit();
		__enable_irq();

		jump_addr = *(__IO uint32_t*) (program_addr + 4);
		callback = (jump_callback)jump_addr;
		__set_MSP(*(__IO uint32_t*)program_addr);
		callback();

		return true;
	}

	return false;
}
