#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	char name[16];
	char version[16];
	char date[16];
	char time[16];
	uint32_t start_addr;
	
} header_t;


#ifdef __cplusplus
}
#endif

#endif /* HEADER_H */
