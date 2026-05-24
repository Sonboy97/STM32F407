#ifndef __SSBL_H__
#define __SSBL_H__

#include "common.h"
#include "header.h"

// SSBL头
extern const header_t Header;

// APP分区标识
extern char AppPartitionID;

void SSBL_Boot(void);

#endif
