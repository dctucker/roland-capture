#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "lib/types.h"
#include "lib/roland.h"
#include "lib/capture.h"
#include "lib/memory.h"
#include "comm.h"

void listener(u8 *msgbuf, size_t msglen)
{
	int datalen = msglen - 13;
	RolandSysex *sysex = parse_sysex(msgbuf, msglen); //addr, data = Roland.parse_sysex(message)
	if( sysex == NULL ) return;
	Addr addr = bytes_addr(sysex->addr);
	
	printf("cmd=%x addr=%08x data=", sysex->cmd, addr);
	for(int i=0; i < datalen; i++)
		printf("%02x ", sysex->data[i]);

	memory_set(addr, sysex->data, datalen); //self.app.mixer.memory.set(addr, data)
	char name[256];
	addr_name(addr, name); //name = self.mixer.memory.addr_name(addr)
	printf("name=%s ", name);

	char value[256];
	ValueType type = addr_type(addr);
	printf("type=%s ", type_name(type));
	
	Unpacked unpacked = unpack_type(type, sysex->data);
	printf("unpacked=0x%x ", unpacked.as_int);

	format_unpacked(type, unpacked, value); // value = self.app.mixer.memory.get_formatted(addr)
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
		Addr addr = name_addr(control);
		if( addr == None )
		{
			fprintf(stderr, "Unknown control: %s\n", control);
			return 1;
		}
		ValueType type = addr_type(addr);
		int type_len = type_size(type);

		u8 sysex_buf[16];
		if( strlen(value) == 0 ) // get
		{
			int sysex_len = make_receive_sysex(sysex_buf, addr, type_len);

			for(int i=0; i < sysex_len; i++)
				printf("0x%02x ", sysex_buf[i]);
			printf("\n");

			int ok = setup_midi(); if( ! ok ) return 2;
			send_midi(sysex_buf, sysex_len);

			int i = 0;
			do
			{
				read_midi();
				usleep(10000);
			}
			while( i++ < 50 );

			cleanup_midi();
			return 0;
		}
		else // set
		{
			Unpacked unpacked = parse_type(type, value);
			if( unpacked.as_int == Unset )
			{
				fprintf(stderr, "Unable to parse value: %s\n", value);
				return 1;
			}
			char data[8];
			pack_type(type, unpacked, data);

			int sysex_len = make_send_sysex(sysex_buf, addr, data, type_len);

			int ok = setup_midi(); if( ! ok ) return 2;
			send_midi(sysex_buf, sysex_len);
			read_midi();
		}
	}
	else // interactive
	{
		int ok = setup_midi();
		while(ok)
		{
			read_midi();
		}
		cleanup_midi();
		return 0;
	}
}
