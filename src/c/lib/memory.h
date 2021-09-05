#pragma once

#include "types.h"
#define N_MEMSECTION 32
#define N_MEMBUF   2048

typedef struct memory {
	struct memory_section {
		u8 buffer[N_MEMBUF];
	} section[N_MEMSECTION];
} Memory;

typedef struct memory_coord {
	int section;
	int offset;
} Coord;

Coord addr_coord(Addr addr);
void  memory_init();
int   memory_erase(Addr addr, size_t len);
u8 *  memory_get(Addr addr);
int   memory_set(Addr addr, u8 *data, size_t len);

