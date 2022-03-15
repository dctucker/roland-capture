#pragma once

#include "common.h"

typedef uint32_t capmix_fixed;  ///< 32-bit value representing the value of a given mixer control such as volume which is the largest with six raw bytes, 24 bits of which are significant, so we can compress this for ease-of-use

/**
 * @defgroup Types
 * @brief Some documentation goes here
 */
/**
 * @defgroup ValueTypes
 * @ingroup Types
 * @brief Some documentation goes here
 */

/// @ingroup ValueTypes
/// constants to identify each control's value type
typedef enum capmix_type_e {
	TValue = 0,   ///< Unknown or invalid type. Use with caution.
	TByte,        ///< Simple one-byte value. Parent: `TValue`
	TBoolean,     ///< An on/off switch. Parent: `TByte`
	TVolume,      ///< Six-byte logarithmic volume value. Parent: `TValue`
	TPan,         ///< Four-byte pan value. Parent: `TValue`
	TMeter,       ///< Two-byte meter value. Parent: `TValue`
	TClipMask,    ///< Bit-mask indicating which channels have clipped. Parent: `TValue`
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
	TPreDelay,    ///< Enumerated reverb pre-delay time. Parent: `TEnum`
	TPatch,       ///< Enumerated value representing selected source for a given output. Parent: `TEnum`
	TAutoSens,    ///< Enumerated value representing auto-sens state. Parent: `TEnum`
	TReverbTime,  ///< Scaled reverb time. Parent: `TScaled`
	TRawData,     ///< An arbitrary fields of bytes. Parent: `TValue`
	TOctaLoad,    ///< Populated by load_settings on OCTA-CAPTURE. Parent: `TRawData`
	TStudioLoad,  ///< Populated by load_settings on STUDIO-CAPTURE. Parent: `TRawData`
	NTypes,       ///< dummy value to represent the number of enumerated types
} capmix_type_t;

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

_API capmix_unpacked_t  capmix_unpack_type  (capmix_type_t, uint8_t *); ///< @ingroup API
_API capmix_unpacked_t  capmix_parse_type   (capmix_type_t, const char *); ///< @ingroup API
_API int                capmix_format_type  (capmix_type_t, capmix_unpacked_t, char *); ///< @ingroup API
_API int                capmix_pack_type    (capmix_type_t, capmix_unpacked_t, uint8_t *); ///< @ingroup API

_API capmix_type_info_t *  capmix_type         (capmix_type_t type);
_API const char *          capmix_type_name    (capmix_type_t);
int                        capmix_type_size    (capmix_type_t);
capmix_type_t              capmix_type_parent  (capmix_type_t type);
