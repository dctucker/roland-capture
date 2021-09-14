#include <math.h>
#include "lib/memory.h"
#include "test.h"

bool test_addr_coord(capmix_coord_t expected, capmix_addr_t addr)
{
	capmix_coord_t coord = capmix_addr_coord(addr);
	printf("0x%08x -> { section = 0x%x, offset = 0x%x }", addr, coord.section, coord.offset);
	return coord.section == expected.section
		&& coord.offset  == expected.offset;
}

bool test_readback( char *data, int len, capmix_addr_t addr )
{
	bool ret = 1;
	int      actual_len  = capmix_memory_set( addr, data, len );
	uint8_t *actual_data = capmix_memory_get( addr );

	printf("len=%d, data=", actual_len);
	for(int i=0; i < actual_len; i++) printf("0x%02x ", actual_data[i]);
	printf(" expected len=%d", len);

	if( actual_len != len )
		ret = 0;
	else
	{
		printf(", data=");
		for(int i=0; i < len; i++)
		{
			printf("0x%02x ", data[i]);
			ret = ret && data[i] == actual_data[i];
		}
	}
	return ret;
}

#define COORD(SEC,OFF) (capmix_coord_t){.section=SEC,.offset=OFF}
int main(int argc, char *argv[])
{
	bool b;
	printf("\nTesting memory...\n");

	TEST( test_addr_coord(COORD(6, 0x24e), 0x0006120e) );
	TEST( test_addr_coord(COORD(5, 0x0e7), 0x00050707) );

	char data[] = { 0x20, 0x7f, 0x7f, 0x7f };
	TEST( test_readback( data, 4, 0x00060004) );
	TEST( test_readback( data, 4, 0x00061104) );

	DONE;
}
