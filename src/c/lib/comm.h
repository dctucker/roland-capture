#pragma once

#include "lib/types.h"

int   capmix_setup_midi();
int   capmix_read_midi();
int   capmix_send_midi(uint8_t *, int);
void  capmix_cleanup_midi();

