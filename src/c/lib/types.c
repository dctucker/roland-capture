#include <math.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "types/byte.h"
#include "types/boolean.h"
#include "types/volume.h"
#include "types/pan.h"
#include "types/meter.h"
#include "types/clipmask.h"
#include "types/scaled.h"
#include "types/enum.h"
#include "types/unset.h"

/// the canonical array of type information used in this module
static capmix_type_info_t capmix_types[NTypes] = {
	[TValue] = {
		.type = TValue,
		.name = "Value",
		.unpack = capmix_unset_unpack,
		.parse  = capmix_unset_parse,
		.format = capmix_unset_format,
		.pack   = capmix_unset_pack,
	},
	[TByte] = {
		.type = TByte,
		.name = "Byte",
		.parent = TValue,
		.min = {.discrete= 0x0 }, .max = {.discrete= 0x7f }, .step = {.discrete= 0x1 },
		.unpack = capmix_byte_unpack,
		.parse  = capmix_byte_parse,
		.format = capmix_byte_format,
		.pack   = capmix_byte_pack,
	},
	[TBoolean] = {
		.type = TBoolean,
		.name = "Boolean",
		.parent = TByte,
		.min = {.discrete= 0x0 }, .max = {.discrete= 0x1 }, .step = {.discrete= 0x1 },
		.unpack = capmix_boolean_unpack,
		.parse  = capmix_boolean_parse,
		.format = capmix_boolean_format,
		//.pack = capmix_byte_pack,
	},
	[TVolume] = {
		.type = TVolume,
		.name = "Volume",
		.parent = TValue,
		.min = {.continuous = -71. }, .max = {.continuous = 12. }, .step = {.continuous = 1. },
		.unpack = capmix_volume_unpack,
		.parse  = capmix_volume_parse,
		.format = capmix_volume_format,
		.pack   = capmix_volume_pack,
	},
	[TPan] = {
		.type = TPan,
		.name = "Pan",
		.parent = TValue,
		.min = {.continuous = -100. }, .max = {.continuous = 100. }, .step = {.continuous = 1. },
		.unpack = capmix_pan_unpack,
		.parse  = capmix_pan_parse,
		.format = capmix_pan_format,
		.pack   = capmix_pan_pack,
	},
	[TMeter] = {
		.type = TMeter,
		.name = "Meter",
		.parent = TValue,
		.min = {.continuous = 0. }, .max = {.continuous = 1. }, .step = {.continuous = 0.01 },
		.unpack = capmix_meter_unpack,
		//.parse  = capmix_pan_parse,
		.format = capmix_meter_format,
		//.pack   = capmix_pan_pack,
	},

	[TScaled] = {
		.type = TScaled,
		.name = "Scaled",
		.parent  = TByte,
		.unpack  = capmix_scaled_unpack,
		//.parse = capmix_byte_parse,
		.format  = capmix_scaled_format,
		//.pack  = capmix_byte_pack,
	},
	[TSens] = {
		.type = TSens,
		.name = "Sens",
		.parent = TScaled,
		.min = {.continuous =   1. }, .max = {.continuous =  58. }, .step = {.continuous = 0.5 },
	},
	[TThreshold] = {
		.type = TThreshold,
		.name = "Threshold",
		.parent = TScaled,
		.min = {.continuous = -40. }, .max = {.continuous =   0. }, .step = {.continuous = 1. },
	},
	[TGain] = {
		.type = TGain,
		.name = "Gain",
		.parent = TScaled,
		.min = {.continuous = -40. }, .max = {.continuous =  40. }, .step = {.continuous = 1. },
	},
	[TGate] = {
		.type = TGate,
		.name = "Gate",
		.parent = TScaled,
		.min = {.continuous = -70. }, .max = {.continuous = -20. }, .step = {.continuous = 1. },
	},
	[TReverbTime] = {
		.type = TReverbTime,
		.name = "ReverbTime",
		.parent = TScaled,
		.min = {.continuous = 0.1 }, .max = {.continuous = 5. }, .step = {.continuous = 0.1 },
	},

	[TEnum] = {
		.type = TEnum,
		.name = "Enum",
		.parent = TByte,
		//.unpack = capmix_byte_unpack,
		.parse    = capmix_enum_parse,
		.format   = capmix_enum_format,
		//.pack   = capmix_byte_pack,
	},
	[TRatio] = {
		.type = TRatio,
		.name = "Ratio",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 13 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"1", "1.12", "1.25", "1.4", "1.6", "1.8", "2", "2.5", "3.2", "4", "5.6", "8", "16", "inf",
		},
	},
	[TAttack] = {
		.type = TAttack,
		.name = "Attack",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 124 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			 "0.0",  "0.1",  "0.2",  "0.3",  "0.4",  "0.5",  "0.6",  "0.7",  "0.8",  "0.9",
			 "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.7",  "1.8",  "1.9",
			 "2.0",  "2.1",  "2.2",  "2.4",  "2.5",  "2.7",  "2.8",  "3.0",  "3.2",  "3.3",  "3.6",  "3.8",
			 "4.0",  "4.2",  "4.5",  "4.7",  "5.0",  "5.3",  "5.6",  "6.0",  "6.3",  "6.7",  "7.1",  "7.5",
			 "8.0",  "8.4",  "9.0",  "9.4", "10.0", "10.6", "11.2", "12.0", "12.5", "13.3", "14.0", "15.0",
			"16.0", "17.0", "18.0", "19.0", "20.0", "21.0", "22.4", "23.7", "25.0", "26.6", "28.0", "30.0",
			"31.5", "33.5", "35.5", "37.6", "40.0", "42.2", "45.0", "47.3", "50.0", "53.0", "56.0", "60.0",
			"63.0", "67.0", "71.0", "75.0", "80.0", "84.0", "90.0", "94.4", "100",  "106",  "112",  "120",
			"125",  "133",  "140",  "150",  "160",  "170",  "180",  "190",  "200",  "210",  "224",  "237",
			"250",  "266",  "280",  "300",  "315",  "335",  "355",  "376",  "400",  "422",  "450",  "473",
			"500",  "530",  "560",  "600",  "630",  "670",  "710",  "750",  "800",
		},
	},
	[TRelease] = {
		.type = TRelease,
		.name = "Release",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 124 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			   "0",    "1",    "2",    "3",    "4",    "5",    "6",    "7",    "8",    "9",
			  "10",   "11",   "12",   "13",   "14",   "15",   "16",   "17",   "18",   "19",
			  "20",   "21",   "22",   "24",   "25",   "27",   "28",   "30",   "32",   "33",   "36",   "38",
			  "40",   "42",   "45",   "47",   "50",   "53",   "56",   "60",   "63",   "67",   "71",   "75",
			  "80",   "84",   "90",   "94",  "100",  "106",  "112",  "120",  "125",  "133",  "140",  "150",
			 "160",  "170",  "180",  "190",  "200",  "210",  "224",  "237",  "250",  "266",  "280",  "300",
			 "315",  "335",  "355",  "376",  "400",  "422",  "450",  "473",  "500",  "530",  "560",  "600",
			 "630",  "670",  "710",  "750",  "800",  "840",  "900",  "944", "1000", "1060", "1120", "1200",
			"1250", "1330", "1400", "1500", "1600", "1700", "1800", "1900", "2000", "2100", "2240", "2370",
			"2500", "2660", "2800", "3000", "3150", "3350", "3550", "3760", "4000", "4220", "4500", "4730",
			"5000", "5300", "5600", "6000", "6300", "6700", "7100", "7500", "8000",
		},
	},
	[TKnee] = {
		.type = TKnee,
		.name = "Knee",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 9 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"HARD", "SOFT1", "SOFT2", "SOFT3", "SOFT4", "SOFT5", "SOFT6", "SOFT7", "SOFT8", "SOFT9",
		},
	},
	[TAttenuation] = {
		.type = TAttenuation,
		.name = "Attenuation",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 2 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"-20","-10","+4",
		},
	},
	[TReverbType] = {
		.type = TReverbType,
		.name = "ReverbType",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 5 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"off", "echo", "room", "small_hall", "large_hall", "plate",
		},
	},
	[TPreDelay] = {
		.type = TPreDelay,
		.name = "PreDelay",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 12 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"0.0", "0.1", "0.2", "0.4", "0.8", "1.6", "3.2", "6.4", "10", "20", "40", "80", "160",
		},
	},
	[TPatch] = {
		.type = TPatch,
		.name = "Patch",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 8 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"MIX A", "MIX B", "MIX C", "MIX D",
			"WAVE 1/2", "WAVE 3/4", "WAVE 5/6", "WAVE 7/8", "WAVE 9/10",
		},
	},
	[TAutoSens] = {
		.type = TAutoSens,
		.name = "PAutoSens",
		.parent = TEnum,
		.min = {.discrete= 0 }, .max = {.discrete= 2 }, .step = {.discrete= 1 },
		.enum_names = (const char *[]){
			"off", "on", "cancel",
		},
	},
	[TClipMask] = {
		.type = TClipMask,
		.name = "ClipMask",
		.parent = TByte,
		.min = {.discrete= 0 }, .max = {.discrete= 0x7f}, .step = {.discrete= 0 },
		.format = capmix_clipmask_format,
	},
};

