#!bridge/bin/python3

import os
import sys

from queue import Queue
from threading import Thread
import alsa_midi
from bindings.capmix import capmix, EVENT, Value, Type, UNPACKED
import time

from alsa_midi import SequencerClient, ControlChangeEvent, SysExEvent, alsa, ffi, PortUnsubscribedEvent

monitors = ['a','b','c','d']
mutes  = { (ch+1): { mon: 0 for mon in monitors } for ch in range(0, 16) }
solos  = { (ch+1): { mon: 0 for mon in monitors } for ch in range(0, 16) }
stereo = { (ch+1): 0 for ch in range(0,16) }
pans = { (ch+1): Value.parse(Type.Pan, "C") for ch in range(0, 16) }

def reset_model():
	global mutes, solos, stereo, monitors
	for c in range(0, 16):
		for m in monitors:
			mutes[c+1][m] = 0
			solos[c+1][m] = 0
			stereo[c+1] = 0


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

logged = False

eighths = {
	0: " ",
	1: "\u258f",
	2: "\u258e",
	3: "\u258d",
	4: "\u258c",
	5: "\u258b",
	6: "\u258a",
	7: "\u2589",
	8: "\u2588",
}

def pan_graphic(pan, width=4):
	p = int(str(pan).replace('R','').replace('L','-').replace('C','0'))
	x = p * (width / 2) / 100.0
	i = int(abs(x))
	f = int(abs(x * 8)) % 8
	w2 = int(width / 2)
	if p < 0:
		if p == -100:
			return '\u2588\u2588%s ' % (eighths[4])
		return (' ' * max(0, (w2 - i) - 1)) + "\033[7m" + eighths[8-f] + "\033[27m" + ("\u2588" * i) + eighths[4] + (' ' * (w2-1))
	elif p > 0:
		if p == 100:
			return '   \033[7m' + eighths[4] + '\033[27m' + (eighths[8] * 2)
		return '   \033[7m' + eighths[4] + '\033[27m' + (eighths[8] * i) + eighths[f] + (' ' * max(0, (w2 - i) - 1))
	return '  \u2503  '

def print_pan(ch):
	col = 6 * (ch-1) + 3
	print("\033[?25l\033[%d;%df" % (5, col), end='')
	pan = pans[ch]
	graphic = pan_graphic(pan)
	if ch % 2 == 1 or str(pan) == 'C':
		print(" ", end='')
		#graphic = " " + graphic
	print("\033[33m%s" % graphic, end='')

size = lambda: None
size.columns = 99
size.lines = 27

#size = os.get_terminal_size()
def cursor_to_log():
	height = 16
	print("\033[%d;%dr\033[%d;1f\033[?25h" % (height, size.lines, size.lines-1))

def print_monitor_mutes():
	global logged
	#print("\033[r",end='')

	print("\033[?25l\033[H\033[2K    ", end='')
	for ch in range(0, 16):
		s = ' /' if stereo[ch+1] else '  '
		if ch % 2 == 0:
			k = str(ch + 1).rjust(2) + ' '
		else:
			k = ' ' + str(ch + 1).ljust(2)
		print("%s%s" % (k, s), end='')
		if ch < 15:
			print(" ", end='')
	print()
	print("\033[2K")
	print("\033[2K ", end='')
	for ch in range(0, 16, 2):
		if stereo[ch+1]:
			st = "STEREO"
			sc = "\033[7;36m"
		else:
			st = ""
			sc = "\033[7;1;30m"
		print("   %s%7s  \033[0m" % (sc, st), end='')
	print()
	print("\033[2K")
	print("\033[2K  ", end='')
	for ch in range(0, 16):
		pan = pans[ch+1]
		graphic = pan_graphic(pan)
		print(" \033[33m%s" % graphic, end='')
	print()

	for mon in monitors:
		print("\033[2K")
		print("\033[2K", end='')
		print("%2s " % (mon.upper()), end='')
		for ch in range(0, 16):
			value = mutes[ch+1][mon]
			if value == 0:
				msg = "     "
				color = "\033[7;2;30m"
			else:
				msg = "  M  "
				color = "\033[7;2;31m"
			print("%s%5s\033[0m " % (color,msg), end='')
		print()
	print("\033[2K")
	print("\033[2K ",end='')
	for label in labels:
		print("  %8s  " % label, end='')
	print()
	print("\033[2K")
	#print("\0338", end='')

	cursor_to_log()
	logged = False

def log(*msg):
	global logged
	now = time.strftime("%H:%M:%S")
	logged = True
	print("\033[1;30m", end='')
	print(now, end=' ')
	print(*msg, end='')
	print("\033[0m")

