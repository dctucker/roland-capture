#pragma once

#include <inttypes.h>

typedef uint32_t capmix_fixed;

#define warn(...) fprintf(stderr, __VA_ARGS__)
#define CCA(members...) (const char *[]){ members, 0 }
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

#define capmix_Unset 0xff
#define capmix_UnpackedFloat(F) (capmix_Unpacked){ .as_float = (F) }
#define capmix_UnpackedInt(I)   (capmix_Unpacked){ .as_int = (I) }
#define capmix_UnsetInt (capmix_Unpacked){ .as_int = capmix_Unset }

typedef union capmix_unpacked
{
	uint32_t as_int;
	float    as_float;
} capmix_Unpacked;

typedef struct capmix_type_info {
	capmix_ValueType type;
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
	int size;
	capmix_Unpacked (*unpack) (struct capmix_type_info *, capmix_fixed);
	capmix_Unpacked (*parse)  (struct capmix_type_info *, const char *);
	void            (*format) (struct capmix_type_info *, capmix_Unpacked, char *);
	void            (*pack)   (struct capmix_type_info *, capmix_Unpacked, uint8_t *);
	const char *const name;
	const char **enum_names;
	
} capmix_type_info;

capmix_fixed     capmix_nibbles_to_fixed  (uint8_t *, int );
void             capmix_fixed_to_nibbles  (capmix_fixed, int, uint8_t *);

capmix_fixed     capmix_fixed_from_packed (capmix_ValueType, uint8_t *);

__attribute__ ((visibility("default")))  capmix_Unpacked  capmix_unpack_type  (capmix_ValueType, uint8_t *);
__attribute__ ((visibility("default")))  capmix_Unpacked  capmix_parse_type   (capmix_ValueType, const char *);
__attribute__ ((visibility("default")))  void             capmix_format_type  (capmix_ValueType, capmix_Unpacked, char *);
__attribute__ ((visibility("default")))  void             capmix_pack_type    (capmix_ValueType, capmix_Unpacked, uint8_t *);

capmix_type_info *  capmix_type(capmix_ValueType type);
const char *        capmix_type_name         (capmix_ValueType);
int                 capmix_type_size         (capmix_ValueType);
capmix_ValueType    capmix_type_parent       (capmix_ValueType type);

