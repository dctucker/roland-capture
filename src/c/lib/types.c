#include <math.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

#ifndef DOXYGEN_SKIP
#define UNPACK(NAME) static capmix_unpacked_t capmix_unpack_##NAME (capmix_type_info_t *type_info, capmix_fixed value)
#define PARSE(NAME)  static capmix_unpacked_t capmix_parse_##NAME  (capmix_type_info_t *type_info, const char *str)
#define FORMAT(NAME) static int               capmix_format_##NAME (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str)
#define PACK(NAME)   static int               capmix_pack_##NAME   (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf)
#define TYPE_NAME(NAME) [ T##NAME ] = #NAME
#define MARSHALL( OP, DEFAULT ) \
	capmix_type_t t = type;\
	capmix_type_info_t *type_info = &capmix_types[t];\
	while( type_info->OP == NULL ){\
		t = type_info->parent;\
		if( t == TValue ){\
			DEFAULT; }\
		type_info = &capmix_types[t]; }
#endif

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
 * @brief convert a signed floating point value representing dB into a fixed integer value
 * @param db the value in decibels to be converted ranging from -inf to +12
 * @return a capmix_fixed integer where +12 dB is represented as `0x800000`, +0 dB as `0x200000`, -3 dB as `0x100000`, and -INFINITY as `0x000000`
 */
capmix_fixed      capmix_db_to_fixed      (float db)
{
	// convert -6.02 to 0x100000
	return (int)(pow(10., db/20.) * 0x200000);
}
/**
 * @brief convert a fixed integer value into a floating point value representing dB
 * @param fx the fixed value to be converted
 * @return a signed floating point value representing decibels ranging from -inf to +12
 */
float             capmix_fixed_to_db      (capmix_fixed fx)
{
	// convert 0x200000 to 0
	float ratio = fx / 2097152.;
	if( ratio == 0 ) return -INFINITY;
	return 20.*log10(ratio);
}
/**
 * @brief convert a signed floating point value representing left-right pan into a fixed integer value
 * @param db the floating point value to be converted ranging from -100 on the left to +100 on the right
 * @return a capmix_fixed integer where Left 100% is represented as `0x0000`, Center as `0x4000`, and Right 100% as `0x8000`
 */
float             capmix_fixed_to_pan     (capmix_fixed fx)
{
	return round(100. * (fx - 16384.) / 16384.);
}
/**
 * @brief convert a fixed integer value into a signed floating point value representing left-right pan
 * @param fx the capmix_fixed integer to be converted where L 100% is represented as 0x0000, Center as 0x4000, and Right 100% as 0x8000
 * @return a signed floating point value ranging from -100 on the left to +100 on the right
 */
capmix_fixed      capmix_pan_to_fixed     (float pan)
{
	return (int)(16384. + (16384. * (pan / 100.))) & 0xffff;
}

#ifndef DOXYGEN_SKIP
UNPACK(byte)
{
	return capmix_UnpackedInt(value & 0x7f);
}
PARSE(byte)
{
	int value;
	if( sscanf(str, "%d", &value) > 0 )
		return capmix_UnpackedInt(value);
	return capmix_UnsetInt;
}
FORMAT(byte)
{
	if( unpacked.as_int == capmix_Unset )
		sprintf(str, "?");
	else
		sprintf(str, "0x%02x", unpacked.as_int);
	return strlen(str);
}
PACK(byte)
{
	buf[0] = unpacked.as_int & 0x7f;
	return type_info->size;
}

UNPACK(boolean)
{
	return capmix_UnpackedInt(value == 0 ? 0 : 1);
}
PARSE(boolean)
{
	if( strcmp(str, "0")==0 || strcasecmp(str, "off")==0 || strcasecmp(str,"false")==0 )
		return capmix_UnpackedInt(0);
	else if( strcmp(str,"1")==0 || strcasecmp(str, "on")==0 || strcasecmp(str,"true")==0 )
		return capmix_UnpackedInt(1);
	return capmix_UnsetInt;
}
FORMAT(boolean)
{
	if( unpacked.as_int )
		sprintf(str, "ON");
	else
		sprintf(str, "off");
	return strlen(str);
}
PACK(boolean)
{
	buf[0] = unpacked.as_int == 0 ? 0 : 1;
	return type_info->size;
}

