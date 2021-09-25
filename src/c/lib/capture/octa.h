#ifndef DOXYGEN_SKIP

DEF_MEMAREA(input_10_channels) = {
	CHANNEL(0x000, input), CHANNEL(0x100, input),
	CHANNEL(0x200, input), CHANNEL(0x300, input),
	CHANNEL(0x400, input), CHANNEL(0x500, input),
	CHANNEL(0x600, input), CHANNEL(0x700, input),
	CHANNEL(0x800, input), CHANNEL(0x900, input),
	ENDA
};
DEF_MEMAREA(input_10_channel) = { { .name=capmix_str.channel, MEMAREA(input_10_channels) }, ENDA };
DEF_MEMAREA(input_10_monitor) = {
	MONITOR(0x0000, input_10_channel),
	MONITOR(0x1000, input_10_channel),
	MONITOR(0x2000, input_10_channel),
	MONITOR(0x3000, input_10_channel),
	ENDA
};


DEF_MEMAREA(meter_1_8_level) = {
	{ .offset = 0x00, .name=capmix_str.channels[0x0], MEMAREA(meter_level) },
	{ .offset = 0x02, .name=capmix_str.channels[0x1], MEMAREA(meter_level) },
	{ .offset = 0x04, .name=capmix_str.channels[0x2], MEMAREA(meter_level) },
	{ .offset = 0x06, .name=capmix_str.channels[0x3], MEMAREA(meter_level) },
	{ .offset = 0x08, .name=capmix_str.channels[0x4], MEMAREA(meter_level) },
	{ .offset = 0x0a, .name=capmix_str.channels[0x5], MEMAREA(meter_level) },
	{ .offset = 0x0c, .name=capmix_str.channels[0x6], MEMAREA(meter_level) },
	{ .offset = 0x0e, .name=capmix_str.channels[0x7], MEMAREA(meter_level) },
	ENDA
};
DEF_MEMAREA(meter_8_ins)  = { { .name=capmix_str.channel, MEMAREA(meter_1_8_level) }, ENDA };
DEF_MEMAREA(meter_8_gr)   = { { .name=capmix_str.channel, MEMAREA(meter_1_8_level) }, ENDA };
DEF_MEMAREA(meter_8_gate) = { { .name=capmix_str.channel, MEMAREA(meter_1_8_level) }, ENDA };
DEF_MEMAREA(meter_8_outs) = { { .name=capmix_str.channel, MEMAREA(meter_1_8_level) }, ENDA };

DEF_MEMAREA(meter_coax_channel) = {
	{ .offset = 0x00, .name=capmix_str.channels[0xc], MEMAREA(meter_level) },
	{ .offset = 0x02, .name=capmix_str.channels[0xd], MEMAREA(meter_level) },
	ENDA
};
DEF_MEMAREA(meter_coax) = { { .name="channel", MEMAREA(meter_coax_channel) }, ENDA };

DEF_MEMAREA(meter_8_clips) = {
	{ .offset = 0x00, .name= "1-4" , MEMAREA(meter_clip_mask)},
	{ .offset = 0x01, .name= "5-8" , MEMAREA(meter_clip_mask)},
	ENDA
};
DEF_MEMAREA(meter_8_clip) = {
	{ .offset = 0x00, .name=pre , MEMAREA(meter_8_clips) },
	{ .offset = 0x02, .name=post, MEMAREA(meter_8_clips) },
	ENDA
};

