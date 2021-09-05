#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib/capture.h"

#define debug(X) printf("%s\n", X);

bool test_name_addr(Addr expected, const char *desc)
{
	Addr actual = name_addr(desc);
	printf("%s -> 0x%08x", desc, actual);
	return actual == expected;
}

bool test_addr_name(const char *expected, Addr addr)
{
	char actual[256];
	addr_name(addr, actual);
	printf("0x%08x -> %s", addr, actual);
	return strcmp(actual, expected) == 0;
}

void fail()
{
	printf("\033[u\033[31m✘\033[0m\n");
	exit(1);
}

void pass()
{
	printf("\033[u\033[32m✔\033[0m\n");
}

#define TEST(x) printf("\033[s  "); if( !(x) ) fail(); else pass()

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

	printf("\nTesting C library...\n");
	bool b;
	TEST( test_name_addr(0x00071208, "daw_monitor.b.channel.3.volume") );
	TEST( test_name_addr(0x0006230e, "input_monitor.c.channel.4.reverb") );
	TEST( test_name_addr(None, "daw_monitor.d.channel.5.reverb") );
	TEST( test_name_addr(None, "daw_monitor.e.reverb") );
	TEST( test_addr_name("input_monitor.b.channel.3.reverb", 0x0006120e) );
	printf("Done.\n\n");
	return 0;
}

