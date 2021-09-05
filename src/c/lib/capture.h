#include <stddef.h>
#include "types.h"

#define DEF_MEMAREA(NAME) MemMap NAME ## _area[]
#define MEMAREA(NAME) .area=(MemMap **)& NAME ## _area
#define OFFSET_AREA( OFFSET, NAME ) { .offset=OFFSET, .name=#NAME, MEMAREA(NAME)}
#define None (0xffffffff)
#define ENDA { .offset=None }
#define MEMNODE(OFFSET, NAME) [OFFSET] = { .name = #NAME, .offset = OFFSET }

typedef struct memory_area {
	Addr offset;
	const char *name;
	struct memory_area **area;
} MemMap;

void print_map(struct memory_area *map, char *prefix, Addr old_offset);
MemMap * lookup_map(MemMap *map, char *part);
u32 name_addr(const char *desc);
void addr_name(Addr addr, char *desc);
