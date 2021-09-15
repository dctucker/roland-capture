#pragma once

#include "types.h"

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

int                   capmix_make_receive_sysex (uint8_t *, capmix_addr_t , capmix_addr_t);
int                   capmix_make_send_sysex    (uint8_t *, capmix_addr_t, uint8_t *, int);
static int            capmix_make_sysex         (uint8_t *, uint8_t, int);
capmix_sysex_t *      capmix_parse_sysex        (uint8_t *, int);
