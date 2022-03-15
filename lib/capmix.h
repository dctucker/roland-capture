#pragma once

#include "capture.h"

/**
 * @defgroup API
 * @brief The functions intended to be used in the context of a shared library
 */

/**
 * @brief structure describing an event of receiving a SysEx message from the device
 */
typedef struct capmix_event_s {
	int sysex_data_length;         ///< length in bytes of the SysEx message
	capmix_sysex_t *sysex;         ///< structure describing the SysEx message
	capmix_addr_t addr;            ///< the device address this message considers
	capmix_type_info_t *type_info; ///< structure describing the type of the data stored at this location in device memory
	capmix_unpacked_t unpacked;    ///< the value stored in device memory
} capmix_event_t;

typedef void (capmix_event_handler_t)(capmix_event_t);

_API int                capmix_connect( capmix_event_handler_t * );
_API int                capmix_listen();
_API capmix_event_t     capmix_get(capmix_addr_t);
_API capmix_event_t     capmix_put(capmix_addr_t, capmix_unpacked_t);
_API void               capmix_disconnect();

_API capmix_unpacked_t  capmix_memory_get_unpacked (capmix_addr_t);
_API void               capmix_memory_set_unpacked (capmix_addr_t, capmix_unpacked_t unpacked);
