#include <math.h>
#include "lib/memory.h"
#include "test.h"

bool test_addr_coord(capmix_Coord expected, capmix_Addr addr)
{
	capmix_Coord coord = capmix_addr_coord(addr);
	printf("0x%08x -> Coord{.section=%x,.offset=%x}", addr, coord.section, coord.offset);
	return coord.section == expected.section
		&& coord.offset  == expected.offset;
}

int main(int argc, char *argv[])
{
	bool b;
	printf("\nTesting memory...\n");

	TEST( test_addr_coord((capmix_Coord){.section=6,.offset=0x24e}, 0x0006120e) );

	printf("Done.\n\n"); return exit_code;
}
