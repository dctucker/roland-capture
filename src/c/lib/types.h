#pragma once

#include <inttypes.h>

typedef uint32_t capmix_fixed;

#define warn(...) fprintf(stderr, __VA_ARGS__)
#define CCA(members...) (const char *[]){ members, 0 }
#define capmix_addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define capmix_bytes_addr(ADDR) ((capmix_addr_t)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
#define _API __attribute__ ((visibility("default")))

typedef uint32_t capmix_addr_t;

typedef enum capmix_type_t {
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
} capmix_type_t;

#define capmix_Unset            0xff
#define capmix_UnpackedFloat(F) (capmix_unpacked_t){ .as_float = (F) }
#define capmix_UnpackedInt(I)   (capmix_unpacked_t){ .as_int = (I) }
#define capmix_UnsetInt         (capmix_unpacked_t){ .as_int = capmix_Unset }

typedef union capmix_unpacked
{
	uint32_t as_int;
	float    as_float;
} capmix_unpacked_t;

typedef struct capmix_type_info {
	capmix_type_t type;
	capmix_type_t parent;
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
	capmix_unpacked_t (*unpack) (struct capmix_type_info *, capmix_fixed);
	capmix_unpacked_t (*parse)  (struct capmix_type_info *, const char *);
	int               (*format) (struct capmix_type_info *, capmix_unpacked_t, char *);
	int               (*pack)   (struct capmix_type_info *, capmix_unpacked_t, uint8_t *);
	const char *const name;
	const char **enum_names;
} capmix_type_info;

capmix_fixed            capmix_nibbles_to_fixed  (uint8_t *, int );
void                    capmix_fixed_to_nibbles  (capmix_fixed, int, uint8_t *);
capmix_fixed            capmix_fixed_from_packed (capmix_type_t, uint8_t *);

_API capmix_unpacked_t  capmix_unpack_type  (capmix_type_t, uint8_t *);
_API capmix_unpacked_t  capmix_parse_type   (capmix_type_t, const char *);
_API int                capmix_format_type  (capmix_type_t, capmix_unpacked_t, char *);
_API int                capmix_pack_type    (capmix_type_t, capmix_unpacked_t, uint8_t *);

capmix_type_info *      capmix_type         (capmix_type_t type);
_API const char *       capmix_type_name    (capmix_type_t);
int                     capmix_type_size    (capmix_type_t);
capmix_type_t           capmix_type_parent  (capmix_type_t type);

