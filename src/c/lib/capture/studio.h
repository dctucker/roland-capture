#ifndef DOXYGEN_SKIP

DEF_MEMAREA(input_16_channels) = {
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
DEF_MEMAREA(input_16_channel) = { { .name=capmix_str.channel, MEMAREA(input_16_channels) }, ENDA };
DEF_MEMAREA(input_16_monitor) = {
	MONITOR(0x0000, input_16_channel),
	MONITOR(0x1000, input_16_channel),
	MONITOR(0x2000, input_16_channel),
	MONITOR(0x3000, input_16_channel),
	ENDA
};


DEF_MEMAREA(meter_1_12_level) = {
	{ .offset = 0x00, .name=capmix_str.channels[0x0], MEMAREA(meter_level) },
	{ .offset = 0x02, .name=capmix_str.channels[0x1], MEMAREA(meter_level) },
	{ .offset = 0x04, .name=capmix_str.channels[0x2], MEMAREA(meter_level) },
	{ .offset = 0x06, .name=capmix_str.channels[0x3], MEMAREA(meter_level) },
	{ .offset = 0x08, .name=capmix_str.channels[0x4], MEMAREA(meter_level) },
	{ .offset = 0x0a, .name=capmix_str.channels[0x5], MEMAREA(meter_level) },
	{ .offset = 0x0c, .name=capmix_str.channels[0x6], MEMAREA(meter_level) },
	{ .offset = 0x0e, .name=capmix_str.channels[0x7], MEMAREA(meter_level) },
	{ .offset = 0x10, .name=capmix_str.channels[0x8], MEMAREA(meter_level) },
	{ .offset = 0x12, .name=capmix_str.channels[0x9], MEMAREA(meter_level) },
	{ .offset = 0x14, .name=capmix_str.channels[0xa], MEMAREA(meter_level) },
	{ .offset = 0x16, .name=capmix_str.channels[0xb], MEMAREA(meter_level) },
	ENDA
};
DEF_MEMAREA(meter_12_ins)  = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_12_gr)   = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_12_gate) = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_12_outs) = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };

DEF_MEMAREA(meter_lines_channel) = {
	{ .offset = 0x00, .name=capmix_str.channels[0xc], MEMAREA(meter_level) },
	{ .offset = 0x02, .name=capmix_str.channels[0xd], MEMAREA(meter_level) },
	{ .offset = 0x04, .name=capmix_str.channels[0xe], MEMAREA(meter_level) },
	{ .offset = 0x06, .name=capmix_str.channels[0xf], MEMAREA(meter_level) },
	ENDA
};
DEF_MEMAREA(meter_lines) = { { .name="channel", MEMAREA(meter_lines_channel) }, ENDA };

DEF_MEMAREA(meter_16_clips) = {
	{ .offset = 0x00, .name= "1-4" , MEMAREA(meter_clip_mask)},
	{ .offset = 0x01, .name= "5-8" , MEMAREA(meter_clip_mask)},
	{ .offset = 0x02, .name= "9-12", MEMAREA(meter_clip_mask)},
	{ .offset = 0x03, .name="13-16", MEMAREA(meter_clip_mask)},
	ENDA
};
DEF_MEMAREA(meter_16_clip) = {
	{ .offset = 0x00, .name=pre , MEMAREA(meter_16_clips) },
	{ .offset = 0x04, .name=post, MEMAREA(meter_16_clips) },
	ENDA
};

DEF_MEMAREA(meters_16) = {
	{ .offset=0x0000, .name=capmix_str.meters[0], .type=TBoolean },
	{ .offset=0x0001, .name=capmix_str.meters[1], MEMAREA(meter_12_ins  ) }, // pre-compressor
	{ .offset=0x0019, .name=capmix_str.meters[2], MEMAREA(meter_12_outs ) }, // post-compressor
	{ .offset=0x0029, .name=capmix_str.meters[3], MEMAREA(meter_lines) }, // line input??
	{ .offset=0x0031, .name=capmix_str.meters[4], MEMAREA(meter_lines) }, // line input
	{ .offset=0x0039, .name=capmix_str.meters[5], MEMAREA(meter_daw  ) }, // daw output
	{ .offset=0x004d, .name=capmix_str.meters[6], MEMAREA(meter_12_gr   ) }, // compressor gain reduction
	{ .offset=0x0065, .name=capmix_str.meters[7], MEMAREA(meter_12_gate ) }, // gate
	{ .offset=0x0101, .name=capmix_str.meters[8], MEMAREA(meter_16_clip) },
	ENDA
};


DEF_MEMAREA(auto_sens_16_mask) = {
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


DEF_MEMAREA(auto_sens_16_channels) = {
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

DEF_MEMAREA(auto_sens_16) = {
	{ .offset=0x00, .name="auto"  , .type=TBoolean },
	{ .offset=0x01, .name="margin", .type=TByte }, // ugh, it's zero to twelve
	{ .offset=0x02, .name="mode"  , .type=TAutoSens },
	{ .offset=0x03, .name="reset" , .type=TBoolean },
	{ .offset=0x10, .name="select"          , MEMAREA(auto_sens_16_mask) },
	{ .offset=0x20, .name=capmix_str.channel, MEMAREA(auto_sens_16_channels) },
	ENDA
};

// auto sens on      : 02=1, 03=1
// auto sens cancel  : 02=2, 03=0
// auto sens off     : 02=0
DEF_MEMAREA(settings_16) = {
	{ .offset=0x0004, .name="input_selector_auto"  , .type=TBoolean },
	{ .offset=0x0005, .name="sync", .type=TBoolean }, // auto
	{ .offset=0x0006, .name="stereo-link", .type=TBoolean }, // paired
	{ .offset=0x0007, .name="dim_solo", .type=TByte }, // another TEnum: { OFF, -6, -12, -18 }
	{ .offset=0x0100, .name="auto-sens", MEMAREA(auto_sens_16) },
	ENDA
};
#endif

static const capmix_mem_t studio_memory_map[] = {
	[0x0] = { .offset = 0x00000002 , .name = capmix_str.top_map[0x0] },
	[0x2] = { .offset = O_SETTINGS , .name = capmix_str.top_map[0x2] , MEMAREA(settings_16)     },
	[0x3] = { .offset = O_PATCHBAY , .name = capmix_str.top_map[0x3] , MEMAREA(patchbay)      },
	[0x4] = { .offset = O_REVERB   , .name = capmix_str.top_map[0x4] , MEMAREA(reverb)        },
	[0x5] = { .offset = O_PREAMP   , .name = capmix_str.top_map[0x5] , MEMAREA(preamp)        },
	[LINE]= { .offset = O_LINE     , .name = capmix_str.top_map[LINE], MEMAREA(line)          }, // this is upsetting
	[0x6] = { .offset = O_INPUT_MON, .name = capmix_str.top_map[0x6] , MEMAREA(input_16_monitor) },
	[0x7] = { .offset = O_DAW_MON  , .name = capmix_str.top_map[0x7] , MEMAREA(daw_monitor)   },
	[0x8] = { .offset = O_MASTER   , .name = capmix_str.top_map[0x8] , MEMAREA(master)        },
	[0xa] = { .offset = O_METERS   , .name = capmix_str.top_map[0xa] , MEMAREA(meters_16) },
	[0xf] = { .offset = 0x01000000 , .name = capmix_str.top_map[0xf] },
	ENDA
};
DEF_MEMAREA(studio_top_map) = {
	{ .name="capmix", .area=(const capmix_mem_t **const)& studio_memory_map },
	ENDA
};
