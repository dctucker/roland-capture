#include <stdio.h>
#include <string.h>
#include "types.h"

#define UNPACK(NAME) capmix_Unpacked capmix_unpack_##NAME(capmix_type_info *type_info, capmix_fixed value)
#define PARSE(NAME)  capmix_Unpacked capmix_parse_##NAME (capmix_type_info *type_info, const char *str)
#define FORMAT(NAME) void            capmix_format_##NAME(capmix_type_info *type_info, capmix_Unpacked unpacked, char *str)
#define PACK(NAME)   void            capmix_pack_##NAME  (capmix_type_info *type_info, capmix_Unpacked unpacked, uint8_t *buf)

#define TYPE_NAME(NAME) [ T##NAME ] = #NAME

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

capmix_fixed      capmix_db_to_fixed      (float db)
{
	// convert -6.02 to 0x100000
	return (int)(pow(10., db/20.) * 0x200000);
}
float             capmix_fixed_to_db      (capmix_fixed fx)
{
	// convert 0x200000 to 0
	float ratio = fx / 2097152.;
	if( ratio == 0 ) return -inf;
	return 20.*log10(ratio);
}
float             capmix_fixed_to_pan     (capmix_fixed fx)
{
	return round(100. * (fx - 16384.) / 16384.);
}
capmix_fixed      capmix_pan_to_fixed     (float pan)
{
	return (int)(16384. + (16384. * (pan / 100.))) & 0xffff;
}

UNPACK(byte)
{
	return capmix_UnpackedInt(value & 0x7f);
}
PARSE(byte)
{
	int value;
	if( sscanf(str, "%d", &value) > 0 )
		return capmix_UnpackedInt(value);
	return capmix_UnpackedInt(capmix_Unset);
}
FORMAT(byte)
{
	if( unpacked.as_int == capmix_Unset )
		sprintf(str, "?");
	else
		sprintf(str, "0x%02x", unpacked.as_int);
}
PACK(byte)
{
	buf[0] = unpacked.as_int & 0x7f;
}

UNPACK(boolean)
{
	return capmix_UnpackedInt(value == 0 ? 0 : 1);
}
PARSE(boolean)
{
	if( strcmp(str,"1") || strcasecmp(str, "on") || strcasecmp(str,"true") )
		return capmix_UnpackedInt(1);
	else if( strcmp(str, "0") || strcasecmp(str, "off") || strcasecmp(str,"false") )
		return capmix_UnpackedInt(0);
	return capmix_UnpackedInt(capmix_Unset);
}
FORMAT(boolean)
{
	if( unpacked.as_int )
		sprintf(str, "ON");
	else
		sprintf(str, "off");
}
PACK(boolean)
{
	buf[0] = unpacked.as_int == 0 ? 0 : 1;
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
	return capmix_UnpackedInt(capmix_Unset);
}
FORMAT(volume)
{
	sprintf(str, "%+1.0f", unpacked.as_float);
}
PACK(volume)
{
	float value = unpacked.as_float;
	capmix_fixed fx = capmix_db_to_fixed(value);
	if( fx > 0x7fffff ) fx = 0x7fffff;
	if( value != -inf && round(value) == 0 )
		fx = 0x200000;
	capmix_fixed_to_nibbles(fx, 6, buf);
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
		return capmix_UnpackedInt(capmix_Unset);
	}
	return capmix_UnpackedInt(capmix_Unset);
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
}
PACK(pan)
{
	float value = unpacked.as_float;
	capmix_fixed fx = capmix_pan_to_fixed(value);
	capmix_fixed_to_nibbles(fx, 4, buf);
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
}
PACK(scaled)
{
	uint8_t value = (unpacked.as_float - type_info->min_f) / type_info->step_f;
	*buf = value;
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
		sprintf(stderr, "\nvalue too large 0x%x\n", v);
		sprintf(str, "?");
		return;
	}
	const char *name = type_info->enum_names[v];
	sprintf(str, "%s", name);
}
//PACK(enum) // pack_byte

/*
void             capmix_format_packed     (capmix_ValueType type, uint8_t *data, char *str)
{
	int len = capmix_type_size(type);
	capmix_fixed fx = capmix_nibbles_to_fixed(data, len);
	capmix_Unpacked unpacked = capmix_unpack_volume(type, fx);
	capmix_format_type(type, unpacked, str);
}
*/

