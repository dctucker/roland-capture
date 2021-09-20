#define DEF_MEMAREA(NAME) static const capmix_mem_t NAME ## _area[] ///< define an area in device memory by name
#define MEMAREA(NAME) .area=(const capmix_mem_t **const)& NAME ## _area ///< assign the child area to the given named area

#define ENDA { .offset=capmix_None } ///< end of memory map, required for each memory area
#define MEMNODE( OFFSET, TYPE, NAME) [OFFSET] = { .name = NAME, .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map
#define CMEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = capmix_str.NAME[OFFSET] , .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map using a predefined capmix_str_t entry

#define PREAMP( OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for the preamp
#define CHANNEL(OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for an input channel
#define MONITOR(OFFSET, AREA) [OFFSET>>12] = { .offset=OFFSET, .name = capmix_str.monitors[OFFSET>>12], MEMAREA(AREA) } ///< define a child area in memory for a monitor mix
#define REVERB( OFFSET )      [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.reverb_types[OFFSET>>8], MEMAREA(reverb_params) } ///< define a child area in memory for a reverb type

#ifndef DOXYGEN_SKIP
DEF_MEMAREA(capmix_mem_none) = { { .name = "?" }, ENDA };

DEF_MEMAREA(patchbay_output) = { { .type=TPatch, .name=capmix_str.source }, ENDA };
DEF_MEMAREA(patchbay) = {
	{ .offset = 0, .name = capmix_str.patchbay[0], MEMAREA(patchbay_output) },
	{ .offset = 1, .name = capmix_str.patchbay[1], MEMAREA(patchbay_output) },
	{ .offset = 2, .name = capmix_str.patchbay[2], MEMAREA(patchbay_output) },
	{ .offset = 3, .name = capmix_str.patchbay[3], MEMAREA(patchbay_output) },
	{ .offset = 4, .name = capmix_str.patchbay[4], MEMAREA(patchbay_output) },
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

DEF_MEMAREA(meter_level) = { { .type=TMeter, .name="level" }, ENDA };
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
DEF_MEMAREA(meter_ins)  = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_gr)   = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_gate) = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };
DEF_MEMAREA(meter_outs) = { { .name=capmix_str.channel, MEMAREA(meter_1_12_level) }, ENDA };

DEF_MEMAREA(meter_lines_channel) = {
	{ .offset = 0x00, .name=capmix_str.channels[0xc], MEMAREA(meter_level) },
	{ .offset = 0x02, .name=capmix_str.channels[0xd], MEMAREA(meter_level) },
	{ .offset = 0x04, .name=capmix_str.channels[0xe], MEMAREA(meter_level) },
	{ .offset = 0x06, .name=capmix_str.channels[0xf], MEMAREA(meter_level) },
	ENDA
};
DEF_MEMAREA(meter_lines) = { { .name="channel", MEMAREA(meter_lines_channel) }, ENDA };

DEF_MEMAREA(meter_daw_channel) = {
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
	ENDA
};
DEF_MEMAREA(meter_daw)  = { { .name="channel", MEMAREA(meter_daw_channel) }, ENDA };

DEF_MEMAREA(meter_clip_mask) = { { .type=TClipMask, .name="mask" }, ENDA };
DEF_MEMAREA(meter_clips) = {
	{ .offset = 0x00, .name= "1-4" , MEMAREA(meter_clip_mask)},
	{ .offset = 0x01, .name= "5-8" , MEMAREA(meter_clip_mask)},
	{ .offset = 0x02, .name= "9-12", MEMAREA(meter_clip_mask)},
	{ .offset = 0x03, .name="13-16", MEMAREA(meter_clip_mask)},
	ENDA
};
DEF_MEMAREA(meter_clip) = {
	{ .offset = 0x00, .name=pre , MEMAREA(meter_clips) },
	{ .offset = 0x04, .name=post, MEMAREA(meter_clips) },
	ENDA
};

