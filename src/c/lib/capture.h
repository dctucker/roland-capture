#pragma once

#include <stddef.h>
#include "types.h"

#define DEF_MEMAREA(NAME) capmix_MemMap NAME ## _area[]
#define MEMAREA(NAME) .area=(capmix_MemMap **)& NAME ## _area
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

typedef struct capmix_memory_area {
	capmix_Addr offset;
	capmix_ValueType type;
	const char *name;
	struct capmix_memory_area **area;
} capmix_MemMap;

void              capmix_print_map(capmix_MemMap *map, char *prefix, capmix_Addr old_offset);
capmix_MemMap *   capmix_lookup_map(capmix_MemMap *map, char *part);
u32               capmix_name_addr(const char *desc);
void              capmix_addr_name(capmix_Addr addr, char *desc);
capmix_ValueType  capmix_addr_type(capmix_Addr addr);