capture_hash = {}
def capmix_send(param, value):
	global capture_hash
	capture_hash[capmix.parse_addr(param)] = value

def capmix_flush():
	global capture_hash
	for k,v in capture_hash.items():
		capmix.put(k, v)
	capture_hash = {}

cache = {}
def get_cache(k):
	if k in cache:
		return cache[k]

def capmix_value(param):
	global cache
	addr = capmix.parse_addr(param)
	val = get_cache(addr)
	if val is None:
		val = Value(Type.Value, 0)
	ty = capmix.addr_type(addr)
	return Value(ty, val.unpacked)


queue = []
enable_dimming = False
dimmed = False
brightness = 48

def backlight(value):
	global brightness
	if value > 15:
		brightness = value
	with open('/sys/class/backlight/10-0045/brightness','w') as f:
		f.write(str(value) + '\n')
		log('backlight: %d' % value)

def dim(dark=True, force=False):
	global enable_dimming, dimmed, queue
	if enable_dimming or force:
		if dark:
			if not dimmed:
				backlight(0)
			queue += [[15, 113, 0]]
			queue += [[0, 82, 0]]
			reset_model()
		else:
			if dimmed:
				backlight(brightness)
			queue += [[15, 113, 127]]
		dimmed = dark


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
		self.last_monitor = 'a'
		self.armed = False
		self.muted = False
		self.soloed = False

	def listener(self, event):
		global logged
		logged = True
		k = super().listener(event)
		if k == 'arm':
			self.do_arm(event.value)
		elif k == 'mute':
			self.do_mute(event.value)
		elif k == 'fader':
			self.do_fader(event.value)
		elif k == 'knob':
			self.do_knob(event.value)
			logged = False
		#log(repr(self))

	def mixer_channel(self):
		return 1 + (self.channel * 2)

	def do_arm(self, down):
		ch = self.mixer_channel()
		if down == 0:
			return

		val = capmix_value('input_monitor.d.channel.%d.mute' % (ch))
		if val.unpacked.discrete == 0:
			capmix_send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), 0)
			capmix_send('input_monitor.%s.channel.%d.mute' % ('d', ch), 1)
			self.armed = True
		else:
			capmix_send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), 1)
			capmix_send('input_monitor.%s.channel.%d.mute' % ('d', ch), 0)
			self.armed = False
	
	def do_fader(self, val):
		ch = self.mixer_channel()
		if val == 127:
			capmix_send('input_monitor.a.channel.%d.mute' % (ch), 0)
			capmix_send('input_monitor.c.channel.%d.mute' % (ch), 1)
			self.last_monitor = 'a'
		elif val == 0:
			capmix_send('input_monitor.a.channel.%d.mute' % (ch), 1)
			capmix_send('input_monitor.c.channel.%d.mute' % (ch), 0)
			self.last_monitor = 'c'
		else:
			# TODO
			pass
	
	def do_mute(self, val):
		ch = self.mixer_channel()
		v = 0 if val == 0 else 1
		if self.armed:
			capmix_send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), v)
		else:
			capmix_send('input_monitor.%s.channel.%d.mute' % ('d', ch), v)
		self.muted = v == 1

	def do_solo(self, val):
		ch = self.mixer_channel()
		v = 0 if val == 0 else 1
		if self.armed:
			capmix_send('input_monitor.%s.channel.%d.solo' % (self.last_monitor, ch), v)
		else:
			capmix_send('input_monitor.%s.channel.%d.solo' % ('d', ch), v)
	
	def do_knob(self, val):
		if self.stop_held:
			if self.channel == 0:
				backlight(val)
			return
		global pans

		ch = self.mixer_channel()
		width = int(100 * val / 127.0)
		if width == 0:
			pan_l = 'C'
			pan_r = 'C'
		else:
			pan_l = 'L%d' % width
			pan_r = 'R%d' % width

		pan_l = Value.parse(Type.Pan, pan_l)
		pan_r = Value.parse(Type.Pan, pan_r)

		capmix_send('input_monitor.a.channel.%d.pan' % (ch)  , pan_l.unpacked)
		capmix_send('input_monitor.a.channel.%d.pan' % (ch+1), pan_r.unpacked)
		capmix_send('input_monitor.c.channel.%d.pan' % (ch)  , pan_l.unpacked)
		capmix_send('input_monitor.c.channel.%d.pan' % (ch+1), pan_r.unpacked)
		capmix_send('input_monitor.d.channel.%d.pan' % (ch)  , pan_l.unpacked)
		capmix_send('input_monitor.d.channel.%d.pan' % (ch+1), pan_r.unpacked)
		pans[ch]   = pan_l
		pans[ch+1] = pan_r

		print_pan(ch)
		print_pan(ch+1)
		cursor_to_log()
	

