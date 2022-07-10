#include <poll.h>
#include <alsa/asoundlib.h>
#include "common.h"
#include "comm.h"

#ifndef DOXYGEN_SKIP
#define TRY_SEQ(X, MSG) if( (err = X) < 0 ){ warn( MSG ": %s\n", snd_strerror(err) ); return 0; }
#define MIDI_BYTES_PER_SEC (31250 / (1 + 8 + 2))
#endif

//const char *port_name = "hw:CARD=STUDIOCAPTURE,DEV=0,SUBDEV=1";
//const char *port_name = "24:1";
static const char *const port_names[] = {
	[MNone]   = "",
	[MQuad]   = "QUAD-CAPTURE:1",
	[MOcta]   = "OCTA-CAPTURE:1",
	[MStudio] = "STUDIO-CAPTURE:1",
};
static unsigned char  buf[8192];
static unsigned int   msglen = 0;
static int            buflen = 0;
static unsigned int   sysex_start = sizeof(buf);
static capmix_listener_t *capmix_listener = NULL;

static snd_seq_t *      seq;
static int              port_in, port_out;
static snd_seq_addr_t   device_port;
static int              n_descriptors;
static struct pollfd *  descriptors;

/**
 * @brief return port name for the selected model
 * @return pointer to the port name
 */
const char *  capmix_port_name()
{
	return port_names[capmix_model];
}

/**
 * @brief call the operating system to set up MIDI communication with the device
 * @param listener pointer to a callback function that receives a data buffer and the length of the buffer when MIDI messages are received
 * @return 1 on succes, 0 on failure
 */
int   capmix_setup_midi( capmix_listener_t *listener )
{
	const char *port_name = capmix_port_name();
	int err;
	TRY_SEQ( snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0), "Unable to open sequencer" )
	TRY_SEQ( snd_seq_set_client_name(seq, "libcapmix")            , "Unable to set client name" )
	TRY_SEQ( snd_seq_set_input_buffer_size(seq, 2048) , "Unable to set input buffer size" );
	TRY_SEQ( snd_seq_set_output_buffer_size(seq, 2048), "Unable to set output buffer size" );

	//input port
	TRY_SEQ( snd_seq_create_simple_port(seq, "libcapmix",
		 SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
		 SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION) , "Unable to create port" )
	port_in = err;

	//output port
	TRY_SEQ( snd_seq_create_simple_port(seq, "libcapmix",
		 SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
		 SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION) , "Unable to create port" )
	port_out = err;
	
	//connect application to device
	printf("Trying %s...\n", port_name);
	TRY_SEQ( snd_seq_parse_address(seq, &device_port, port_name)          , "Port not found")
	TRY_SEQ( snd_seq_connect_from(seq, port_in , device_port.client, device_port.port) , "Unable to connect input port" )
	TRY_SEQ( snd_seq_connect_to  (seq, port_out, device_port.client, device_port.port) , "Unable to connect output port" )
	TRY_SEQ( snd_seq_nonblock(seq, 1)                                           , "Unable to set nonblock mode" )

	//setup poll descriptors for read
	n_descriptors = snd_seq_poll_descriptors_count(seq, POLLIN);
	descriptors = malloc(sizeof(*descriptors) * n_descriptors);

    capmix_listener = listener;
	warn("Opened %s (%d,%d)\n", port_name, port_in, port_out);
	return 1;
}

/**
 * @brief poll the device, read any available MIDI data, and forward it to the listener callback
 * @return number of bytes received, 0 if none available, negative on error
 */
int   capmix_read_midi()
{
	int err;
	int timeout_ms = 1;
	snd_seq_poll_descriptors(seq, descriptors, n_descriptors, POLLIN);
	if (poll(descriptors, n_descriptors, timeout_ms) <= 0)
		return 0;
	do
	{
		snd_seq_event_t *event;
		//if( snd_seq_event_input_pending(seq, 0) <= 0 ) return 0;
		err = snd_seq_event_input(seq, &event);
		//printf("%d bytes remaining\n", err);
		if (err < 0)
		{
			//printf("err: %s\n", snd_strerror(err));
			return 0;
		}
		if( event == NULL )
			continue;

		if( event->type != SND_SEQ_EVENT_SYSEX )
			continue;

		//printf("MIDI read:");
		//for (int i = 0; i < event->data.ext.len; i++)
		//	printf(" %02X", ((unsigned char*)event->data.ext.ptr)[i]);
		//printf("\n");

		for(int i=0; i < event->data.ext.len; i++)
		{
			buf[buflen] = ((uint8_t *)event->data.ext.ptr)[i];
			buflen++;
			if( buf[buflen-1] == 0xf7 )
			{
				capmix_listener( buf, buflen );
				buflen = 0;
				continue;
			}
		}
		//printf("buflen: %d, err: %d\n", buflen, err);
	}
	while (err > 0);

	fflush(stdout);
	return buflen;
}

/**
 * @brief send MIDI data to the device
 * @param data the data buffer containing MIDI message to send
 * @param len length of the data buffer in bytes
 */
int   capmix_send_midi(uint8_t *data, int len)
{
	int err;
	snd_seq_event_t event;
	snd_seq_ev_clear( &event );
	snd_seq_ev_set_source( &event, port_out );
	snd_seq_ev_set_subs( &event );
	snd_seq_ev_set_direct( &event );
	snd_seq_ev_set_sysex( &event, len, data );

	TRY_SEQ( snd_seq_event_output( seq, &event ), "Error sending MIDI data" )
	snd_seq_drain_output( seq );

	return 0;
}

/**
 * @brief termniate the MIDI connection to the device and free up resources
 */
void  capmix_cleanup_midi()
{
	int err;
	err = snd_seq_drain_output(seq);
	err = snd_seq_sync_output_queue(seq);

	snd_seq_close(seq);
	free(descriptors);
}
