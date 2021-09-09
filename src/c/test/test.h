#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define TEST(x) printf("\033[s  "); if( !(x) ) fail(); else pass()
#define None capmix_None

int exit_code = 0;

void fail()
{
	printf("\033[u\033[31m✘\033[0m\n");
	exit_code = 1;
}

void pass()
{
	printf("\033[u\033[32m✔\033[0m\n");
}

