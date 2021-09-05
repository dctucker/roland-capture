#include "memory.h"

Memory memory;

Coord addr_coord(Addr addr)
{
	return (Coord){
		.section = ((addr & 0x0f000000) >> 4) | (addr & 0x000f0000),
		.offset  = ((addr & 0x0000ff00) >> 3) | (addr & 0x000000ff),
	};
}

void memory_init()
{
	for(int s=0; s < N_MEMSECTION; s++)
		for(int o=0; o < N_MEMBUF; o++)
			memory.section[s].buffer[o] = Unset;
}

int memory_erase(Addr addr, size_t len)
{
	Coord coord = addr_coord(addr);
	if( coord.section > N_MEMSECTION || len + coord.offset > N_MEMBUF )
		return 0;
	u8 *mem = &(memory.section[coord.section].buffer[coord.offset]);
	for(int i=0; i < len; i++)
		mem[i] = Unset;
	return 1;
}

u8 *memory_get(Addr addr)
{
	Coord coord = addr_coord(addr);
	u8 *mem = &( memory.section[coord.section].buffer[coord.offset] );
	return mem;
}

int memory_set(Addr addr, u8 *data, size_t len)
{
	Coord coord = addr_coord(addr);
	if( coord.section > N_MEMSECTION || len + coord.offset > N_MEMBUF )
		return 0;
	u8 *mem = &(memory.section[coord.section].buffer[coord.offset]);
	for(int i=0; i < len; i++)
		mem[i] = data[i];
	return 1;
}

