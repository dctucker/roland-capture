#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib/types.h"
#include "lib/capture.h"

#define debug(X) printf("%s\n", X);

void fail()
{
	printf("\033[u\033[31m✘\033[0m\n");
	exit(1);
}

void pass()
{
	printf("\033[u\033[32m✔\033[0m\n");
}

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

#define TEST(x) printf("\033[s  "); if( !(x) ) fail(); else pass()

/*
void test_input_channel()
{
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
}
*/

bool test_type(const char *expected, ValueType type, u8 *bytes)
{
	char str[256];
	Value val = { .as_value = bytes };
	fixed fx = fixed_from_packed(type, bytes);
	Unpacked unpacked = unpack_type(type, val);
	format_unpacked(type, unpacked, str);
	printf("typed value 0x%x -> %s, expected %s", fx, str, expected);
	return strcmp(str, expected) == 0;
}

bool test_pack(const u8 *expected, ValueType type, Unpacked unpacked)
{
	char str[256];
	char buf[6];
	pack_type(type, unpacked, buf);
	format_unpacked(type, unpacked, str);
	printf("unpacked %s %s -> ", type_name(type), str);
	int size = type_size(type);
	for(int i=0; i < size; i++)
		printf("0x%02x ", buf[i]);
	printf(", expected ");
	for(int i=0; i < size; i++)
		printf("0x%02x ", expected[i]);
	return memcmp(buf, expected, size) == 0;
}

bool test_volume_format(const char *expected, int fixed)
{
	char buf[6];
	to_nibbles(fixed, 6, buf);
	Value val = { .as_value = buf };
	return test_type(expected, TVolume, val.as_value);
}

int main(int argc, char *argv[])
{

	bool b;
	printf("\nTesting C library...\n");

	TEST( test_name_addr(0x00071208, "daw_monitor.b.channel.3.volume") );
	TEST( test_name_addr(0x0006230e, "input_monitor.c.channel.4.reverb") );
	TEST( test_name_addr(None, "daw_monitor.d.channel.5.reverb") );
	TEST( test_name_addr(None, "daw_monitor.e.reverb") );
	TEST( test_addr_name("input_monitor.b.channel.3.reverb", 0x0006120e) );

	TEST( test_volume_format("+12", 0x800000) );
	TEST( test_volume_format("+0", 0x200000) );
	TEST( test_volume_format("-6", 0x100000) );
	TEST( test_volume_format("-inf", 0x000000) );

	char buf_zero[] = {2,0,0,0,0,0};
	char buf_min[] = {0,0,0,0,0,0};
	TEST( test_type("+0", TVolume, buf_zero ));

	TEST( test_pack(buf_zero, TVolume, UnpackedFloat(0.0)) );
	TEST( test_pack(buf_min, TVolume, UnpackedFloat(-inf)) );

	TEST( test_pack(buf_min, TByte, UnpackedInt(0)) );
	TEST( test_pack(buf_zero, TByte, UnpackedInt(2)) );

	char pan_max[] = { 8,0,0,0 };
	char pan_zero[] = { 4,0,0,0 };
	TEST( test_pack(buf_min, TPan, UnpackedFloat(-100.)) );
	TEST( test_pack(pan_zero, TPan, UnpackedFloat(0.)) );
	TEST( test_pack(pan_max, TPan, UnpackedFloat(100.)) );

	printf("Done.\n\n"); return 0;
}

