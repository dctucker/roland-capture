#!bridge/bin/python3

import os
import alsa_midi
from bindings.capmix import capmix, EVENT, Value
import time

from alsa_midi import SequencerClient, ControlChangeEvent, SysExEvent, alsa, ffi, PortUnsubscribedEvent

TPan = 4

#control_name = 'mioXL-16-BCR'
#control_name = 'BCR2000 MIDI 1'
control_name = 'nanoKONTROL2 nanoKONTROL2 _ CTR'
capmix_name = 'STUDIO-CAPTURE MIDI 2'

monitors = ['a','b','c','d']
mutes = { (ch+1): { mon: 0 for mon in monitors } for ch in range(0, 16) }
stereo = { (ch+1): 0 for ch in range(0,16) }
pans = { (ch+1): 0 for ch in range(0, 16) }

labels = [
	'  Mic ',
	'Guitar',
	'Deluge',
	'Opsix',
	'Phatty',
	' SM10 ',
	'Circuit',
	' JUNO  ',
]

"""
Concept:

	<	>					S	o	S	o	S	o	S	o	S	o	S	o	S	o	S	o
	Cycle		Set	<	>	M	|	M	|	M	|	M	|	M	|	M	|	M	|	M	|
	<<	>>	[]	|>	()		R	+	R	+	R	+	R	+	R	+	R	+	R	+	R	+

Ch:	16						1		2		3		4		5		6		7		8
CC	85	86					80	10	80	10	80	10	80	10	80	10	80	10	80	10	80	10
CC	87		88	89	90		81		81    	81		81		81		81		81		81
CC	111	112	113	114	115		82	7	82	7 	82	7 	82	7 	82	7 	82	7 	82	7 	82	7

Knob: Stereo width; channel N pan = -knob, channel N+1 pan = +knob
Solo: toggle ABCD solo
Mute: toggle ABCD mute
Arm: fader crossfades between A and C when armed, fader controls D volume when unarmed

"""


def print_monitor_mutes():
	size = os.get_terminal_size()
	print("\033[r",end='')

	#print("\0337",end='')
	print("\033[H\033[2K   ", end='')
	for ch in range(0, 16):
		s = '/' if stereo[ch+1] else ' '
		print("%2d %s" % (ch+1, s), end='')
	print()
	print("\033[2K", end='')
	for ch in range(0, 16, 2):
		st = "LINK" if stereo[ch+1] else ""
		print("\033[36m  %6s\033[0m" % st, end='')
	print()
	print('  ',end='')
	for ch in range(0, 16):
		pan = pans[ch+1]
		print("\033[33m%4s" % str(pan), end='')
	print()

	for mon in monitors:
		print("\033[2K", end='')
		print("%2s " % (mon.upper()), end='')
		for ch in range(0, 16):
			value = mutes[ch+1][mon]
			if value == 0:
				msg = "---"
				color = "\033[1;33m"
			else:
				msg = "M "
				color = "\033[4;7;2;31m"
			print("%s%3s\033[0m " % (color,msg), end='')
		print()
	print("\033[2K")
	print("\033[2K ",end='')
	for label in labels:
		print("  %6s" % label, end='')
	print()
	print("\033[2K")
	#print("\0338", end='')

	print("\033[10;%dr\033[%d;1f" % (size.lines, size.lines-1))

def log(*msg):
	print(*msg)
	print_monitor_mutes()

nrpn_msb = 0
nrpn_lsb = 0
control_ok = False
def control_listener(event):
	global nrpn_msb, nrpn_lsb, control_ok
	if isinstance(event, PortUnsubscribedEvent):
		control_ok = False
		return
	elif isinstance(event, SysExEvent):
		control_ok = True
		log(repr(event))
		print_monitor_mutes()
		return
	if event.channel != 15:
		return
	if event.param == 99: # MSB
		nrpn_msb = event.value
	if event.param == 98: # LSB
		nrpn_lsb = event.value
	if event.param == 6: # data
		handle_nrpn(nrpn_msb, nrpn_lsb, event.value)

	log(repr(event))
	print_monitor_mutes()

def capmix_send(param, value):
	capmix.put(capmix.parse_addr(param), value)

