#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "capture.h"
#include "strings.h"

#define DEF_MEMAREA(NAME) static const capmix_mem_t NAME ## _area[] ///< define an area in device memory by name
#define MEMAREA(NAME) .area=(const capmix_mem_t **const)& NAME ## _area ///< assign the child area to the given named area
//j#define NODE(T,N) .area=(const capmix_mem_t **const)&{ .type=T, .name=#N }
#define ENDA { .offset=capmix_None } ///< end of memory map, required for each memory area
#define MEMNODE( OFFSET, TYPE, NAME) [OFFSET] = { .name = NAME, .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map
#define CMEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = capmix_str.NAME[OFFSET] , .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map using a predefined capmix_str_t entry
#define NODE(TYPE,NAME) CMEMNODE(0, TYPE,NAME)

#define PREAMP( OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for the preamp
#define CHANNEL(OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for an input channel
#define MONITOR(OFFSET, AREA) [OFFSET>>12] = { .offset=OFFSET, .name = capmix_str.monitors[OFFSET>>12], MEMAREA(AREA) } ///< define a child area in memory for a monitor mix
#define REVERB( OFFSET )      [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.reverb_types[OFFSET>>8], MEMAREA(reverb_params) } ///< define a child area in memory for a reverb type
#define LINE 0xe ///< constant value for the logically separate line input section

static const char *const stereo = "stereo";
static const char *const volume = "volume";

static const capmix_str_t capmix_str = {
	.top_map = {
		[0x0] = "initial_setting",
		[0x2] = "auto_sens",
		[0x3] = "patchbay",
		[0x4] = "reverb",
		[0x5] = "preamp",
		[0x6] = "input_monitor",
		[0x7] = "daw_monitor",
		[0x8] = "master",
		[LINE]= "line",
		[0xa] = "meters",
		[0xf] = "load_settings",
	},
	.patchbay = { "1-2", "3-4", "5-6", "7-8", "9-10", },
	.type = "type",
	.reverb_types = {
		[0x1] = "echo",
		[0x2] = "room",
		[0x3] = "small_hall",
		[0x4] = "large_hall",
		[0x5] = "plate",
	},
	.reverb_params = {
		[0x1] = "pre_delay" ,
		[0x2] = "time"      ,
	},
	.preamp_params = {
		[0x0] = "+48"       ,
		[0x1] = "lo-cut"    ,
		[0x2] = "phase"     ,
		[0x3] = "hi-z"      ,
		[0x4] = "sens"      ,
		[0x5] = stereo      ,
		[0x6] = "bypass"    ,
		[0x7] = "gate"      ,
		[0x8] = "attack"    ,
		[0x9] = "release"   ,
		[0xa] = "threshold" ,
		[0xb] = "ratio"     ,
		[0xc] = "gain"      ,
		[0xd] = "knee"      ,
	},
	.monitors = { "a","b","c","d", },
	.channel = "channel",
	.channels = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", },
	.channel_params = {
		[0x0] = stereo,
		[0x1] = "attenuation",
		[0x2] = "solo",
		[0x3] = "mute",
		[0x4] = "pan",
		[0x8] = volume,
		[0xe] = "reverb",
	},
	.master = "master",
	.master_channels = {
		"direct_monitor",
		"daw_monitor",
	},
	.left_right = {
		[0x0] = "left",
		[0x1] = "right",
	},
	.master_params = {
		[0x0] = stereo,
		[0x1] = volume,
	},
	.reverb_return = "reverb_return",
	.link = "link",
};

#ifndef DOXYGEN_SKIP
DEF_MEMAREA(patchbay) = {
	CMEMNODE(0x0, Patch, patchbay),
	CMEMNODE(0x1, Patch, patchbay),
	CMEMNODE(0x2, Patch, patchbay),
	CMEMNODE(0x3, Patch, patchbay),
	CMEMNODE(0x4, Patch, patchbay),
	ENDA
};

DEF_MEMAREA(reverb_params) = {
	CMEMNODE(0x1, PreDelay,   reverb_params),
	CMEMNODE(0x2, ReverbTime, reverb_params),
	ENDA
};
DEF_MEMAREA(reverb) = {
	MEMNODE(0x0, ReverbType, capmix_str.type),
	REVERB(0x100), REVERB(0x200), REVERB(0x300), REVERB(0x400), REVERB(0x500),
	ENDA
};

