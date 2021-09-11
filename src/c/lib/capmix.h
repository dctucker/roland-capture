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

__attribute__ ((visibility("default")))  int                  capmix_connect(void (*)(struct capmix_event) );
__attribute__ ((visibility("default")))  int                  capmix_listen();
__attribute__ ((visibility("default")))  struct capmix_event  capmix_get(capmix_Addr);
__attribute__ ((visibility("default")))  struct capmix_event  capmix_put(capmix_Addr, capmix_Unpacked);
__attribute__ ((visibility("default")))  void                 capmix_disconnect();

