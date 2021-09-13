#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define TEST(x) printf("\033[s  "); if( !(x) ) fail(); else pass()
#define None capmix_None

int n_failed = 0;
int n_passed = 0;

void fail()
{
	printf("\033[u\033[31m✘\033[0m\n");
	++n_failed;
}

void pass()
{
	printf("\033[u\033[32m✔\033[0m\n");
	++n_passed;
}


int done()
{
	printf("\nPassed: %d, Failed: %d\n", n_passed, n_failed);
	return n_failed == 0 ? 0 : 1;
}
