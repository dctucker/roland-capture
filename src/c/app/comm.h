#pragma once

#include "lib/types.h"

int setup_midi();
int read_midi();
int send_midi(u8 *buf, int len);
void cleanup_midi();

extern void listener(u8 *, size_t);
