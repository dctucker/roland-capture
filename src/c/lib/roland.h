#pragma once

#include "types.h"

typedef struct roland_sysex {
	u8 header[6];
	u8 cmd;
	u8 addr[4];
	u8 data[];
} RolandSysex;

int         make_receive_sysex(u8 *, Addr , Addr );
int         make_send_sysex(u8 *, Addr, u8 *, int );
int         make_sysex(u8 *, u8, int);
RolandSysex *  parse_sysex(u8 *, int );

