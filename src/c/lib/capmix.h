#pragma once

#include "roland.h"
#include "types.h"
#include "capture.h"

struct capmix_event {
	int sysex_data_length;
	capmix_sysex_t *sysex;
	capmix_addr_t addr;
	capmix_type_info *type_info;
	capmix_unpacked_t unpacked;
};
typedef struct capmix_event capmix_event_t;

_API int                capmix_connect(void (*)(struct capmix_event) );
_API int                capmix_listen();
_API capmix_event_t     capmix_get(capmix_addr_t);
_API capmix_event_t     capmix_put(capmix_addr_t, capmix_unpacked_t);
_API void               capmix_disconnect();

_API capmix_unpacked_t  capmix_memory_get_unpacked (capmix_addr_t);
_API void               capmix_memory_set_unpacked (capmix_addr_t, capmix_unpacked_t unpacked);
