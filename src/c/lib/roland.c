#include <stdlib.h>
#include "roland.h"

#define DATA_START 7
uint8_t capture_sysex[] = { 0xf0, 0x41, 0x10, 0x00, 0x00, 0x6b };

static uint8_t checksum(uint8_t *data, int len)
{
	uint8_t sum = 0;
	for(int i=0; i < len; i++)
	{
		sum += data[i];
	}
	return (0x80 - (sum % 0x80)) & 0x7f;
}

static int capmix_make_sysex(uint8_t *buffer, uint8_t cmd, int data_len)
{
	int i;
	for(i=0; i < sizeof(capture_sysex); i++)
	{
		buffer[i] = capture_sysex[i];
	}
	buffer[i++] = cmd;
	i += data_len;
	buffer[i++] = checksum(&(buffer[DATA_START]), data_len);
	buffer[i++] = 0xf7;
	return i;
}

int capmix_make_receive_sysex(uint8_t *buffer, capmix_Addr addr, capmix_Addr size)
{
	uint8_t data[] = { capmix_addr_bytes(addr), capmix_addr_bytes(size) };
	int i,j;

	int start=7;
	for(j=0; j < 8; j++)
		buffer[start+j] = data[j];

	return capmix_make_sysex(buffer, 0x11, 8);
}

int capmix_make_send_sysex(uint8_t *buffer, capmix_Addr addr, uint8_t *data, int data_len)
{
	int i,j;
	//uint8_t *data = malloc(4+data_len);
	int start=7;

	uint8_t addr_b[] = { capmix_addr_bytes(addr) };
	for(i=0; i < 4; i++)
		buffer[start+i] = addr_b[i];

	for(j=0; j < data_len; i++, j++)
		buffer[start+i] = data[j];

	return capmix_make_sysex(buffer, 0x12, 4+data_len);
}

//#include <stdio.h>
capmix_RolandSysex * capmix_parse_sysex(uint8_t *buffer, int len)
{
	capmix_RolandSysex *sysex = (capmix_RolandSysex *)buffer;

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

