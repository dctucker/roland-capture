#include <stddef.h>
#include <inttypes.h>
#include "types.h"

#define DEF_MEMAREA(NAME) MemMap NAME ## _area[]
#define MEMAREA(NAME) .area=(MemMap **)& NAME ## _area
#define OFFSET_AREA( OFFSET, NAME ) { .offset=OFFSET, .name=#NAME, MEMAREA(NAME)}
#define None (0xffffffff)
#define ENDA { .offset=None }
#define MEMNODE(OFFSET, NAME) [OFFSET] = { .name = #NAME, .offset = OFFSET }

typedef struct memory_area {
	Addr offset;
	const char *name;
	struct memory_area **area;
} MemMap;

MemMap memory_area;

DEF_MEMAREA(patchbay_output) = {
	MEMNODE(0x0, output),
	ENDA
};
DEF_MEMAREA(patchbay) = {
	MEMNODE(0x0, 1-2),
	MEMNODE(0x1, 3-4),
	MEMNODE(0x2, 5-6),
	MEMNODE(0x3, 7-8),
	MEMNODE(0x4, 9-10),
	ENDA
};

DEF_MEMAREA(reverb_params) = {
	MEMNODE(0x1, pre_delay),
	MEMNODE(0x2, time),
	ENDA
};
DEF_MEMAREA(reverb) = {
	MEMNODE(0x0, type),
	[0x1] = { .offset=0x0100, .name="echo",       MEMAREA(reverb_params) },
	[0x2] = { .offset=0x0200, .name="room",       MEMAREA(reverb_params) },
	[0x3] = { .offset=0x0300, .name="small_hall", MEMAREA(reverb_params) },
	[0x4] = { .offset=0x0400, .name="large_hall", MEMAREA(reverb_params) },
	[0x5] = { .offset=0x0500, .name="plate",      MEMAREA(reverb_params) },
	ENDA
};

DEF_MEMAREA(mic_preamp_params) = {
	MEMNODE(0x0, +48      ),
	MEMNODE(0x1, lo-cut   ),
	MEMNODE(0x2, phase    ),
	MEMNODE(0x3, hi-z     ),
	MEMNODE(0x4, sens     ),
	MEMNODE(0x5, stereo   ),
	MEMNODE(0x6, bypass   ),
	MEMNODE(0x7, gate     ),
	MEMNODE(0x8, attack   ),
	MEMNODE(0x9, release  ),
	MEMNODE(0xa, threshold),
	MEMNODE(0xb, ratio    ),
	MEMNODE(0xc, gain     ),
	MEMNODE(0xd, knee     ),
	ENDA
};
DEF_MEMAREA(preamp_params) = {
	MEMNODE(0x0, +48      ),
	MEMNODE(0x1, lo-cut   ),
	MEMNODE(0x2, phase    ),
	MEMNODE(0x4, sens     ),
	MEMNODE(0x5, stereo   ),
	MEMNODE(0x6, bypass   ),
	MEMNODE(0x7, gate     ),
	MEMNODE(0x8, attack   ),
	MEMNODE(0x9, release  ),
	MEMNODE(0xa, threshold),
	MEMNODE(0xb, ratio    ),
	MEMNODE(0xc, gain     ),
	MEMNODE(0xd, knee     ),
	ENDA
};
DEF_MEMAREA(preamp_channels) = {
	[0x0] = { .offset=0x000, .name = "1",  MEMAREA(mic_preamp_params) },
	[0x1] = { .offset=0x100, .name = "2",  MEMAREA(mic_preamp_params) },
	[0x2] = { .offset=0x200, .name = "3",  MEMAREA(preamp_params) },
	[0x3] = { .offset=0x300, .name = "4",  MEMAREA(preamp_params) },
	[0x4] = { .offset=0x400, .name = "5",  MEMAREA(preamp_params) },
	[0x5] = { .offset=0x500, .name = "6",  MEMAREA(preamp_params) },
	[0x6] = { .offset=0x600, .name = "7",  MEMAREA(preamp_params) },
	[0x7] = { .offset=0x700, .name = "8",  MEMAREA(preamp_params) },
	[0x8] = { .offset=0x800, .name = "9",  MEMAREA(preamp_params) },
	[0x9] = { .offset=0x900, .name = "10", MEMAREA(preamp_params) },
	[0xa] = { .offset=0xa00, .name = "11", MEMAREA(preamp_params) },
	[0xb] = { .offset=0xb00, .name = "12", MEMAREA(preamp_params) },
	ENDA
};
DEF_MEMAREA(preamp) = { { .name="channel", MEMAREA(preamp_channels) }, ENDA };