class ControlTransport(ControlSection):
	cc_map = {
		'rew':  111,
		'ffw':  112,
		'stop': 113,
		'play': 114,
		'rec':  115,
		'left' : 85,
		'right': 86,
		'cycle': 87,
		'set':   88,
		'prev':  89,
		'next':  90,
	}
	def __init__(self):
		self.rec = 0
		self.play = 0
		self.stop = 0
		self.rew = 0
		self.ffw = 0
		self.track = 0
		self.left = 0
		self.right = 0
		self.cycle = 0
		self.set = 0
		self.prev = 0
		self.next = 0
		self.state = 'stop'
		self.recording = False
		self.looping = False
		global queue
		queue += [[15, self.cc_map[self.state], 127]]

	def listener(self, event):
		global queue
		k = super().listener(event)
		if k is None: return
		prev_state = self.state
		if k in ['play','stop','rew','ffw']:
			if event.value == 0: return
			self.state = k

		if k == 'rec':
			if event.value == 0: return
			self.recording = not self.recording

		if k == 'cycle':
			if event.value == 0: return
			self.looping = not self.looping

		if k == 'stop' and prev_state == 'play':
			self.recording = False

		for name in ['ffw','rew','play','stop']:
			cc = self.cc_map[name]
			v = 127 if self.state == name else 0
			queue += [[15, cc, v]]
		
		cc = self.cc_map['rec']
		if self.recording:
			queue += [[15, cc, 127]]
		else:
			queue += [[15, cc, 0]]

		cc = self.cc_map['cycle']
		if self.looping:
			queue += [[15, cc, 127]]
		else:
			queue += [[15, cc, 0]]

		if self.left > 0 and self.right > 0 and k == 'stop' and prev_state == 'stop':
			print("\033[H")
			if self.looping:
				print("Rebooting")
				os.system('sudo systemctl reboot')
			else:
				print("Shutting down")
				os.system('sudo systemctl poweroff')
			exit(0)

		dim(False, force=True)
		if event.value == 0 and self.stop > 0:
			if k == 'left':
				os.system('tmux previous-window')

			if k == 'right':
				os.system('tmux next-window')

			if k == 'set':
				dim(True, force=True)
		log(repr(self))

class Control:
	def __init__(self, name='nanoKONTROL2 nanoKONTROL2 _ CTR'):
		self.ok = False
		self.name = name
		self.channels = [ControlChannel(i) for i in range(0,8)]
		self.transport = ControlTransport()

	def connect(self, client, port):
		self.client = client
		self.port = port

		client_ports = client.list_ports()
		#print([x.name for x in client_ports])
		control_ports = list(filter(lambda x: x is not None, [ x if x.name == self.name else None for x in client_ports ]))
		if len(control_ports) > 0:
			control_port = control_ports[0]
			port.connect_to(control_port)
			port.connect_from(control_port)
		else:
			log("Unable to connect to controller")
			return False

		log("Control connected")
		self.ping()
		self.hello()
		return True

	def ping(self):
		data = b'\xf0\x7e\x7f\x06\x01\xf7' # MIDI Device ID request
		event = SysExEvent(data)
		self.client.event_output(event, port=self.port)
		self.client.drain_output()

	def send_cc(self, ch, cc, val):
		self.client.event_output(ControlChangeEvent(channel=ch, param=cc, value=val), port=self.port)
		self.client.drain_output()

	def hello(self, delay=0.01):
		def blink(ch, cc, val):
			self.send_cc(ch, cc, val)
			time.sleep(delay)

		def chase(val):
			for i in range(0,8):
				blink(i, 80, val)
			blink(15, 87, val)

			for i in range(0,8):
				blink(i, 81, val)

			for p in [111, 112, 113, 114, 115]:
				blink(15, p, val)
			for i in range(0,8):
				blink(i, 82, val)

		chase(127)
		chase(0)

	def listen(self):
		event = self.client.event_input(timeout=0.01)
		while event:
			self.listener(event)
			event = self.client.event_input(timeout=0.01)

	def listener(self, event):
		if isinstance(event, PortUnsubscribedEvent):
			self.ok = False
			return
		elif isinstance(event, SysExEvent):
			self.ok = True
			log(repr(event))
			return
		if event.channel == 15:
			self.transport.listener(event)
		elif event.channel < 8:
			self.channels[event.channel].stop_held = self.transport.stop > 0
			self.channels[event.channel].listener(event)

	def send_nrpn(self, ch, msb, lsb, val):
		event1 = ControlChangeEvent(channel=15, param=99, value=msb)
		event2 = ControlChangeEvent(channel=15, param=98, value=lsb)
		event3 = ControlChangeEvent(channel=15, param=6, value=val)

		self.client.event_output(event1, port=self.port)
		self.client.event_output(event2, port=self.port)
		self.client.event_output(event3, port=self.port)

		log(repr(event1), repr(event2), repr(event3))
		self.client.drain_output()

	def sync_mutes(self):
		global queue
		for i, chan in enumerate(self.channels):
			ch = 2 * i + 1

			s = chan.soloed
			m = chan.muted
			r = chan.armed

			chan.soloed = solos[ch]['a'] + solos[ch]['c'] + solos[ch]['d'] == 1
			chan.muted  = mutes[ch]['a'] + mutes[ch]['c'] + mutes[ch]['d'] == 3
			if not chan.muted:
				chan.armed = mutes[ch]['d']

			if s != chan.soloed:
				queue += [[i, chan.cc_map['solo'], 127 if chan.soloed else 0]]
			if m != chan.muted:
				queue += [[i, chan.cc_map['mute'], 127 if chan.muted else 0]]
			if r != chan.armed:
				queue += [[i, chan.cc_map['arm'], 127 if chan.armed else 0]]

	def sync(self):
		self.sync_mutes()

		global queue
		for msg in queue:
			self.send_cc(msg[0], msg[1], msg[2])
		queue = []