DEF_MEMAREA(meters_8) = {
	{ .offset=0x0000, .name=capmix_str.meters[0], .type=TBoolean },
	{ .offset=0x0001, .name=capmix_str.meters[1], MEMAREA(meter_8_ins  ) }, // pre-compressor
	{ .offset=0x0011, .name=capmix_str.meters[2], MEMAREA(meter_8_outs ) }, // post-compressor
	{ .offset=0x0021, .name=capmix_str.meters[3], MEMAREA(meter_coax   ) }, // coax inputs
	{ .offset=0x0024, .name=capmix_str.meters[5], MEMAREA(meter_daw    ) }, // daw output
	{ .offset=0x0039, .name=capmix_str.meters[6], MEMAREA(meter_8_gr   ) }, // compressor gain reduction
	{ .offset=0x0049, .name=capmix_str.meters[7], MEMAREA(meter_8_gate ) }, // gate
	{ .offset=0x0059, .name=capmix_str.meters[8], MEMAREA(meter_8_clip) },
	ENDA
};


DEF_MEMAREA(auto_sens_8_mask) = {
	SENS_MASK(1),
	SENS_MASK(2),
	SENS_MASK(3),
	SENS_MASK(4),
	SENS_MASK(5),
	SENS_MASK(6),
	SENS_MASK(7),
	SENS_MASK(8),
	ENDA
};


DEF_MEMAREA(auto_sens_8_channels) = {
	SENS(1),
	SENS(2),
	SENS(3),
	SENS(4),
	SENS(5),
	SENS(6),
	SENS(7),
	SENS(8),
	ENDA
};

DEF_MEMAREA(auto_sens_8) = {
	{ .offset=0x00, .name="auto"  , .type=TBoolean },
	{ .offset=0x01, .name="margin", .type=TByte }, // ugh, it's zero to twelve
	{ .offset=0x02, .name="mode"  , .type=TAutoSens },
	{ .offset=0x03, .name="reset" , .type=TBoolean },
	{ .offset=0x10, .name="select"          , MEMAREA(auto_sens_8_mask) },
	{ .offset=0x20, .name=capmix_str.channel, MEMAREA(auto_sens_8_channels) },
	ENDA
};

// auto sens on      : 02=1, 03=1
// auto sens cancel  : 02=2, 03=0
// auto sens off     : 02=0
DEF_MEMAREA(settings_8) = {
	{ .offset=0x0004, .name="input_selector_auto"  , .type=TBoolean },
	{ .offset=0x0005, .name="sync", .type=TBoolean }, // auto
	{ .offset=0x0006, .name="stereo-link", .type=TBoolean }, // paired
	{ .offset=0x0007, .name="dim_solo", .type=TByte }, // another TEnum: { OFF, -6, -12, -18 }
	{ .offset=0x0100, .name="auto-sens", MEMAREA(auto_sens_8) },
	ENDA
};

DEF_MEMAREA(octa_load) = {
	{ .name = "data", .type = TRawData },
};
#endif

static const capmix_mem_t octa_memory_map[17] = {
	[0x0] = { .offset = 0x00000002 , .name = capmix_str.top_map[0x0] },
	[0x2] = { .offset = O_SETTINGS , .name = capmix_str.top_map[0x2] , MEMAREA(settings_8)    },
	[0x3] = { .offset = O_PATCHBAY , .name = capmix_str.top_map[0x3] , MEMAREA(patchbay)      },
	[0x4] = { .offset = O_REVERB   , .name = capmix_str.top_map[0x4] , MEMAREA(reverb)        },
	[0x5] = { .offset = O_PREAMP   , .name = capmix_str.top_map[0x5] , MEMAREA(preamp)        },
	[0x6] = { .offset = O_INPUT_MON, .name = capmix_str.top_map[0x6] , MEMAREA(input_10_monitor) },
	[0x7] = { .offset = O_DAW_MON  , .name = capmix_str.top_map[0x7] , MEMAREA(daw_monitor)   },
	[0x8] = { .offset = O_MASTER   , .name = capmix_str.top_map[0x8] , MEMAREA(master)        },
	[0xa] = { .offset = O_METERS   , .name = capmix_str.top_map[0xa] , MEMAREA(meters_8)      },
	[0xf] = { .offset = O_LOAD     , .name = capmix_str.top_map[0xf] , MEMAREA(octa_load)     },
	ENDA
};
DEF_MEMAREA(octa_top_map) = {
	{ .name="capmix", .area=(const capmix_mem_t **const)& octa_memory_map },
	ENDA
};

