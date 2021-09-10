#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "lib/capmix.h"

void handler(struct capmix_event event)
{
	char name[128];
	char value[16];

	capmix_addr_name(event.addr, name);
	capmix_format_type(event.type_info->type, event.unpacked, value);

	printf("cmd=%x addr=%08x data=", event.sysex->cmd, event.addr);
	printf("name=%s ", name);
	printf("type=%s ", event.type_info->name);
	printf("unpacked=0x%x ", event.unpacked.as_int);
	printf("value=%s ", value);
	printf("\n");
}

int main(int argc, const char **argv)
{
	/*
	int opt;
	while ((opt = getopt(argc, argv, "ilw")) != -1)
	{
		switch(opt)
		{
			case '
		}
	}
	*/

	const char *control ="", *value ="";
	int a = 0;
    int i = 0;
    int ok;
	const char *arg0 = argv[a++];
	if( argc > 1 ) control = argv[a++];
	if( argc > 2 ) value   = argv[a++];
	//printf("args: control=%s value=%s\n", control, value);

	if( strlen(control) > 0 ) // non-interactive
	{
		int sysex_len;
		uint8_t  sysex_buf[16];

		capmix_Addr addr = capmix_name_addr(control);
		if( addr == capmix_None )
		{
			fprintf(stderr, "Unknown control: %s\n", control);
			return 1;
		}
		capmix_ValueType type = capmix_addr_type(addr);
		if( strlen(value) == 0 ) // get
		{
			ok = capmix_connect(handler);

			struct capmix_event e = capmix_get(addr);

			for(i=0; i < e.sysex_data_length + 13; i++) printf("0x%02x ", ((uint8_t *)e.sysex)[i]);
			printf("\n");

			if( ! ok ) return 2;

			i = 0;
			while( capmix_listen() <= 0 && i++ < 50)
				usleep(10000);

			capmix_disconnect();
			return 0;
		}
		else // set
		{
			capmix_Unpacked unpacked = capmix_parse_type(type, value);
			if( unpacked.as_int == capmix_Unset )
			{
				fprintf(stderr, "Unable to parse value: %s\n", value);
				return 1;
			}
			ok = capmix_connect(handler);
			struct capmix_event e = capmix_put(addr, unpacked);

			for(i=0; i < e.sysex_data_length + 13; i++) printf("0x%02x ", ((uint8_t *)e.sysex)[i]);
			printf("\n");

			if( ! ok ) return 2;
			capmix_listen();
		}
	}
	else // interactive
	{
		int ok = capmix_connect(handler);
		while(ok)
		{
			capmix_listen();
		}
		capmix_disconnect();
		return 0;
	}
}
