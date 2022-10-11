#include "lib/capture.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	capmix_set_model(MStudio);
	capmix_print_map(NULL, "", 0);

	char name[64];
	capmix_format_addr(0x8100d, name);
	capmix_type_t type = capmix_addr_type(0x8100d);
	printf("name: %s, type: %d\n", name, type);
}