DEF_MEMAREA(mic_preamp_params) = {
	CMEMNODE(0x0, Boolean  , preamp_params),
	CMEMNODE(0x1, Boolean  , preamp_params),
	CMEMNODE(0x2, Boolean  , preamp_params),
	CMEMNODE(0x3, Boolean  , preamp_params),
	CMEMNODE(0x4, Sens     , preamp_params),
	CMEMNODE(0x5, Boolean  , preamp_params),
	CMEMNODE(0x6, Boolean  , preamp_params),
	CMEMNODE(0x7, Gate     , preamp_params),
	CMEMNODE(0x8, Attack   , preamp_params),
	CMEMNODE(0x9, Release  , preamp_params),
	CMEMNODE(0xa, Threshold, preamp_params),
	CMEMNODE(0xb, Ratio    , preamp_params),
	CMEMNODE(0xc, Gain     , preamp_params),
	CMEMNODE(0xd, Knee     , preamp_params),
	ENDA
};
DEF_MEMAREA(preamp_params) = {
	CMEMNODE(0x0, Boolean   , preamp_params),
	CMEMNODE(0x1, Boolean   , preamp_params),
	CMEMNODE(0x2, Boolean   , preamp_params),
	CMEMNODE(0x4, Sens      , preamp_params),
	CMEMNODE(0x5, Boolean   , preamp_params),
	CMEMNODE(0x6, Boolean   , preamp_params),
	CMEMNODE(0x7, Gate      , preamp_params),
	CMEMNODE(0x8, Attack    , preamp_params),
	CMEMNODE(0x9, Release   , preamp_params),
	CMEMNODE(0xa, Threshold , preamp_params),
	CMEMNODE(0xb, Ratio     , preamp_params),
	CMEMNODE(0xc, Gain      , preamp_params),
	CMEMNODE(0xd, Knee      , preamp_params),
	ENDA
};
DEF_MEMAREA(preamp_channels) = {
	PREAMP(0x000, mic_preamp_params),  PREAMP(0x100, mic_preamp_params),
	PREAMP(0x200, preamp_params),      PREAMP(0x300, preamp_params),
	PREAMP(0x400, preamp_params),      PREAMP(0x500, preamp_params),
	PREAMP(0x600, preamp_params),      PREAMP(0x700, preamp_params),
	PREAMP(0x800, preamp_params),      PREAMP(0x900, preamp_params),
	PREAMP(0xa00, preamp_params),      PREAMP(0xb00, preamp_params),
	ENDA
};
DEF_MEMAREA(preamp) = { { .name=capmix_str.channel, MEMAREA(preamp_channels) }, ENDA };

DEF_MEMAREA(line_params) = {
	MEMNODE(0x0, Boolean    , capmix_str.channel_params[0]),
	MEMNODE(0x1, Attenuation, capmix_str.channel_params[1]),
	ENDA
};
DEF_MEMAREA(line_channels) = {
	[0x0] = { .offset=0x000, .name = capmix_str.channels[12], MEMAREA(line_params) },
	[0x1] = { .offset=0x100, .name = capmix_str.channels[13], MEMAREA(line_params) },
	[0x2] = { .offset=0x200, .name = capmix_str.channels[14], MEMAREA(line_params) },
	[0x3] = { .offset=0x300, .name = capmix_str.channels[15], MEMAREA(line_params) },
	ENDA
};
DEF_MEMAREA(line) = { { .name=capmix_str.channel, MEMAREA(line_channels) }, ENDA };

DEF_MEMAREA(input) = {
	MEMNODE(0x0, Boolean, capmix_str.channel_params[0x0]),
	MEMNODE(0x2, Boolean, capmix_str.channel_params[0x2]),
	MEMNODE(0x3, Boolean, capmix_str.channel_params[0x3]),
	MEMNODE(0x4, Pan    , capmix_str.channel_params[0x4]),
	MEMNODE(0x8, Volume , capmix_str.channel_params[0x8]),
	MEMNODE(0xe, Volume , capmix_str.channel_params[0xe]),
	ENDA
};

