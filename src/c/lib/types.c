#include <stdio.h>
#include <string.h>
#include "types.h"

#define UNPACK(NAME) capmix_Unpacked capmix_unpack_##NAME(capmix_ValueType type, fixed value)
#define PARSE(NAME)  capmix_Unpacked capmix_parse_##NAME (capmix_ValueType type, const char *str)
#define FORMAT(NAME) void            capmix_format_##NAME(capmix_ValueType type, capmix_Unpacked unpacked, char *str)
#define PACK(NAME)   void            capmix_pack_##NAME  (capmix_ValueType type, capmix_Unpacked unpacked, u8 *buf)

int capmix_type_sizes[] = {
	[TByte]        = 1,
	[TBoolean]     = 1,
	[TVolume]      = 6,
	[TPan]         = 4,
	[TScaled]      = 1,
	[TSens]        = 1,
	[TThreshold]   = 1,
	[TGain]        = 1,
	[TGate]        = 1,
	[TEnum]        = 1,
	[TRatio]       = 1,
	[TAttack]      = 1,
	[TRelease]     = 1,
	[TKnee]        = 1,
	[TAttenuation] = 1,
	[TReverbType]  = 1,
	[TPreDelay]    = 1,
	[TPatch]       = 1,
	[TReverbTime]  = 1,
};

capmix_ValueType capmix_type_parents[] = {
	[TByte       ] = TValue   ,
	[TBoolean    ] = TByte    ,
	[TVolume     ] = TValue   ,
	[TPan        ] = TValue   ,
	[TScaled     ] = TByte    ,
	[TSens       ] = TScaled  ,
	[TThreshold  ] = TScaled  ,
	[TGain       ] = TScaled  ,
	[TGate       ] = TScaled  ,
	[TEnum       ] = TByte    ,
	[TRatio      ] = TEnum    ,
	[TAttack     ] = TEnum    ,
	[TRelease    ] = TEnum    ,
	[TKnee       ] = TEnum    ,
	[TAttenuation] = TEnum    ,
	[TReverbType ] = TEnum    ,
	[TPreDelay   ] = TEnum    ,
	[TPatch      ] = TEnum    ,
	[TReverbTime ] = TScaled  ,
};

#define TYPE_NAME(NAME) [ T##NAME ] = #NAME
const char *capmix_type_names[] = {
	TYPE_NAME(Byte),
	TYPE_NAME(Boolean),
	TYPE_NAME(Volume),
	TYPE_NAME(Pan),
	TYPE_NAME(Scaled),
	TYPE_NAME(Sens),
	TYPE_NAME(Threshold),
	TYPE_NAME(Gain),
	TYPE_NAME(Gate),
	TYPE_NAME(Enum),
	TYPE_NAME(Ratio),
	TYPE_NAME(Attack),
	TYPE_NAME(Release),
	TYPE_NAME(Knee),
	TYPE_NAME(Attenuation),
	TYPE_NAME(ReverbType),
	TYPE_NAME(PreDelay),
	TYPE_NAME(Patch),
	TYPE_NAME(ReverbTime),
};

void capmix_to_nibbles(fixed val, int n, u8 *buf)
{
	// convert 0x0123 to [ 0x00, 0x01, 0x02, 0x03 ]
	fixed acc = val;
	for(int i = n - 1; i >= 0; i--)
	{
		buf[i] = acc & 0x0f;
		acc >>= 4;
	}
}

fixed capmix_nibbles_to_fixed(u8 *buf, int len)
{
	// convert [ 0x00, 0x01, 0x02, 0x03 ] to 0x0123
	fixed acc = 0;
	for(int i=0; i < len; i++)
	{
		acc <<= 4;
		acc += buf[i];
	}
	return acc;
}

u32 capmix_db_to_fixed(f32 db)
{
	// convert -6.02 to 0x100000
	return (int)(pow(10., db/20.) * 0x200000);
}

f32 capmix_fixed_to_db(u32 fixed)
{
	// convert 0x200000 to 0
	f32 ratio = fixed / 2097152.;
	if( ratio == 0 ) return -inf;
	return 20.*log10(ratio);
}

f32 capmix_fixed_to_pan(fixed f)
{
	return round(100. * (f - 16384.) / 16384.);
}

