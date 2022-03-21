#pragma once

#include <inttypes.h>

/// the type of audio card
typedef enum capmix_model_e {
	MNone = 0,
	MDuo, // (pretty sure this does not have a software mixer interface)
	MQuad,
	MOcta,
	MStudio,
} capmix_model_t;

/// structure representing the header of a SysEx message
typedef struct capmix_sysex_fields_s {
	uint8_t status;       ///< must be 0xf0
	uint8_t manufacturer; ///< Roland is 0x41
	uint8_t device_id;    ///< defaults to 0x10
	uint8_t model_id[3];  ///< three bytes describing the device model
} capmix_sysex_fields_t;

/// structure for accessing SysEx messages semantically
typedef struct capmix_sysex_s {
	union {
		capmix_sysex_fields_t fields; ///< individual fields of the SysEx header
		uint8_t header[6]; ///< six-byte header
	};
	uint8_t cmd;       ///< command, 0x11 for receive, 0x12 for send
	uint8_t addr[4];   ///< device memory address to consider
	uint8_t data[];    ///< variable length data, ends in checksum followed by 0xf7
} capmix_sysex_t;

typedef uint32_t capmix_addr_t; ///< 32-bit value holding a four-byte device address

/// unpacked values are what we use internally to represent the position of mixer controls
typedef union capmix_unpacked_u
{
	uint32_t discrete;   ///< for discrete values such as reverb type
	float    continuous; ///< for continuous values such as pan and volume
} capmix_unpacked_t;

#define capmix_Unset            0xff                                            ///< unintialized memory gets this value to represent unknowns
#define capmix_UnpackedFloat(F) (capmix_unpacked_t){ .continuous = (F) }        ///< convenience macro to represent continuous values
#define capmix_UnpackedInt(I)   (capmix_unpacked_t){ .discrete = (I) }          ///< convenience macro to represent discrete values
#define capmix_UnsetInt         (capmix_unpacked_t){ .discrete = capmix_Unset } ///< convenience macro for unset values

#ifndef DOXYGEN_SKIP
#define _API __attribute__ ((visibility("default")))
#define CCA(members...) (const char *[]){ members, 0 }
#define warn(...) fprintf(stderr, __VA_ARGS__)
#define capmix_addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define capmix_bytes_addr(ADDR) ((capmix_addr_t)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
#endif

typedef struct cursor_s {
	int x;
	int y;
} cursor_t;

extern capmix_model_t capmix_model;
extern const capmix_sysex_fields_t *capture_sysex;
extern const capmix_addr_t *load_map;