UNPACK(volume)
{
	float db = capmix_fixed_to_db(value);
	return capmix_UnpackedFloat(db);
}
PARSE(volume)
{
	float value;
	if( sscanf(str, "%f", &value) > 0 )
		return capmix_UnpackedFloat(value);
	return capmix_UnsetInt;
}
FORMAT(volume)
{
	sprintf(str, "%+1.0f", unpacked.as_float);
	return strlen(str);
}
PACK(volume)
{
	float value = unpacked.as_float;
	capmix_fixed fx = capmix_db_to_fixed(value);
	if( fx > 0x7fffff ) fx = 0x7fffff;
	if( value != -INFINITY && round(value) == 0 )
		fx = 0x200000;
	capmix_fixed_to_nibbles(fx, 6, buf);
	return type_info->size;
}

UNPACK(pan)
{
	float db = capmix_fixed_to_pan(value);
	return capmix_UnpackedFloat(db);
}
PARSE(pan)
{
	char side;
	float value;
	if( sscanf(str, "%c%f", &side, &value) > 0 )
	{
		if( side == 'l' || side == 'L' )
			return capmix_UnpackedFloat(-value);
		else if( side == 'r' || side == 'R' )
			return capmix_UnpackedFloat(fabs(value));
		else if( side == 'c' || side == 'C' )
			return capmix_UnpackedFloat(0);
		return capmix_UnsetInt;
	}
	return capmix_UnsetInt;
}
FORMAT(pan)
{
	float pan = unpacked.as_float;
	if( pan < 0 )
		sprintf(str, "L%1.0f", fabs(pan));
	else if( pan > 0 )
		sprintf(str, "R%1.0f", pan);
	else
		sprintf(str, "C");
	return strlen(str);
}
PACK(pan)
{
	float value = unpacked.as_float;
	capmix_fixed fx = capmix_pan_to_fixed(value);
	capmix_fixed_to_nibbles(fx, 4, buf);
	return type_info->size;
}

UNPACK(scaled)
{
	return capmix_UnpackedFloat(type_info->min_f + type_info->step_f * value);
}
//PARSE(scaled) // # TODO
FORMAT(scaled)
{
	if( type_info->step == 1. )
		sprintf(str, "%1.0f", unpacked.as_float);
	else
		sprintf(str, "%1.1f", unpacked.as_float);
	return strlen(str);
}
PACK(scaled)
{
	uint8_t value = (unpacked.as_float - type_info->min_f) / type_info->step_f;
	*buf = value;
	return type_info->size;
}

//UNPACK(enum) // unpack_byte
PARSE(enum)
{
	for(int i=0; i < type_info->max; i++)
	{
		const char *name = type_info->enum_names[i];
		if( strcmp(str, name) == 0 )
			return capmix_UnpackedInt(i);
	}
	return capmix_UnpackedInt(capmix_Unset);
}
FORMAT(enum)
{
	uint32_t v = unpacked.as_int;
	if( v > type_info->max )
	{
		warn("\nvalue too large 0x%x\n", v);
		sprintf(str, "?");
		return 1;
	}
	const char *name = type_info->enum_names[v];
	sprintf(str, "%s", name);
	return strlen(str);
}
//PACK(enum) // pack_byte

/*
void             capmix_format_packed       (capmix_type_t type, uint8_t *data, char *str)
{
	int len = capmix_type_size(type);
	capmix_fixed fx = capmix_nibbles_to_fixed(data, len);
	capmix_unpacked_t unpacked = capmix_unpack_volume(type, fx);
	capmix_format_type(type, unpacked, str);
}
*/

capmix_unpacked_t capmix_unpack_unset       (capmix_type_info_t *type_info, capmix_fixed fx)
{
	return capmix_UnsetInt;
}
capmix_unpacked_t capmix_parse_unset        (capmix_type_info_t *type_info, const char *data)
{
	return capmix_UnsetInt;
}
int capmix_format_unset                     (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str)
{
	str[0] = '?'; str[1] = '\0'; return 1;
}
int capmix_pack_unset                       (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf)
{
	return 0;
}
#endif

