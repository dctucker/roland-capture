#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lib/capmix.h"

static volatile int quitting = 0;
static int sysex_len;
static uint8_t  sysex_buf[16];
static bool meters = false;

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
	for(int j=0; j < 62; j++)
	{
		capmix_addr_t addr = 0xa0001 + 2 * j;
		capmix_unpacked_t unpacked = capmix_memory_get_unpacked(addr);
		//capmix_format_type(TMeter, unpacked, value);
		printf("\033[48;5;%dm.", 232 + (int)(unpacked.continuous * 23.));
	}
	for(int j=0; j < 4; j++)
	{
		capmix_addr_t addr = 0xa0101 + 2 * j;
		capmix_unpacked_t unpacked = capmix_memory_get_unpacked(addr);
		//capmix_format_type(TMeter, unpacked, value);
		printf("\033[48;5;%dm.", 232 + (int)(unpacked.continuous * 23.));
	}
	printf("\033[40m\n");
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

