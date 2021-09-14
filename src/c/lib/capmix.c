#include "memory.h"
#include "comm.h"
#include "capmix.h"
#include "comm.h"

static uint8_t sysex_buf[16];
static uint8_t data_buf[8];
static int connected;
static void (*capmix_event_handler)(struct capmix_event) = NULL;

static capmix_event_t   capmix_event_factory(uint8_t *msgbuf, int msglen)
{
	int datalen = msglen - 13;
	capmix_sysex_t *sysex = capmix_parse_sysex(msgbuf, msglen);
	if( sysex == NULL ) return (struct capmix_event){};
	capmix_addr_t addr = capmix_bytes_addr(sysex->addr);
	capmix_memory_set(addr, sysex->data, datalen);

	capmix_type_t type = capmix_addr_type(addr);
	capmix_type_info *type_info = capmix_type(type);
	
	capmix_unpacked_t unpacked = capmix_unpack_type(type, sysex->data);

	return (struct capmix_event){
		.sysex_data_length = datalen,
		.sysex = sysex,
		.addr = addr,
		.type_info = type_info,
		.unpacked = unpacked,
	};
}

static void             listener(uint8_t *buf, int len)
{
	struct capmix_event event = capmix_event_factory(buf, len);
	if( event.sysex != NULL )
		capmix_event_handler(event);
}

int                     capmix_connect( void (*event_handler)(struct capmix_event) )
{
	capmix_memory_init();
	capmix_event_handler = event_handler;
	connected = capmix_setup_midi(listener);
	return connected;
}

int                     capmix_listen()
{
	if( ! connected ) return -2;
	return capmix_read_midi();
}

capmix_event_t          capmix_get(capmix_addr_t addr)
{
	capmix_type_t type = capmix_addr_type(addr);
	int type_len = capmix_type_size(type);
	int sysex_len = capmix_make_receive_sysex(sysex_buf, addr, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	return (struct capmix_event){
		.sysex = (capmix_sysex_t *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
	};
}

capmix_event_t          capmix_put(capmix_addr_t addr, capmix_unpacked_t unpacked)
{
	capmix_type_t type = capmix_addr_type(addr);
	capmix_pack_type(type, unpacked, data_buf);
	int type_len = capmix_type_size(type);
	int sysex_len = capmix_make_send_sysex(sysex_buf, addr, data_buf, type_len);
	if( connected )
		capmix_send_midi(sysex_buf, sysex_len);
	return (struct capmix_event){
		.sysex = (capmix_sysex_t *)sysex_buf,
		.sysex_data_length = sysex_len - 13,
		.addr = addr,
		.type_info = capmix_type(type),
		.unpacked = unpacked,
	};
}

void                    capmix_disconnect()
{
	return capmix_cleanup_midi();
}

capmix_unpacked_t         capmix_memory_get_unpacked(capmix_addr_t addr)
{
	capmix_type_t type = capmix_addr_type(addr);
	uint8_t *data = capmix_memory_get(addr);
	if( *data == 0xff )
		return capmix_UnsetInt;
	return capmix_unpack_type(type, data);
}

void                    capmix_memory_set_unpacked(capmix_addr_t addr, capmix_unpacked_t unpacked)
{
	capmix_type_t type = capmix_addr_type(addr);
	int len = capmix_pack_type(type, unpacked, data_buf);
	capmix_memory_set(addr, data_buf, len);
}