/// the canonical array of type information used in this module
static capmix_type_info_t capmix_types[NTypes] = {
	[TValue] = {
		.type = TValue,
		.name = "Value",
		.unpack = capmix_unpack_unset,
		.parse = capmix_parse_unset,
		.format = capmix_format_unset,
		.pack = capmix_pack_unset,
	},
	[TByte] = {
		.type = TByte,
		.name = "Byte",
		.parent = TValue,
		.min = 0x0, .max = 0x7f, .step = 0x1,
		.unpack = capmix_unpack_byte,
		.parse = capmix_parse_byte,
		.format = capmix_format_byte,
		.pack = capmix_pack_byte,
	},
	[TBoolean] = {
		.type = TBoolean,
		.name = "Boolean",
		.parent = TByte,
		.min = 0x0, .max = 0x1, .step = 0x1,
		.unpack = capmix_unpack_boolean,
		.parse = capmix_parse_boolean,
		.format = capmix_format_boolean,
	},
	[TVolume] = {
		.type = TVolume,
		.name = "Volume",
		.parent = TValue,
		.min_f = -71., .max_f = 12., .step_f = 1.,
		.unpack = capmix_unpack_volume,
		.parse = capmix_parse_volume,
		.format = capmix_format_volume,
		.pack = capmix_pack_volume,
	},
	[TPan] = {
		.type = TPan,
		.name = "Pan",
		.parent = TValue,
		.min_f = -100., .max_f = 100., .step_f = 1.,
		.unpack = capmix_unpack_pan,
		.parse = capmix_parse_pan,
		.format = capmix_format_pan,
		.pack = capmix_pack_pan,
	},

	[TScaled] = {
		.type = TScaled,
		.name = "Scaled",
		.parent = TByte,
		.unpack = capmix_unpack_scaled,
		//.parse = capmix_parse_byte,
		.format = capmix_format_scaled,
	},
	[TSens] = {
		.type = TSens,
		.name = "Sens",
		.parent = TScaled,
		.min_f =   1., .max_f =  58., .step_f = 0.5,
	},
	[TThreshold] = {
		.type = TThreshold,
		.name = "Threshold",
		.parent = TScaled,
		.min_f = -40., .max_f =   0., .step_f = 1.,
	},
	[TGain] = {
		.type = TGain,
		.name = "Gain",
		.parent = TScaled,
		.min_f = -40., .max_f =  40., .step_f = 1.,
	},
	[TGate] = {
		.type = TGate,
		.name = "Gate",
		.parent = TScaled,
		.min_f = -70., .max_f = -20., .step_f = 1.,
	},
	[TReverbTime] = {
		.type = TReverbTime,
		.name = "ReverbTime",
		.parent = TScaled,
		.min_f =  0.1, .max_f =  5., .step_f = 0.1,
	},

	[TEnum] = {
		.type = TEnum,
		.name = "Enum",
		.parent = TByte,
		//.unpack = capmix_unpack_byte,
		.parse = capmix_parse_enum,
		.format = capmix_format_enum,
		//.pack = capmix_pack_byte,
	},
	[TRatio] = {
		.type = TRatio,
		.name = "Ratio",
		.parent = TEnum,
		.min = 0, .max = 13, .step = 1,
		.enum_names = (const char *[]){
			"1", "1.12", "1.25", "1.4", "1.6", "1.8", "2", "2.5", "3.2", "4", "5.6", "8", "16", "inf",
		},
	},
	[TAttack] = {
		.type = TAttack,
		.name = "Attack",
		.parent = TEnum,
		.min = 0, .max = 124, .step = 1,
		.enum_names = (const char *[]){
			 "0.0",  "0.1",  "0.2",  "0.3",  "0.4",  "0.5",  "0.6",  "0.7",  "0.8",  "0.9",
			 "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.7",  "1.8",  "1.9",
			 "2.0",  "2.1",  "2.2",  "2.4",  "2.5",  "2.7",  "2.8",  "3.0",  "3.2",  "3.3",  "3.6",  "3.8",
			 "4.0",  "4.2",  "4.5",  "4.7",  "5.0",  "5.3",  "5.6",  "6.0",  "6.3",  "6.7",  "7.1",  "7.5",
			 "8.0",  "8.4",  "9.0",  "9.4", "10.0", "10.6", "11.2", "12.0", "12.5", "13.3", "14.0",
			"15.0", "16.0", "17.0", "18.0", "19.0", "20.0", "21.0", "22.4", "23.7", "25.0", "26.6", "28.0",
			"30.0", "31.5", "33.5", "35.5", "37.6", "40.0", "42.2", "45.0", "47.3", "50.0", "53.0", "56.0",
			"60.0", "63.0", "67.0", "71.0", "75.0", "80.0", "84.0", "90.0", "94.4", "100",  "106",  "112",
			"120",  "125",  "133",  "140",  "150",  "160",  "170",  "180",  "190",  "200",  "210",  "224",
			"237",  "250",  "266",  "280",  "300",  "315",  "335",  "355",  "376",  "400",  "422",  "450",
			"473",  "500",  "530",  "560",  "600",  "630",  "670",  "710",  "750",  "800",
		},
	},
	[TRelease] = {
		.type = TRelease,
		.name = "Release",
		.parent = TEnum,
		.min = 0, .max = 124, .step = 1,
		.enum_names = (const char *[]){
			 "0",      "1",    "2",    "3",    "4",    "5",    "6",    "7",    "8",    "9",
			 "10",    "11",   "12",   "13",   "14",   "15",   "16",   "17",   "18",   "19",
			 "20",    "21",   "22",   "24",   "25",   "27",   "28",   "30",   "32",   "33",   "36",   "38",
			 "40",    "42",   "45",   "47",   "50",   "53",   "56",   "60",   "63",   "67",   "71",   "75",
			 "80",    "84",   "90",   "94",  "100",  "106",  "112",  "120",  "125",  "133",  "140",
			"150",   "160",  "170",  "180",  "190",  "200",  "210",  "224",  "237",  "250",  "266",  "280",
			"300",   "315",  "335",  "355",  "376",  "400",  "422",  "450",  "473",  "500",  "530",  "560",
			"600",   "630",  "670",  "710",  "750",  "800",  "840",  "900",  "944", "1000", "1060", "1120",
			"1200", "1250", "1330", "1400", "1500", "1600", "1700", "1800", "1900", "2000", "2100", "2240",
			"2370", "2500", "2660", "2800", "3000", "3150", "3350", "3550", "3760", "4000", "4220", "4500",
			"4730", "5000", "5300", "5600", "6000", "6300", "6700", "7100", "7500", "8000",
		},
	},
	[TKnee] = {
		.type = TKnee,
		.name = "Knee",
		.parent = TEnum,
		.min = 0, .max = 9, .step = 1,
		.enum_names = (const char *[]){
			"HARD", "SOFT1", "SOFT2", "SOFT3", "SOFT4", "SOFT5", "SOFT6", "SOFT7", "SOFT8", "SOFT9",
		},
	},
	[TAttenuation] = {
		.type = TAttenuation,
		.name = "Attenuation",
		.parent = TEnum,
		.min = 0, .max = 2, .step = 1,
		.enum_names = (const char *[]){
			"-20","-10","+4",
		},
	},
	[TReverbType] = {
		.type = TReverbType,
		.name = "ReverbType",
		.parent = TEnum,
		.min = 0, .max = 5, .step = 1,
		.enum_names = (const char *[]){
			"off", "echo", "room", "small_hall", "large_hall", "plate",
		},
	},
	[TPreDelay] = {
		.type = TPreDelay,
		.name = "PreDelay",
		.parent = TEnum,
		.min = 0, .max = 12, .step = 1,
		.enum_names = (const char *[]){
			"0.0", "0.1", "0.2", "0.4", "0.8", "1.6", "3.2", "6.4", "10", "20", "40", "80", "160",
		},
	},
	[TPatch] = {
		.type = TPatch,
		.name = "Patch",
		.parent = TEnum,
		.min = 0, .max = 8, .step = 1,
		.enum_names = (const char *[]){
			"MIX A", "MIX B", "MIX C", "MIX D",
			"WAVE 1/2", "WAVE 3/4", "WAVE 5/6", "WAVE 7/8", "WAVE 9/10",
		},
	},
};

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
	int fx = capmix_nibbles_to_fixed(data, len);
	return fx;
}

/**
 * @brief unpacks raw data received from the device
 * @param type the type of the value to be unpacked
 * @param data the buffer holding the value to be converted
 * @return the unpacked value
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
 */
int                capmix_format_type       (capmix_type_t type, capmix_unpacked_t unpacked, char *str)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	if( unpacked.as_int == capmix_Unset )
		return capmix_format_unset(type_info, unpacked, str);
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
 */
int                capmix_pack_type         (capmix_type_t type, capmix_unpacked_t unpacked, uint8_t *buf)
{
	capmix_type_t t = type;
	capmix_type_info_t *type_info = &capmix_types[t];
	if( unpacked.as_int == capmix_Unset )
		return capmix_pack_unset(type_info, unpacked, buf);
	while( type_info->pack == NULL )
	{
		t = type_info->parent;
		type_info = &capmix_types[t];
	}

	type_info->pack(&capmix_types[type], unpacked, buf);
	return type_info->size;
}
