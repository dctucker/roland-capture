#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "lib/mixer.h"
#include "test.h"

void print_page(const capmix_mixer_page_t *page)
{
	printf("\n");
	for(int col = 0; col < page->cols; col++)
	{
		printf("%s\t", page->headers[col]);
	}
	printf("\n");
	for(int row = 0; row < page->rows; row++)
	{
		for(int col = 0; col < page->cols; col++)
		{
			capmix_addr_t control = page->controls[row][col];
			if( control == 0 )
				printf("\t");
			else
				printf("%x\t", control);
		}
		printf("%s\n", page->labels[row]);
	}
	printf("\n");
}

bool test_mixer(const capmix_mixer_page_t *mixer_page)
{
	//char *control = input_pages[0].controls[0][1];
	{
		int bak, new;

		fflush(stdout);
		bak = dup(1);
		new = open("/dev/null", O_WRONLY);
		dup2(new, 1);
		close(new);
		{
			print_page(mixer_page);
		}
		fflush(stdout);
		dup2(bak, 1);
		close(bak);
	}
	printf("# %s", mixer_page->name);
	return 1;
}

void run_mixer_tests()
{
	for(int i = 0; i < N_Pages; i++)
	{
		const capmix_mixer_page_t *mixer_page = capmix_get_page(i);
		TEST( test_mixer(mixer_page) );
		//printf("\n"); for(int i=0; i < mixer_page->rows; i++) printf("\n");
	}
}

int main()
{
	printf("\nTesting mixer...\n");
	run_mixer_tests();

	//init_mixer_pages();
	//printf("init_mixer_pages");

	DONE;
}
