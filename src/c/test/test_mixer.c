#include <stdio.h>
#include <string.h>
#include "lib/mixer.h"
#include "test.h"

bool test_mixer(MixerPage *mixer_page)
{
	//char *control = input_pages[0].controls[0][1];
	printf("# %s", mixer_page->name);
	print_page(mixer_page);
	return 1;
}

bool test_mixers()
{
	for(int i = 0; i < N_Pages; i++)
	{
		MixerPage *mixer_page = &mixer_pages[i];
		TEST( test_mixer(mixer_page) );
		printf("\n");
		for(int i=0; i <= mixer_page->rows; i++)
			printf("\n");
	}
}

int main()
{
	bool b;
	TEST( test_mixers() );

	//init_mixer_pages();
	//printf("init_mixer_pages");

	printf("Done.\n\n"); return exit_code;
}
