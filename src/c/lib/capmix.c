#include <unistd.h>
#include <stdio.h>

#include "capmix.h"
#include "memory.h"
#include "roland.h"
#include "comm.h"

static uint8_t sysex_buf[16];
static uint8_t data_buf[8];
static int     connected;
static capmix_event_handler_t *capmix_event_handler = NULL;

/**
 * @brief create a capmix_event_t for a given MIDI message buffer
 * @param msgbuf the byte buffer containing a SysEx message
 * @param msglen length of the message buffer in bytes
 * @return a capmix_event_t detailing the request
 */
static capmix_event_t   capmix_event_factory(uint8_t *msgbuf, int msglen)
{
	int datalen = msglen - 13;
	capmix_sysex_t *sysex = capmix_parse_sysex(msgbuf, msglen);
	if( sysex == NULL ) return (capmix_event_t){};
	capmix_addr_t addr = capmix_bytes_addr(sysex->addr);
	capmix_memory_set(addr, sysex->data, datalen);

	capmix_type_t type = capmix_addr_type(addr);
	capmix_type_info_t *type_info = capmix_type(type);
	
	capmix_unpacked_t unpacked = capmix_unpack_type(type, sysex->data);

	return (capmix_event_t){
		.sysex_data_length = datalen,
		.sysex = sysex,
		.addr = addr,
		.type_info = type_info,
		.unpacked = unpacked,
	};
}

/**
 * @brief processes received SysEx messages and send them on to the event handler
 * @param buf the byte buffer containing a SysEx message
 * @param len length of the message buffer in bytes
 */
static void             listener(uint8_t *buf, int len)
{
	capmix_event_t event = capmix_event_factory(buf, len);
	if( event.sysex != NULL )
		capmix_event_handler(event);
}

/**
 * @brief establish a connection with the audio device
 * @ingroup API
 * @param event_handler the callback function that will receive events
 * @return 1 on success, 0 when no connection could be established
 */
int                     capmix_connect( capmix_event_handler_t *event_handler )
{
	capmix_memory_init();
	capmix_event_handler = event_handler;
	connected = capmix_setup_midi(listener);
	return connected;
}

/**
 * @brief listen for SysEx messages and attempt to read from the device
 * @ingroup API
 * @return number of bytes read on success, 0 if no bytes read, or negative on error
 */
int                     capmix_listen()
{
	if( ! connected ) return -2;
	return capmix_read_midi();
}

/**
 * @brief send a message to the device requesting data for the given device memory address
 * @ingroup API
 * @param addr address in device memory to read
 * @return event structure describing the message sent to the device (not the response)
 */
capmix_event_t          capmix_get(capmix_addr_t addr)
{
	capmix_type_t type = capmix_addr_type(addr);
	int type_len = capmix_type_size(type);
	if( type_len > 1 ) type_len /= 2;
	int sysex_len = capmix_make_receive_sysex(sysex_buf, addr, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	else
		fprintf(stderr, "not connected\n");
	return (capmix_event_t){
		.sysex = (capmix_sysex_t *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
	};
}

/**
 * @brief send a message to the device requesting data for the given device memory address
 * @ingroup API
 * @param addr the address in device memory to affect
 * @param unpacked the value to store in device memory
 * @return event structure describing the message sent to the device (not the response)
 */
capmix_event_t          capmix_put(capmix_addr_t addr, capmix_unpacked_t unpacked)
{
	capmix_type_t type = capmix_addr_type(addr);
	capmix_pack_type(type, unpacked, data_buf);
	int type_len = capmix_type_size(type);
	int sysex_len = capmix_make_send_sysex(sysex_buf, addr, data_buf, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	return (capmix_event_t){
		.sysex = (capmix_sysex_t *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
		.unpacked = unpacked,
	};
}

/**
 * @brief disconnect from the device and clean up handles
 * @ingroup API
 */
void                    capmix_disconnect()
{
	return capmix_cleanup_midi();
}

/**
 * @brief access local memory to return the current value for a given device address
 * @ingroup API
 * @param addr the device memory address
 * @return unpacked value according to local memory
 */
capmix_unpacked_t         capmix_memory_get_unpacked(capmix_addr_t addr)
{
	capmix_type_t type = capmix_addr_type(addr);
	uint8_t *data = capmix_memory_get(addr);
	if( *data == 0xff )
		return capmix_UnsetInt;
	return capmix_unpack_type(type, data);
}

/**
 * @brief update local memory to store a value for a given device address
 * @ingroup API
 * @param addr the device memory address
 * @param unpacked the value to store in local memory
 */
void                    capmix_memory_set_unpacked(capmix_addr_t addr, capmix_unpacked_t unpacked)
{
	capmix_type_t type = capmix_addr_type(addr);
	int len = capmix_pack_type(type, unpacked, data_buf);
	capmix_memory_set(addr, data_buf, len);
}
