#include <stdlib.h>
#include "roland.h"

u8 capture_sysex[] = { 0xf0,0x41,0x10,0x00,0x00,0x6b };

u8 checksum(u8 *data, int len)
{
	u8 sum = 0;
	for(int i=0; i < len; i++)
	{
		sum += data[i];
	}
	return (0x80 - (sum % 0x80)) & 0x7f;
}

void make_sysex(u8 *buffer, u8 cmd, u8 *data, int len)
{
	int i;
	for(i=0; i < sizeof(capture_sysex); i++)
	{
		buffer[i] = capture_sysex[i];
	}
	buffer[i++] = cmd;
	for(int j=0; j < len; j++)
	{
		buffer[i++] = data[j];
	}
	buffer[i++] = checksum(data, len);
	buffer[i++] = 0xf7;
}

void make_receive_sysex(u8 *buffer, Addr addr, Addr size)
{
	u8 data[] = { addr_bytes(addr), addr_bytes(size) };
	make_sysex(buffer, 0x11, data, sizeof(data));
}

void make_send_sysex(u8 *buffer, Addr addr, u8 *values, int len)
{
	int i,j;
	u8 *data = malloc(4+len);

	for(i=0; i < 4; i++)
		data[i] = (addr >> (8*(4-i))) & 0xff;

	for(j=0; j < len; j++)
		data[i++] = values[j];

	make_sysex(buffer, 0x12, data, 4+len);
	free(data);
}

//#include <stdio.h>
RolandSysex * parse_sysex(u8 *buffer, int len)
{
	RolandSysex *sysex = (RolandSysex *)buffer;

	for(int i=0; i < 6; i++)
	{
		if( sysex->header[i] != capture_sysex[i] )
		{
			//printf("Expected %x, got %x\n", sysex->header[i], capture_sysex[i]);
			return NULL;
		}
	}

	return sysex;
}

/*
def parse_sysex(message):
	if message[0:6] != Roland.capture_sysex:
		return None, None
	if message[6] == 0x12:
		addr = bytes_to_long(message[7:11])
		data = message[11:-2]
	return addr, data
*/
