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

_API int                  capmix_connect(void (*)(struct capmix_event) );
_API int                  capmix_listen();
_API struct capmix_event  capmix_get(capmix_Addr);
_API struct capmix_event  capmix_put(capmix_Addr, capmix_Unpacked);
_API void                 capmix_disconnect();

_API capmix_Unpacked      capmix_memory_get_unpacked (capmix_Addr);
_API void                 capmix_memory_set_unpacked (capmix_Addr, capmix_Unpacked unpacked);