DEF_MEMAREA(input_channels) = {
	CHANNEL(0x000, input),  CHANNEL(0x100, input),
	CHANNEL(0x200, input),  CHANNEL(0x300, input),
	CHANNEL(0x400, input),  CHANNEL(0x500, input),
	CHANNEL(0x600, input),  CHANNEL(0x700, input),
	CHANNEL(0x800, input),  CHANNEL(0x900, input),
	CHANNEL(0xa00, input),  CHANNEL(0xb00, input),
	CHANNEL(0xc00, input),  CHANNEL(0xd00, input),
	CHANNEL(0xe00, input),  CHANNEL(0xf00, input),
	ENDA
};
DEF_MEMAREA(input_channel) = { { .name=capmix_str.channel, MEMAREA(input_channels) }, ENDA };
DEF_MEMAREA(input_monitor) = {
	MONITOR(0x0000, input_channel), MONITOR(0x1000, input_channel),
	MONITOR(0x2000, input_channel), MONITOR(0x3000, input_channel),
	ENDA
};

DEF_MEMAREA(daw) = {
	MEMNODE(0x0, Boolean, capmix_str.channel_params[0x0]),
	MEMNODE(0x2, Boolean, capmix_str.channel_params[0x2]),
	MEMNODE(0x3, Boolean, capmix_str.channel_params[0x3]),
	MEMNODE(0x4, Pan    , capmix_str.channel_params[0x4]),
	MEMNODE(0x8, Volume , capmix_str.channel_params[0x8]),
	ENDA
};
DEF_MEMAREA(daw_channels) = {
	CHANNEL(0x000, daw),  CHANNEL(0x100, daw),
	CHANNEL(0x200, daw),  CHANNEL(0x300, daw),
	CHANNEL(0x400, daw),  CHANNEL(0x500, daw),
	CHANNEL(0x600, daw),  CHANNEL(0x700, daw),
	CHANNEL(0x800, daw),  CHANNEL(0x900, daw),
	ENDA
};
DEF_MEMAREA(daw_channel) = { { .name=capmix_str.channel, MEMAREA(daw_channels) }, ENDA };
DEF_MEMAREA(daw_monitor) = {
	MONITOR(0x0000, daw_channel),
	MONITOR(0x1000, daw_channel),
	MONITOR(0x2000, daw_channel),
	MONITOR(0x3000, daw_channel),
	ENDA
};

DEF_MEMAREA(master_params) = {
	MEMNODE(0x0, Boolean, capmix_str.master_params[0]),
	MEMNODE(0x1, Volume, capmix_str.master_params[1]),
	ENDA
};
DEF_MEMAREA(direct_monitor_a) = {
	{ .offset=0x0000, .name=capmix_str.left_right[0], MEMAREA(master_params) },
	{ .offset=0x0100, .name=capmix_str.left_right[1], MEMAREA(master_params) },
	{ .offset=0x0007, .name=capmix_str.reverb_return, .type=TVolume },
	{ .offset=0x000d, .name=capmix_str.link, .type=TBoolean },
	ENDA
};
DEF_MEMAREA(direct_monitor) = {
	{ .offset=0x0000, .name=capmix_str.left_right[1],  MEMAREA(master_params) },
	{ .offset=0x0100, .name=capmix_str.left_right[0],  MEMAREA(master_params) },
	{ .offset=0x000d, .name=capmix_str.link },
	ENDA
};
DEF_MEMAREA(master_direct_monitors) = {
	MONITOR(0x0000, direct_monitor_a),
	MONITOR(0x1000, direct_monitor),
	MONITOR(0x2000, direct_monitor),
	MONITOR(0x3000, direct_monitor),
	ENDA
};
DEF_MEMAREA(daw_monitor_lr) = {
	[0x0] = { .offset=0x0000, .name = capmix_str.left_right[0], MEMAREA(master_params) },
	[0x1] = { .offset=0x0100, .name = capmix_str.left_right[1], MEMAREA(master_params) },
	ENDA
};
DEF_MEMAREA(master_daw_monitors) = {
	MONITOR(0x0000, daw_monitor_lr),
	MONITOR(0x1000, daw_monitor_lr),
	MONITOR(0x2000, daw_monitor_lr),
	MONITOR(0x3000, daw_monitor_lr),
	ENDA
};

