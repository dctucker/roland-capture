from time import sleep
from view import log
from bindings.capmix import capmix, Type

def addr_channel(addr):
	return ((addr & 0x0f00) >> 8) + 1

def addr_monitor(addr):
	return ((addr & 0x0f000) >> 12)

class Capture:
	def __init__(self, name='STUDIO-CAPTURE MIDI 2'):
		capmix.set_model(4)
		self.name = name
		self.ok = False

	def listener(self, event):
		value = event.value()
		self.model.set_cache(event.addr, value)

		addr = capmix.format_addr(event.addr)
		if 'input_monitor.' in addr:
			dirty = True
			if '.mute' in addr:
				ch  = addr_channel(event.addr)
				mon = self.model.monitors[addr_monitor(event.addr)]
				mute = value.unpacked.discrete
				self.model.mutes[ch][mon] = mute
				#self.model.queue.put([ord(mon) - ord('a'), ch, 0 if mute == 0 else 127])
				if mon == 'd': # TODO may not be needed
					self.model.queue.put([int((ch-1)/2), 82, 0 if mute == 0 else 127])
			elif '.solo' in addr:
				ch  = addr_channel(event.addr)
				mon = self.model.monitors[addr_monitor(event.addr)]
				solo = value.unpacked.discrete
				self.model.solos[ch][mon] = solo
			elif '.stereo' in addr:
				ch  = addr_channel(event.addr)
				self.model.stereo[ch] = value.unpacked.discrete
			elif '.pan' in addr:
				ch  = addr_channel(event.addr)
				mon = self.model.monitors[addr_monitor(event.addr)]
				self.model.pans[ch][mon] = self.model.pan_to_int(value) #capmix.format_type(Type.Pan, value.unpacked) #.unpacked.discrete >> 24

		log("addr=%x=%s type=%s v=%s" % (event.addr, addr, event.type_name(), value))

	def connect(self):
		self.ok = capmix.connect(self.listener)
		if not self.ok:
			log("Unable to connect to STUDIO-CAPTURE")
			return False
		self.view.dim(False)
		sleep(1)
		self.get_mixer_data()
		self.ok = True
		return True

	def disconnect(self):
		return capmix.disconnect()

	def listen(self):
		return capmix.listen()

	def query(self, name):
		capmix.get(capmix.parse_addr(name))
		self.listen()

	def get_mixer_data(self):
		for ch in range(0,16,2):
			self.query("input_monitor.a.channel.{}.stereo".format(ch+1))
		for ch in range(0,16):
			for mon in self.model.monitors:
				self.query("input_monitor.{}.channel.{}.mute".format(mon, ch+1))
				self.query("input_monitor.{}.channel.{}.solo".format(mon, ch+1))
				self.query("input_monitor.a.channel.{}.pan".format(ch+1))
				self.query("input_monitor.{}.channel.{}.volume".format(mon, ch+1))

