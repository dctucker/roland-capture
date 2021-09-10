#include "lib/comm.h"
#include <alsa/asoundlib.h>

snd_rawmidi_t *midi_in;
snd_rawmidi_t *midi_out;

const char *port_name = "hw:CARD=STUDIOCAPTURE,DEV=0,SUBDEV=1";
unsigned char buf[8192];
int msglen = 0;
void (*capmix_listener)(uint8_t *, int) = NULL;

int capmix_setup_midi( void (*listener)(uint8_t *, int) )
{
	int err;
	if( (err = snd_rawmidi_open(&midi_in, &midi_out, port_name, SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK)) < 0 )
	{
		midi_in = NULL;
		midi_out = NULL;
		return 0;
	}
    capmix_listener = listener;
	printf("Opened %s (%x, %x)\n", port_name, midi_in, midi_out);
	return 1;
}

int capmix_read_midi()
{
	int err;
	err = snd_rawmidi_read(midi_in, buf+msglen, sizeof(buf)-msglen);
	if( err <= 0 ) return err;
	for(int i=msglen; i < msglen + err; i++)
	{
		if( buf[i] == 0xf7 )
		{
			msglen += err;
			capmix_listener(buf, msglen);
			int pmsglen = msglen;
			msglen = 0;
			return pmsglen;
		}
	}
	msglen += err;
	return msglen;
}

int capmix_send_midi(uint8_t *data, int len)
{
	int err;
	if( (err = snd_rawmidi_write(midi_out, data, len)) < 0 )
	{
		fprintf(stderr, "cannot send data: %s", snd_strerror(err));
		return 0;
	}
	return err;
}

void capmix_cleanup_midi()
{
	if( midi_out ) snd_rawmidi_close(midi_out);
	if( midi_in )  snd_rawmidi_close(midi_in);
}