DEF_MEMAREA(master) = {
	[0x0] = { .offset=0x00000, .name=capmix_str.master_channels[0], MEMAREA(master_direct_monitors) },
	[0x1] = { .offset=0x10000, .name=capmix_str.master_channels[1], MEMAREA(master_daw_monitors) },
	ENDA
};
#endif

DEF_MEMAREA(meter_ins) = {
	{ .type=TMeter, .offset = 0x00, .name="1" },
	{ .type=TMeter, .offset = 0x02, .name="2" },
	{ .type=TMeter, .offset = 0x04, .name="3" },
	{ .type=TMeter, .offset = 0x06, .name="4" },
	{ .type=TMeter, .offset = 0x08, .name="5" },
	{ .type=TMeter, .offset = 0x0a, .name="6" },
	{ .type=TMeter, .offset = 0x0c, .name="7" },
	{ .type=TMeter, .offset = 0x0e, .name="8" },
	{ .type=TMeter, .offset = 0x10, .name="9" },
	{ .type=TMeter, .offset = 0x12, .name="10" },
	{ .type=TMeter, .offset = 0x14, .name="11" },
	{ .type=TMeter, .offset = 0x16, .name="12" },
	ENDA
};
DEF_MEMAREA(meter_outs) = {
	{ .type=TMeter, .offset = 0x00, .name="1" },
	{ .type=TMeter, .offset = 0x02, .name="2" },
	{ .type=TMeter, .offset = 0x04, .name="3" },
	{ .type=TMeter, .offset = 0x06, .name="4" },
	{ .type=TMeter, .offset = 0x08, .name="5" },
	{ .type=TMeter, .offset = 0x0a, .name="6" },
	{ .type=TMeter, .offset = 0x0c, .name="7" },
	{ .type=TMeter, .offset = 0x0e, .name="8" },
	{ .type=TMeter, .offset = 0x10, .name="9" },
	{ .type=TMeter, .offset = 0x12, .name="10" },
	{ .type=TMeter, .offset = 0x14, .name="11" },
	{ .type=TMeter, .offset = 0x16, .name="12" },

	{ .type=TMeter, .offset = 0x18, .name="x" }, //?
	{ .type=TMeter, .offset = 0x1a, .name="x" }, //?
	{ .type=TMeter, .offset = 0x1c, .name="x" }, //?
	{ .type=TMeter, .offset = 0x1e, .name="x" }, //?
	ENDA
};
DEF_MEMAREA(meter_lines) = {
	{ .type=TMeter, .offset = 0x00, .name="13" },
	{ .type=TMeter, .offset = 0x02, .name="14" },
	{ .type=TMeter, .offset = 0x04, .name="15" },
	{ .type=TMeter, .offset = 0x06, .name="16" },
	ENDA
};
DEF_MEMAREA(meter_daw) = {
	{ .type=TMeter, .offset = 0x00, .name="1" },
	{ .type=TMeter, .offset = 0x02, .name="2" },
	{ .type=TMeter, .offset = 0x04, .name="3" },
	{ .type=TMeter, .offset = 0x06, .name="4" },
	{ .type=TMeter, .offset = 0x08, .name="5" },
	{ .type=TMeter, .offset = 0x0a, .name="6" },
	{ .type=TMeter, .offset = 0x0c, .name="7" },
	{ .type=TMeter, .offset = 0x0e, .name="8" },
	{ .type=TMeter, .offset = 0x10, .name="9" },
	{ .type=TMeter, .offset = 0x12, .name="10" },
	ENDA
};
DEF_MEMAREA(meter_gr) = {
	{ .type=TMeter, .offset = 0x00, .name="1" },
	{ .type=TMeter, .offset = 0x02, .name="2" },
	{ .type=TMeter, .offset = 0x04, .name="3" },
	{ .type=TMeter, .offset = 0x06, .name="4" },
	{ .type=TMeter, .offset = 0x08, .name="5" },
	{ .type=TMeter, .offset = 0x0a, .name="6" },
	{ .type=TMeter, .offset = 0x0c, .name="7" },
	{ .type=TMeter, .offset = 0x0e, .name="8" },
	{ .type=TMeter, .offset = 0x10, .name="9" },
	{ .type=TMeter, .offset = 0x12, .name="10" },
	{ .type=TMeter, .offset = 0x14, .name="11" },
	{ .type=TMeter, .offset = 0x16, .name="12" },
	ENDA
};
DEF_MEMAREA(meter_gate) = {
	{ .type=TMeter, .offset = 0x00, .name="1" },
	{ .type=TMeter, .offset = 0x02, .name="2" },
	{ .type=TMeter, .offset = 0x04, .name="3" },
	{ .type=TMeter, .offset = 0x06, .name="4" },
	{ .type=TMeter, .offset = 0x08, .name="5" },
	{ .type=TMeter, .offset = 0x0a, .name="6" },
	{ .type=TMeter, .offset = 0x0c, .name="7" },
	{ .type=TMeter, .offset = 0x0e, .name="8" },
	{ .type=TMeter, .offset = 0x10, .name="9" },
	{ .type=TMeter, .offset = 0x12, .name="10" },
	{ .type=TMeter, .offset = 0x14, .name="11" },
	{ .type=TMeter, .offset = 0x16, .name="12" },
	ENDA
};
DEF_MEMAREA(meter_channels) = {
	{ .offset = 0x00, .name="pre" , MEMAREA(meter_ins  ) }, // pre-compressor
	{ .offset = 0x18, .name="post", MEMAREA(meter_outs ) }, // post-compressor
	{ .offset = 0x30, .name="line", MEMAREA(meter_lines) }, // line input
	{ .offset = 0x38, .name="daw" , MEMAREA(meter_daw  ) }, // daw output
	{ .offset = 0x4c, .name="gr"  , MEMAREA(meter_gr   ) }, // compressor gain reduction
	{ .offset = 0x64, .name="gate", MEMAREA(meter_gate ) }, // gate
	ENDA
};