def handle_nrpn(msb, lsb, value):
	param = ""
	if msb >= 0 and msb <= 3:
		mon = monitors[msb]
		param = "input_monitor." + mon
	if lsb >= 1 and lsb <= 16:
		ch = lsb
		param += ".channel." + str(ch) + ".mute"
		value = 1 if value > 0 else 0
	addr = capmix.parse_addr(param)
	capmix.put(addr, value)


queue = []

class ControlSection:
	def listener(self, event):
		for k,v in self.cc_map.items():
			if event.param == v:
				self.__dict__[k] = event.value
				return k
	def __repr__(self):
		return repr(self.__dict__)

class ControlChannel(ControlSection):
	cc_map = {
		'fader': 7,
		'knob': 10,
		'solo': 80,
		'mute': 81,
		'arm' : 82,
	}
	def __init__(self, ch):
		self.channel = ch
		self.solo = 0
		self.mute = 0
		self.arm  = 0
		self.fader = 0
		self.knob = 0

	def listener(self, event):
		k = super().listener(event)
		if k == 'arm':
			self.do_arm(event.value)
		elif k == 'fader':
			self.do_fader(event.value)
		elif k == 'knob':
			self.do_knob(event.value)
		log(repr(self))

	def mixer_channel(self):
		return 1 + (self.channel * 2)

	def do_arm(self, val):
		ch = self.mixer_channel()
		if val == 0:
			capmix_send('input_monitor.a.channel.%d.mute' % (ch), 1)
			capmix_send('input_monitor.c.channel.%d.mute' % (ch), 1)
			capmix_send('input_monitor.d.channel.%d.mute' % (ch), 0)
		else:
			self.do_fader(self.fader)
			capmix_send('input_monitor.d.channel.%d.mute' % (ch), 1)
	
	def do_fader(self, val):
		ch = self.mixer_channel()
		if val == 0:
			capmix_send('input_monitor.a.channel.%d.mute' % (ch), 1)
			capmix_send('input_monitor.c.channel.%d.mute' % (ch), 0)
		elif val == 127:
			capmix_send('input_monitor.a.channel.%d.mute' % (ch), 0)
			capmix_send('input_monitor.c.channel.%d.mute' % (ch), 1)
		else:
			# TODO
			pass
	
	def do_knob(self, val):
		global pans
		ch = self.mixer_channel()
		width = int(100 * val / 127.0)
		if width == 0:
			pan_l = 'C'
			pan_r = 'C'
		else:
			pan_l = 'L%d' % width
			pan_r = 'R%d' % width

		#print(repr(capmix.type(TPan).name))

		pan_l = Value.parse(TPan, pan_l)
		pan_r = Value.parse(TPan, pan_r)

		#print(repr(pan_l))
		#print(repr(pan_r))

		capmix_send('input_monitor.a.channel.%d.pan' % (ch)  , pan_l.unpacked)
		capmix_send('input_monitor.a.channel.%d.pan' % (ch+1), pan_r.unpacked)
		#capmix_send('input_monitor.c.channel.%d.pan' % (ch)  , pan_l)
		#capmix_send('input_monitor.c.channel.%d.pan' % (ch+1), pan_r)
		#capmix_send('input_monitor.d.channel.%d.pan' % (ch)  , pan_l)
		#capmix_send('input_monitor.d.channel.%d.pan' % (ch+1), pan_r)
		pans[ch] = pan_l
		pans[ch+1] = pan_r
	

class ControlTransport(ControlSection):
	cc_map = {
		'rew':  111,
		'ffw':  112,
		'stop': 113,
		'play': 114,
		'rec':  115,
		'cycle': 87,
	}
	def __init__(self):
		self.rec = 0
		self.play = 0
		self.stop = 127
		self.rew = 0
		self.ffw = 0
		self.cycle = 0
		self.track = 0

	def listener(self, event):
		super().listener(event)
		log(repr(self))

