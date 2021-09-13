#include <math.h>
#include "lib/types.h"
#include "lib/capture.h"
#include "test.h"

bool test_name_addr   (capmix_Addr expected, const char *desc)
{
	capmix_Addr actual = capmix_parse_addr(desc);
	printf("%s -> 0x%08x  expected 0x%08x", desc, actual, expected);
	return actual == expected;
}

bool test_format_addr (const char *expected, capmix_Addr addr)
{
	char actual[256];
	capmix_format_addr(addr, actual);
	printf("0x%08x -> %s  expected %s", addr, actual, expected);
	return strcmp(actual, expected) == 0;
}

bool test_addr_type   (capmix_ValueType expected, capmix_Addr addr)
{
	capmix_ValueType type = capmix_addr_type(addr);
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
	TEST( test_addr_type(TVolume, 0x00062108) );
}
