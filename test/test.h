#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define TEST(x) printf("\033[s  "); if( !(x) ) fail(); else pass()
#define DONE return done()
#define None capmix_None
#define SILENT(X) { int prev_stdout, new_stdout; \
	fflush(stdout); prev_stdout = dup(1); \
	new_stdout = open("/dev/null", O_WRONLY); \
	dup2(new_stdout, 1); close(new_stdout); \
	X fflush(stdout); dup2(prev_stdout, 1); close(prev_stdout); }

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
	printf("Passed: %d, Failed: %d\n\n", n_passed, n_failed);
	return n_failed == 0 ? 0 : 1;
}
