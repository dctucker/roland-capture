#include "lib/roland.h"
#include "comm.h"

snd_rawmidi_t *midi_in;
snd_rawmidi_t *midi_out;

const char *port_name = "hw:CARD=STUDIOCAPTURE,DEV=0,SUBDEV=1";
unsigned char buf[8192];
int msglen = 0;

void setup_midi()
{
	int err;
	if( (err = snd_rawmidi_open(&midi_in, &midi_out, port_name, SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK)) < 0 )
	{
		return;
	}
	printf("Opened %s (%x, %x)\n", port_name, midi_in, midi_out);
}

void print_message()
{
	RolandSysex *sysex = parse_sysex(buf, msglen);
	if( sysex == NULL ) return;
	
	printf("cmd=%08x ", sysex->cmd);
	printf("addr=%08x data=", bytes_addr(sysex->addr));

	for(int i=0; i < msglen - 11 - 2; i++)
	{
		printf("%02x ", sysex->data[i]);
	}

	printf("\n");
	msglen = 0;
}

void read_midi()
{
	int err;
	err = snd_rawmidi_read(midi_in, buf+msglen, sizeof(buf)-msglen);
	if( err <= 0 ) return;
	for(int i=msglen; i < msglen + err; i++)
	{
		if( buf[i] == 0xf7 )
		{
			msglen += err;
			print_message();
			return;
		}
	}
	msglen += err;
}

void cleanup_midi()
{
	if( midi_in )  snd_rawmidi_close(midi_in);
	if( midi_out ) snd_rawmidi_close(midi_out);
}
