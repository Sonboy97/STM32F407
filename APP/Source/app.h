#ifndef __FSBL_H__
#define __FSBL_H__

#include "common.h"
#include "header.h"

#if defined(APP)
#define APP_PARTITION_ADDR   SAPP_PARTITION_ADDR
#define APP_PARTITION_SIZE   SAPP_PARTITION_SIZE
#elif defined(APP1)
#define APP_PARTITION_ADDR   APP1_PARTITION_ADDR
#define APP_PARTITION_SIZE   APP1_PARTITION_SIZE
#elif defined(APP2)
#define APP_PARTITION_ADDR   APP2_PARTITION_ADDR
#define APP_PARTITION_SIZE   APP2_PARTITION_SIZE
#endif

// APP头
extern const header_t Header;

#endif
