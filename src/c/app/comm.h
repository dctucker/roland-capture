#pragma once

#include "lib/types.h"

int setup_midi();
int read_midi();
void cleanup_midi();

extern void listener(u8 *, size_t);