class Capture:

	def __init__(self, name='STUDIO-CAPTURE MIDI 2'):
		self.name = name
		self.ok = False

	@classmethod
	def listener(cls, event):
		global queue, cache
		value = event.value()
		cache[event.addr] = value

		addr = capmix.format_addr(event.addr)
		if 'input_monitor.' in addr:
			dirty = True
			if '.mute' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				mon = monitors[(event.addr & 0xf000) >> 12]
				mute = value.unpacked.discrete
				mutes[ch][mon] = mute
				#queue += [[ord(mon) - ord('a'), ch, 0 if mute == 0 else 127]]
				if mon == 'd':
					queue += [[int((ch-1)/2), 82, 0 if mute == 0 else 127]]
			elif '.stereo' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				stereo[ch] = value.unpacked.discrete
			elif '.pan' in addr:
				ch  = ((event.addr & 0x0f00) >> 8) + 1
				pans[ch] = value #capmix.format_type(Type.Pan, value.unpacked) #.unpacked.discrete >> 24

		#log("addr=%x=%s type=%s v=%s" % (event.addr, addr, event.type_name(), value))

	def connect(self):
		self.ok = capmix.connect(Capture.listener)
		if not self.ok:
			log("Unable to connect to STUDIO-CAPTURE")
			return False
		dim(False)
		time.sleep(1)
		self.get_mixer_data()
		self.ok = True
		return True

	def get_mixer_data(self):
		for ch in range(0,16,2):
			capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.stereo".format(ch+1)))
		for ch in range(0,16):
			for mon in monitors:
				capmix.get(capmix.parse_addr("input_monitor.{}.channel.{}.mute".format(mon, ch+1)))
				capmix.get(capmix.parse_addr("input_monitor.a.channel.{}.pan".format(ch+1)))

first_attempt = True
def main():
	global first_attempt
	global logged

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
		global first_attempt
		client_ports = client.list_ports()

		control_found = False
		capture_found = False
		for p in client_ports:
			if p.name == control.name:
				control_found = True
				control.ok = True
			if p.name == capture.name:
				capture_found = True

		if not control_found:
			control.ok = False
			log("Control disconnected")

		if not capture_found:
			if capture.ok or first_attempt:
				capture.ok = False
				dim(True)
			log("STUDIO-CAPTURE disconnected")

		if capture_found and not capture.ok:
			log("STUDIO-CAPTURE connected")
			capture.ok = capture.connect()
			dim(False)

		control.ping()
		first_attempt = False

		return (control.ok, capture.ok)

	last_ok = 0
	last_attempt = 0
	capture.ok = False

	try:
		while True:
			now = time.time()

			# attempt to reconnect
			if not ( control.ok and capture.ok ):
				if now - last_attempt > 5:
					if not control.ok:
						control.ok = control.connect(client, port)
					if not capture.ok:
						capture.ok = capture.connect()
					last_attempt = now
			if capture.ok:
				x = capmix.listen()

			if control.ok:
				control.sync()
				control.listen()

			# check for disconnect
			if capture.ok or control.ok:
				if now - last_ok > 5:
					control.ok, capture.ok = check_connections(client, port)
					last_ok = now

			capmix_flush()

			if logged:
				print_monitor_mutes()

	except KeyboardInterrupt:
		pass
	capmix.disconnect()
	log("Done.")

if __name__ == '__main__':
	main()
