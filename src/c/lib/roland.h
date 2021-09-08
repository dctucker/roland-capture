#pragma once

#include "types.h"

typedef struct capmix_roland_sysex {
	u8 header[6];
	u8 cmd;
	u8 addr[4];
	u8 data[];
} capmix_RolandSysex;

int                   capmix_make_receive_sysex(u8 *, capmix_Addr , capmix_Addr );
int                   capmix_make_send_sysex(u8 *, capmix_Addr, u8 *, int );
int                   capmix_make_sysex(u8 *, u8, int);
capmix_RolandSysex *  capmix_parse_sysex(u8 *, int );

