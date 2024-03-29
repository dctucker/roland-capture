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

bool test_addr_suffix (const char *expected, capmix_addr_t addr)
{
	const char *actual = capmix_addr_suffix(addr);
	printf("addr_suffix 0x%08x -> %s  expected %s", addr, actual, expected);
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

void main_octa()
{
	capmix_set_model(MOcta);
	printf("\nTesting OCTA-CAPTURE...\n");

	TEST( test_name_addr(0x00040000, "reverb.type") );
	TEST( test_format_addr("reverb.type", 0x00040000) );
	TEST( test_addr_suffix("type", 0x00040000) );
	TEST( test_addr_suffix("volume", 0x00071208) );

	TEST( test_name_addr(0x00060000, "input_monitor.a.channel.1.stereo") );
	TEST( test_name_addr(0x00071208, "daw_monitor.b.channel.3.volume") );
	TEST( test_name_addr(0x0006230e, "input_monitor.c.channel.4.reverb") );
	TEST( test_name_addr(None, "daw_monitor.d.channel.5.reverb") );
	TEST( test_name_addr(None, "daw_monitor.e.reverb") );
	TEST( test_format_addr("input_monitor.b.channel.3.reverb", 0x0006120e) );
	TEST( test_format_addr("input_monitor.a.channel.1.stereo", 0x00060000) );
	TEST( test_addr_type(TBoolean, 0x00060000) );
	TEST( test_addr_type(TVolume , 0x00062108) );
	TEST( test_addr_type(TValue    , 0x00051000) );
	TEST( test_addr_type(TValue, 0x00051001) );
	TEST( test_format_addr("?", 0x00051000) );
	TEST( test_format_addr("?", 0x00051001) );

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
	/*
	TEST( test_format_addr("meters.pre.channel.1.level", 0x000a0001) );
	TEST( test_format_addr("meters.pre.channel.2.level", 0x000a0003) );
	TEST( test_format_addr("meters.post.channel.1.level", 0x000a0019) );
	TEST( test_format_addr("meters.post.channel.2.level", 0x000a001b) );
	TEST( test_format_addr("meters.line.channel.13.level", 0x000a0031) );
	TEST( test_format_addr("meters.line.channel.14.level", 0x000a0033) );
	TEST( test_format_addr("meters.line.channel.15.level", 0x000a0035) );
	TEST( test_format_addr("meters.line.channel.16.level", 0x000a0037) );
	TEST( test_format_addr("meters.daw.channel.1.level", 0x000a0039) );
	TEST( test_format_addr("meters.daw.channel.10.level", 0x000a004b) );
	TEST( test_format_addr("meters.gr.channel.1.level", 0x000a004d) );
	TEST( test_format_addr("meters.gate.channel.1.level", 0x000a0065) );
	TEST( test_format_addr("meters.clip.pre.1-4.mask"  , 0x000a0101) );
	TEST( test_format_addr("meters.clip.post.9-12.mask", 0x000a0107) );
	TEST( test_format_addr("meters.clip.post.13-16.?", 0x000a3f3f) );
	*/

	TEST( test_addr_type(TValue, 0x000a3f3f) );
}

void main_studio()
{
	capmix_set_model(MStudio);
	printf("\nTesting STUDIO-CAPTURE...\n");

	TEST( test_name_addr(0x00040000, "reverb.type") );
	TEST( test_format_addr("reverb.type", 0x00040000) );
	TEST( test_addr_suffix("type", 0x00040000) );
	TEST( test_addr_suffix("volume", 0x00071208) );

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
	TEST( test_format_addr("meters.pre.channel.1.level", 0x000a0001) );
	TEST( test_format_addr("meters.pre.channel.2.level", 0x000a0003) );
	TEST( test_format_addr("meters.post.channel.1.level", 0x000a0019) );
	TEST( test_format_addr("meters.post.channel.2.level", 0x000a001b) );
	TEST( test_format_addr("meters.line.channel.13.level", 0x000a0031) );
	TEST( test_format_addr("meters.line.channel.14.level", 0x000a0033) );
	TEST( test_format_addr("meters.line.channel.15.level", 0x000a0035) );
	TEST( test_format_addr("meters.line.channel.16.level", 0x000a0037) );
	TEST( test_format_addr("meters.daw.channel.1.level", 0x000a0039) );
	TEST( test_format_addr("meters.daw.channel.10.level", 0x000a004b) );
	TEST( test_format_addr("meters.gr.channel.1.level", 0x000a004d) );
	TEST( test_format_addr("meters.gate.channel.1.level", 0x000a0065) );
	TEST( test_format_addr("meters.clip.pre.1-4.mask"  , 0x000a0101) );
	TEST( test_format_addr("meters.clip.post.9-12.mask", 0x000a0107) );

	TEST( test_addr_type(TValue, 0x000a3f3f) );
	TEST( test_format_addr("meters.clip.post.13-16.?", 0x000a3f3f) );
}
int main(int argc, char *argv[])
{
	main_octa();
	main_studio();
}
