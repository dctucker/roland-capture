#pragma once

#include "types.h"

typedef struct capmix_sysex_t {
	uint8_t header[6];
	uint8_t cmd;
	uint8_t addr[4];
	uint8_t data[];
} capmix_sysex_t;

int                   capmix_make_receive_sysex (uint8_t *, capmix_addr_t , capmix_addr_t);
int                   capmix_make_send_sysex    (uint8_t *, capmix_addr_t, uint8_t *, int);
static int            capmix_make_sysex         (uint8_t *, uint8_t, int);
capmix_sysex_t *      capmix_parse_sysex        (uint8_t *, int);

