#pragma once

#include <stddef.h>
#include "types.h"

#define capmix_None (0xffffffff)

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
	const char *const name;
	const struct capmix_memory_area **const area;
} capmix_MemMap;

struct capmix_str {
	const char
		*const top_map[16],
		*const patchbay[5],
		*const type,
		*const reverb_types[6],
		*const preamp_params[15],
		*const reverb_params[3],
		*const monitors[4],
		*const channel,
		*const channels[16],
		*const channel_params[15],
		*const master,
		*const master_channels[2],
		*const left_right[2],
		*const master_params[2],
		*const reverb_return,
		*const link
	;
};

void              capmix_print_map(capmix_MemMap *map, char *prefix, capmix_Addr old_offset);
capmix_MemMap *   capmix_lookup_map(capmix_MemMap *map, char *part);
uint32_t          capmix_name_addr(const char *desc);
void              capmix_addr_name(capmix_Addr addr, char *desc);
capmix_ValueType  capmix_addr_type(capmix_Addr addr);

extern const capmix_MemMap memory_map[];
