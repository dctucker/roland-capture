#include "lib/types.h"
#include "lib/capture.h"
#include "lib/mixer.h"
#include "test.h"

#define debug(X) printf("%s\n", X);

bool test_name_addr(capmix_Addr expected, const char *desc)
{
	capmix_Addr actual = capmix_name_addr(desc);
	printf("%s -> 0x%08x  expected 0x%08x", desc, actual, expected);
	return actual == expected;
}

bool test_addr_name(const char *expected, capmix_Addr addr)
{
	char actual[256];
	capmix_addr_name(addr, actual);
	printf("0x%08x -> %s  expected %s", addr, actual, expected);
	return strcmp(actual, expected) == 0;
}


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

bool test_addr_type(capmix_ValueType expected, capmix_Addr addr)
{
	capmix_ValueType type = capmix_addr_type(addr);
	const char *name = capmix_type_name(type);
	const char *expected_name = capmix_type_name(expected);
	printf("addr_type %08x -> %s  expected %s", addr, name, expected_name);
	return type == expected;
}

bool test_parse(capmix_Unpacked expected, capmix_ValueType type, const char *str)
{
	const char *name = capmix_type_name(type);
	printf("parse_type %s %s -> ", name, str);
	capmix_Unpacked unpacked = capmix_parse_type(type, str);
	printf("0x%x / %f", unpacked.as_int, unpacked.as_float);
	printf("  expected 0x%x", expected.as_int );
	return expected.as_int == unpacked.as_int || expected.as_float == unpacked.as_float;
}

bool test_type(const char *expected, capmix_ValueType type, u8 *bytes)
{
	char str[256];
	fixed fx = capmix_fixed_from_packed(type, bytes);
	capmix_Unpacked unpacked = capmix_unpack_type(type, bytes);
	capmix_format_unpacked(type, unpacked, str);
	printf("typed value 0x%x -> %s  expected %s", fx, str, expected);
	return strcmp(str, expected) == 0;
}

bool test_format(const char *expected, capmix_ValueType type, capmix_Unpacked unpacked)
{
	char str[256];
	capmix_format_unpacked(type, unpacked, str);
	printf("format %s 0x%x -> %s  expected %s", capmix_type_name(type), unpacked.as_int, str, expected);
	return strcmp(str, expected) == 0;
}

bool test_pack(const u8 *expected, capmix_ValueType type, capmix_Unpacked unpacked)
{
	char str[256];
	char buf[6];
	capmix_pack_type(type, unpacked, buf);
	capmix_format_unpacked(type, unpacked, str);
	printf("unpacked %s %s -> ", capmix_type_name(type), str);
	int size = capmix_type_size(type);
	for(int i=0; i < size; i++)
		printf("0x%02x ", buf[i]);
	printf("  expected ");
	for(int i=0; i < size; i++)
		printf("0x%02x ", expected[i]);
	return memcmp(buf, expected, size) == 0;
}

bool test_volume_format(const char *expected, int fixed)
{
	char buf[6];
	capmix_to_nibbles(fixed, 6, buf);
	return test_type(expected, TVolume, buf);
}

bool test_true(bool pass, const char *message)
{
	printf("%s", message);
	return pass;
}

int main(int argc, char *argv[])
{
	//capmix_print_map(memory_map, "", 0);
	//return 0;
	bool b;
	printf("\nTesting C library...\n");

	TEST( test_name_addr(0x00040000, "reverb.type") );
	TEST( test_addr_name("reverb.type", 0x00040000) );

	TEST( test_name_addr(0x00071208, "daw_monitor.b.channel.3.volume") );
	TEST( test_name_addr(0x0006230e, "input_monitor.c.channel.4.reverb") );
	TEST( test_name_addr(None, "daw_monitor.d.channel.5.reverb") );
	TEST( test_name_addr(None, "daw_monitor.e.reverb") );
	TEST( test_addr_name("input_monitor.b.channel.3.reverb", 0x0006120e) );

	TEST( test_volume_format("+12",  0x800000) );
	TEST( test_volume_format("+0",   0x200000) );
	TEST( test_volume_format("-6",   0x100000) );
	TEST( test_volume_format("-inf", 0x000000) );

	char buf_zero[] = {2,0,0,0,0,0};
	char buf_min[] = {0,0,0,0,0,0};
	TEST( test_type("+0", TVolume, buf_zero ));

	TEST( test_pack(buf_zero, TVolume, capmix_UnpackedFloat(0.0)) );
	TEST( test_pack(buf_min, TVolume, capmix_UnpackedFloat(-inf)) );

	TEST( test_pack(buf_min , TByte, capmix_UnpackedInt(0)) );
	TEST( test_pack(buf_zero, TByte, capmix_UnpackedInt(2)) );

	char pan_max[] = { 8,0,0,0 };
	char pan_zero[] = { 4,0,0,0 };
	TEST( test_pack(buf_min, TPan, capmix_UnpackedFloat(-100.)) );
	TEST( test_pack(pan_zero, TPan, capmix_UnpackedFloat(0.)) );
	TEST( test_pack(pan_max, TPan, capmix_UnpackedFloat(100.)) );

	char bool_true[] = { 1 };
	char bool_false[] = { 0 };
	TEST( test_pack(bool_true, TBoolean, capmix_UnpackedInt(1)) );
	TEST( test_pack(bool_false, TBoolean, capmix_UnpackedInt(0)) );

	char ratio_2_5[] = { 7 };
	TEST( test_pack(ratio_2_5, TRatio, capmix_UnpackedInt(7)) );

	TEST( test_format("2.5", TRatio, capmix_UnpackedInt(7)) );
	TEST( test_format("?", TRatio, capmix_UnpackedInt(120)) );

	TEST( test_addr_type(TVolume, 0x00062108) );

	TEST( test_parse(capmix_UnpackedFloat(1.0), TVolume, "+1.0") );
	TEST( test_parse(capmix_UnpackedFloat(0.0), TVolume, "0") );
	TEST( test_parse(capmix_UnpackedFloat(50.0), TPan, "R50") );
	TEST( test_parse(capmix_UnpackedFloat(-50.0), TPan, "L50") );
	TEST( test_parse(capmix_UnpackedFloat(0), TPan, "C") );
	TEST( test_parse(capmix_UnpackedInt(capmix_Unset), TPan, "30") );
	TEST( test_true(capmix_UnpackedInt(capmix_Unset).as_float != capmix_UnpackedFloat(0.0).as_float, "UnpackedInt(capmix_Unset) != UnpackedFloat(0.0)") );

	printf("Done.\n\n"); return exit_code;
}

