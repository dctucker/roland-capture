#pragma once

#include "lib/types.h"
#include "lib/capture.h"

typedef struct capmix_mixer_page {
	int rows;
	int cols;
	const char *name;
	const char **headers;
	const char **labels;
	capmix_Addr controls[8][16];
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
	N_Pages,
};

void capmix_print_page(const capmix_MixerPage *);

extern const capmix_MixerPage capmix_mixer_pages[];
