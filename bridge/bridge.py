#!bridge/bin/python3

import os
import alsa_midi
from bindings.capmix import capmix, EVENT
import time

from alsa_midi import SequencerClient, ControlChangeEvent, SysExEvent, alsa, ffi, PortUnsubscribedEvent

#bcr_name = 'mioXL-16-BCR'
bcr_name = 'BCR2000 MIDI 1'
capmix_name = 'STUDIO-CAPTURE MIDI 2'

monitors = ['a','b','c','d']
mutes = { (ch+1): { mon: 0 for mon in monitors } for ch in range(0, 16) }
stereo = { (ch+1): 0 for ch in range(0,16) }

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
control_map = {
	1, 3, 5, 7, 9, 11, 13, 15,
	257, 259, 261, 263, 265, 267, 269, 271,
}

def log(*msg):
	print(*msg)

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

	print("\033[10;%dr\033[%d;1f" % (size.lines, size.lines), end='')

nrpn_msb = 0
nrpn_lsb = 0
bcr_ok = False
def bcr_listener(event):
	global nrpn_msb, nrpn_lsb, bcr_ok
	if isinstance(event, PortUnsubscribedEvent):
		bcr_ok = False
		return
	elif isinstance(event, SysExEvent):
		bcr_ok = True
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

class BCR:
	def connect(self, client, port):
		self.client = client
		self.port = port

		client_ports = client.list_ports()
		# rtpmidi needed to access the BCR-2000
		bcr_ports = list(filter(lambda x: x is not None, [ x if x.name == bcr_name else None for x in client_ports ]))
		if len(bcr_ports) > 0:
			bcr_port = bcr_ports[0]
			port.connect_to(bcr_port)
			port.connect_from(bcr_port)
		else:
			log("Unable to connect to BCR-2000")
			return False

		log("BCR connected")
		self.ping()
		return True

	def ping(self):
		data = b'\xf0\x00\x20\x32\x7f\x15\x01\xf7'
		event = SysExEvent(data)
		self.client.event_output(event, port=self.port)
		self.client.drain_output()

	def listen(self):
		event = self.client.event_input(timeout=0.01)
		if event:
			bcr_listener(event)

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
		for msg in queue:
			self.send_nrpn(15, msg[0], msg[1], msg[2])
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

		log("addr=%x=%s type=%s v=%s" % (event.addr, addr, event.type_name(), value))
		print_monitor_mutes()

	def connect(self):
		capmix_ok = capmix.connect(Capture.listener)
		if not capmix_ok:
			log("Unable to connect to STUDIO-CAPTURE")
			return False
		self.get_mixer_data()
		return True

	def get_mixer_data(self):
		for ch in range(0,16,2):
			capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.stereo".format(ch+1)))
		for ch in range(0,16):
			for mon in monitors:
				capmix.get(capmix.parse_addr("input_monitor.{}.channel.{}.mute".format(mon, ch+1)))

def main():
	capture = Capture()
	bcr = BCR()

	# setup capmix
	capmix.set_model(4)

	# setup MIDI I/O
	client = SequencerClient("midipi")
	client.set_output_buffer_size(2048)
	client.set_input_buffer_size(2048)
	port = client.create_port("io")

	def check_connections(client, port):
		client_ports = client.list_ports()
		bcr_ok = False
		capmix_ok = False
		for p in client_ports:
			if p.name == bcr_name:
				bcr_ok = True
			if p.name == capmix_name:
				capmix_ok = True
		if not bcr_ok:
			log("BCR disconnected")
		if not capmix_ok:
			log("STUDIO-CAPTURE disconnected")

		bcr.ping()

		return (bcr_ok, capmix_ok)

	last_ok = 0
	last_attempt = 0
	capmix_ok = False
	global bcr_ok

	try:
		while True:
			now = time.time()
			if not ( bcr_ok and capmix_ok ):
				if now - last_attempt > 5:
					if not bcr_ok:
						bcr_ok = bcr.connect(client, port)
					if not capmix_ok:
						capmix_ok = capture.connect()
					last_attempt = now
			if capmix_ok:
				x = capmix.listen()
				bcr.sync()
			if bcr_ok:
				bcr.listen()
			if capmix_ok or bcr_ok:
				if now - last_ok > 5:
					bcr_ok, capmix_ok = check_connections(client, port)
					last_ok = now

	except KeyboardInterrupt:
		pass
	capmix.disconnect()
	log("Done.")

if __name__ == '__main__':
	main()
