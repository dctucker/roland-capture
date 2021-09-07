#include <stdlib.h>
#include "roland.h"

#define DATA_START 7
u8 capture_sysex[] = { 0xf0, 0x41, 0x10, 0x00, 0x00, 0x6b };

u8 checksum(u8 *data, size_t len)
{
	u8 sum = 0;
	for(int i=0; i < len; i++)
	{
		sum += data[i];
	}
	return (0x80 - (sum % 0x80)) & 0x7f;
}

size_t make_sysex(u8 *buffer, u8 cmd, size_t len)
{
	int i;
	for(i=0; i < sizeof(capture_sysex); i++)
	{
		buffer[i] = capture_sysex[i];
	}
	buffer[i++] = cmd;
	i += len;
	buffer[i++] = checksum(&(buffer[DATA_START]), len);
	buffer[i++] = 0xf7;
	return i;
}

size_t make_receive_sysex(u8 *buffer, Addr addr, Addr size)
{
	u8 data[] = { addr_bytes(addr), addr_bytes(size) };
	return make_sysex(buffer, 0x11, sizeof(data));
}

size_t make_send_sysex(u8 *buffer, Addr addr, u8 *values, size_t len)
{
	int i,j;
	//u8 *data = malloc(4+len);
	int start=7;

	for(i=0; i < 4; i++)
		buffer[start+i] = (addr >> (8*(4-i))) & 0xff;

	for(j=0; j < len; i++, j++)
		buffer[start+i] = values[j];

	return make_sysex(buffer, 0x12, 4+len);
}

//#include <stdio.h>
RolandSysex * parse_sysex(u8 *buffer, size_t len)
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

