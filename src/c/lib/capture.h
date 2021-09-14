#pragma once

#include <stddef.h>
#include "types.h"
#include "memory.h"

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
	capmix_addr_t offset;
	capmix_type_t type;
	const char *const name;
	const struct capmix_memory_area **const area;
} capmix_mem_t;

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

void                   capmix_print_map(capmix_mem_t *map, char *prefix, capmix_addr_t old_offset);
static capmix_mem_t *  capmix_lookup_map(capmix_mem_t *map, char *part);

_API uint32_t          capmix_parse_addr  (const char *desc);
_API void              capmix_format_addr (capmix_addr_t addr, char *desc);
_API capmix_type_t     capmix_addr_type   (capmix_addr_t addr);
_API const char *      capmix_addr_suffix (capmix_addr_t addr);
