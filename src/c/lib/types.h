#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <math.h>

typedef uint32_t capmix_fixed;

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
	float min;
	float max;
	float step;
} capmix_ScaledType;

#define capmix_UnpackedFloat(F) (capmix_Unpacked){ .as_float = (F) }
#define capmix_UnpackedInt(I)   (capmix_Unpacked){ .as_int = (I) }
typedef union capmix_unpacked
{
	uint32_t as_int;
	float    as_float;
} capmix_Unpacked;

typedef struct capmix_type_info {
	capmix_ValueType parent;
	union {
		uint32_t min;
		float    min_f;
	};
	union {
		uint32_t max;
		float    max_f;
	};
	union {
		uint32_t step;
		float    step_f;
	};
	capmix_Unpacked (*unpack) (capmix_ValueType, capmix_fixed);
	capmix_Unpacked (*parse)  (capmix_ValueType, const char *);
	void            (*format) (capmix_ValueType, capmix_Unpacked, char *);
	void            (*pack)   (capmix_ValueType, capmix_Unpacked, char *);
	const char *const name;
} capmix_type_info;

capmix_fixed     capmix_nibbles_to_fixed  (uint8_t *, int );
void             capmix_fixed_to_nibbles  (capmix_fixed, int, uint8_t *);

capmix_fixed     capmix_fixed_from_packed (capmix_ValueType, uint8_t *);

capmix_Unpacked  capmix_unpack_type       (capmix_ValueType, uint8_t *);
capmix_Unpacked  capmix_parse_type        (capmix_ValueType, const char *);
void             capmix_format_type       (capmix_ValueType, capmix_Unpacked, char *);
void             capmix_pack_type         (capmix_ValueType, capmix_Unpacked, uint8_t *);

capmix_ValueType capmix_type_parent       (capmix_ValueType type);
int              capmix_type_size         (capmix_ValueType);
const char *     capmix_type_name         (capmix_ValueType);
