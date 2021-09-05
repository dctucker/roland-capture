#include <stdio.h>
#include <math.h>
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

UNPACK(volume)
{
	f32 db = fixed_to_db(value);
	return (Unpacked){ .as_float = db };
}
FORMAT(volume)
{
	Unpacked db = unpack_volume(value);
	sprintf(str, "%+1.0f", db.as_float);
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

FORMAT(byte)
{
	if( value == Unset )
		sprintf(str, "?");
	else
		sprintf(str, "0x%02x", value);
}

FORMAT(boolean)
{
	if( value )
		sprintf(str, "ON");
	else
		sprintf(str, "off");
}

void (*formatters[])(fixed, char *) = {
	[TValue]  = format_byte,
	[TByte]   = format_byte,
	[TVolume] = format_volume,
};
Unpacked (*unpackers[])(fixed) = {
	[TVolume] = unpack_volume,
};
void (*packers[])(Unpacked, u8 *) = {
	[TVolume] = pack_volume,
};

void format_value(ValueType type, Value value, char *str)
{
	int len = type_sizes[type];
	int fx = nibbles_to_fixed(value.as_value, len);
	formatters[type](fx, str);
}

fixed fixed_from_packed(ValueType type, u8 *data)
{
	int len = type_sizes[type];
	int fx = nibbles_to_fixed(data, len);
	return fx;
}

Unpacked unpack_type(ValueType type, Value value)
{
	fixed fx = fixed_from_packed(type, value.as_value);
	return unpackers[type](fx);
}