capmix_type_info capmix_types[NTypes] = {
	[TValue] = {
		.name = "Value",
	},
	[TByte] = {
		.name = "Byte",
		.parent = TValue,
		.min = 0x0, .max = 0x7f, .step = 0x1,
		.unpack = capmix_unpack_byte,
		.parse = capmix_parse_byte,
		.format = capmix_format_byte,
		.pack = capmix_pack_byte,
	},
	[TBoolean] = {
		.name = "Boolean",
		.parent = TByte,
		.min = 0x0, .max = 0x1, .step = 0x1,
		.unpack = capmix_unpack_boolean,
		.parse = capmix_parse_boolean,
		.format = capmix_format_boolean,
	},
	[TVolume] = {
		.name = "Volume",
		.parent = TValue,
		.min_f = -71., .max_f = 12., .step_f = 1.,
		.unpack = capmix_unpack_volume,
		.parse = capmix_parse_volume,
		.format = capmix_format_volume,
		.pack = capmix_pack_volume,
	},
	[TPan] = {
		.name = "Pan",
		.parent = TValue,
		.min_f = -100., .max_f = 100., .step_f = 1.,
		.unpack = capmix_unpack_pan,
		.parse = capmix_parse_pan,
		.format = capmix_format_pan,
		.pack = capmix_pack_pan,
	},

	[TScaled] = {
		.name = "Scaled",
		.parent = TByte,
		.unpack = capmix_unpack_scaled,
		//.parse = capmix_parse_byte,
		.format = capmix_format_scaled,
	},
	[TSens] = {
		.name = "Sens",
		.parent = TScaled,
		.min_f =   1., .max_f =  58., .step_f = 0.5,
	},
	[TThreshold] = {
		.name = "Threshold",
		.parent = TScaled,
		.min_f = -40., .max_f =   0., .step_f = 1.,
	},
	[TGain] = {
		.name = "Gain",
		.parent = TScaled,
		.min_f = -40., .max_f =  40., .step_f = 1.,
	},
	[TGate] = {
		.name = "Gate",
		.parent = TScaled,
		.min_f = -70., .max_f = -20., .step_f = 1.,
	},
	[TReverbTime] = {
		.name = "ReverbTime",
		.parent = TScaled,
		.min_f =  0.1, .max_f =  5., .step_f = 0.1,
	},

	[TEnum] = {
		.name = "Enum",
		.parent = TByte,
		//.unpack = capmix_unpack_byte,
		.parse = capmix_parse_enum,
		.format = capmix_format_enum,
		//.pack = capmix_pack_byte,
	},
	[TRatio] = {
		.name = "Ratio",
		.parent = TEnum,
		.min = 0, .max = 13, .step = 1,
		.enum_names = (const char *[]){
			"1", "1.12", "1.25", "1.4", "1.6", "1.8", "2", "2.5", "3.2", "4", "5.6", "8", "16", "inf",
		},
	},
	[TAttack] = {
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
		.name = "Knee",
		.parent = TEnum,
		.min = 0, .max = 9, .step = 1,
		.enum_names = (const char *[]){
			"HARD", "SOFT1", "SOFT2", "SOFT3", "SOFT4", "SOFT5", "SOFT6", "SOFT7", "SOFT8", "SOFT9",
		},
	},
	[TAttenuation] = {
		.name = "Attenuation",
		.parent = TEnum,
		.min = 0, .max = 2, .step = 1,
		.enum_names = (const char *[]){
			"-20","-10","+4",
		},
	},
	[TReverbType] = {
		.name = "ReverbType",
		.parent = TEnum,
		.min = 0, .max = 5, .step = 1,
		.enum_names = (const char *[]){
			"off", "echo", "room", "small_hall", "large_hall", "plate",
		},
	},
	[TPreDelay] = {
		.name = "PreDelay",
		.parent = TEnum,
		.min = 0, .max = 12, .step = 1,
		.enum_names = (const char *[]){
			"0.0", "0.1", "0.2", "0.4", "0.8", "1.6", "3.2", "6.4", "10", "20", "40", "80", "160",
		},
	},
	[TPatch] = {
		.name = "Patch",
		.parent = TEnum,
		.min = 0, .max = 8, .step = 1,
		.enum_names = (const char *[]){
			"MIX A", "MIX B", "MIX C", "MIX D",
			"WAVE 1/2", "WAVE 3/4", "WAVE 5/6", "WAVE 7/8", "WAVE 9/10",
		},
	},
};

const char *     capmix_type_name         (capmix_ValueType type)
{
	return capmix_types[type].name;
}
int              capmix_type_size         (capmix_ValueType type)
{
	switch(type)
	{
		case TVolume : return 6;
		case TPan    : return 4;
		default      : return 1;
	}
}
capmix_fixed     capmix_fixed_from_packed (capmix_ValueType type, uint8_t *data)
{
	int len = capmix_type_size(type);
	int fx = capmix_nibbles_to_fixed(data, len);
	return fx;
}

#define MARSHALL( OP, DEFAULT ) \
	capmix_ValueType t = type;\
	capmix_type_info *type_info = &capmix_types[t];\
	while( type_info->OP == NULL ){\
		t = type_info->parent;\
		if( t == TValue ){\
			DEFAULT; }\
		type_info = &capmix_types[t]; }

capmix_Unpacked  capmix_unpack_type       (capmix_ValueType type, uint8_t *data)
{
	MARSHALL( unpack, return capmix_UnpackedInt(capmix_Unset) )
	capmix_fixed fx = capmix_fixed_from_packed(type, data);
	return type_info->unpack(&capmix_types[type], fx);
}

capmix_Unpacked  capmix_parse_type        (capmix_ValueType type, const char *str)
{
	MARSHALL( parse, return capmix_UnpackedInt(capmix_Unset) )
	return type_info->parse(&capmix_types[type], str);
}

void             capmix_format_type       (capmix_ValueType type, capmix_Unpacked unpacked, char *str)
{
	MARSHALL( format, str[0] = '\0'; return )
	type_info->format(&capmix_types[type], unpacked, str);
}
void             capmix_pack_type         (capmix_ValueType type, capmix_Unpacked unpacked, uint8_t *buf)
{
	MARSHALL( pack, *buf = capmix_Unset; return );
	type_info->pack(&capmix_types[type], unpacked, buf);
}
