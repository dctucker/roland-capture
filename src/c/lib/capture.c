#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "capture.h"
#include "strings.h"

#define DEF_MEMAREA(NAME) static const capmix_mem_t NAME ## _area[] ///< define an area in device memory by name
#define MEMAREA(NAME) .area=(const capmix_mem_t **const)& NAME ## _area ///< assign the child area to the given named area
#define ENDA { .offset=capmix_None } ///< end of memory map, required for each memory area
#define MEMNODE( OFFSET, TYPE, NAME) [OFFSET] = { .name = NAME, .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map
#define CMEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = capmix_str.NAME[OFFSET] , .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map using a predefined capmix_str_t entry

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
	MEMNODE(0x1, Boolean, capmix_str.master_params[1]),
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
	[0x0] = { .offset=0x00000000, .name=capmix_str.master_channels[0], MEMAREA(master_direct_monitors) },
	[0x1] = { .offset=0x00010000, .name=capmix_str.master_channels[1], MEMAREA(master_daw_monitors) },
	ENDA
};
#endif

#define METER(N) [N] = { .offset = 2*(N-1), .name=#N, .type=TMeter }
DEF_MEMAREA(meter_channels) = {
	METER(1),
	METER(2),
	METER(3),
	METER(4),
	METER(5),
	METER(6),
	METER(7),
	METER(8),
	METER(9),
	METER(10),
	METER(11),
	METER(12),
	METER(13),
	METER(14),
	METER(15),
	METER(16),

	METER(17),
	METER(18),
	METER(19),
	METER(20),
	METER(21),
	METER(22),
	METER(23),
	METER(24),
	METER(25),
	METER(26),
	METER(27),
	METER(28),
	METER(29),
	METER(30),
	METER(31),
	METER(32),
	METER(33),
	METER(34),
	METER(35),
	METER(36),
	METER(37),
	METER(38),
	METER(39),
	METER(40),
	METER(41),
	METER(42),
	METER(43),
	METER(44),
	METER(45),
	METER(46),
	METER(47),
	METER(48),
	METER(49),
	METER(50),
	METER(51),
	METER(52),
	METER(53),
	METER(54),
	METER(55),
	METER(56),
	METER(57),
	METER(58),
	METER(59),
	METER(60),
	METER(61),
	METER(62),

	ENDA
};

DEF_MEMAREA(meter_more) = {
	METER(1),
	METER(2),
	METER(3),
	METER(4),
};

DEF_MEMAREA(meters) = {
	[0x0] = { .offset=0x0000, .name="active" , .type=TBoolean },
	[0x1] = { .offset=0x0001, .name="channel", MEMAREA(meter_channels) },
	[0x2] = { .offset=0x0101, .name="more"   , MEMAREA(meter_more) },
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
capmix_addr_t           capmix_parse_addr(const char *desc)
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
 * @brief write a string describing the given device memory address, e.g. 0x0006120e -> daw_monitor.b.channel.3.volume
 * @param[in] the device memory address to describe
 * @param[out] the string buffer to write into
 * @ingroup API
 */
void                    capmix_format_addr(capmix_addr_t addr, char *desc)
{
	*desc = '\0';
	int section;
	if( addr >> 12 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	int countdown = addr;
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map[section].area);
	if( memory_map[section].name == NULL )
	{
		strcat(desc, "?");
		return;
	}
	strcat(desc, memory_map[section].name);
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
		
		if( candidate == NULL ) return;

		strcat(desc, ".");
		strcat(desc, candidate->name);
		countdown -= candidate->offset; // 0x120e
		map = (const capmix_mem_t *)(candidate->area);
	}
	if( countdown != 0 )
	{
		desc[strlen(desc) - strlen(candidate->name)] = '\0';
		strcat(desc, "?");
	}
	if( map != NULL && map->name != NULL )
	{
		strcat(desc, ".");
		strcat(desc, map->name);
	}
}

/**
 * @brief returns the leaf-level name of the given device memory address
 * @param addr the device memory address to describe
 * @return the string that describes this address
 * @ingroup API
 */
const char *            capmix_addr_suffix(capmix_addr_t addr)
{
	int section;
	if( addr >> 12 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	int countdown = addr;
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map[section].area);
	countdown -= memory_map[section].offset;

	const char *name = "";
	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		const capmix_mem_t *candidate = NULL;

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
		
		if( candidate == NULL ) return "";

		countdown -= candidate->offset; // 0x120e
		name = candidate->name;
		map = (const capmix_mem_t *)(candidate->area);
	}
	if( countdown !=0 )
		return "";
	if( map != NULL && map->name != NULL )
	{
		name = map->name;
	}
	return name;
}

/**
 * @brief get the value type of a device memory address
 * @param addr the device memory address to describe
 * @return the type identifier
 * @ingroup API
 */
capmix_type_t           capmix_addr_type(capmix_addr_t addr)
{
	int section;
	if( addr >> 12 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	int countdown = addr;
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map[section].area);
	countdown -= memory_map[section].offset;

	capmix_type_t type = memory_map[section].type;
	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		const capmix_mem_t *candidate = NULL;

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
		
		if( candidate == NULL ) return TValue;

		countdown -= candidate->offset; // 0x120e
		type = candidate->type;
		map = (const capmix_mem_t *)(candidate->area);
	}
	if( countdown != 0 )
		return TValue;
	if( map != NULL && map->name != NULL )
	{
		if( map->type != TValue )
			type = map->type;
	}
	return type;
}
