#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "lib/types.h"
#include "lib/roland.h"
#include "lib/capture.h"
#include "lib/memory.h"
#include "comm.h"

void capmix_listener(u8 *msgbuf, size_t msglen)
{
	int datalen = msglen - 13;
	capmix_RolandSysex *sysex = capmix_parse_sysex(msgbuf, msglen); //addr, data = Roland.parse_sysex(message)
	if( sysex == NULL ) return;
	capmix_Addr addr = capmix_bytes_addr(sysex->addr);
	
	printf("cmd=%x addr=%08x data=", sysex->cmd, addr);
	for(int i=0; i < datalen; i++)
		printf("%02x ", sysex->data[i]);

	capmix_memory_set(addr, sysex->data, datalen); //self.app.mixer.memory.set(addr, data)
	char name[256];
	capmix_addr_name(addr, name); //name = self.mixer.memory.addr_name(addr)
	printf("name=%s ", name);

	char value[256];
	capmix_ValueType type = capmix_addr_type(addr);
	printf("type=%s ", capmix_type_name(type));
	
	capmix_Unpacked unpacked = capmix_unpack_type(type, sysex->data);
	printf("unpacked=0x%x ", unpacked.as_int);

	capmix_format_unpacked(type, unpacked, value); // value = self.app.mixer.memory.get_formatted(addr)
	printf("value=%s ", value);

	/*
	self.app.mixer.memory.set(addr, data)
	self.dispatch(addr, value)
	self.app.interface.notify_control(name)
	*/
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
	const char *arg0 = argv[a++];
	if( argc > 1 ) control = argv[a++];
	if( argc > 2 ) value   = argv[a++];
	//printf("args: control=%s value=%s\n", control, value);

	if( strlen(control) > 0 ) // non-interactive
	{
		capmix_Addr addr = capmix_name_addr(control);
		if( addr == None )
		{
			fprintf(stderr, "Unknown control: %s\n", control);
			return 1;
		}
		capmix_ValueType type = capmix_addr_type(addr);
		int type_len = capmix_type_size(type);

		u8 sysex_buf[16];
		if( strlen(value) == 0 ) // get
		{
			int sysex_len = capmix_make_receive_sysex(sysex_buf, addr, type_len);

			for(int i=0; i < sysex_len; i++)
				printf("0x%02x ", sysex_buf[i]);
			printf("\n");

			int ok = capmix_setup_midi(); if( ! ok ) return 2;
			capmix_send_midi(sysex_buf, sysex_len);

			int i = 0;
			do
			{
				if( capmix_read_midi() > 0 )
					break;
				else
					usleep(10000);
			}
			while( i++ < 50 );

			capmix_cleanup_midi();
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
			char data[8];
			capmix_pack_type(type, unpacked, data);

			int sysex_len = capmix_make_send_sysex(sysex_buf, addr, data, type_len);

			int ok = capmix_setup_midi(); if( ! ok ) return 2;
			capmix_send_midi(sysex_buf, sysex_len);
			capmix_read_midi();
		}
	}
	else // interactive
	{
		int ok = capmix_setup_midi();
		while(ok)
		{
			capmix_read_midi();
		}
		capmix_cleanup_midi();
		return 0;
	}
}
