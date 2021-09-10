#pragma once

#include "roland.h"
#include "types.h"
#include "capture.h"

struct capmix_event {
	int sysex_data_length;
	capmix_RolandSysex *sysex;
	capmix_Addr addr;
	capmix_type_info *type_info;
	capmix_Unpacked unpacked;
};

int   capmix_connect(void (*)(struct capmix_event) );
int   capmix_listen();
struct capmix_event  capmix_get(capmix_Addr);
struct capmix_event  capmix_put(capmix_Addr, capmix_Unpacked);
void  capmix_disconnect();

