#pragma once

#include <stddef.h>
#include "types.h"

#define DEF_MEMAREA(NAME) MemMap NAME ## _area[]
#define MEMAREA(NAME) .area=(MemMap **)& NAME ## _area
#define OFFSET_AREA( OFFSET, NAME ) { .offset=OFFSET, .name=#NAME, MEMAREA(NAME)}
#define None (0xffffffff)
#define ENDA { .offset=None }
#define MEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = #NAME, .offset = OFFSET, .type = T##TYPE }

#define O_PATCHBAY  0x00030000
#define O_REVERB    0x00040000
#define O_PREAMP    0x00050000
#define O_LINE      0x00051000
#define O_INPUT_MON 0x00060000
#define O_DAW_MON   0x00070000
#define O_MASTER    0x00080000
#define O_MON_A     0x00001000
#define O_MON_B     0x00002000
#define O_MON_C     0x00003000
#define O_MON_D     0x00004000

typedef struct memory_area {
	Addr offset;
	const char *name;
	ValueType type;
	struct memory_area **area;
} MemMap;

void      print_map(struct memory_area *map, char *prefix, Addr old_offset);
MemMap *  lookup_map(MemMap *map, char *part);
u32       name_addr(const char *desc);
void      addr_name(Addr addr, char *desc);
ValueType addr_type(Addr addr);
