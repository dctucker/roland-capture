#include <string.h>
#include <stdio.h>
#include "mixer.h"

static const char *channel_headers[]   = CCA( "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" );
static const char *input_labels[]      = CCA( "Stereo","Mute","Solo","Reverb","Pan","Volume" );
static const char *output_labels[]     = CCA( "Stereo","Mute","Solo","","Pan","Volume" );

static const capmix_MixerPage capmix_mixer_pages[] = {
	[PInputA] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.a",
		.headers = (const char **) channel_headers,
		.labels = (const char **) input_labels,
		.controls = {
			{0x60000, 0x60200, 0x60400, 0x60600, 0x60800, 0x60a00, 0x60c00, 0x60e00}, // stereo
			{0x60002, 0x60102, 0x60202, 0x60302, 0x60402, 0x60502, 0x60602, 0x60702, 0x60802, 0x60902, 0x60a02, 0x60b02, 0x60c02, 0x60d02, 0x60e02, 0x60f02}, // solo
			{0x60003, 0x60103, 0x60203, 0x60303, 0x60403, 0x60503, 0x60603, 0x60703, 0x60803, 0x60903, 0x60a03, 0x60b03, 0x60c03, 0x60d03, 0x60e03, 0x60f03}, // mute
			{0x6000e, 0x6010e, 0x6020e, 0x6030e, 0x6040e, 0x6050e, 0x6060e, 0x6070e, 0x6080e, 0x6090e, 0x60a0e, 0x60b0e, 0x60c0e, 0x60d0e, 0x60e0e, 0x60f0e}, // reverb
			{0x60004, 0x60104, 0x60204, 0x60304, 0x60404, 0x60504, 0x60604, 0x60704, 0x60804, 0x60904, 0x60a04, 0x60b04, 0x60c04, 0x60d04, 0x60e04, 0x60f04}, // pan
			{0x60008, 0x60108, 0x60208, 0x60308, 0x60408, 0x60508, 0x60608, 0x60708, 0x60808, 0x60908, 0x60a08, 0x60b08, 0x60c08, 0x60d08, 0x60e08, 0x60f08}, // volume
		},
	},
	[PInputB] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.b",
		.headers = (const char **) channel_headers,
		.labels = (const char **) input_labels,
		.controls = {
			{0x61000, 0x61200, 0x61400, 0x61600, 0x61800, 0x61a00, 0x61c00, 0x61e00}, // stereo
			{0x61002, 0x61102, 0x61202, 0x61302, 0x61402, 0x61502, 0x61602, 0x61702, 0x61802, 0x61902, 0x61a02, 0x61b02, 0x61c02, 0x61d02, 0x61e02, 0x61f02}, // solo
			{0x61003, 0x61103, 0x61203, 0x61303, 0x61403, 0x61503, 0x61603, 0x61703, 0x61803, 0x61903, 0x61a03, 0x61b03, 0x61c03, 0x61d03, 0x61e03, 0x61f03}, // mute
			{0x6100e, 0x6110e, 0x6120e, 0x6130e, 0x6140e, 0x6150e, 0x6160e, 0x6170e, 0x6180e, 0x6190e, 0x61a0e, 0x61b0e, 0x61c0e, 0x61d0e, 0x61e0e, 0x61f0e}, // reverb
			{0x61004, 0x61104, 0x61204, 0x61304, 0x61404, 0x61504, 0x61604, 0x61704, 0x61804, 0x61904, 0x61a04, 0x61b04, 0x61c04, 0x61d04, 0x61e04, 0x61f04}, // pan
			{0x61008, 0x61108, 0x61208, 0x61308, 0x61408, 0x61508, 0x61608, 0x61708, 0x61808, 0x61908, 0x61a08, 0x61b08, 0x61c08, 0x61d08, 0x61e08, 0x61f08}, // volume
		},
	},
	[PInputC] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.c",
		.headers = (const char **) channel_headers,
		.labels = (const char **) input_labels,
		.controls = {
			{0x62000, 0x62200, 0x62400, 0x62600, 0x62800, 0x62a00, 0x62c00, 0x62e00}, // stereo
			{0x62002, 0x62102, 0x62202, 0x62302, 0x62402, 0x62502, 0x62602, 0x62702, 0x62802, 0x62902, 0x62a02, 0x62b02, 0x62c02, 0x62d02, 0x62e02, 0x62f02}, // solo
			{0x62003, 0x62103, 0x62203, 0x62303, 0x62403, 0x62503, 0x62603, 0x62703, 0x62803, 0x62903, 0x62a03, 0x62b03, 0x62c03, 0x62d03, 0x62e03, 0x62f03}, // mute
			{0x6200e, 0x6210e, 0x6220e, 0x6230e, 0x6240e, 0x6250e, 0x6260e, 0x6270e, 0x6280e, 0x6290e, 0x62a0e, 0x62b0e, 0x62c0e, 0x62d0e, 0x62e0e, 0x62f0e}, // reverb
			{0x62004, 0x62104, 0x62204, 0x62304, 0x62404, 0x62504, 0x62604, 0x62704, 0x62804, 0x62904, 0x62a04, 0x62b04, 0x62c04, 0x62d04, 0x62e04, 0x62f04}, // pan
			{0x62008, 0x62108, 0x62208, 0x62308, 0x62408, 0x62508, 0x62608, 0x62708, 0x62808, 0x62908, 0x62a08, 0x62b08, 0x62c08, 0x62d08, 0x62e08, 0x62f08}, // volume
		},
	},
	[PInputD] = {
		.rows = 6, .cols = 16,
		.name = "input_monitor.d",
		.headers = (const char **) channel_headers,
		.labels = (const char **) input_labels,
		.controls = {
			{0x63000, 0x63200, 0x63400, 0x63600, 0x63800, 0x63a00, 0x63c00, 0x63e00}, // stereo
			{0x63002, 0x63102, 0x63202, 0x63302, 0x63402, 0x63502, 0x63602, 0x63702, 0x63802, 0x63902, 0x63a02, 0x63b02, 0x63c02, 0x63d02, 0x63e02, 0x63f02}, // solo
			{0x63003, 0x63103, 0x63203, 0x63303, 0x63403, 0x63503, 0x63603, 0x63703, 0x63803, 0x63903, 0x63a03, 0x63b03, 0x63c03, 0x63d03, 0x63e03, 0x63f03}, // mute
			{0x6300e, 0x6310e, 0x6320e, 0x6330e, 0x6340e, 0x6350e, 0x6360e, 0x6370e, 0x6380e, 0x6390e, 0x63a0e, 0x63b0e, 0x63c0e, 0x63d0e, 0x63e0e, 0x63f0e}, // reverb
			{0x63004, 0x63104, 0x63204, 0x63304, 0x63404, 0x63504, 0x63604, 0x63704, 0x63804, 0x63904, 0x63a04, 0x63b04, 0x63c04, 0x63d04, 0x63e04, 0x63f04}, // pan
			{0x63008, 0x63108, 0x63208, 0x63308, 0x63408, 0x63508, 0x63608, 0x63708, 0x63808, 0x63908, 0x63a08, 0x63b08, 0x63c08, 0x63d08, 0x63e08, 0x63f08}, // volume
		},
	},
	[POutputA] = {
		.rows = 6, .cols = 10,
		.name = "daw_monitor.a",
		.headers = (const char **) channel_headers,
		.labels = (const char **) output_labels,
		.controls = {
			{0x70000, 0x70200, 0x70400, 0x70600, 0x70800}, // stereo
			{0x70002, 0x70102, 0x70202, 0x70302, 0x70402, 0x70502, 0x70602, 0x70702, 0x70802, 0x70902}, // solo
			{0x70003, 0x70103, 0x70203, 0x70303, 0x70403, 0x70503, 0x70603, 0x70703, 0x70803, 0x70903}, // mute
			{0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0    }, // no reverb
			{0x70004, 0x70104, 0x70204, 0x70304, 0x70404, 0x70504, 0x70604, 0x70704, 0x70804, 0x70904}, // pan
			{0x70008, 0x70108, 0x70208, 0x70308, 0x70408, 0x70508, 0x70608, 0x70708, 0x70808, 0x70908}, // volume
		},
	},
	[POutputB] = {
		.rows = 6, .cols = 10,
		.name = "daw_monitor.b",
		.headers = (const char **) channel_headers,
		.labels = (const char **) output_labels,
		.controls = {
			{0x71000, 0x71200, 0x71400, 0x71600, 0x71800}, // stereo
			{0x71002, 0x71102, 0x71202, 0x71302, 0x71402, 0x71502, 0x71602, 0x71702, 0x71802, 0x71902}, // solo
			{0x71003, 0x71103, 0x71203, 0x71303, 0x71403, 0x71503, 0x71603, 0x71703, 0x71803, 0x71903}, // mute
			{0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0    }, // no reverb
			{0x71004, 0x71104, 0x71204, 0x71304, 0x71404, 0x71504, 0x71604, 0x71704, 0x71804, 0x71904}, // pan
			{0x71008, 0x71108, 0x71208, 0x71308, 0x71408, 0x71508, 0x71608, 0x71708, 0x71808, 0x71908}, // volume
		},
	},
	[POutputC] = {
		.rows = 6, .cols = 10,
		.name = "daw_monitor.c",
		.headers = (const char **) channel_headers,
		.labels = (const char **) output_labels,
		.controls = {
			{0x72000, 0x72200, 0x72400, 0x72600, 0x72800}, // stereo
			{0x72002, 0x72102, 0x72202, 0x72302, 0x72402, 0x72502, 0x72602, 0x72702, 0x72802, 0x72902}, // solo
			{0x72003, 0x72103, 0x72203, 0x72303, 0x72403, 0x72503, 0x72603, 0x72703, 0x72803, 0x72903}, // mute
			{0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0    }, // no reverb
			{0x72004, 0x72104, 0x72204, 0x72304, 0x72404, 0x72504, 0x72604, 0x72704, 0x72804, 0x72904}, // pan
			{0x72008, 0x72108, 0x72208, 0x72308, 0x72408, 0x72508, 0x72608, 0x72708, 0x72808, 0x72908}, // volume
		},
	},
	[POutputD] = {
		.rows = 6, .cols = 10,
		.name = "daw_monitor.d",
		.headers = (const char **) channel_headers,
		.labels = (const char **) output_labels,
		.controls = {
			{0x73000, 0x73200, 0x73400, 0x73600, 0x73800}, // stereo
			{0x73002, 0x73102, 0x73202, 0x73302, 0x73402, 0x73502, 0x73602, 0x73702, 0x73802, 0x73902}, // solo
			{0x73003, 0x73103, 0x73203, 0x73303, 0x73403, 0x73503, 0x73603, 0x73703, 0x73803, 0x73903}, // mute
			{0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0    }, // no reverb
			{0x73004, 0x73104, 0x73204, 0x73304, 0x73404, 0x73504, 0x73604, 0x73704, 0x73804, 0x73904}, // pan
			{0x73008, 0x73108, 0x73208, 0x73308, 0x73408, 0x73508, 0x73608, 0x73708, 0x73808, 0x73908}, // volume
		},
	},
	[PPreamp] = {
		.rows = 8, .cols = 12,
		.name = "preamp",
		.headers = (const char **) channel_headers,
		.labels = CCA( "stereo","hi-z","+48","lo-cut","phase","sens","bypass","gate" ),
		.controls = {
			{ 0x50005, 0x0    , 0x50205, 0x0    , 0x50405, 0x0    , 0x50605, 0x0    , 0x50805, 0x0    , 0x50a05, 0x0     }, // stereo
			{ 0x50003, 0x50103, 0x0    , 0x0    , 0x0    , 0x0    , 0x0    , 0x0    , 0x0    , 0x0    , 0x0    , 0x0     }, // hi-z
			{ 0x50000, 0x50100, 0x50200, 0x50300, 0x50400, 0x50500, 0x50600, 0x50700, 0x50800, 0x50900, 0x50a00, 0x50b00 }, // +48
			{ 0x50001, 0x50101, 0x50201, 0x50301, 0x50401, 0x50501, 0x50601, 0x50701, 0x50801, 0x50901, 0x50a01, 0x50b01 }, // lo-cut
			{ 0x50002, 0x50102, 0x50202, 0x50302, 0x50402, 0x50502, 0x50602, 0x50702, 0x50802, 0x50902, 0x50a02, 0x50b02 }, // phase
			{ 0x50004, 0x50104, 0x50204, 0x50304, 0x50404, 0x50504, 0x50604, 0x50704, 0x50804, 0x50904, 0x50a04, 0x50b04 }, // sens
			{ 0x50006, 0x50106, 0x50206, 0x50306, 0x50406, 0x50506, 0x50606, 0x50706, 0x50806, 0x50906, 0x50a06, 0x50b06 }, // bypass
			{ 0x50007, 0x50107, 0x50207, 0x50307, 0x50407, 0x50507, 0x50607, 0x50707, 0x50807, 0x50907, 0x50a07, 0x50b07 }, // gate
		},
	},
	[PCompressor] = {
		.rows = 8, .cols = 12,
		.name = "compressor",
		.headers = (const char **) channel_headers,
		.labels = CCA( "bypass","gate","threshold","ratio","attack","release","gain","knee" ),
		.controls = {
			{ 0x50006, 0x50106, 0x50206, 0x50306, 0x50406, 0x50506, 0x50606, 0x50706, 0x50806, 0x50906, 0x50a06, 0x50b06 }, // bypass
			{ 0x50007, 0x50107, 0x50207, 0x50307, 0x50407, 0x50507, 0x50607, 0x50707, 0x50807, 0x50907, 0x50a07, 0x50b07 }, // gate
			{ 0x50008, 0x50108, 0x50208, 0x50308, 0x50408, 0x50508, 0x50608, 0x50708, 0x50808, 0x50908, 0x50a08, 0x50b08 }, // attack
			{ 0x50009, 0x50109, 0x50209, 0x50309, 0x50409, 0x50509, 0x50609, 0x50709, 0x50809, 0x50909, 0x50a09, 0x50b09 }, // release
			{ 0x5000a, 0x5010a, 0x5020a, 0x5030a, 0x5040a, 0x5050a, 0x5060a, 0x5070a, 0x5080a, 0x5090a, 0x50a0a, 0x50b0a }, // threshold
			{ 0x5000b, 0x5010b, 0x5020b, 0x5030b, 0x5040b, 0x5050b, 0x5060b, 0x5070b, 0x5080b, 0x5090b, 0x50a0b, 0x50b0b }, // ratio
			{ 0x5000c, 0x5010c, 0x5020c, 0x5030c, 0x5040c, 0x5050c, 0x5060c, 0x5070c, 0x5080c, 0x5090c, 0x50a0c, 0x50b0c }, // gain
			{ 0x5000d, 0x5010d, 0x5020d, 0x5030d, 0x5040d, 0x5050d, 0x5060d, 0x5070d, 0x5080d, 0x5090d, 0x50a0d, 0x50b0d }, // knee
		},
	},
	[PLine] = {
		.rows = 2, .cols = 4,
		.name = "line",
		.headers = (const char **) &(channel_headers[12]),
		.labels = CCA( "Stereo","Attenuation" ),
		.controls = {
			{ 0x51000, 0x0    , 0x51200, 0x0     }, // stereo
			{ 0x51001, 0x51101, 0x51201, 0x51301 }, // attenuation
		},
	},
	[PReverb] = {
		.rows = 6, .cols = 3,
		.name = "reverb",
		.headers = CCA( "Type", "Pre delay [ms]", "Time [s]" ),
		.labels = CCA( "","Echo","Room","Small Hall","Large Hall","Plate" ),
		.controls = {
			// type    time     pre_delay
			{ 0x40000, 0x0    , 0x80007 }, // reverb type
			{ 0x0    , 0x40101, 0x40102 }, // echo
			{ 0x0    , 0x40201, 0x40202 }, // room
			{ 0x0    , 0x40301, 0x40302 }, // small_hall
			{ 0x0    , 0x40401, 0x40402 }, // large_hall
			{ 0x0    , 0x40501, 0x40502 }, // plate
		},
	},
};


const capmix_MixerPage *  capmix_get_page(enum capmix_pages page)
{
	return &capmix_mixer_pages[page];
}

void capmix_print_page(const capmix_MixerPage *page)
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
			capmix_Addr control = page->controls[row][col];
			if( control == 0 )
				printf("\t");
			else
				printf("%x\t", control);
		}
		printf("%s\n", page->labels[row]);
	}
	printf("\n");
}
