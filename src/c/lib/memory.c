#include "memory.h"

static capmix_Memory memory;

capmix_coord_t  capmix_addr_coord(capmix_addr_t addr)
{
	int upper = ((addr & 0x0f000000) >> 4) | (addr & 0x000f0000);
	return (capmix_coord_t){
		.section = upper >> 16,
		.offset  = ((addr & 0x0000ff00) >> 3) | (addr & 0x0000001f),
	};
}

void            capmix_memory_init()
{
	for(int s=0; s < N_MEMSECTION; s++)
		for(int o=0; o < N_MEMBUF; o++)
			memory.section[s].buffer[o] = capmix_Unset;
}

int             capmix_memory_erase(capmix_addr_t addr, int len)
{
	capmix_coord_t coord = capmix_addr_coord(addr);
	if( coord.section > N_MEMSECTION || len + coord.offset > N_MEMBUF )
		return 0;
	uint8_t *mem = &(memory.section[coord.section].buffer[coord.offset]);
	for(int i=0; i < len; i++)
		mem[i] = capmix_Unset;
	return 1;
}

uint8_t *       capmix_memory_get(capmix_addr_t addr)
{
	capmix_coord_t coord = capmix_addr_coord(addr);
	uint8_t *mem = &( memory.section[coord.section].buffer[coord.offset] );
	return mem;
}

int             capmix_memory_set(capmix_addr_t addr, uint8_t *data, int len)
{
	capmix_coord_t coord = capmix_addr_coord(addr);
	if( coord.section > N_MEMSECTION || len + coord.offset > N_MEMBUF )
		return 0;
	uint8_t *mem = &(memory.section[coord.section].buffer[coord.offset]);
	for(int i=0; i < len; i++)
		mem[i] = data[i];
	return len;
}
