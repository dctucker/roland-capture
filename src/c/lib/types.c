#include <stdio.h>
#include "types.h"

int type_sizes[] = {
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

ValueType type_parent[] = {
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
	[TReverbTime ] = TValue   ,
};

#define TYPE_NAME(NAME) [ T##NAME ] = #NAME
const char *type_names[] = {
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

void to_nibbles(fixed val, int n, u8 *buf)
{
	// convert 0x0123 to [ 0x00, 0x01, 0x02, 0x03 ]
	fixed acc = val;
	for(int i = n - 1; i >= 0; i--)
	{
		buf[i] = acc & 0x0f;
		acc >>= 4;
	}
}

fixed nibbles_to_fixed(u8 *buf, int len)
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

u32 db_to_fixed(f32 db)
{
	// convert -6.02 to 0x100000
	return (int)(pow(10., db/20.) * 0x200000);
}

f32 fixed_to_db(u32 fixed)
{
	// convert 0x200000 to 0
	f32 ratio = fixed / 2097152.;
	if( ratio == 0 ) return -inf;
	return 20.*log10(ratio);
}

f32 fixed_to_pan(fixed f)
{
	return round(100. * (f - 16384) / 16384.);
}

fixed pan_to_fixed(f32 pan)
{
	return (int)(16384. + (16384. * (pan / 100.))) & 0xffff;
}

UNPACK(byte)
{
	return UnpackedInt(value & 0x7f);
}
FORMAT(byte)
{
	if( unpacked.as_int == Unset )
		sprintf(str, "?");
	else
		sprintf(str, "0x%02x", unpacked.as_int);
}
PACK(byte)
{
	buf[0] = unpacked.as_int;
}

UNPACK(boolean)
{
	return UnpackedInt(value == 0 ? 0 : 1);
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
	f32 db = fixed_to_db(value);
	return UnpackedFloat(db);
}
FORMAT(volume)
{
	sprintf(str, "%+1.0f", unpacked.as_float);
}
PACK(volume)
{
	f32 value = unpacked.as_float;
	fixed fx = db_to_fixed(value);
	if( fx > 0x7fffff ) fx = 0x7fffff;
	if( value != -inf && round(value) == 0 )
		fx = 0x200000;
	to_nibbles(fx, 6, buf);
}

UNPACK(pan)
{
	f32 db = fixed_to_pan(value);
	return UnpackedFloat(db);
}
FORMAT(pan)
{
	f32 pan = unpacked.as_float;
	if( pan < 0 )
		sprintf(str, "L%1f", -pan);
	else if( pan > 0 )
		sprintf(str, "R%1f", pan);
	else
		sprintf(str, "C");
}
PACK(pan)
{
	f32 value = unpacked.as_float;
	fixed fx = pan_to_fixed(value);
	to_nibbles(fx, 4, buf);
}

void (*formatters[])(Unpacked, char *) = {
	[TValue]   = format_byte, //
	[TByte]    = format_byte,
	[TBoolean] = format_boolean,
	[TVolume]  = format_volume,
	[TPan]     = format_pan,
};
Unpacked (*unpackers[])(fixed) = {
	[TByte]    = unpack_byte,
	[TBoolean] = unpack_boolean,
	[TVolume]  = unpack_volume,
	[TPan]     = unpack_pan,
};
void (*packers[])(Unpacked, u8 *) = {
	[TByte]    = pack_byte,
	[TBoolean] = pack_boolean,
	[TVolume]  = pack_volume,
	[TPan]     = pack_pan,
};

fixed fixed_from_packed(ValueType type, u8 *data)
{
	int len = type_sizes[type];
	int fx = nibbles_to_fixed(data, len);
	return fx;
}

void format_unpacked(ValueType type, Unpacked unpacked, char *str)
{
	if( formatters[type] == NULL )
	{
		*str = '\0';
		return;
	}
	formatters[type](unpacked, str);
}

void format_value(ValueType type, Value value, char *str)
{
	int len = type_sizes[type];
	fixed fx = nibbles_to_fixed(value.as_value, len);
	Unpacked unpacked = unpack_volume(fx);
	format_unpacked(type, unpacked, str);
}

Unpacked unpack_type(ValueType type, Value value)
{
	if( unpackers[type] == NULL )
		return UnpackedInt(Unset);
	fixed fx = fixed_from_packed(type, value.as_value);
	return unpackers[type](fx);
}

void pack_type(ValueType type, Unpacked unpacked, u8 *buf)
{
	if( packers[type] == NULL )
	{
		*buf = Unset;
		return;
	}
	packers[type](unpacked, buf);
}

int type_size(ValueType type)
{
	return type_sizes[type];
}

const char * type_name(ValueType type)
{
	return type_names[type];
}
