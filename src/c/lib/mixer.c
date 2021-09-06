#include <string.h>
#include <stdio.h>
#include "mixer.h"

char input_page_controls[6][16][256];
const char *input_headers[] = CCA( "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" );
const char *input_labels[] = CCA( "stereo","mute","solo","reverb","pan","volume" );
const char *output_headers[] = CCA( "1","2","3","4","5","6","7","8","9","10" );
const char *output_labels[] = CCA( "stereo","mute","solo","","pan","volume" );
const char *preamp_labels[] = CCA( "stereo","hi-z","+48","lo-cut","phase","sens","bypass","gate" );
MixerPage mixer_pages[] = {
	[PInputA] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.a",
		.headers = (const char **) input_headers,
		.labels = (const char **) input_labels,
	},
	[PInputB] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.b",
		.headers = (const char **) input_headers,
		.labels = (const char **) input_labels,
	},
	[PInputC] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.c",
		.headers = (const char **) input_headers,
		.labels = (const char **) input_labels,
	},
	[PInputD] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.d",
		.headers = (const char **) input_headers,
		.labels = (const char **) input_labels,
	},
	[POutputA] = {
		.rows = 6, .cols = 16,
		.name = "daw_monitor.a",
		.headers = (const char **) output_headers,
		.labels = (const char **) output_labels,
	},
	[POutputB] = {
		.rows = 6, .cols = 16,
		.name = "daw_monitor.b",
		.headers = (const char **) output_headers,
		.labels = (const char **) output_labels,
	},
	[POutputC] = {
		.rows = 6, .cols = 16,
		.name = "daw_monitor.c",
		.headers = (const char **) output_headers,
		.labels = (const char **) output_labels,
	},
	[POutputD] = {
		.rows = 6, .cols = 16,
		.name = "daw_monitor.d",
		.headers = (const char **) output_headers,
		.labels = (const char **) output_labels,
	},
	[PPreamp] = {
		.rows = 8, .cols = 12,
		.name = "preamp",
		.headers = (const char **) input_headers,
		.labels = (const char **) preamp_labels,
	},
};

void init_io_page(MixerPage *page)
{
	for(int row = 0; row < page->rows; row++)
	{
		for(int col = 0; col < page->cols; col++)
		{
			const char *control = page->labels[row];
			if( col % 2 == 1 && strcmp(control,"stereo") == 0 ) continue;
			if( strlen(control) == 0 ) continue;
			if( col >= 2 && strcmp(control,"hi-z") == 0 ) continue;
			sprintf( page->controls[row][col], "%s.channel.%d.%s", page->name, col+1, control );
		}
	}
}

void init_mixer_pages()
{
	//for(int i = 0; i < N_Pages; i++)
	//{
	init_io_page(&mixer_pages[PInputA]);
	init_io_page(&mixer_pages[PInputB]);
	init_io_page(&mixer_pages[PInputC]);
	init_io_page(&mixer_pages[PInputD]);
	init_io_page(&mixer_pages[POutputA]);
	init_io_page(&mixer_pages[POutputB]);
	init_io_page(&mixer_pages[POutputC]);
	init_io_page(&mixer_pages[POutputD]);
	init_io_page(&mixer_pages[PPreamp]);
	//}
}

void print_page(MixerPage *page)
{
	for(int col = 0; col < page->cols; col++)
	{
		printf("%s\t", page->headers[col]);
	}
	printf("\n");
	for(int row = 0; row < page->rows; row++)
	{
		for(int col = 0; col < page->cols; col++)
		{
			const char *control = page->controls[row][col];
			printf("%s\t", control);
		}
		printf("%s\n", page->labels[row]);
	}
}

/*
const char *pages = {
	"input_monitor.a": InputPage(self, 'a'),
	"input_monitor.b": InputPage(self, 'b'),
	"input_monitor.c": InputPage(self, 'c'),
	"input_monitor.d": InputPage(self, 'd'),
	"daw_monitor.a" : OutputPage(self, 'a'),
	"daw_monitor.b" : OutputPage(self, 'b'),
	"daw_monitor.c" : OutputPage(self, 'c'),
	"daw_monitor.d" : OutputPage(self, 'd'),
	"preamp"        : PreampPage(self),
	"compressor"    : CompressorPage(self),
	"line"          : LinePage(self),
	"reverb"        : ReverbPage(self),
	"patchbay"      : Patchbay(self),
};
*/
