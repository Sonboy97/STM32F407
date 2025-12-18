#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FSBL_PARTITION_ADDR  0x08000000
#define FSBL_PARTITION_SIZE  0x00004000

#define SSBL_PARTITION_ADDR  0x08010000
#define SSBL_PARTITION_SIZE  0x00030000

#define SAPP_PARTITION_ADDR  0x08040000
#define SAPP_PARTITION_SIZE  0x000C0000

#define APP1_PARTITION_ADDR  0x08040000
#define APP1_PARTITION_SIZE  0x00060000

#define APP2_PARTITION_ADDR  0x080A0000
#define APP2_PARTITION_SIZE  0x00060000


_Bool program_jump(uint32_t program_addr);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
