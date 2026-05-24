#ifndef __FSBL_H__
#define __FSBL_H__

#include "common.h"
#include "header.h"

// FSBL头
extern const header_t Header;

void FSBL_Boot(void);

void FSBL_Download(void);

#endif
