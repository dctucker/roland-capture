#pragma once

#include "common.h"

/// structure describing the visual layout of a page in the mixer
typedef struct capmix_mixer_page_s {
	int rows;                      ///< number of rows with controls
	int cols;                      ///< maximum number of columns with controls
	const char *name;              ///< human-readable name of this page
	const char **headers;          ///< header shown above each column
	const char **labels;           ///< header shown next to each row
	capmix_addr_t controls[8][16]; ///< memory address matrix
} capmix_mixer_page_t;

enum capmix_pages_e {
	PInputA,     ///< Direct Input Monitor A
	PInputB,     ///< Direct Input Monitor B
	PInputC,     ///< Direct Input Monitor C
	PInputD,     ///< Direct Input Monitor D
	POutputA,    ///< DAW Output Monitor A
	POutputB,    ///< DAW Output Monitor B
	POutputC,    ///< DAW Output Monitor C
	POutputD,    ///< DAW Output Monitor D
	PPreamp,     ///< Preamps
	PCompressor, ///< Compressors
	PReverb,     ///< Reverb Settings
	PLine,       ///< Line Inputs
	PPatchbay,   ///< Output routing Patchbay
	N_Pages,     ///< total number of mixer pages
};

const capmix_mixer_page_t *  capmix_get_page(enum capmix_pages_e page);

//extern const capmix_mixer_page_t  capmix_mixer_pages[];
