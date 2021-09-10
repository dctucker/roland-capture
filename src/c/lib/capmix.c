#include "memory.h"
#include "comm.h"
#include "capmix.h"
#include "comm.h"

static uint8_t sysex_buf[16];
static uint8_t data_buf[8];
static int connected;

struct capmix_event capmix_event_factory(uint8_t *msgbuf, int msglen)
{
	int datalen = msglen - 13;
	capmix_RolandSysex *sysex = capmix_parse_sysex(msgbuf, msglen);
	if( sysex == NULL ) return (struct capmix_event){};
	capmix_Addr addr = capmix_bytes_addr(sysex->addr);
	capmix_memory_set(addr, sysex->data, datalen);

	capmix_ValueType type = capmix_addr_type(addr);
	capmix_type_info *type_info = capmix_type(type);
	
	capmix_Unpacked unpacked = capmix_unpack_type(type, sysex->data);

	return (struct capmix_event){
		.sysex_data_length = datalen,
		.sysex = sysex,
		.addr = addr,
		.type_info = type_info,
		.unpacked = unpacked,
	};
}

void (*capmix_event_handler)(struct capmix_event) = NULL;

static void listener(uint8_t *buf, int len)
{
	struct capmix_event event = capmix_event_factory(buf, len);
	capmix_event_handler(event);
}

int capmix_connect( void (*event_handler)(struct capmix_event) )
{
	capmix_event_handler = event_handler;
	connected = capmix_setup_midi(listener);
	return connected;
}

int capmix_listen()
{
	if( ! connected ) return -2;
	return capmix_read_midi();
}

struct capmix_event capmix_get(capmix_Addr addr)
{
	capmix_ValueType type = capmix_addr_type(addr);
	int type_len = capmix_type_size(type);
	int sysex_len = capmix_make_receive_sysex(sysex_buf, addr, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	return (struct capmix_event){
		.sysex = (capmix_RolandSysex *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
	};
}

struct capmix_event capmix_put(capmix_Addr addr, capmix_Unpacked unpacked)
{
	capmix_ValueType type = capmix_addr_type(addr);
	capmix_pack_type(type, unpacked, data_buf);
	int type_len = capmix_type_size(type);
	int sysex_len = capmix_make_send_sysex(sysex_buf, addr, data_buf, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	return (struct capmix_event){
		.sysex = (capmix_RolandSysex *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
		.unpacked = unpacked,
	};
}

void capmix_disconnect()
{
	return capmix_cleanup_midi();
}

