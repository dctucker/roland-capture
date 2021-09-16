#pragma once

#include "common.h"

int                   capmix_make_receive_sysex (uint8_t *, capmix_addr_t , capmix_addr_t);
int                   capmix_make_send_sysex    (uint8_t *, capmix_addr_t, uint8_t *, int);
static int            capmix_make_sysex         (uint8_t *, uint8_t, int);
capmix_sysex_t *      capmix_parse_sysex        (uint8_t *, int);