DEF_MEMAREA(meter_clips) = {
	{ .type=TClipMask, .offset = 0x00, .name= "1-4"  },
	{ .type=TClipMask, .offset = 0x01, .name= "5-8"  },
	{ .type=TClipMask, .offset = 0x02, .name= "9-12" },
	{ .type=TClipMask, .offset = 0x03, .name="13-16" },
	ENDA
};
DEF_MEMAREA(meter_clip) = {
	{ .offset = 0x00, .name="pre" , MEMAREA(meter_clips) },
	{ .offset = 0x04, .name="post", MEMAREA(meter_clips) },
	ENDA
};

DEF_MEMAREA(meters) = {
	[0x0] = { .offset=0x0000, .name="active" , .type=TBoolean },
	[0x1] = { .offset=0x0001, .name="channel", MEMAREA(meter_channels), .type=TMeter },
	[0x2] = { .offset=0x0101, .name="clip"   , MEMAREA(meter_clip) },
	ENDA
};


// auto sens on      : 02=1, 03=1
// auto sens cancel  : 02=2, 03=0
// auto sens off     : 02=0
DEF_MEMAREA(auto_sens_setup) = {
	[0x2] = { .offset=0x02, .name="mode", .type=TAutoSens },
	[0x3] = { .offset=0x03, .name="reset" , .type=TBoolean },
	ENDA
};

DEF_MEMAREA(auto_sens_mask_node) = { { .name="execute", .type=TBoolean }, ENDA };
#define SENS_MASK(N) [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_mask_node) }
DEF_MEMAREA(auto_sens_mask) = {
	SENS_MASK(1),
	SENS_MASK(2),
	SENS_MASK(3),
	SENS_MASK(4),
	SENS_MASK(5),
	SENS_MASK(6),
	SENS_MASK(7),
	SENS_MASK(8),
	SENS_MASK(9),
	SENS_MASK(10),
	SENS_MASK(11),
	SENS_MASK(12),
	SENS_MASK(13),
	SENS_MASK(14),
	SENS_MASK(15),
	SENS_MASK(16),
	ENDA
};

DEF_MEMAREA(auto_sens_preamp_node) = { { .name="sens", .type=TSens }, ENDA };
DEF_MEMAREA(auto_sens_line_node)   = { { .name="attenuation", .type=TAttenuation }, ENDA };

