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
#define Unset 0xff

#define addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define bytes_addr(ADDR) ((Addr)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
typedef uint32_t Addr;

typedef struct Byte         { u8 value;    } Byte;
typedef struct Boolean      { u8 value;    } Boolean;
typedef struct Volume       { u8 value[6]; } Volume;
typedef struct Pan          { u8 value[4]; } Pan;
typedef struct Scaled       { u8 value;    } Scaled;
typedef struct Sens         { u8 value;    } Sens;
typedef struct Threshold    { u8 value;    } Threshold;
typedef struct Gain         { u8 value;    } Gain;
typedef struct Gate         { u8 value;    } Gate;
typedef struct Enum         { u8 value;    } Enum;
typedef struct Ratio        { u8 value;    } Ratio;
typedef struct Attack       { u8 value;    } Attack;
typedef struct Release      { u8 value;    } Release;
typedef struct Knee         { u8 value;    } Knee;
typedef struct Attenuation  { u8 value;    } Attenuation;
typedef struct ReverbType   { u8 value;    } ReverbType;
typedef struct PreDelay     { u8 value;    } PreDelay;
typedef struct Patch        { u8 value;    } Patch;
typedef struct ReverbTime   { u8 value;    } ReverbTime;

typedef union input_channel {
	struct {
		Boolean  stereo;
		u8 _unused;
		Boolean  solo;
		Boolean  mute;
		Pan      pan;
		Volume   volume;
		Volume   reverb;
	};
	u8 bytes[20];
} input_channel;

typedef enum ValueType {
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
} ValueType;

typedef struct NameTable {
	int size;
	const char **names;
} NameTable;

typedef struct ScaledType {
	f32 min;
	f32 max;
	f32 step;
} ScaledType;

#define UnpackedFloat(F) (Unpacked){ .as_float = F }
#define UnpackedInt(I) (Unpacked){ .as_int = I }
typedef union unpacked
{
	u32 as_int;
	f32 as_float;
} Unpacked;

#define UNPACK(NAME) Unpacked unpack_##NAME(ValueType type, fixed value)
#define FORMAT(NAME) void     format_##NAME(ValueType type, Unpacked unpacked, char *str)
#define PACK(NAME)   void     pack_##NAME  (ValueType type, Unpacked unpacked, u8 *buf)

void          format_unpacked(ValueType, Unpacked, char *);
void          format_value(ValueType, u8 *, char *);
fixed         nibbles_to_fixed(u8 *, int );
void          to_nibbles(fixed, int, u8 *);
fixed         fixed_from_packed(ValueType, u8 *);
Unpacked      unpack_type(ValueType, u8 *);
void          pack_type(ValueType, Unpacked, u8 *);
int           type_size(ValueType);
const char *  type_name(ValueType);
