#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib/capture.h"

#define debug(X) printf("%s\n", X);

void print_map(struct memory_area *map, char *prefix, Addr old_offset)
{
	for(int i = 0; map[i].offset != 0xffffffff; i++ )
	{
		if( map[i].name == NULL ) continue;
		u32 offset = map[i].offset;
		const char *name = map[i].name;
		char new_prefix[256];
		if( strlen(prefix) == 0 )
			sprintf(new_prefix, "%s", name);
		else
			sprintf(new_prefix, "%s.%s", prefix, name);

		if( map[i].area == NULL )
		{
			printf("0x%08x %s\n", old_offset + offset, new_prefix);
			continue;
		}

		print_map((MemMap *)(map[i].area), new_prefix, old_offset + offset);
	}
}

MemMap * lookup_map(MemMap *map, char *part)
{
	for(int i = 0; map[i].offset != None; i++ )
	{
		if( map[i].name == NULL ) continue;
		if( strcmp(map[i].name, part) != 0 ) continue;

		//debug(map[i].name);
		return &map[i];
	}
}


u32 name_addr(const char *desc)
{
	Addr ret = 0;
	MemMap *map = (MemMap *)(memory_map);
	char *desc_ = strdup(desc);
	char *tok = strtok(desc_, ".");
	while( tok != NULL )
	{
		map = lookup_map(map, tok);
		if( map == NULL )
		{
			return None;
		}
		ret += map->offset;
		//printf("0x%08x\n", ret);
		tok = strtok(NULL, ".");
		map = (MemMap *)(map->area);
	}
	free(desc_);
	return ret;
}

bool test_name_addr(Addr expected, const char *desc)
{
	Addr addr = name_addr(desc);
	printf("%s -> 0x%08x\n", desc, addr);
	return addr == expected;
}

void fail()
{
	printf("FAIL\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	/*
	input_channel in1 = {
		.stereo = 1,
		.solo = 1,
		.mute = 1,
		.pan = { 2,0,0,0 },
		.volume = { .value = {2,0,0,0,0,0} },
		.reverb = { .value = {1,0,0,0,0,0} }
	};
	for(int i=0; i < 20; i++)
	{
		printf("%02x ", in1.bytes[i]);
	}
	printf("hello\n");
	*/

	//char prefix[256];
	//print_map(memory_map, prefix, 0);

	bool b = test_name_addr(0x01071208, "daw_monitor.b.channel.3.volume");
	if( ! b ) fail();
	return 0;
}