DEF_MEMAREA(meters) = {
	{ .offset=0x0000, .name=capmix_str.meters[0], .type=TBoolean },
	{ .offset=0x0001, .name=capmix_str.meters[1], MEMAREA(meter_ins  ) }, // pre-compressor
	{ .offset=0x0019, .name=capmix_str.meters[2], MEMAREA(meter_outs ) }, // post-compressor
	{ .offset=0x0029, .name=capmix_str.meters[3], MEMAREA(meter_lines) }, // line input
	{ .offset=0x0031, .name=capmix_str.meters[4], MEMAREA(meter_lines) }, // line input
	{ .offset=0x0039, .name=capmix_str.meters[5], MEMAREA(meter_daw  ) }, // daw output
	{ .offset=0x004d, .name=capmix_str.meters[6], MEMAREA(meter_gr   ) }, // compressor gain reduction
	{ .offset=0x0065, .name=capmix_str.meters[7], MEMAREA(meter_gate ) }, // gate
	{ .offset=0x0101, .name=capmix_str.meters[8], MEMAREA(meter_clip) },
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

DEF_MEMAREA(auto_sens_preamp_node) = { { .name=capmix_str.preamp_params[4] , .type=TSens }, ENDA };
DEF_MEMAREA(auto_sens_line_node)   = { { .name=capmix_str.channel_params[1], .type=TAttenuation }, ENDA };

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
	{ .offset=0x00, .name="auto"  , .type=TBoolean },
	{ .offset=0x01, .name="margin", .type=TByte }, // ugh, it's zero to twelve
	{ .offset=0x02, .name="mode"  , .type=TAutoSens },
	{ .offset=0x03, .name="reset" , .type=TBoolean },
	{ .offset=0x10, .name="select"          , MEMAREA(auto_sens_mask) },
	{ .offset=0x20, .name=capmix_str.channel, MEMAREA(auto_sens_channels) },
	ENDA
};

// auto sens on      : 02=1, 03=1
// auto sens cancel  : 02=2, 03=0
// auto sens off     : 02=0
DEF_MEMAREA(settings) = {
	{ .offset=0x0004, .name="input_selector_auto"  , .type=TBoolean },
	{ .offset=0x0005, .name="sync", .type=TBoolean }, // auto
	{ .offset=0x0006, .name="stereo-link", .type=TBoolean }, // paired
	{ .offset=0x0007, .name="dim_solo", .type=TByte }, // another TEnum: { OFF, -6, -12, -18 }
	{ .offset=0x0100, .name="auto-sens", MEMAREA(auto_sens) },
	ENDA
};
#endif

static const capmix_mem_t memory_map[] = {
	[0x0] = { .offset = 0x00000002 , .name = capmix_str.top_map[0x0] },
	[0x2] = { .offset = O_SETTINGS , .name = capmix_str.top_map[0x2] , MEMAREA(settings)     },
	[0x3] = { .offset = O_PATCHBAY , .name = capmix_str.top_map[0x3] , MEMAREA(patchbay)      },
	[0x4] = { .offset = O_REVERB   , .name = capmix_str.top_map[0x4] , MEMAREA(reverb)        },
	[0x5] = { .offset = O_PREAMP   , .name = capmix_str.top_map[0x5] , MEMAREA(preamp)        },
	[LINE]= { .offset = O_LINE     , .name = capmix_str.top_map[LINE], MEMAREA(line)          }, // this is upsetting
	[0x6] = { .offset = O_INPUT_MON, .name = capmix_str.top_map[0x6] , MEMAREA(input_monitor) },
	[0x7] = { .offset = O_DAW_MON  , .name = capmix_str.top_map[0x7] , MEMAREA(daw_monitor)   },
	[0x8] = { .offset = O_MASTER   , .name = capmix_str.top_map[0x8] , MEMAREA(master)        },
	[0xa] = { .offset = O_METERS   , .name = capmix_str.top_map[0xa] , MEMAREA(meters) },
	[0xf] = { .offset = 0x01000000 , .name = capmix_str.top_map[0xf] },
	ENDA
};
DEF_MEMAREA(top_map) = {
	{ .name="capmix", .area=(const capmix_mem_t **const)& memory_map },
	ENDA
};
