#include <math.h>
#include "lib/types.h"
#include "lib/capture.h"
#include "test.h"

bool test_name_addr   (capmix_addr_t expected, const char *desc)
{
	capmix_addr_t actual = capmix_parse_addr(desc);
	printf("%s -> 0x%08x  expected 0x%08x", desc, actual, expected);
	return actual == expected;
}

bool test_format_addr (const char *expected, capmix_addr_t addr)
{
	char actual[64];
	capmix_format_addr(addr, actual);
	printf("format_addr 0x%08x -> %s  expected %s", addr, actual, expected);
	return strcmp(actual, expected) == 0;
}

bool test_addr_type   (capmix_type_t expected, capmix_addr_t addr)
{
	capmix_type_t type = capmix_addr_type(addr);
	const char *name = capmix_type_name(type);
	const char *expected_name = capmix_type_name(expected);
	printf("addr_type %08x -> %s  expected %s", addr, name, expected_name);
	return type == expected;
}

int main(int argc, char *argv[])
{
	printf("\nTesting capture...\n");

	TEST( test_name_addr(0x00040000, "reverb.type") );
	TEST( test_format_addr("reverb.type", 0x00040000) );

	TEST( test_name_addr(0x00060000, "input_monitor.a.channel.1.stereo") );
	TEST( test_name_addr(0x00071208, "daw_monitor.b.channel.3.volume") );
	TEST( test_name_addr(0x0006230e, "input_monitor.c.channel.4.reverb") );
	TEST( test_name_addr(None, "daw_monitor.d.channel.5.reverb") );
	TEST( test_name_addr(None, "daw_monitor.e.reverb") );
	TEST( test_format_addr("input_monitor.b.channel.3.reverb", 0x0006120e) );
	TEST( test_format_addr("input_monitor.a.channel.1.stereo", 0x00060000) );
	TEST( test_addr_type(TBoolean, 0x00060000) );
	TEST( test_addr_type(TVolume , 0x00062108) );
	TEST( test_addr_type(TBoolean    , 0x00051000) );
	TEST( test_addr_type(TAttenuation, 0x00051001) );
	TEST( test_format_addr("line.channel.13.stereo", 0x00051000) );
	TEST( test_format_addr("line.channel.13.attenuation", 0x00051001) );

	TEST( test_format_addr("master.direct_monitor.a.left.volume" , 0x00080001) );
	TEST( test_format_addr("master.direct_monitor.a.right.volume", 0x00080101) );
	TEST( test_addr_type(TBoolean, 0x00060000) );
	TEST( test_addr_type(TVolume , 0x00062108) );

	TEST( test_format_addr("master.daw_monitor.a.left.volume" , 0x00090001) );
	TEST( test_format_addr("master.daw_monitor.a.right.volume", 0x00090101) );
	TEST( test_addr_type(TBoolean    , 0x00080000) );
	TEST( test_addr_type(TVolume     , 0x00080001) );
	TEST( test_addr_type(TBoolean    , 0x00090000) );
	TEST( test_addr_type(TVolume     , 0x00090001) );
	TEST( test_name_addr(0x00080000, "master.direct_monitor.a.left.stereo") );
	TEST( test_name_addr(0x00080101, "master.direct_monitor.a.right.volume") );
	TEST( test_name_addr(0x00090000, "master.daw_monitor.a.left.stereo") );
	TEST( test_name_addr(0x00090101, "master.daw_monitor.a.right.volume") );

	TEST( test_format_addr("meters.active", 0x000a0000) );
	TEST( test_name_addr(0x000a0000, "meters.active" ) );
	TEST( test_addr_type(TBoolean, 0x000a0000) );

	TEST( test_addr_type(TMeter, 0x000a0001) );
	TEST( test_addr_type(TMeter, 0x000a0003) );
	TEST( test_format_addr("meters.channel.1", 0x000a0001) );
	TEST( test_format_addr("meters.channel.2", 0x000a0003) );
	TEST( test_format_addr("meters.more.1", 0x000a0101) );

	TEST( test_addr_type(TValue, 0x000a3f3f) );
	TEST( test_format_addr("meters.channel.?", 0x000a3f3f) );
}
