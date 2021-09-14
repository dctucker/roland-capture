#pragma once

#include "lib/types.h"
#include "lib/capture.h"

typedef struct capmix_mixer_page {
	int rows;
	int cols;
	const char *name;
	const char **headers;
	const char **labels;
	capmix_addr_t controls[8][16];
} capmix_MixerPage;

enum capmix_pages {
	PInputA,
	PInputB,
	PInputC,
	PInputD,
	POutputA,
	POutputB,
	POutputC,
	POutputD,
	PPreamp,
	PCompressor,
	PReverb,
	PLine,
	PPatchbay,
};
#define N_Pages PPatchbay

const capmix_MixerPage *  capmix_get_page(enum capmix_pages page);

//extern const capmix_MixerPage  capmix_mixer_pages[];
