#pragma once

#include "types.h"

typedef struct capmix_roland_sysex {
	uint8_t header[6];
	uint8_t cmd;
	uint8_t addr[4];
	uint8_t data[];
} capmix_RolandSysex;

int                   capmix_make_receive_sysex (uint8_t *, capmix_Addr , capmix_Addr);
int                   capmix_make_send_sysex    (uint8_t *, capmix_Addr, uint8_t *, int);
static int            capmix_make_sysex         (uint8_t *, uint8_t, int);
capmix_RolandSysex *  capmix_parse_sysex        (uint8_t *, int);

