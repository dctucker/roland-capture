#pragma once

#include "types.h"
#define N_MEMSECTION 32
#define N_MEMBUF   2048

typedef struct capmix_memory {
	struct capmix_memory_section {
		uint8_t buffer[N_MEMBUF];
	} section[N_MEMSECTION];
} capmix_Memory;

typedef struct memory_coord {
	int section;
	int offset;
} capmix_Coord;

capmix_Coord          capmix_addr_coord          (capmix_Addr addr);
void                  capmix_memory_init         ();
int                   capmix_memory_erase        (capmix_Addr addr, int len);
uint8_t *             capmix_memory_get          (capmix_Addr addr);
int                   capmix_memory_set          (capmix_Addr addr, uint8_t *data, int len);
