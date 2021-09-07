#pragma once

#include "types.h"

typedef struct roland_sysex {
	u8 header[6];
	u8 cmd;
	u8 addr[4];
	u8 data[];
} RolandSysex;

size_t         make_receive_sysex(u8 *, Addr , Addr );
size_t         make_send_sysex(u8 *, Addr, u8 *, size_t );
size_t         make_sysex(u8 *, u8, size_t);
RolandSysex *  parse_sysex(u8 *, size_t );