class Control:
	def __init__(self):
		self.channels = [ControlChannel(i) for i in range(0,8)]
		self.transport = ControlTransport()

	def connect(self, client, port):
		self.client = client
		self.port = port

		client_ports = client.list_ports()
		#print([x.name for x in client_ports])
		control_ports = list(filter(lambda x: x is not None, [ x if x.name == control_name else None for x in client_ports ]))
		if len(control_ports) > 0:
			control_port = control_ports[0]
			port.connect_to(control_port)
			port.connect_from(control_port)
		else:
			log("Unable to connect to controller")
			return False

		log("Control connected")
		self.ping()
		return True

	def ping(self):
		# data = b'\xf0\x00\x20\x32\x7f\x15\x01\xf7' # BCR
		data = b'\xf0\x7e\x7f\x06\x01\xf7' # nanoKONTROL2
		event = SysExEvent(data)
		self.client.event_output(event, port=self.port)
		self.client.drain_output()

	def listen(self):
		event = self.client.event_input(timeout=0.01)
		if event:
			#control_listener(event)
			self.listener(event)

	def listener(self, event):
		if isinstance(event, PortUnsubscribedEvent):
			control_ok = False
			return
		elif isinstance(event, SysExEvent):
			control_ok = True
			log(repr(event))
			print_monitor_mutes()
			return
		if event.channel == 15:
			self.transport.listener(event)
		elif event.channel < 8:
			self.channels[event.channel].listener(event)

	def send_nrpn(self, ch, msb, lsb, val):
		event1 = ControlChangeEvent(channel=15, param=99, value=msb)
		event2 = ControlChangeEvent(channel=15, param=98, value=lsb)
		event3 = ControlChangeEvent(channel=15, param=6, value=val)

		self.client.event_output(event1, port=self.port)
		self.client.event_output(event2, port=self.port)
		self.client.event_output(event3, port=self.port)

		log(repr(event1), repr(event2), repr(event3))
		print_monitor_mutes()
		self.client.drain_output()

	def sync(self):
		global queue
		#for msg in queue:
		#	self.send_nrpn(15, msg[0], msg[1], msg[2])
		queue = []

class Capture:
	@classmethod
	def listener(cls, event):
		global queue
		value = event.value()
		addr = capmix.format_addr(event.addr)
		if 'input_monitor.' in addr:
			dirty = True
			if '.mute' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				mon = monitors[(event.addr & 0xf000) >> 12]
				mute = value.unpacked.discrete
				mutes[ch][mon] = mute
				queue += [[ord(mon) - ord('a'), ch, 0 if mute == 0 else 127]]
			elif '.stereo' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				stereo[ch] = value.unpacked.discrete
			elif '.pan' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				pans[ch] = value #capmix.format_type(TPan, value.unpacked) #.unpacked.discrete >> 24

		log("addr=%x=%s type=%s v=%s" % (event.addr, addr, event.type_name(), value))
		print_monitor_mutes()

	def connect(self):
		capmix_ok = capmix.connect(Capture.listener)
		if not capmix_ok:
			log("Unable to connect to STUDIO-CAPTURE")
			return False
		time.sleep(0.3)
		self.get_mixer_data()
		return True

	def get_mixer_data(self):
		for ch in range(0,16,2):
			capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.stereo".format(ch+1)))
		for ch in range(0,16):
			for mon in monitors:
				capmix.get(capmix.parse_addr("input_monitor.{}.channel.{}.mute".format(mon, ch+1)))
				capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.pan".format(ch+1)))

def main():
	capture = Capture()
	control = Control()

	# setup capmix
	capmix.set_model(4)

	# setup MIDI I/O
	client = SequencerClient("midipi")
	client.set_output_buffer_size(2048)
	client.set_input_buffer_size(2048)
	port = client.create_port("io")

	def check_connections(client, port):
		client_ports = client.list_ports()
		control_ok = False
		capmix_ok = False
		for p in client_ports:
			if p.name == control_name:
				control_ok = True
			if p.name == capmix_name:
				capmix_ok = True
		if not control_ok:
			log("Control disconnected")
		if not capmix_ok:
			log("STUDIO-CAPTURE disconnected")

		control.ping()

		return (control_ok, capmix_ok)

	last_ok = 0
	last_attempt = 0
	capmix_ok = False
	global control_ok

	try:
		while True:
			now = time.time()
			if not ( control_ok and capmix_ok ):
				if now - last_attempt > 5:
					if not control_ok:
						control_ok = control.connect(client, port)
					if not capmix_ok:
						capmix_ok = capture.connect()
					last_attempt = now
			if capmix_ok:
				x = capmix.listen()
				control.sync()
			if control_ok:
				control.listen()
			if capmix_ok or control_ok:
				if now - last_ok > 5:
					control_ok, capmix_ok = check_connections(client, port)
					last_ok = now

	except KeyboardInterrupt:
		pass
	capmix.disconnect()
	log("Done.")

if __name__ == '__main__':
	main()