/**
 * @brief convert fixed value into an array where each byte represents four bits (`0x1234` -> `{0x01,0x02,0x03,0x04}`)
 * @param val the value to be converted
 * @param n   the number of bytes to produce
 * @param buf the buffer to write into
 */
void              capmix_fixed_to_nibbles (capmix_fixed val, int n, uint8_t *buf)
{
	// convert 0x0123 to [ 0x00, 0x01, 0x02, 0x03 ]
	capmix_fixed acc = val;
	for(int i = n - 1; i >= 0; i--)
	{
		buf[i] = acc & 0x0f;
		acc >>= 4;
	}
}

/**
 * @brief convert an array of bytes into a fixed value (`{0x0a,0x0b,0x0c,0x0d}` -> `0xabcd`)
 * @param buf the buffer to read
 * @param len the number of bytes to read in the buffer
 * @return a capmix_fixed integer
 */
capmix_fixed      capmix_nibbles_to_fixed (uint8_t *buf, int len)
{
	// convert [ 0x00, 0x01, 0x02, 0x03 ] to 0x0123
	capmix_fixed acc = 0;
	for(int i=0; i < len; i++)
	{
		acc <<= 4;
		acc += buf[i];
	}
	return acc;
}

/**
 * @brief get the type descriptor for a given type identifier
 * @param type the identifier of the type to be described
 * @return structure representing the constraints of the given type
 */