const capmix_addr_t octa_load_map[2048] = {
	0,0,0,0,0,
	0x30000, 0x30001, 0x30002, 0x30003, 0x30004,
	0x40000,
	0x40101, 0x40201, 0x40301, 0x40401, 0x40501,
	0x40102, 0x40202, 0x40302, 0x40402, 0x40502,
	0x50000, 0x50400, // +48
	0x50001, 0x50401, // lo-cut
	0x50002, 0x50402, // phase
	0x50003, 0,       // hi-z, ?
	0x50004, 0x50104, 0x50204, 0x50304, 0x50404, 0x50504, 0x50604, 0x50704, // sens
	0x50005, // compressor stereo
	0x50006, 0x50406,
	0x50007, 0x50107, 0x50207, 0x50307, 0x50407, 0x50507, 0x50607, 0x50707, // gate
	0x50008, 0x50108, 0x50208, 0x50308, 0x50408, 0x50508, 0x50608, 0x50708,
	0x50009, 0x50109, 0x50209, 0x50309, 0x50409, 0x50509, 0x50609, 0x50709,
	0x5000a, 0x5010a, 0x5020a, 0x5030a, 0x5040a, 0x5050a, 0x5060a, 0x5070a,
	0x5000b, 0x5010b, 0x5020b, 0x5030b, 0x5040b, 0x5050b, 0x5060b, 0x5070b,
	0x5000c, 0x5010c, 0x5020c, 0x5030c, 0x5040c, 0x5050c, 0x5060c, 0x5070c, // gain
	0x60000, 0x60800, 0x61000, 0x61800, 0x62000, 0x62800, 0x63000, 0x63800, // a,b,c,d 1-8,9-10 stereo
	0,0,0,0,0,0,0,0, // ?
	0x60002, 0x60402, 0x60802, // solo a
	0x61002, 0x61402, 0x61802, // solo b
	0x62002, 0x62402, 0x62802, // solo c
	0x63002, 0x63402, 0x63802, // solo d
	0x60003, 0x60403, 0x60803, // mute a
	0x61003, 0x61403, 0x61803, // mute b
	0x62003, 0x62403, 0x62803, // mute c
	0x63003, 0x63403, 0x63803, // mute d
	0x60004, 0x60005, 0x60006, 0x60007, // pan a
	0x60104, 0x60105, 0x60106, 0x60107,
	0x60204, 0x60205, 0x60206, 0x60207,
	0x60304, 0x60305, 0x60306, 0x60307,
	0x60404, 0x60405, 0x60406, 0x60407,
	0x60504, 0x60505, 0x60506, 0x60507,
	0x60604, 0x60605, 0x60606, 0x60607,
	0x60704, 0x60705, 0x60706, 0x60707,
	0x60804, 0x60805, 0x60806, 0x60807,
	0x60904, 0x60905, 0x60906, 0x60907,
	0x61004, 0x61005, 0x61006, 0x61007, // pan b
	0x61104, 0x61105, 0x61106, 0x61107,
	0x61204, 0x61205, 0x61206, 0x61207,
	0x61304, 0x61305, 0x61306, 0x61307,
	0x61404, 0x61405, 0x61406, 0x61407,
	0x61504, 0x61505, 0x61506, 0x61507,
	0x61604, 0x61605, 0x61606, 0x61607,
	0x61704, 0x61705, 0x61706, 0x61707,
	0x61804, 0x61805, 0x61806, 0x61807,
	0x61904, 0x61905, 0x61906, 0x61907,
	0x62004, 0x62005, 0x62006, 0x62007, // pan c
	0x62104, 0x62105, 0x62106, 0x62107,
	0x62204, 0x62205, 0x62206, 0x62207,
	0x62304, 0x62305, 0x62306, 0x62307,
	0x62404, 0x62405, 0x62406, 0x62407,
	0x62504, 0x62505, 0x62506, 0x62507,
	0x62604, 0x62605, 0x62606, 0x62607,
	0x62704, 0x62705, 0x62706, 0x62707,
	0x62804, 0x62805, 0x62806, 0x62807,
	0x62904, 0x62905, 0x62906, 0x62907,
	0x63004, 0x63005, 0x63006, 0x63007, // pan d
	0x63104, 0x63105, 0x63106, 0x63107,
	0x63204, 0x63205, 0x63206, 0x63207,
	0x63304, 0x63305, 0x63306, 0x63307,
	0x63404, 0x63405, 0x63406, 0x63407,
	0x63504, 0x63505, 0x63506, 0x63507,
	0x63604, 0x63605, 0x63606, 0x63607,
	0x63704, 0x63705, 0x63706, 0x63707,
	0x63804, 0x63805, 0x63806, 0x63807,
	0x63904, 0x63905, 0x63906, 0x63907,
	0x60008, 0x60009, 0x6000a, 0x6000b, 0x6000c, 0x6000d, // volume a
	0x60108, 0x60109, 0x6010a, 0x6010b, 0x6010c, 0x6010d,
	0x60208, 0x60209, 0x6020a, 0x6020b, 0x6020c, 0x6020d,
	0x60308, 0x60309, 0x6030a, 0x6030b, 0x6030c, 0x6030d,
	0x60408, 0x60409, 0x6040a, 0x6040b, 0x6040c, 0x6040d,
	0x60508, 0x60509, 0x6050a, 0x6050b, 0x6050c, 0x6050d,
	0x60608, 0x60609, 0x6060a, 0x6060b, 0x6060c, 0x6060d,
	0x60708, 0x60709, 0x6070a, 0x6070b, 0x6070c, 0x6070d,
	0x60808, 0x60809, 0x6080a, 0x6080b, 0x6080c, 0x6080d,
	0x60908, 0x60909, 0x6090a, 0x6090b, 0x6090c, 0x6090d,
	0x61008, 0x61009, 0x6100a, 0x6100b, 0x6100c, 0x6100d, // volume b
	0x61108, 0x61109, 0x6110a, 0x6110b, 0x6110c, 0x6110d,
	0x61208, 0x61209, 0x6120a, 0x6120b, 0x6120c, 0x6120d,
	0x61308, 0x61309, 0x6130a, 0x6130b, 0x6130c, 0x6130d,
	0x61408, 0x61409, 0x6140a, 0x6140b, 0x6140c, 0x6140d,
	0x61508, 0x61509, 0x6150a, 0x6150b, 0x6150c, 0x6150d,
	0x61608, 0x61609, 0x6160a, 0x6160b, 0x6160c, 0x6160d,
	0x61708, 0x61709, 0x6170a, 0x6170b, 0x6170c, 0x6170d,
	0x61808, 0x61809, 0x6180a, 0x6180b, 0x6180c, 0x6180d,
	0x61908, 0x61909, 0x6190a, 0x6190b, 0x6190c, 0x6190d,
	0x62008, 0x62009, 0x6200a, 0x6200b, 0x6200c, 0x6200d, // volume c
	0x62108, 0x62109, 0x6210a, 0x6210b, 0x6210c, 0x6210d,
	0x62208, 0x62209, 0x6220a, 0x6220b, 0x6220c, 0x6220d,
	0x62308, 0x62309, 0x6230a, 0x6230b, 0x6230c, 0x6230d,
	0x62408, 0x62409, 0x6240a, 0x6240b, 0x6240c, 0x6240d,
	0x62508, 0x62509, 0x6250a, 0x6250b, 0x6250c, 0x6250d,
	0x62608, 0x62609, 0x6260a, 0x6260b, 0x6260c, 0x6260d,
	0x62708, 0x62709, 0x6270a, 0x6270b, 0x6270c, 0x6270d,
	0x62808, 0x62809, 0x6280a, 0x6280b, 0x6280c, 0x6280d,
	0x62908, 0x62909, 0x6290a, 0x6290b, 0x6290c, 0x6290d,
	0x63008, 0x63009, 0x6300a, 0x6300b, 0x6300c, 0x6300d, // volume d
	0x63108, 0x63109, 0x6310a, 0x6310b, 0x6310c, 0x6310d,
	0x63208, 0x63209, 0x6320a, 0x6320b, 0x6320c, 0x6320d,
	0x63308, 0x63309, 0x6330a, 0x6330b, 0x6330c, 0x6330d,
	0x63408, 0x63409, 0x6340a, 0x6340b, 0x6340c, 0x6340d,
	0x63508, 0x63509, 0x6350a, 0x6350b, 0x6350c, 0x6350d,
	0x63608, 0x63609, 0x6360a, 0x6360b, 0x6360c, 0x6360d,
	0x63708, 0x63709, 0x6370a, 0x6370b, 0x6370c, 0x6370d,
	0x63808, 0x63809, 0x6380a, 0x6380b, 0x6380c, 0x6380d,
	0x63908, 0x63909, 0x6390a, 0x6390b, 0x6390c, 0x6390d,
	0x6000e, 0x6000f, 0x60010, 0x60011, 0x60012, 0x60013, // reverb send
	0x6010e, 0x6010f, 0x60110, 0x60111, 0x60112, 0x60113,
	0x6020e, 0x6020f, 0x60210, 0x60211, 0x60212, 0x60213,
	0x6030e, 0x6030f, 0x60310, 0x60311, 0x60312, 0x60313,
	0x6040e, 0x6040f, 0x60410, 0x60411, 0x60412, 0x60413,
	0x6050e, 0x6050f, 0x60510, 0x60511, 0x60512, 0x60513,
	0x6060e, 0x6060f, 0x60610, 0x60611, 0x60612, 0x60613,
	0x6070e, 0x6070f, 0x60710, 0x60711, 0x60712, 0x60713,
	0x6080e, 0x6080f, 0x60810, 0x60811, 0x60812, 0x60813,
	0x6090e, 0x6090f, 0x60910, 0x60911, 0x60912, 0x60913,
	0x70000, 0x70800, 0x71000, 0x71800, 0x72000, 0x72800, 0x73000, 0x73800, // DAW a,b,c,d 1-8,9-10 stereo
	0,0,0,0,0,0,0,0, // ?
	0x70002, 0x70402, 0x70802, // solo a
	0x71002, 0x71402, 0x71802, // solo b
	0x72002, 0x72402, 0x72802, // solo c
	0x73002, 0x73402, 0x73802, // solo d
	0x70003, 0x70403, 0x70803, // mute a
	0x71003, 0x71403, 0x71803, // mute b
	0x72003, 0x72403, 0x72803, // mute c
	0x73003, 0x73403, 0x73803, // mute d
	0x70004, 0x70005, 0x70006, 0x70007, // pan a
	0x70104, 0x70105, 0x70106, 0x70107,
	0x70204, 0x70205, 0x70206, 0x70207,
	0x70304, 0x70305, 0x70306, 0x70307,
	0x70404, 0x70405, 0x70406, 0x70407,
	0x70504, 0x70505, 0x70506, 0x70507,
	0x70604, 0x70605, 0x70606, 0x70607,
	0x70704, 0x70705, 0x70706, 0x70707,
	0x70804, 0x70805, 0x70806, 0x70807,
	0x70904, 0x70905, 0x70906, 0x70907,
	0x71004, 0x71005, 0x71006, 0x71007, // pan b
	0x71104, 0x71105, 0x71106, 0x71107,
	0x71204, 0x71205, 0x71206, 0x71207,
	0x71304, 0x71305, 0x71306, 0x71307,
	0x71404, 0x71405, 0x71406, 0x71407,
	0x71504, 0x71505, 0x71506, 0x71507,
	0x71604, 0x71605, 0x71606, 0x71607,
	0x71704, 0x71705, 0x71706, 0x71707,
	0x71804, 0x71805, 0x71806, 0x71807,
	0x71904, 0x71905, 0x71906, 0x71907,
	0x72004, 0x72005, 0x72006, 0x72007, // pan c
	0x72104, 0x72105, 0x72106, 0x72107,
	0x72204, 0x72205, 0x72206, 0x72207,
	0x72304, 0x72305, 0x72306, 0x72307,
	0x72404, 0x72405, 0x72406, 0x72407,
	0x72504, 0x72505, 0x72506, 0x72507,
	0x72604, 0x72605, 0x72606, 0x72607,
	0x72704, 0x72705, 0x72706, 0x72707,
	0x72804, 0x72805, 0x72806, 0x72807,
	0x72904, 0x72905, 0x72906, 0x72907,
	0x73004, 0x73005, 0x73006, 0x73007, // pan d
	0x73104, 0x73105, 0x73106, 0x73107,
	0x73204, 0x73205, 0x73206, 0x73207,
	0x73304, 0x73305, 0x73306, 0x73307,
	0x73404, 0x73405, 0x73406, 0x73407,
	0x73504, 0x73505, 0x73506, 0x73507,
	0x73604, 0x73605, 0x73606, 0x73607,
	0x73704, 0x73705, 0x73706, 0x73707,
	0x73804, 0x73805, 0x73806, 0x73807,
	0x73904, 0x73905, 0x73906, 0x73907,
	0x70008, 0x70009, 0x7000a, 0x7000b, 0x7000c, 0x7000d, // volume a
	0x70108, 0x70109, 0x7010a, 0x7010b, 0x7010c, 0x7010d,
	0x70208, 0x70209, 0x7020a, 0x7020b, 0x7020c, 0x7020d,
	0x70308, 0x70309, 0x7030a, 0x7030b, 0x7030c, 0x7030d,
	0x70408, 0x70409, 0x7040a, 0x7040b, 0x7040c, 0x7040d,
	0x70508, 0x70509, 0x7050a, 0x7050b, 0x7050c, 0x7050d,
	0x70608, 0x70609, 0x7060a, 0x7060b, 0x7060c, 0x7060d,
	0x70708, 0x70709, 0x7070a, 0x7070b, 0x7070c, 0x7070d,
	0x70808, 0x70809, 0x7080a, 0x7080b, 0x7080c, 0x7080d,
	0x70908, 0x70909, 0x7090a, 0x7090b, 0x7090c, 0x7090d,
	0x71008, 0x71009, 0x7100a, 0x7100b, 0x7100c, 0x7100d, // volume b
	0x71108, 0x71109, 0x7110a, 0x7110b, 0x7110c, 0x7110d,
	0x71208, 0x71209, 0x7120a, 0x7120b, 0x7120c, 0x7120d,
	0x71308, 0x71309, 0x7130a, 0x7130b, 0x7130c, 0x7130d,
	0x71408, 0x71409, 0x7140a, 0x7140b, 0x7140c, 0x7140d,
	0x71508, 0x71509, 0x7150a, 0x7150b, 0x7150c, 0x7150d,
	0x71608, 0x71609, 0x7160a, 0x7160b, 0x7160c, 0x7160d,
	0x71708, 0x71709, 0x7170a, 0x7170b, 0x7170c, 0x7170d,
	0x71808, 0x71809, 0x7180a, 0x7180b, 0x7180c, 0x7180d,
	0x71908, 0x71909, 0x7190a, 0x7190b, 0x7190c, 0x7190d,
	0x72008, 0x72009, 0x7200a, 0x7200b, 0x7200c, 0x7200d, // volume c
	0x72108, 0x72109, 0x7210a, 0x7210b, 0x7210c, 0x7210d,
	0x72208, 0x72209, 0x7220a, 0x7220b, 0x7220c, 0x7220d,
	0x72308, 0x72309, 0x7230a, 0x7230b, 0x7230c, 0x7230d,
	0x72408, 0x72409, 0x7240a, 0x7240b, 0x7240c, 0x7240d,
	0x72508, 0x72509, 0x7250a, 0x7250b, 0x7250c, 0x7250d,
	0x72608, 0x72609, 0x7260a, 0x7260b, 0x7260c, 0x7260d,
	0x72708, 0x72709, 0x7270a, 0x7270b, 0x7270c, 0x7270d,
	0x72808, 0x72809, 0x7280a, 0x7280b, 0x7280c, 0x7280d,
	0x72908, 0x72909, 0x7290a, 0x7290b, 0x7290c, 0x7290d,
	0x73008, 0x73009, 0x7300a, 0x7300b, 0x7300c, 0x7300d, // volume d
	0x73108, 0x73109, 0x7310a, 0x7310b, 0x7310c, 0x7310d,
	0x73208, 0x73209, 0x7320a, 0x7320b, 0x7320c, 0x7320d,
	0x73308, 0x73309, 0x7330a, 0x7330b, 0x7330c, 0x7330d,
	0x73408, 0x73409, 0x7340a, 0x7340b, 0x7340c, 0x7340d,
	0x73508, 0x73509, 0x7350a, 0x7350b, 0x7350c, 0x7350d,
	0x73608, 0x73609, 0x7360a, 0x7360b, 0x7360c, 0x7360d,
	0x73708, 0x73709, 0x7370a, 0x7370b, 0x7370c, 0x7370d,
	0x73808, 0x73809, 0x7380a, 0x7380b, 0x7380c, 0x7380d,
	0x73908, 0x73909, 0x7390a, 0x7390b, 0x7390c, 0x7390d,
	0x80000, 0x81000, 0x82000, 0x83000, // monitor A,B,C,D stereo
	0x80001, 0x80002, 0x80003, 0x80004, 0x80005, 0x80006, // monitor a volume
	0x80101, 0x80102, 0x80103, 0x80104, 0x80105, 0x80106,
	0x81001, 0x81002, 0x81003, 0x81004, 0x81005, 0x81006, // monitor b volume
	0x81101, 0x81102, 0x81103, 0x81104, 0x81105, 0x81106,
	0x82001, 0x82002, 0x82003, 0x82004, 0x82005, 0x82006, // monitor c volume
	0x82101, 0x82102, 0x82103, 0x82104, 0x82105, 0x82106,
	0x83001, 0x83002, 0x83003, 0x83004, 0x83005, 0x83006, // monitor d volume
	0x83101, 0x83102, 0x83103, 0x83104, 0x83105, 0x83106,
	0x80007, 0x80008, 0x80009, 0x8000a, 0x8000b, 0x8000c, // reverb return
	0x8000d, 0x8100d, 0x8200d, 0x8300d, // master link a,b,c,d
	0,0,0,0, // ?
	0x90000, 0x91000, 0x92000, 0x93000, // master daw stereo
	0x90001, 0x90002, 0x90003, 0x90004, 0x90005, 0x90006, // monitor a volume
	0x90101, 0x90102, 0x90103, 0x90104, 0x90105, 0x90106,
	0x91001, 0x91002, 0x91003, 0x91004, 0x91005, 0x91006, // monitor b volume
	0x91101, 0x91102, 0x91103, 0x91104, 0x91105, 0x91106,
	0x92001, 0x92002, 0x92003, 0x92004, 0x92005, 0x92006, // monitor c volume
	0x92101, 0x92102, 0x92103, 0x92104, 0x92105, 0x92106,
	0x93001, 0x93002, 0x93003, 0x93004, 0x93005, 0x93006, // monitor d volume
	0x93101, 0x93102, 0x93103, 0x93104, 0x93105, 0x93106,
};
