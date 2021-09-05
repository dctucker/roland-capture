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
	addr_name(addr, name);
	printf("%s ", name);

	printf("\n");
	/*
	name = self.mixer.memory.addr_name(addr)
	value = self.app.mixer.memory.get_formatted(addr)
	self.app.debug("listener hears 0x%08x=%s; %s %s" % (addr, render_bytes(data), name, value))

	self.app.mixer.memory.set(addr, data)
	self.dispatch(addr, value)
	self.app.interface.notify_control(name)
	*/
}

int main(int argc, const char **argv)
{
	int ok = setup_midi();

	while(ok)
	{
		read_midi();
	}
	cleanup_midi();
	return 0;
}