fixed capmix_pan_to_fixed(f32 pan)
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
	f32 db = capmix_fixed_to_db(value);
	return capmix_UnpackedFloat(db);
}
PARSE(volume)
{
	f32 value;
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
	f32 value = unpacked.as_float;
	fixed fx = capmix_db_to_fixed(value);
	if( fx > 0x7fffff ) fx = 0x7fffff;
	if( value != -inf && round(value) == 0 )
		fx = 0x200000;
	capmix_to_nibbles(fx, 6, buf);
}

UNPACK(pan)
{
	f32 db = capmix_fixed_to_pan(value);
	return capmix_UnpackedFloat(db);
}
PARSE(pan)
{
	char side;
	f32 value;
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
	f32 pan = unpacked.as_float;
	if( pan < 0 )
		sprintf(str, "L%1.0f", fabs(pan));
	else if( pan > 0 )
		sprintf(str, "R%1.0f", pan);
	else
		sprintf(str, "C");
}
PACK(pan)
{
	f32 value = unpacked.as_float;
	fixed fx = capmix_pan_to_fixed(value);
	capmix_to_nibbles(fx, 4, buf);
}

capmix_ScaledType capmix_scaled_types[] = {
	[TSens]      = (capmix_ScaledType){ .min =   1. , .max =  58. , .step = 0.5 },
	[TThreshold] = (capmix_ScaledType){ .min = -40. , .max =   0. , .step = 1. },
	[TGain]      = (capmix_ScaledType){ .min = -40. , .max =  40. , .step = 1. },
	[TGate]      = (capmix_ScaledType){ .min = -70. , .max = -20. , .step = 1. },
	[TReverbTime]= (capmix_ScaledType){ .min =  0.1 , .max =  5.  , .step = 0.1 },
};
UNPACK(scaled)
{
	capmix_ScaledType *scale = &capmix_scaled_types[type];
	return capmix_UnpackedFloat(scale->min + scale->step * value);
}
FORMAT(scaled)
{
	capmix_ScaledType *scale = &capmix_scaled_types[type];
	if( scale->step == 1. )
		sprintf(str, "%1.0f", unpacked.as_float);
	else
		sprintf(str, "%1.1f", unpacked.as_float);
}
PACK(scaled)
{
	capmix_ScaledType *scale = &capmix_scaled_types[type];
	u8 value = (unpacked.as_float - scale->min) / scale->step;
	*buf = value;
}

capmix_NameTable capmix_enum_names[] = {
	[TRatio] = (capmix_NameTable) { .size = 14, .names = (const char *[]){
		"1", "1.12", "1.25", "1.4", "1.6", "1.8", "2", "2.5", "3.2", "4", "5.6", "8", "16", "inf",
	}},
	[TAttack] = (capmix_NameTable) { .size = 125, .names = (const char *[]){
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
	}},
	[TRelease] = (capmix_NameTable) { .size = 125, .names = (const char *[]){
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
	}},
	[TKnee] = (capmix_NameTable) { .size = 10, .names = (const char *[]){
		"HARD", "SOFT1", "SOFT2", "SOFT3", "SOFT4", "SOFT5", "SOFT6", "SOFT7", "SOFT8", "SOFT9",
	}},
	[TAttenuation] = (capmix_NameTable) { .size = 3, .names = (const char *[]){
		"-20","-10","+4",
	}},
	[TReverbType] = (capmix_NameTable) { .size = 6, .names = (const char *[]){
		"off", "echo", "room", "small_hall", "large_hall", "plate",
	}},
	[TPreDelay] = (capmix_NameTable) { .size = 13, .names = (const char *[]){
		"0.0", "0.1", "0.2", "0.4", "0.8", "1.6", "3.2", "6.4", "10", "20", "40", "80", "160",
	}},
	[TPatch] = (capmix_NameTable) { .size = 9, .names = (const char *[]){
		"MIX A", "MIX B", "MIX C", "MIX D",
		"WAVE 1/2", "WAVE 3/4", "WAVE 5/6", "WAVE 7/8", "WAVE 9/10",
	}},
};
//UNPACK(enum) // unpack_byte
FORMAT(enum)
{
	u32 v = unpacked.as_int;
	capmix_NameTable *names = &capmix_enum_names[type];
	if( v >= names->size )
	{
		sprintf(str, "?");
		return;
	}
	const char *name = names->names[v];
	sprintf(str, "%s", name);
}
//PACK(enum) // pack_byte

