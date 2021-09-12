#include "lib/comm.h"
#include <alsa/asoundlib.h>

static snd_rawmidi_t *midi_in;
static snd_rawmidi_t *midi_out;

const char *port_name = "hw:CARD=STUDIOCAPTURE,DEV=0,SUBDEV=1";
static unsigned char buf[8192];
static unsigned int msglen = 0;
static unsigned int sysex_start = sizeof(buf);
static void (*capmix_listener)(uint8_t *, int) = NULL;

int capmix_setup_midi( void (*listener)(uint8_t *, int) )
{
	int err;
	if( (err = snd_rawmidi_open(&midi_in, &midi_out, port_name,
			SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK | SND_RAWMIDI_SYNC
	  )) < 0 )
	{
		midi_in = NULL;
		midi_out = NULL;
		return 0;
	}
    capmix_listener = listener;
	warn("Opened %s (%x, %x)\n", port_name, midi_in, midi_out);
	return 1;
}

int capmix_read_midi()
{
	int err;
	err = snd_rawmidi_read(midi_in, &buf[msglen], sizeof(buf)-msglen);
	if( err <= 0 ) return err;
	//warn("received %d\n", err);
	for(int i=msglen; i < msglen + err; i++)
	{
		//printf("0x%02x ", buf[i]);
		if( buf[i] == 0xf0 )
		{
			sysex_start = i;
		}
		else if( buf[i] == 0xf7 )
		{
			msglen += err;
			if( msglen + sysex_start < sizeof(buf) )
				capmix_listener(&buf[sysex_start], 1 + i - sysex_start);
			sysex_start = sizeof(buf);

			return i;
		}
	}
	printf("\n");
	msglen += err;
	if( msglen >= sizeof(buf) )
	{
		msglen = 0;
		sysex_start = sizeof(buf);
	}
	return msglen;
}

int capmix_send_midi(uint8_t *data, int len)
{
	int err;
	if( (err = snd_rawmidi_write(midi_out, data, len)) < 0 )
	{
		warn("cannot send data: %s", snd_strerror(err));
		return 0;
	}
	return err;
}

void capmix_cleanup_midi()
{
	if( midi_out ) snd_rawmidi_close(midi_out);
	if( midi_in )  snd_rawmidi_close(midi_in);
}
