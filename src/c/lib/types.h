#pragma once

#include <inttypes.h>

#ifndef DOXYGEN_SKIP
#define warn(...) fprintf(stderr, __VA_ARGS__)
#define CCA(members...) (const char *[]){ members, 0 }
#define capmix_addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define capmix_bytes_addr(ADDR) ((capmix_addr_t)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
#define _API __attribute__ ((visibility("default")))
#endif

typedef uint32_t capmix_fixed;  ///< 32-bit value representing the value of a given mixer control such as volume which is the largest with six raw bytes, 24 bits of which are significant, so we can compress this for ease-of-use
typedef uint32_t capmix_addr_t; ///< 32-bit value holding a four-byte device address

/// constants to identify each control's value type
typedef enum capmix_type_e {
	TValue = 0,   ///< Unknown or invalid type. Use with caution.
	TByte,        ///< Simple one-byte value. Parent: `TValue`
	TBoolean,     ///< An on/off switch. Parent: `TByte`
	TVolume,      ///< Six-byte logarithmic volume value. Parent: `TValue`
	TPan,         ///< Four-byte pan value. Parent: `TValue`
	TScaled,      ///< Single-byte value scaled to fit a range. Parent: `TByte`
	TSens,        ///< Scaled preamp sensitivity value. Parent: `TScaled`
	TThreshold,   ///< Scaled compressor threshold value. Parent: `TScaled`
	TGain,        ///< Scaled compressor post-gain value. Parent: `TScaled`
	TGate,        ///< Scaled gate threshold value. Parent: `TScaled`
	TEnum,        ///< Byte value representing one of a list of options. Parent: `TByte`
	TRatio,       ///< Enumerated compressor ratio. Parent: `TEnum`
	TAttack,      ///< Enumerated compressor attack time. Parent: `TEnum`
	TRelease,     ///< Enumerated compressor release time. Parent: `TEnum`
	TKnee,        ///< Enumerated compressor knee hard/soft values. Parent: `TEnum`
	TAttenuation, ///< Enumerated line input attenuation {-20, -10, +4}. Parent: `TEnum`
	TReverbType,  ///< Enumerated value representing reverb type. Parent: `TEnum`
	TPreDelay,    ///< Enumerated reverb pre-delay time . Parent: `TEnum`
	TPatch,       ///< Enumerated value representing selected source for a given output. Parent: `TEnum`
	TReverbTime,  ///< Scaled reverb time. Parent: `TScaled`
	NTypes,       ///< dummy value to represent the number of enumerated types
} capmix_type_t;

#define capmix_Unset            0xff                                            ///< unintialized memory gets this value to represent unknowns
#define capmix_UnpackedFloat(F) (capmix_unpacked_t){ .continuous = (F) }        ///< convenience macro to represent continuous values
#define capmix_UnpackedInt(I)   (capmix_unpacked_t){ .discrete = (I) }          ///< convenience macro to represent discrete values
#define capmix_UnsetInt         (capmix_unpacked_t){ .discrete = capmix_Unset } ///< convenience macro for unset values

/// unpacked values are what we use internally to represent the position of mixer controls
typedef union capmix_unpacked_u
{
	uint32_t discrete;   ///< for discrete values such as reverb type
	float    continuous; ///< for continuous values such as pan and volume
} capmix_unpacked_t;

typedef struct capmix_type_info_s capmix_type_info_t;                                           ///< structure holding the constraints, taxonomy, and marshalling for a given type
typedef capmix_unpacked_t (* capmix_unpacker_t  )(capmix_type_info_t *, capmix_fixed);                 ///< pointer to a function that unpacks fixed values
typedef capmix_unpacked_t (* capmix_parser_t    )(capmix_type_info_t *, const char *);                 ///< pointer to a function that parses strings into unpacked values
typedef int               (* capmix_formatter_t )(capmix_type_info_t *, capmix_unpacked_t, char *);    ///< pointer to a function that formats strings from unpacked values
typedef int               (* capmix_packer_t    )(capmix_type_info_t *, capmix_unpacked_t, uint8_t *); ///< pointer to a function that packs bytes from unpacked values

/// structure holding the constraints, taxonomy, and marshalling for a given type
struct capmix_type_info_s {
	capmix_type_t type;        ///< primary key, identifies the type to be described
	capmix_type_t parent;      ///< the supertype from which this type inherits
	capmix_unpacked_t min;     ///< minimum unpacked value
	capmix_unpacked_t max;     ///< maximum unpacked value
	capmix_unpacked_t step;    ///< granularity of the unpacked value
	int size;                  ///< number of bytes stored in device memory for this type
	capmix_unpacker_t  unpack; ///< pointer to the function that unpacks fixed values for this type
	capmix_parser_t    parse;  ///< pointer to the function that parses strings into unpacked values for this type
	capmix_formatter_t format; ///< pointer to the function that formats strings from unpacked values for this type
	capmix_packer_t    pack;   ///< pointer to the function that packs bytes from unpacked values for this type
	const char *const name;    ///< string representing the type's name
	const char **enum_names;   ///< array of strings describing discrete values for enumerated types
};

capmix_fixed            capmix_nibbles_to_fixed  (uint8_t *, int );
void                    capmix_fixed_to_nibbles  (capmix_fixed, int, uint8_t *);
capmix_fixed            capmix_fixed_from_packed (capmix_type_t, uint8_t *);

_API capmix_unpacked_t  capmix_unpack_type  (capmix_type_t, uint8_t *);
_API capmix_unpacked_t  capmix_parse_type   (capmix_type_t, const char *);
_API int                capmix_format_type  (capmix_type_t, capmix_unpacked_t, char *);
_API int                capmix_pack_type    (capmix_type_t, capmix_unpacked_t, uint8_t *);

capmix_type_info_t *    capmix_type         (capmix_type_t type);
_API const char *       capmix_type_name    (capmix_type_t);
int                     capmix_type_size    (capmix_type_t);
capmix_type_t           capmix_type_parent  (capmix_type_t type);
