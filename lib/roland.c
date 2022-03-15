#include <stdlib.h>
#include <stdio.h>
#include "roland.h"

/**
 * @brief calculate the checksum for a given data buffer
 * @param data the buffer to sum
 * @param len the number of bytes to sum
 * @return a seven-bit unsigned value
 */
static uint8_t    checksum(uint8_t *data, int len)
{
	uint8_t sum = 0;
	for(int i=0; i < len; i++)
		sum += data[i];
	return (0x80 - (sum % 0x80)) & 0x7f;
}

/**
 * @brief attach status, header, command, checksum to a buffer containing data to be included in a SysEx message
 * @param buffer the buffer to write into
 * @param cmd the command to include in the message (0x11 = receive, 0x12 = send)
 * @param len length of the data segment
 * @return the number of bytes the message occupies
 */
static int        capmix_make_sysex(uint8_t *buffer, uint8_t cmd, int data_len)
{
	int i;
	for(i=0; i < sizeof(capmix_sysex_fields_t); i++)
	{
		buffer[i] = ((uint8_t *)capture_sysex)[i];
	}
	buffer[i++] = cmd;
	i += data_len;
	buffer[i++] = checksum(&(buffer[sizeof(capmix_sysex_fields_t)+1]), data_len);
	buffer[i++] = 0xf7;
	return i;
}

/**
 * @brief populate a buffer with a SysEx Data Receive message
 * @param addr the device memory address to request
 * @param size the number of bytes to request from device memory
 * @return the number of bytes the message occupies
 */
int               capmix_make_receive_sysex(uint8_t *buffer, capmix_addr_t addr, capmix_addr_t size)
{
	uint8_t data[] = { capmix_addr_bytes(addr), capmix_addr_bytes(size) };
	int i,j;

	int start=7;
	for(j=0; j < 8; j++)
		buffer[start+j] = data[j];

	return capmix_make_sysex(buffer, 0x11, 8);
}

/**
 * @brief populate a buffer with a SysEx Data Receive message
 * @param addr the device memory address to overwrite
 * @param data the data to write into device memory
 * @param size the number of bytes included in the data to be written
 * @return the number of bytes the message occupies
 */
int               capmix_make_send_sysex(uint8_t *buffer, capmix_addr_t addr, uint8_t *data, int data_len)
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

/**
 * @brief validate a SysEx message from a buffer
 * @param buffer the buffer to scan for a SysEx message
 * @param len the number of bytes in the buffer
 * @return a pointer to the structure for the validated SysEx message, or NULL if invalid
 */
capmix_sysex_t *  capmix_parse_sysex(uint8_t *buffer, int len)
{
	capmix_sysex_t *sysex = (capmix_sysex_t *)buffer;

	int good = 1;
	for(int i=0; i < 6; i++)
	{
		uint8_t c = ((uint8_t *)capture_sysex)[i];
		if( sysex->header[i] != c )
		{
			printf("Expected 0x%x, got 0x%x\n", c, sysex->header[i]);
			good = 0;
		}
	}

	if( !good )
		return NULL;
	return sysex;
}
