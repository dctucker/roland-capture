#include <stdio.h>
#include <string.h>
#include "lib/capture.h"

void print_map(struct memory_area *map, char *prefix, u32 old_offset)
{
	for(int i = 0; map[i].offset != 0xffffffff; i++ ){
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

		print_map((struct memory_area *)(map[i].area), new_prefix, old_offset + offset);
	}
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

	char prefix[256];
	print_map(memory_map, prefix, 0);
	return 0;
}
