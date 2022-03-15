#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lib/capmix.h"
#include "lib/memory.h"

static volatile int quitting = 0;
static int sysex_len;
static uint8_t  sysex_buf[16];

static bool show_sysex  = false;
static bool show_memory = false;
static bool show_meters = true;

void print_meter(int value)
{
	//float v = (float)b / (float)0x7f;
	//printf("\033[48;5;%dm.", 232 + (int)(v * 23.));
	if( value == 0x7f )
		printf("\033[31m");
	else if( value > 0x70 )
		printf("\033[33m");
	else if( value > 0x40 )
		printf("\033[32m");
	else if( value > 10 )
		printf("\033[36m");
	else
		printf("\033[34m");

	printf("%02x ", value);
}

#define METER_COLOR 0
int  meter_colors[]   = { 232,  16,  17,  18,  19,  20,  27,  33,  39,  81, 116, 121, 120, 119, 155, 154, 190, 191, 227, 226, 220, 214, 208, 196 };
int  meter_color(float value)
{
	if( value == capmix_UnsetInt.continuous )
		return METER_COLOR;
	if( value >= 0. )
		return METER_COLOR+23;
	else if( value <= -70 )
		return METER_COLOR;
	for(int i=0; i < 22; i++)
	{
		if( value >= -i*3. )
			return METER_COLOR+23-i;
	}
	return METER_COLOR+1;
}

void print_float_meter(float value)
{
	printf("\033[48;5;%dm", meter_colors[meter_color(value)]);
	//printf("\033[38;5;%fm", value);
	printf(".");//, value);
}

void  handler(capmix_event_t event)
{
	char name[128];
	char value[16];

	//capmix_format_addr(event.addr, name);
	//capmix_format_type(event.type_info->type, event.unpacked, value);

	/*
	printf("cmd=%x addr=%08x  data=", event.sysex->cmd, event.addr);
	for(int i=0; i < event.sysex_data_length; i++)
	{
		char c = event.sysex->data[i];
		printf("%02x ", c);
	}
	//printf("name=%s ", name);
	//printf("type=%s ", event.type_info->name);
	//printf("unpacked=0x%x ", event.unpacked.discrete);
	//printf("value=%s ", value);
	*/

	if( show_meters )
	{
		for(int j=0; j < 62; j++)
		{
			capmix_addr_t addr = 0xa0001 + 2 * j;
			capmix_unpacked_t unpacked = capmix_memory_get_unpacked(addr);
			//capmix_format_type(TMeter, unpacked, value);
			//printf("%s ",value);
			print_float_meter(unpacked.continuous);
		}
		printf("\033[0m");
		for(int j=0; j < 8; j++)
		{
			capmix_addr_t addr = 0xa0101 + j;
			capmix_unpacked_t unpacked = capmix_memory_get_unpacked(addr);
			capmix_format_type(TClipMask, unpacked, value);
			printf("%s ",value);
		}
		printf("\033[0m\n");
		for(int i=0; i < 124; i+=2)
			printf("%x", i % 0x10);
		printf("0123 4567 89ab cdef 0123 4567 89ab cdef");
		printf("\n\033[A");
	}
	
	if( show_memory )
	{
		// memory integrity
		for(int j=0; j < 124; j++)
		{
			capmix_addr_t addr = 0xa0001 + j;
			int b = *capmix_memory_get(addr);
			print_meter(*capmix_memory_get(addr));
			//capmix_format_type(TMeter, unpacked, value);
		}
		for(int j=0; j < 8; j++)
		{
			capmix_addr_t addr = 0xa0101 + j;
			int8_t b = *capmix_memory_get(addr);
			//capmix_format_type(TMeter, unpacked, value);
			print_meter(*capmix_memory_get(addr));
		}
		printf("\n");
	}

	if( show_sysex )
	{
		// raw sysex values
		uint8_t b = 0;
		printf("\033[0mlength %d ", event.sysex_data_length);
		for(int j=0; j < event.sysex_data_length; j++)
		{
			b = event.sysex->data[j];
			print_meter(b);
		}
		printf("\n");
	}
}

int   get(const char *control)
{
	int i;
	capmix_addr_t addr = capmix_parse_addr(control);
	if( addr == capmix_None )
	{
		fprintf(stderr, "Unknown control: %s\n", control);
		return 1;
	}

	int ok = capmix_connect(handler);
	capmix_event_t e = capmix_get(addr);

	for(i=0; i < e.sysex_data_length + 13; i++) printf("0x%02x ", ((uint8_t *)e.sysex)[i]);
	printf("\n");

	if( ! ok ) return 2;

	i = 0;
	while( capmix_listen() <= 0 && i++ < 50)
		usleep(10000);

	capmix_disconnect();

	return 0;
}

int   set(const char *control, const char *value)
{
    int i = 0;
	capmix_addr_t addr = capmix_parse_addr(control);
	if( addr == capmix_None )
	{
		fprintf(stderr, "Unknown control: %s\n", control);
		return 1;
	}

	capmix_type_t type = capmix_addr_type(addr);
	capmix_unpacked_t unpacked = capmix_parse_type(type, value);
	if( unpacked.discrete == capmix_Unset )
	{
		fprintf(stderr, "Unable to parse value: %s\n", value);
		return 1;
	}
	int ok = capmix_connect(handler);
	capmix_event_t e = capmix_put(addr, unpacked);

	for(i=0; i < e.sysex_data_length + 13; i++) printf("0x%02x ", ((uint8_t *)e.sysex)[i]);
	printf("\n");

	if( ! ok ) return 2;

	i = 0;
	while( capmix_listen() <= 0 && i++ < 50)
		usleep(10000);

	return 0;
}

void  sigint_handler(int _)
{
	quitting = 1;
}

int   main(int argc, const char **argv)
{
	signal(SIGINT, sigint_handler);

	const char *control ="", *value ="";
	int a = 0;
	const char *arg0 = argv[a++];
	if( argc > 1 ) control = argv[a++];
	if( argc > 2 ) value   = argv[a++];
	//printf("args: control=%s value=%s\n", control, value);

	int ok = capmix_connect(handler);
	if( ok ) set("meters.active", "on");

	while(ok && ! quitting )
	{
		capmix_listen();
	}
	set("meters.active", "off");
	capmix_disconnect();

	printf("\nDone.\n");
	return 0;
}

