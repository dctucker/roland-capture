#include "lib/types.h"
#include "comm.h"

int main(int argc, const char **argv)
{
	setup_midi();

	while(1)
	{
		read_midi();
	}
	cleanup_midi();
	return 0;
}

