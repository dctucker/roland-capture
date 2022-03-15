#define DEF_MEMAREA(NAME) static const capmix_mem_t NAME ## _area[] ///< define an area in device memory by name
#define MEMAREA(NAME) .area=(const capmix_mem_t **const)& NAME ## _area ///< assign the child area to the given named area

#define ENDA { .offset=capmix_None } ///< end of memory map, required for each memory area
#define MEMNODE( OFFSET, TYPE, NAME) [OFFSET] = { .name = NAME, .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map
#define CMEMNODE(OFFSET, TYPE, NAME) [OFFSET] = { .name = capmix_str.NAME[OFFSET] , .offset = OFFSET, .type = T##TYPE } ///< describes a leaf node within the memory map using a predefined capmix_str_t entry

#define PREAMP( OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for the preamp
#define CHANNEL(OFFSET, AREA) [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.channels[OFFSET>> 8], MEMAREA(AREA) } ///< define a child area in memory for an input channel
#define MONITOR(OFFSET, AREA) [OFFSET>>12] = { .offset=OFFSET, .name = capmix_str.monitors[OFFSET>>12], MEMAREA(AREA) } ///< define a child area in memory for a monitor mix
#define REVERB( OFFSET )      [OFFSET>> 8] = { .offset=OFFSET, .name = capmix_str.reverb_types[OFFSET>>8], MEMAREA(reverb_params) } ///< define a child area in memory for a reverb type

#include "common.h"
#include "octa.h"
#include "studio.h"
