#pragma once

#include "common.h"

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
	PChannel1,   ///< Channel controls
	PChannel2,   ///< Channel controls
	PChannel3,   ///< Channel controls
	PChannel4,   ///< Channel controls
	PChannel5,   ///< Channel controls
	PChannel6,   ///< Channel controls
	PChannel7,   ///< Channel controls
	PChannel8,   ///< Channel controls
	PChannel9,   ///< Channel controls
	PChannel10,  ///< Channel controls
	PChannel11,  ///< Channel controls
	PChannel12,  ///< Channel controls
	PChannel13,  ///< Channel controls
	PChannel14,  ///< Channel controls
	PChannel15,  ///< Channel controls
	PChannel16,  ///< Channel controls
	N_Pages,     ///< total number of mixer pages
};

/// structure describing the visual layout of a page in the mixer
typedef struct capmix_mixer_page_s {
	enum capmix_pages_e id;        ///< primary key, identifies a mixer page
	int rows;                      ///< number of rows with controls
	int cols;                      ///< maximum number of columns with controls
	const char *name;              ///< human-readable name of this page
	const char **headers;          ///< header shown above each column
	const char **labels;           ///< header shown next to each row
	capmix_addr_t controls[8][16]; ///< memory address matrix for controls
	int meter_rows;                ///< number of rows with meters
	capmix_addr_t meters[4][16];   ///< memory address matrix for meters
} capmix_mixer_page_t;

const capmix_mixer_page_t *  capmix_get_page(enum capmix_pages_e page);

void capmix_mixer_foreach(const capmix_mixer_page_t *page, void (*func)(capmix_addr_t, int, int));
cursor_t capmix_mixer_addr_xy( const capmix_mixer_page_t *page, capmix_addr_t addr );
int capmix_mixer_rowlen( const capmix_mixer_page_t *page, int row );
