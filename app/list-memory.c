#include "lib/capture.h"

int main(int argc, char **argv)
{
	capmix_set_model(MStudio);
	capmix_print_map(NULL, "", 0);
}