DEF_MEMAREA(line_params) = {
	MEMNODE(0x0, stereo),
	MEMNODE(0x1, attenuation),
	ENDA
};
DEF_MEMAREA(line_channels) = {
	[0x0] = { .offset=0x000, .name = "13", MEMAREA(line_params) },
	[0x1] = { .offset=0x100, .name = "14", MEMAREA(line_params) },
	[0x2] = { .offset=0x200, .name = "15", MEMAREA(line_params) },
	[0x3] = { .offset=0x300, .name = "16", MEMAREA(line_params) },
	ENDA
};
DEF_MEMAREA(line) = { { .name="channel", MEMAREA(line_channels) }, ENDA };

DEF_MEMAREA(input) = {
	MEMNODE(0x0, stereo),
	MEMNODE(0x2, solo),
	MEMNODE(0x3, mute),
	MEMNODE(0x4, pan),
	MEMNODE(0x8, volume),
	MEMNODE(0xe, reverb),
	ENDA
};
DEF_MEMAREA(input_channels) = {
	[0x0] = { .offset=0x000, .name="1",  MEMAREA(input) },
	[0x1] = { .offset=0x100, .name="2",  MEMAREA(input) },
	[0x2] = { .offset=0x200, .name="3",  MEMAREA(input) },
	[0x3] = { .offset=0x300, .name="4",  MEMAREA(input) },
	[0x4] = { .offset=0x400, .name="5",  MEMAREA(input) },
	[0x5] = { .offset=0x500, .name="6",  MEMAREA(input) },
	[0x6] = { .offset=0x600, .name="7",  MEMAREA(input) },
	[0x7] = { .offset=0x700, .name="8",  MEMAREA(input) },
	[0x8] = { .offset=0x800, .name="9",  MEMAREA(input) },
	[0x9] = { .offset=0x900, .name="10", MEMAREA(input) },
	[0xa] = { .offset=0xa00, .name="11", MEMAREA(input) },
	[0xb] = { .offset=0xb00, .name="12", MEMAREA(input) },
	[0xc] = { .offset=0xc00, .name="13", MEMAREA(input) },
	[0xd] = { .offset=0xd00, .name="14", MEMAREA(input) },
	[0xe] = { .offset=0xe00, .name="15", MEMAREA(input) },
	[0xf] = { .offset=0xf00, .name="16", MEMAREA(input) },
	ENDA
};
DEF_MEMAREA(input_channel) = { { .name="channel", MEMAREA(input_channels) }, ENDA };
DEF_MEMAREA(input_monitor) = {
	[0x0] = { .offset=0x0000, .name="a", MEMAREA(input_channel) },
	[0x1] = { .offset=0x1000, .name="b", MEMAREA(input_channel) },
	[0x2] = { .offset=0x2000, .name="c", MEMAREA(input_channel) },
	[0x3] = { .offset=0x3000, .name="d", MEMAREA(input_channel) },
	ENDA
};

