#pragma once

#include "types.h"

#define N_MEMSECTION 32 ///< 2^5 sections. We are able to reduce the address space locally because it appears the top two address bytes only use four bits each.
#define N_MEMBUF   2048 ///< 2^11 bytes in each section. The third byte of an address only uses five bits, and the least significant byte never seems to exceed five bits (reverb is at 0x0e + 6)

/// structure for storing device memory locally without requiring 4 GB of address space
typedef struct capmix_memory_s {
	struct capmix_memory_section_s {
		uint8_t buffer[N_MEMBUF];  ///< 2^11 bytes in each section
	} section[N_MEMSECTION];       ///< 2^5 sections
} capmix_memory_t;

/// coordinate for accessing our local representation of device memory
typedef struct memory_coord_s {
	int section; ///< section, ranging from 0 to 2^5-1
	int offset;  ///< offset within section, ranging from 0 to 2^11-1
} capmix_coord_t;

capmix_coord_t        capmix_addr_coord          (capmix_addr_t addr);
void                  capmix_memory_init         ();
int                   capmix_memory_erase        (capmix_addr_t addr, int len);
uint8_t *             capmix_memory_get          (capmix_addr_t addr);
int                   capmix_memory_set          (capmix_addr_t addr, uint8_t *data, int len);