capmix_Unpacked (*unpackers[NTypes])(capmix_ValueType, fixed) = {
	[TByte]    = capmix_unpack_byte,
	[TBoolean] = capmix_unpack_boolean,
	[TVolume]  = capmix_unpack_volume,
	[TPan]     = capmix_unpack_pan,
	[TEnum]    = capmix_unpack_byte, //
	[TScaled]  = capmix_unpack_scaled,
};
capmix_Unpacked (*parsers[NTypes])(capmix_ValueType, const char *) = {
	[TByte]    = capmix_parse_byte,
	[TBoolean] = capmix_parse_boolean,
	[TVolume]  = capmix_parse_volume,
	[TPan]     = capmix_parse_pan,
};
void (*formatters[NTypes])(capmix_ValueType, capmix_Unpacked, char *) = {
	[TByte]    = capmix_format_byte,
	[TBoolean] = capmix_format_boolean,
	[TVolume]  = capmix_format_volume,
	[TPan]     = capmix_format_pan,
	[TEnum]    = capmix_format_enum,
	[TScaled]  = capmix_format_scaled,
};
void (*packers[NTypes])(capmix_ValueType, capmix_Unpacked, u8 *) = {
	[TByte]    = capmix_pack_byte,
	[TBoolean] = capmix_pack_boolean,
	[TVolume]  = capmix_pack_volume,
	[TPan]     = capmix_pack_pan,
	[TEnum]    = capmix_pack_byte, //
	[TScaled]  = capmix_pack_scaled,
};

capmix_Unpacked capmix_parse_type(capmix_ValueType type, const char *str)
{
	capmix_ValueType t = type;
	capmix_Unpacked (*parser)(capmix_ValueType, const char *) = parsers[t];
	while( parser == NULL )
	{
		t = capmix_type_parents[t];
		if( t == TValue )
		{
			return capmix_UnpackedInt(capmix_Unset);
		}
		parser = parsers[t];
	}
	return parser(type, str);
}
void capmix_format_unpacked(capmix_ValueType type, capmix_Unpacked unpacked, char *str)
{
	capmix_ValueType t = type;
	void (*formatter)(capmix_ValueType, capmix_Unpacked, char*) = formatters[t];
	while( formatter == NULL )
	{
		t = capmix_type_parents[t];
		if( t == TValue )
		{
			str[0] = '\0';
			return;
		}
		formatter = formatters[t];
	}
	formatter(type, unpacked, str);
}
void capmix_format_type(capmix_ValueType type, u8 *data, char *str)
{
	int len = capmix_type_size(type);
	fixed fx = capmix_nibbles_to_fixed(data, len);
	capmix_Unpacked unpacked = capmix_unpack_volume(type, fx);
	capmix_format_unpacked(type, unpacked, str);
}

fixed capmix_fixed_from_packed(capmix_ValueType type, u8 *data)
{
	int len = capmix_type_size(type);
	int fx = capmix_nibbles_to_fixed(data, len);
	return fx;
}
capmix_Unpacked capmix_unpack_type(capmix_ValueType type, u8 *data)
{
	capmix_ValueType t = type;
	capmix_Unpacked (*unpacker)(capmix_ValueType, fixed) = unpackers[t];
	while( unpacker == NULL )
	{
		t = capmix_type_parents[t];
		if( type == TValue )
		{
			return capmix_UnpackedInt(capmix_Unset);
		}
		unpacker = unpackers[t];
	}
	fixed fx = capmix_fixed_from_packed(type, data);
	return unpacker(type, fx);
}

void capmix_pack_type(capmix_ValueType type, capmix_Unpacked unpacked, u8 *buf)
{
	capmix_ValueType t = type;
	void (*packer)(capmix_ValueType, capmix_Unpacked, u8 *) = packers[t];
	while( packer == NULL )
	{
		t = capmix_type_parents[t];
		if( type == TValue )
		{
			*buf = capmix_Unset;
			return;
		}
		packer = packers[t];
	}
	packer(type, unpacked, buf);
}

int capmix_type_size(capmix_ValueType type)
{
	return capmix_type_sizes[type];
}

const char * capmix_type_name(capmix_ValueType type)
{
	return capmix_type_names[type];
}
