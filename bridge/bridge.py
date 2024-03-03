#!bridge/bin/python3

import alsa_midi
from bindings.capmix import capmix, EVENT

from alsa_midi import SequencerClient, ControlChangeEvent

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
	' JUNO ',
]
control_map = {
	1, 3, 5, 7, 9, 11, 13, 15,
	257, 259, 261, 263, 265, 267, 269, 271,
}

def print_monitor_mutes():
	print("\0337\033[H\033[2K   ", end='')
	for ch in range(0, 16):
		print("%4d  " % (ch+1), end='')
	print()
	print("\033[2K", end='')
	for ch in range(0, 16, 2):
		st = "STEREO" if stereo[ch+1] else ""
		print("  %10s" % st, end='')
	print()
	for mon in monitors:
		print("\033[2K", end='')
		print("%2s " % (mon.upper()), end='')
		for ch in range(0, 16):
			value = mutes[ch+1][mon]
			if value == 0:
				msg = "----"
			else:
				msg = "MUTE"
			print("%5s " % (msg), end='')
		print()
	print("\033[2K")
	print("\033[2K",end='')
	for label in labels:
		print("  %10s" % label, end='')
	print()
	print("\033[2K")
	print("\0338", end='')

nrpn_msb = 0
nrpn_lsb = 0
def bcr_listener(event):
	global nrpn_msb, nrpn_lsb
	if event.channel != 15:
		return
	if event.param == 99: # MSB
		nrpn_msb = event.value
	if event.param == 98: # LSB
		nrpn_lsb = event.value
	if event.param == 6: # data
		handle_nrpn(nrpn_msb, nrpn_lsb, event.value)

	print(repr(event))
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
def main():
	# setup MIDI I/O
	client = SequencerClient("midipi")
	port = client.create_port("io")
	client_ports = client.list_ports()
	# rtpmidi needed to access the BCR-2000
	bcr_ports = list(filter(lambda x: x is not None, [ x if x.name == 'mioXL-16-BCR' else None for x in client_ports ]))
	if len(bcr_ports) > 0:
		bcr_port = bcr_ports[0]
		port.connect_to(bcr_port)
		port.connect_from(bcr_port)
	else:
		print("Unable to connect to BCR-2000")

	# setup capmix
	capmix.set_model(4)
	def listener(event):
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

		print("addr=%x=%s type=%s v=%s" % (event.addr, addr, event.type_name(), value))
		print_monitor_mutes()

	def bcr_listen(client):
		event = client.event_input(timeout=0.01)
		if event:
			bcr_listener(event)

	def bcr_sync(client):
		global queue
		for msg in queue:
			event = ControlChangeEvent(channel=15, param=99, value=msg[0])
			client.event_output(event, port=port)
			#print(repr(event))

			event = ControlChangeEvent(channel=15, param=98, value=msg[1])
			client.event_output(event, port=port)
			#print(repr(event))

			event = ControlChangeEvent(channel=15, param=6, value=msg[2])
			client.event_output(event, port=port)
			#print(repr(event))
		client.drain_output()

		queue = []

	ok = capmix.connect(listener)
	if not ok:
		print("Unable to connect to STUDIO-CAPTURE")

	try:
		if ok:
			for ch in range(0,16,2):
				capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.stereo".format(ch+1)))
			for ch in range(0,16):
				for mon in monitors:
					capmix.get(capmix.parse_addr("input_monitor.{}.channel.{}.mute".format(mon, ch+1)))
		while True:
			if ok:
				x = capmix.listen()
				bcr_sync(client)
			bcr_listen(client)
	except KeyboardInterrupt:
		pass
	capmix.disconnect()
	print("Done.")

if __name__ == '__main__':
	main()
