#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float f32;
typedef u32 fixed;

#define inf INFINITY
#define capmix_Unset 0xff

#define capmix_addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define capmix_bytes_addr(ADDR) ((capmix_Addr)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
typedef uint32_t capmix_Addr;

typedef enum capmix_ValueType {
	TValue = 0,
	TByte,        // TValue
	TBoolean,     // TByte
	TVolume,      // TValue
	TPan,         // TValue
	TScaled,      // TByte
	TSens,        // TScaled
	TThreshold,   // TScaled
	TGain,        // TScaled
	TGate,        // TScaled
	TEnum,        // TByte
	TRatio,       // TEnum
	TAttack,      // TEnum
	TRelease,     // TEnum
	TKnee,        // TEnum
	TAttenuation, // TEnum
	TReverbType,  // TEnum
	TPreDelay,    // TEnum
	TPatch,       // TEnum
	TReverbTime,  // TValue?
	NTypes, // dummy
} capmix_ValueType;

typedef struct capmix_NameTable {
	int size;
	const char **names;
} capmix_NameTable;

typedef struct capmix_ScaledType {
	f32 min;
	f32 max;
	f32 step;
} capmix_ScaledType;

#define capmix_UnpackedFloat(F) (capmix_Unpacked){ .as_float = (F) }
#define capmix_UnpackedInt(I)   (capmix_Unpacked){ .as_int = (I) }
typedef union capmix_unpacked
{
	u32 as_int;
	f32 as_float;
} capmix_Unpacked;

capmix_Unpacked  capmix_parse_type(capmix_ValueType, const char *);
void             capmix_format_unpacked(capmix_ValueType, capmix_Unpacked, char *);
void             capmix_format_value(capmix_ValueType, u8 *, char *);
fixed            capmix_nibbles_to_fixed(u8 *, int );
void             capmix_to_nibbles(fixed, int, u8 *);
fixed            capmix_fixed_from_packed(capmix_ValueType, u8 *);
capmix_Unpacked  capmix_unpack_type(capmix_ValueType, u8 *);
void             capmix_pack_type(capmix_ValueType, capmix_Unpacked, u8 *);
int              capmix_type_size(capmix_ValueType);
const char *     capmix_type_name(capmix_ValueType);
