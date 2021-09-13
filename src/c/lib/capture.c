#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "capture.h"
#include "strings.h"

#define DEF_MEMAREA(NAME) static capmix_MemMap NAME ## _area[]
#define MEMAREA(NAME) .area=(const capmix_MemMap **const)& NAME ## _area
#define ENDA { .offset=capmix_None }
#define MEMNODE( OFFSET, TYPE, NAME) [OFFSET] = { .name = NAME, .offset = OFFSET, .type = T##TYPE }
#define CMEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = capmix_str.NAME[OFFSET] , .offset = OFFSET, .type = T##TYPE }

#define PREAMP( OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) }
#define CHANNEL(OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) }
#define MONITOR(OFFSET, AREA) [OFFSET>>12] = { .offset=OFFSET, .name = capmix_str.monitors[OFFSET>>12], MEMAREA(AREA) }
#define REVERB( OFFSET ) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.reverb_types[OFFSET>>8], MEMAREA(reverb_params) }

static const char *const stereo = "stereo";
static const char *const volume = "volume";

static const struct capmix_str capmix_str = {
	.top_map = {
		[0x3] = "patchbay",
		[0x4] = "reverb",
		[0x5] = "preamp",
		[0x6] = "input_monitor",
		[0x7] = "daw_monitor",
		[0x8] = "master",
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

#define LINE 0xe
#define OFFSET_AREA( OFFSET, NAME ) { .offset=OFFSET, .name=#NAME, MEMAREA(NAME)}
static const capmix_MemMap memory_map[] = {
	[0x0] = { .offset = 0x00000002 , .name = "initial_setting" },
	[0x3] = OFFSET_AREA(O_PATCHBAY , patchbay),
	[0x4] = OFFSET_AREA(O_REVERB   , reverb),
	[0x5] = OFFSET_AREA(O_PREAMP   , preamp),
	[LINE]= OFFSET_AREA(O_LINE     , line), // this is upsetting
	[0x6] = OFFSET_AREA(O_INPUT_MON, input_monitor),
	[0x7] = OFFSET_AREA(O_DAW_MON  , daw_monitor),
	[0x8] = OFFSET_AREA(O_MASTER   , master),
	/*
	[0x8] = { .offset = O_MASTER, .name = "master", .area = (const *capmix_MemMap[3]){
			[0x0] = (const capmix_MemMap *const)&{ .offset=0x00000000, .name=capmix_str.master_channels[0], MEMAREA(master_direct_monitors) },
			[0x1] = (const capmix_MemMap *const)&{ .offset=0x00010000, .name=capmix_str.master_channels[1], MEMAREA(master_daw_monitors) },
			ENDA
		}
	},
	*/
	[0xa] = { .offset = 0x000a0000 , .name = "meters_active" },
	[0xf] = { .offset = 0x01000000 , .name = "load_settings" },
	ENDA
};

void              capmix_print_map(struct capmix_memory_area *map, char *prefix, capmix_Addr old_offset)
{
	for(int i = 0; map[i].offset != 0xffffffff; i++ )
	{
		if( map[i].name == NULL ) continue;
		uint32_t offset = map[i].offset;
		const char *name = map[i].name;
		char new_prefix[256];
		if( strlen(prefix) == 0 )
			sprintf(new_prefix, "%s", name);
		else
			sprintf(new_prefix, "%s.%s", prefix, name);

		if( map[i].area == NULL )
		{
			printf("0x%08x %s\n", old_offset + offset, new_prefix);
			continue;
		}

		capmix_print_map((capmix_MemMap *)(map[i].area), new_prefix, old_offset + offset);
	}
}

static capmix_MemMap *   capmix_lookup_map(capmix_MemMap *map, char *part)
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

capmix_Addr       capmix_parse_addr(const char *desc)
{
	capmix_Addr ret = 0;
	capmix_MemMap *map = (capmix_MemMap *)(memory_map);
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
		map = (capmix_MemMap *)(map->area);
	}
	free(desc_);
	return ret;
}

void              capmix_format_addr(capmix_Addr addr, char *desc)
{
	// 0x0006120e -> daw_monitor.b.channel.3.volume
	*desc = '\0';
	int section;
	if( addr >> 15 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	int countdown = addr;
	capmix_MemMap *map = (capmix_MemMap *)(memory_map[section].area);
	countdown -= memory_map[section].offset;
	strcat(desc, memory_map[section].name);

	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		capmix_MemMap *candidate = NULL;

		capmix_Addr min_offset = capmix_None;
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
		map = (capmix_MemMap *)(candidate->area);
	}
	if( map != NULL && map->name != NULL )
	{
		strcat(desc, ".");
		strcat(desc, map->name);
	}
}

const char *  capmix_addr_suffix(capmix_Addr addr)
{
	int section;
	if( addr >> 15 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	int countdown = addr;
	capmix_MemMap *map = (capmix_MemMap *)(memory_map[section].area);
	countdown -= memory_map[section].offset;

	const char *name = "";
	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		capmix_MemMap *candidate = NULL;

		capmix_Addr min_offset = capmix_None;
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
		name = candidate->name;
		map = (capmix_MemMap *)(candidate->area);
	}
	if( map != NULL && map->name != NULL )
	{
		name = map->name;
	}
	return name;
}

capmix_ValueType  capmix_addr_type(capmix_Addr addr)
{
	int section;
	if( addr >> 15 == 0x51 )
		section = LINE;
	else
		section = addr >> 16;

	capmix_Addr countdown = addr;
	capmix_MemMap *map = (capmix_MemMap *)(memory_map[section].area);
	countdown -= memory_map[section].offset;

	capmix_ValueType type = TValue;
	while( countdown != 0 )
	{
		if( map == NULL ) break;
		capmix_MemMap *candidate = NULL;

		capmix_Addr min_offset = capmix_None;
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
		map = (capmix_MemMap *)(candidate->area);
	}
	if( map != NULL && map->name != NULL )
	{
		if( map->type != TValue )
			type = map->type;
	}
	return type;
}
