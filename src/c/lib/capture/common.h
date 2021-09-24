DEF_MEMAREA(capmix_mem_none) = { { .name = "?" }, ENDA };

static const capmix_mem_t none_memory_map[] = {
	{ .name = "capmix", .area=(const capmix_mem_t **const)&(capmix_mem_none_area) },
	ENDA
};

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

DEF_MEMAREA(input) = {
	MEMNODE(0x0, Boolean, capmix_str.channel_params[0x0]),
	MEMNODE(0x2, Boolean, capmix_str.channel_params[0x2]),
	MEMNODE(0x3, Boolean, capmix_str.channel_params[0x3]),
	MEMNODE(0x4, Pan    , capmix_str.channel_params[0x4]),
	MEMNODE(0x8, Volume , capmix_str.channel_params[0x8]),
	MEMNODE(0xe, Volume , capmix_str.channel_params[0xe]),
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

DEF_MEMAREA(auto_sens_mask_node) = { { .name="execute", .type=TBoolean }, ENDA };
#define SENS_MASK(N) [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_mask_node) }
#define SENS(N)  [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_preamp_node) }
#define ATTEN(N) [N-1] = { .offset=N-1, .name=#N, MEMAREA(auto_sens_line_node) }
DEF_MEMAREA(auto_sens_preamp_node) = { { .name=capmix_str.preamp_params[4] , .type=TSens }, ENDA };
DEF_MEMAREA(auto_sens_line_node)   = { { .name=capmix_str.channel_params[1], .type=TAttenuation }, ENDA };