capmix_type_info_t *  capmix_type(capmix_type_t type)
{
	return &capmix_types[type];
}

/**
 * @brief convenience function to return the string name of the given type identifier
 * @param type the identifier of the type to be described
 * @return the string holding the type's name
 */
const char *        capmix_type_name         (capmix_type_t type)
{
	return capmix_types[type].name;
}

/**
 * @brief convenience function to return the number of bytes required to store the type's packed value
 * @param type the identifier of the type to be described
 * @return the number of bytes needed to pack the type's value
 */
int                 capmix_type_size         (capmix_type_t type)
{
	switch(type)
	{
		case TVolume : return 6;
		case TPan    : return 4;
		case TMeter  : return 2;
		default      : return 1;
	}
}

/**
 * @brief convert a packed value received from the device into a fixed 32-bit value
 * @param type the type of the value to be converted
 * @param data the buffer holding the value to be converted
 * @return a fixed 32-bit value
 */
capmix_fixed        capmix_fixed_from_packed (capmix_type_t type, uint8_t *data)
{
	int len = capmix_type_size(type);
	int fx;
	if( type == TMeter )
		fx = (((int)data[0] << 7) + (int)data[1]) & 0x3fff;
	else
		fx = capmix_nibbles_to_fixed(data, len);
	return fx;
}

/**
 * @brief unpacks raw data received from the device
 * @param type the type of the value to be unpacked
 * @param data the buffer holding the value to be converted
 * @return the unpacked value
 * @ingroup API
 */
capmix_unpacked_t  capmix_unpack_type       (capmix_type_t type, uint8_t *data)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	while( type_info->unpack == NULL )
	{
		t = type_info->parent;
		type_info = &capmix_types[t];
	}

	capmix_fixed fx = capmix_fixed_from_packed(type, data);
	return type_info->unpack(&capmix_types[type], fx);
}

/**
 * @brief parse a string into an unpacked value for a given type
 * @param type the type of the value to be parsed
 * @param str the string to be parsed
 * @return the unpacked value
 * @ingroup API
 */
capmix_unpacked_t  capmix_parse_type        (capmix_type_t type, const char *str)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	while( type_info->parse == NULL )
	{
		t = type_info->parent;
		type_info = &capmix_types[t];
	}
	return type_info->parse(&capmix_types[type], str);
}

/**
 * @brief format a string representation of the given unpacked value
 * @param type the type of the value to be printed
 * @param unpacked the value to format
 * @param str the string buffer to printed into
 * @return the length of the string produced
 * @ingroup API
 */
int                capmix_format_type       (capmix_type_t type, capmix_unpacked_t unpacked, char *str)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	if( unpacked.discrete == capmix_Unset )
		return capmix_unset_format(type_info, unpacked, str);
	while( type_info->format == NULL )
	{
		t = type_info->parent;
		type_info = &capmix_types[t];
	}
	type_info->format(&capmix_types[type], unpacked, str);
	return strlen(str);
}

/**
 * @brief write a packed value into a buffer for the given type
 * @param type the type of the value to be written
 * @param unpacked the value to be packed
 * @param buf the buffer to be written into
 * @return the length of the data written into the buffer
 * @ingroup API
 */
int                capmix_pack_type         (capmix_type_t type, capmix_unpacked_t unpacked, uint8_t *buf)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	if( unpacked.discrete == capmix_Unset )
		return capmix_unset_pack(type_info, unpacked, buf);
	while( type_info->pack == NULL )
	{
		t = type_info->parent;
		type_info = &capmix_types[t];
	}

	type_info->pack(&capmix_types[type], unpacked, buf);
	return type_info->size;
}