#define SENS(N)  [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_preamp_node) }
#define ATTEN(N) [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_line_node) }
DEF_MEMAREA(auto_sens_channels) = {
	SENS(1),
	SENS(2),
	SENS(3),
	SENS(4),
	SENS(5),
	SENS(6),
	SENS(7),
	SENS(8),
	SENS(9),
	SENS(10),
	SENS(11),
	SENS(12),
	ATTEN(13),
	ATTEN(14),
	ATTEN(15),
	ATTEN(16),
	ENDA
};

DEF_MEMAREA(auto_sens) = {
	[0x0] = { .offset=0x0100, .name="setup"        , MEMAREA(auto_sens_setup) },
	[0x1] = { .offset=0x0110, .name="select"       , MEMAREA(auto_sens_mask) },
	[0x2] = { .offset=0x0120, .name="channel"      , MEMAREA(auto_sens_channels) },
	ENDA
};

DEF_MEMAREA(capmix_mem_none) = { { .name = "?" }, ENDA };

/// describe a top-level section of memory by name
#define OFFSET_AREA( OFFSET, NAME ) { .offset=OFFSET, .name=#NAME, MEMAREA(NAME)}
static const capmix_mem_t memory_map[] = {
	[0x0] = { .offset = 0x00000002 , .name = capmix_str.top_map[0x0] },
	[0x2] = { .offset = 0x00020000 , .name = capmix_str.top_map[0x2] , MEMAREA(auto_sens)     },
	[0x3] = { .offset = O_PATCHBAY , .name = capmix_str.top_map[0x3] , MEMAREA(patchbay)      },
	[0x4] = { .offset = O_REVERB   , .name = capmix_str.top_map[0x4] , MEMAREA(reverb)        },
	[0x5] = { .offset = O_PREAMP   , .name = capmix_str.top_map[0x5] , MEMAREA(preamp)        },
	[LINE]= { .offset = O_LINE     , .name = capmix_str.top_map[LINE], MEMAREA(line)          }, // this is upsetting
	[0x6] = { .offset = O_INPUT_MON, .name = capmix_str.top_map[0x6] , MEMAREA(input_monitor) },
	[0x7] = { .offset = O_DAW_MON  , .name = capmix_str.top_map[0x7] , MEMAREA(daw_monitor)   },
	[0x8] = { .offset = O_MASTER   , .name = capmix_str.top_map[0x8] , MEMAREA(master)        },
	[0xa] = { .offset = 0x000a0000 , .name = capmix_str.top_map[0xa] , MEMAREA(meters) },
	[0xf] = { .offset = 0x01000000 , .name = capmix_str.top_map[0xf] },
	ENDA
};

/**
 * @brief traverse a memory map recursively, printing the address and full name of each entry
 * @param map the map to traverse
 * @param prefix recursion parameter storing the name of the current area of the map
 * @param prev_offset recursion parameter accumulating the starting memory address of the current area of the map
 */
void                    capmix_print_map(const capmix_mem_t *map, const char *prefix, capmix_addr_t prev_offset)
{
	for(int i = 0; map[i].offset != 0xffffffff; i++ )
	{
		if( map[i].name == NULL ) continue;
		uint32_t offset = map[i].offset;
		const char *name = map[i].name;
		char new_prefix[64];
		if( strlen(prefix) == 0 )
			sprintf(new_prefix, "%s", name);
		else
			sprintf(new_prefix, "%s.%s", prefix, name);

		if( map[i].area == NULL )
		{
			printf("0x%08x %s\n", prev_offset + offset, new_prefix);
			continue;
		}

		capmix_print_map((const capmix_mem_t *)(map[i].area), new_prefix, prev_offset + offset);
	}
}

/**
 * @brief iterate through a memory map until the given partial name is found
 * @param map the map to search
 * @param part the name to match
 * @return the map with matching name, or NULL if not found
 */
static const capmix_mem_t *   capmix_lookup_map(const capmix_mem_t *map, const char *part)
{
	for( int i = 0; map[i].offset != capmix_None; i++ )
	{
		if( map[i].name == NULL ) continue;
		if( strcmp(map[i].name, part) != 0 ) continue;

		//debug(map[i].name);
		return &map[i];
	}
	return NULL;
}

