#include "strings.h"

const char *capmix_monitors[] = { "a","b","c","d", };
const char *capmix_channels[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", };

const char *capmix_top_map[] = {
	[3] = "patchbay",
	[4] = "reverb",
	[5] = "preamp",
	[6] = "input_monitor",
	[7] = "daw_monitor",
	[8] = "input_master",
	[9] = "daw_master",
};

const char *capmix_input_names[] = {
	[0x0] = "stereo",
	[0x2] = "solo",
	[0x3] = "mute",
	[0x4] = "pan",
	[0x8] = "volume",
	[0xe] = "reverb",
};

const char *capmix_preamp_names[] = {
	[0x0] = "+48"       ,
	[0x1] = "lo-cut"    ,
	[0x2] = "phase"     ,
	[0x3] = "hi-z"      ,
	[0x4] = "sens"      ,
	[0x5] = "stereo"    ,
	[0x6] = "bypass"    ,
	[0x7] = "gate"      ,
	[0x8] = "attack"    ,
	[0x9] = "release"   ,
	[0xa] = "threshold" ,
	[0xb] = "ratio"     ,
	[0xc] = "gain"      ,
	[0xd] = "knee"      ,
};

const char *capmix_master_names[] = {
	[0x0] = "stereo",
	[0x1] = "volume",
};

const char *capmix_master_left_right[] = {
	[0x0] = "left",
	[0x1] = "right",
};

const char *capmix_reverb_params[] = {
	[0x01] = "pre_delay" ,
	[0x02] = "time"      ,
};

