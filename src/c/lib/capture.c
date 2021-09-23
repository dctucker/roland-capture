#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "capture.h"

#define LINE 0xb ///< constant value for the logically separate line input section
#include "capture/strings.h"
#include "capture/map.h"

static const capmix_mem_t *memory_map   = octa_memory_map;
static const capmix_mem_t *top_map_area = octa_top_map_area;

/**
 * @brief traverse a memory map recursively, printing the address and full name of each entry
 * @param map the map to traverse
 * @param prefix recursion parameter storing the name of the current area of the map
 * @param prev_offset recursion parameter accumulating the starting memory address of the current area of the map
 */
void                   capmix_print_map(const capmix_mem_t *map, const char *prefix, capmix_addr_t prev_offset)
{
	if( map == NULL ) map = (const capmix_mem_t *)(top_map_area[0].area);
	for(int i = 0; map[i].offset != 0xffffffff; i++ )
	{
		if( map[i].name == NULL ) continue;
		uint32_t offset = map[i].offset;
		const char *name = map[i].name;
		char new_prefix[64];
		if( strlen(prefix) == 0 )
			sprintf(new_prefix, "%s", name);
		else
			sprintf(new_prefix, "%s.%s", prefix, name);

		if( map[i].area == NULL )
		{
			printf("0x%08x %s\n", prev_offset + offset, new_prefix);
			continue;
		}

		capmix_print_map((const capmix_mem_t *)(map[i].area), new_prefix, prev_offset + offset);
	}
}

/**
 * @brief iterate through a memory map until the given partial name is found
 * @param map the map to search
 * @param part the name to match
 * @return the map with matching name, or NULL if not found
 */
static const capmix_mem_t *   capmix_lookup_map(const capmix_mem_t *map, const char *part)
{
	for( int i = 0; map[i].offset != capmix_None; i++ )
	{
		if( map[i].name == NULL ) continue;
		if( strcmp(map[i].name, part) != 0 ) continue;

		//debug(map[i].name);
		return &map[i];
	}
	return NULL;
}

/**
 * @brief parse a string into a device memory address
 * @param desc the string to parse
 * @return the device memory address
 * @ingroup API
 */
capmix_addr_t          capmix_parse_addr(const char *desc)
{
	capmix_addr_t ret = 0;
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map);
	char *desc_ = strdup(desc);
	char *tok = strtok(desc_, ".");
	while( tok != NULL )
	{
		map = capmix_lookup_map(map, tok);
		if( map == NULL ) //|| map == (void *)None )
		{
			return capmix_None;
		}
		ret += map->offset;
		//printf("0x%08x\n", ret);
		tok = strtok(NULL, ".");
		map = (const capmix_mem_t *)(map->area);
	}
	free(desc_);
	return ret;
}

/**
 * @brief the top part of the memory map is more efficient to navigate using a piecewise function
 * @param addr the address to examine
 * @return the section index of the top memory map
 */
int                    capmix_top_section(capmix_addr_t addr)
{
	int section;
	if( addr >> 12 == 0x51 )
		section = LINE;
	else if( addr >> 16 == 0x9 )
		section = 0x8;
	else
		section = addr >> 16;
	return section;
}

/**
 * @brief algorithm for decomposing a device memory address into a vector of map pointers
 * @param addr the address to examine
 * @return a vector containing the path across the map taken by this algorithm to find the given address
 */
capmix_mem_vector_t    capmix_mem_vector(capmix_addr_t addr)
{
	int v = 0;
	capmix_mem_vector_t vec = { .areas = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } };

	int section = capmix_top_section(addr);
	const capmix_mem_t *map = (const capmix_mem_t *)(memory_map[section].area);
	if( memory_map[section].name == NULL )
		return vec;

	vec.areas[v++] = &(memory_map[section]);

	int countdown = addr;
	countdown -= memory_map[section].offset;

	const capmix_mem_t *candidate;
	while( countdown >= 0 )
	{
		if( map == NULL ) break;
		candidate = NULL;

		capmix_addr_t min_offset = capmix_None;
		for( int i = 0; map[i].offset != capmix_None; i++ )
		{
			if( map[i].name == NULL ) continue;

			if( map[i].offset <= countdown)
			{
				if( countdown - map[i].offset < min_offset )
				{
					min_offset = countdown - map[i].offset;
					candidate = &(map[i]);
				}
			}
		}
		
		if( candidate == NULL )
		{
			return vec;
			const capmix_mem_t *prev_map = (const capmix_mem_t *)(vec.areas[v-1]->area);
			int next = -1;
			for(; prev_map[next].offset != capmix_None; next++ )
			{
				if( (void *)prev_map[next].area == (void *)map )
					break;
			}
			map = (const capmix_mem_t *)(prev_map[next+1].area);
			v--;
			continue;
		}

		vec.areas[v++] = candidate;
		countdown -= candidate->offset; // 0x120e
		map = (const capmix_mem_t *)(candidate->area);
	}
	if( countdown != 0 )
	{
		vec.areas[--v] = capmix_mem_none_area;
	}
	if( map != NULL && map->name != NULL )
	{
		vec.areas[v++] = map;
	}
	return vec;
}

/**
 * @brief write a string describing the given device memory address, e.g. 0x0006120e -> daw_monitor.b.channel.3.volume
 * @param[in] the device memory address to describe
 * @param[out] the string buffer to write into
 * @ingroup API
 */
void                   capmix_format_addr(capmix_addr_t addr, char *desc)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);

	int a = 0 ;
	const capmix_mem_t *area = vec.areas[a];
	sprintf(desc, "");
	do
	{
		strcat(desc, area->name);
		area = vec.areas[++a];
		if( area == NULL )
			break;
		strcat(desc, ".");
	}
	while( a < 8 );
}

/**
 * @brief returns the leaf-level name of the given device memory address
 * @param addr the device memory address to describe
 * @return the string that describes this address
 * @ingroup API
 */
const char *           capmix_addr_suffix(capmix_addr_t addr)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);
	int a = 0;
	while( a < 8 && vec.areas[a] != NULL && vec.areas[a+1] != NULL )
		a++;
	return vec.areas[a]->name;
}

/**
 * @brief get the value type of a device memory address
 * @param addr the device memory address to describe
 * @return the type identifier
 * @ingroup API
 */
capmix_type_t          capmix_addr_type(capmix_addr_t addr)
{
	capmix_mem_vector_t vec = capmix_mem_vector(addr);
	if( vec.areas[0] == NULL ) return TValue;
	int a = 0;
	while( a < 8 && vec.areas[a+1] != NULL )
		a++;
	return vec.areas[a]->type;
}