DEF_MEMAREA(daw) = {
	MEMNODE(0x0, stereo),
	MEMNODE(0x2, solo),
	MEMNODE(0x3, mute),
	MEMNODE(0x4, pan),
	MEMNODE(0x8, volume),
	ENDA
};
DEF_MEMAREA(daw_channels) = {
	[0x0] = { .offset=0x000, .name="1",  MEMAREA(daw) },
	[0x1] = { .offset=0x100, .name="2",  MEMAREA(daw) },
	[0x2] = { .offset=0x200, .name="3",  MEMAREA(daw) },
	[0x3] = { .offset=0x300, .name="4",  MEMAREA(daw) },
	[0x4] = { .offset=0x400, .name="5",  MEMAREA(daw) },
	[0x5] = { .offset=0x500, .name="6",  MEMAREA(daw) },
	[0x6] = { .offset=0x600, .name="7",  MEMAREA(daw) },
	[0x7] = { .offset=0x700, .name="8",  MEMAREA(daw) },
	[0x8] = { .offset=0x800, .name="9",  MEMAREA(daw) },
	[0x9] = { .offset=0x900, .name="10", MEMAREA(daw) },
	ENDA
};
DEF_MEMAREA(daw_channel) = { { .name="channel", MEMAREA(daw_channels) }, ENDA };
DEF_MEMAREA(daw_monitor) = {
	[0x0] = { .offset=0x0000, .name="a", MEMAREA(daw_channel) },
	[0x1] = { .offset=0x1000, .name="b", MEMAREA(daw_channel) },
	[0x2] = { .offset=0x2000, .name="c", MEMAREA(daw_channel) },
	[0x3] = { .offset=0x3000, .name="d", MEMAREA(daw_channel) },
	ENDA
};

DEF_MEMAREA(master_params) = {
	MEMNODE(0x0, stereo),
	MEMNODE(0x1, volume),
	ENDA
};
DEF_MEMAREA(direct_monitor_a) = {
	{ .offset=0x0000, .name="left"  , MEMAREA(master_params) },
	{ .offset=0x0100, .name="right" , MEMAREA(master_params) },
	{ .offset=0x0007, .name="reverb_return" },
	{ .offset=0x000d, .name="link"          },
	ENDA
};
DEF_MEMAREA(direct_monitor) = {
	{ .offset=0x0000, .name="left" ,  MEMAREA(master_params) },
	{ .offset=0x0100, .name="right",  MEMAREA(master_params) },
	{ .offset=0x000d, .name="link" },
	ENDA
};
DEF_MEMAREA(master_direct_monitors) = {
	[0x0] = { .offset=0x0000, .name="a", MEMAREA(direct_monitor_a) },
	[0x1] = { .offset=0x1000, .name="b", MEMAREA(direct_monitor) },
	[0x2] = { .offset=0x2000, .name="c", MEMAREA(direct_monitor) },
	[0x3] = { .offset=0x3000, .name="d", MEMAREA(direct_monitor) },
	ENDA
};
DEF_MEMAREA(daw_monitor_lr) = {
	[0x0] = { .offset=0x0000, .name = "left",  MEMAREA(master_params) },
	[0x1] = { .offset=0x0100, .name = "right", MEMAREA(master_params) },
	ENDA
};
DEF_MEMAREA(master_daw_monitors) = {
	[0x0] = { .offset=0x0000, .name="a", MEMAREA(daw_monitor_lr) },
	[0x1] = { .offset=0x1000, .name="b", MEMAREA(daw_monitor_lr) },
	[0x2] = { .offset=0x2000, .name="c", MEMAREA(daw_monitor_lr) },
	[0x3] = { .offset=0x3000, .name="d", MEMAREA(daw_monitor_lr) },
	ENDA
};

DEF_MEMAREA(master) = {
	[0x0] = { .offset=0x00000000, .name="direct_monitor", MEMAREA(master_direct_monitors) },
	[0x1] = { .offset=0x00010000, .name="daw_monitor",    MEMAREA(master_daw_monitors) },
	ENDA
};

MemMap memory_map[] = {
	[0x0] = { .offset = 0x00000002, .name = "initial_setting" },
	[0x3] = OFFSET_AREA(0x00030000, patchbay),
	[0x4] = OFFSET_AREA(0x00040000, reverb),
	[0x5] = OFFSET_AREA(0x00050000, preamp),
	[0x9] = OFFSET_AREA(0x00051000, line), // this is upsetting
	[0x6] = OFFSET_AREA(0x00060000, input_monitor),
	[0x7] = OFFSET_AREA(0x00070000, daw_monitor),
	[0x8] = OFFSET_AREA(0x00080000, master),
	[0xa] = { .offset = 0x000a0000, .name = "meters_active" },
	[0xf] = { .offset = 0x01000000, .name = "load_settings" },
	ENDA
};

MemMap memory_map_area = { .area = (MemMap **)&memory_map };