/**
 * @brief parse a string into a device memory address
 * @param desc the string to parse
 * @return the device memory address
 * @ingroup API
 */
capmix_addr_t                 capmix_parse_addr(const char *desc)
{
	capmix_addr_t ret = 0;
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map);
	char *desc_ = strdup(desc);
	char *tok = strtok(desc_, ".");
	while( tok != NULL )
	{
		map = capmix_lookup_map(map, tok);
		if( map == NULL ) //|| map == (void *)None )
		{
			return capmix_None;
		}
		ret += map->offset;
		//printf("0x%08x\n", ret);
		tok = strtok(NULL, ".");
		map = (const capmix_mem_t *)(map->area);
	}
	free(desc_);
	return ret;
}

/**
 * @brief the top part of the memory map is more efficient to navigate using a piecewise function
 * @param addr the address to examine
 * @return the section index of the top memory map
 */
int                 capmix_top_section(capmix_addr_t addr)
{
	int section;
	if( addr >> 12 == 0x51 )
		section = LINE;
	else if( addr >> 16 == 0x9 )
		section = 0x8;
	else
		section = addr >> 16;
	return section;
}

/**
 * @brief algorithm for decomposing a device memory address into a vector of map pointers
 * @param addr the address to examine
 * @return a vector containing the path across the map taken by this algorithm to find the given address
 */
capmix_mem_vector_t           capmix_mem_vector(capmix_addr_t addr)
{
	int v = 0;
	capmix_mem_vector_t vec = { .areas = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } };

	int section = capmix_top_section(addr);
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map[section].area);
	if( memory_map[section].name == NULL )
		return vec;

	vec.areas[v++] = &(memory_map[section]);

	int countdown = addr;
	countdown -= memory_map[section].offset;

	const capmix_mem_t *candidate;
	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		candidate = NULL;

		capmix_addr_t min_offset = capmix_None;
		for( int i = 0; map[i].offset != capmix_None; i++ )
		{
			if( map[i].name == NULL ) continue;

			if( map[i].offset <= countdown)
			{
				if( countdown - map[i].offset < min_offset )
				{
					min_offset = countdown - map[i].offset;
					candidate = &(map[i]);
				}
			}
		}
		
		if( candidate == NULL )
			return vec;

		vec.areas[v++] = candidate;
		countdown -= candidate->offset; // 0x120e
		map = (const capmix_mem_t *)(candidate->area);
	}
	if( countdown != 0 )
	{
		vec.areas[--v] = capmix_mem_none_area;
	}
	if( map != NULL && map->name != NULL )
	{
		vec.areas[v++] = map;
	}
	return vec;
}

/**
 * @brief write a string describing the given device memory address, e.g. 0x0006120e -> daw_monitor.b.channel.3.volume
 * @param[in] the device memory address to describe
 * @param[out] the string buffer to write into
 * @ingroup API
 */
void                    capmix_format_addr(capmix_addr_t addr, char *desc)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);

	int a = 0 ;
	const capmix_mem_t *area = vec.areas[a];
	sprintf(desc, "");
	do
	{
		strcat(desc, area->name);
		area = vec.areas[++a];
		if( area == NULL )
			break;
		strcat(desc, ".");
	}
	while( a < 8 );
}

/**
 * @brief returns the leaf-level name of the given device memory address
 * @param addr the device memory address to describe
 * @return the string that describes this address
 * @ingroup API
 */
const char *            capmix_addr_suffix(capmix_addr_t addr)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);
	int a = 0;
	while( a < 8 && vec.areas[a] != NULL && vec.areas[a+1] != NULL )
		a++;
	return vec.areas[a]->name;
}

/**
 * @brief get the value type of a device memory address
 * @param addr the device memory address to describe
 * @return the type identifier
 * @ingroup API
 */
capmix_type_t           capmix_addr_type(capmix_addr_t addr)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);
	if( vec.areas[0] == NULL ) return TValue;
	int a = 0;
	while( a < 8 && vec.areas[a+1] != NULL )
		a++;
	return vec.areas[a]->type;
}
