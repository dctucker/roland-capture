#pragma once

#include <stddef.h>
#include "types.h"
#include "memory.h"

/// top and intermediate level device memory offsets
enum memory_offset_e {
	O_PATCHBAY  = 0x00030000, ///< patchbay offset
	O_REVERB    = 0x00040000, ///< reverb offset
	O_PREAMP    = 0x00050000, ///< preamp offset
	O_LINE      = 0x00051000, ///< line input offset
	O_INPUT_MON = 0x00060000, ///< input monitor offset
	O_DAW_MON   = 0x00070000, ///< DAW output monitor offset
	O_MASTER    = 0x00080000, ///< master channel offset
	O_MON_A     = 0x00001000, ///< monitor A offset
	O_MON_B     = 0x00002000, ///< monitor B offset
	O_MON_C     = 0x00003000, ///< monitor C offset
	O_MON_D     = 0x00004000, ///< monitor D offset
	capmix_None = 0xffffffff, ///< indicates end of memory area
};

/// tree structure for describing areas of device memory
typedef struct capmix_memory_area_s {
	capmix_addr_t offset;   ///< device memory address offset of current area
	capmix_type_t type;     ///< type of value stored at this address
	const char *const name; ///< name of this area of memory
	const struct capmix_memory_area_s **const area; ///< areas of memory contained within this area
} capmix_mem_t;

/// collection of strings used in memory area names
typedef struct capmix_str_s {
	const char *const top_map[16];
	const char *const patchbay[5];
	const char *const type;
	const char *const reverb_types[6];
	const char *const preamp_params[15];
	const char *const reverb_params[3];
	const char *const monitors[4];
	const char *const channel;
	const char *const channels[16];
	const char *const channel_params[15];
	const char *const master;
	const char *const master_channels[2];
	const char *const left_right[2];
	const char *const master_params[2];
	const char *const reverb_return;
	const char *const link;
} capmix_str_t;

void                   capmix_print_map(capmix_mem_t *map, char *prefix, capmix_addr_t old_offset);
static capmix_mem_t *  capmix_lookup_map(capmix_mem_t *map, char *part);

_API uint32_t          capmix_parse_addr  (const char *desc);
_API void              capmix_format_addr (capmix_addr_t addr, char *desc);
_API capmix_type_t     capmix_addr_type   (capmix_addr_t addr);
_API const char *      capmix_addr_suffix (capmix_addr_t addr);
