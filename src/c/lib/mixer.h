#pragma once

#include "lib/types.h"
#include "lib/capture.h"

#define CCA(members...) (const char *[]){ members, 0 }

typedef struct mixer_page {
	int rows;
	int cols;
	const char *name;
	const char **headers;
	const char **labels;
	Addr controls[8][16];
} MixerPage;

enum mixer_pages {
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

void init_mixer_pages();
void print_page(MixerPage *);

extern MixerPage mixer_pages[];
