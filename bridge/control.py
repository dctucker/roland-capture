import os
from bindings.capmix import Value, Type
from queue import Empty
from time import sleep
from view import log
from alsa_midi import ControlChangeEvent, SysExEvent, PortUnsubscribedEvent

class ControlSection:
	def listener(self, event):
		for k,v in self.cc_map.items():
			if event.param == v:
				self.__dict__[k] = event.value
				return k
	def __repr__(self):
		return repr(self.__dict__)

	def send(self, name, val):
		cc = self.cc_map[name]
		self.model.queue.put([self.channel, cc, val])

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
		self.view.logged = True
		k = super().listener(event)
		if k == 'arm':
			self.do_arm(event.value)
		elif k == 'mute':
			self.do_mute(event.value)
		elif k == 'fader':
			self.do_fader(event.value)
		elif k == 'knob':
			self.do_knob(event.value)
			self.view.logged = False
		#log(repr(self))

	def mixer_channel(self):
		return 1 + (self.channel * 2)

	def do_arm(self, down):
		ch = self.mixer_channel()
		if down == 0:
			return

		val = self.model.value('input_monitor.d.channel.%d.mute' % (ch))
		if val.unpacked.discrete == 0:
			self.model.send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), 0)
			self.model.send('input_monitor.%s.channel.%d.mute' % ('d', ch), 1)
			self.armed = True
		else:
			self.model.send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), 1)
			self.model.send('input_monitor.%s.channel.%d.mute' % ('d', ch), 0)
			self.armed = False
	
	def do_fader(self, val):
		ch = self.mixer_channel()
		if val == 127:
			self.model.send('input_monitor.a.channel.%d.mute' % (ch), 0)
			self.model.send('input_monitor.c.channel.%d.mute' % (ch), 1)
			self.last_monitor = 'a'
		elif val == 0:
			self.model.send('input_monitor.a.channel.%d.mute' % (ch), 1)
			self.model.send('input_monitor.c.channel.%d.mute' % (ch), 0)
			self.last_monitor = 'c'
		else:
			# TODO
			pass
	
	def do_mute(self, val):
		ch = self.mixer_channel()
		v = 0 if val == 0 else 1
		if self.armed:
			self.model.send('input_monitor.%s.channel.%d.mute' % (self.last_monitor, ch), v)
		else:
			self.model.send('input_monitor.%s.channel.%d.mute' % ('d', ch), v)
		self.muted = v == 1

	def do_solo(self, val):
		ch = self.mixer_channel()
		v = 0 if val == 0 else 1
		if self.armed:
			self.model.send('input_monitor.%s.channel.%d.solo' % (self.last_monitor, ch), v)
		else:
			self.model.send('input_monitor.%s.channel.%d.solo' % ('d', ch), v)
	
	def do_knob(self, val):
		if self.stop_held:
			if self.channel == 0:
				self.view.backlight(val)
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

		self.model.send('input_monitor.a.channel.%d.pan' % (ch)  , pan_l.unpacked)
		self.model.send('input_monitor.a.channel.%d.pan' % (ch+1), pan_r.unpacked)
		self.model.send('input_monitor.c.channel.%d.pan' % (ch)  , pan_l.unpacked)
		self.model.send('input_monitor.c.channel.%d.pan' % (ch+1), pan_r.unpacked)
		self.model.send('input_monitor.d.channel.%d.pan' % (ch)  , pan_l.unpacked)
		self.model.send('input_monitor.d.channel.%d.pan' % (ch+1), pan_r.unpacked)

		self.model.pans[ch]   = pan_l
		self.model.pans[ch+1] = pan_r

		self.view.print_pan(ch)
		self.view.print_pan(ch+1)
		self.view.cursor_to_log()
	

class ControlTransport(ControlSection):
	channel = 15
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

	def ready(self):
		self.send(self.state, 127)

	def listener(self, event):
		k = super().listener(event)
		if k is None: return
		prev_state = self.state
		if k in ['play','stop','rew','ffw']:
			if event.value == 0: return
			self.state = k
			log(self.state)

			for state in ['ffw','rew','play','stop']:
				self.send(state, 127 if self.state == state else 0)

		if k == 'rec':
			if event.value == 0: return
			self.recording = not self.recording

		if k == 'cycle':
			if event.value == 0: return
			self.looping = not self.looping

		if k == 'stop' and prev_state == 'play':
			self.recording = False

		self.send('rec'  , 127 if self.recording else 0)
		self.send('cycle', 127 if self.looping   else 0)

		if self.left > 0 and self.right > 0 and k == 'stop' and prev_state == 'stop':
			os.system("reset")
			if self.looping:
				print("Rebooting")
				sleep(2)
				os.system('sudo systemctl reboot')
			else:
				print("Shutting down")
				sleep(2)
				os.system('sudo systemctl poweroff')
			exit(0)

		self.view.dim(False, force=True)
		if event.value == 0 and self.stop > 0:
			if k == 'left':
				os.system('tmux previous-window')

			if k == 'right':
				os.system('tmux next-window')

			if k == 'set':
				self.dim()
				self.view.dim(True, force=True)
		log(repr(self))

class Control:
	def __init__(self, name='nanoKONTROL2 nanoKONTROL2 _ CTR'):
		self.ok = False
		self.name = name
		self.channels = [ControlChannel(i) for i in range(0,8)]
		self.transport = ControlTransport()

	@property
	def model(self):
		return self._model

	@model.setter
	def model(self, model):
		for c in self.channels:
			c.model = model
		self.transport.model = model
		self._model = model
		self.transport.dim = self.dim

	@property
	def view(self):
		return self._view

	@view.setter
	def view(self, view):
		log("setting view")
		for c in self.channels:
			c.view = view
		self.transport.view = view
		self._view = view

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

	def send(self, name, val):
		if name in self.transport.cc_map:
			self.transport.send(name, val)

	def dim(self):
		self.send('stop', 0)
		self.send('cycle', 0)
		self.channels[0].send('mute', 0)

	def hello(self, delay=0.01):
		def blink(ch, cc, val):
			self.send_cc(ch, cc, val)
			sleep(delay)

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

		self.transport.ready()

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
			self.channels[event.channel].stop_held = (self.transport.stop > 0)
			self.channels[event.channel].listener(event)

	def sync_mutes(self):
		for i, chan in enumerate(self.channels):
			ch = 2 * i + 1

			s = chan.soloed
			m = chan.muted
			r = chan.armed

			solos = self.model.solos
			mutes = self.model.mutes
			chan.soloed = solos[ch]['a'] + solos[ch]['c'] + solos[ch]['d'] == 1
			chan.muted  = mutes[ch]['a'] + mutes[ch]['c'] + mutes[ch]['d'] == 3
			if not chan.muted:
				chan.armed = mutes[ch]['d']

			if s != chan.soloed:
				chan.send('solo', 127 if chan.soloed else 0)
			if m != chan.muted:
				chan.send('mute', 127 if chan.muted  else 0)
			if r != chan.armed:
				chan.send('arm' , 127 if chan.armed  else 0)

	def sync(self):
		self.sync_mutes()

		while True:
			try:
				msg = self.model.queue.get(block=False)
				self.send_cc(msg[0], msg[1], msg[2])
			except Empty:
				break

