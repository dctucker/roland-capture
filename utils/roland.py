import math

def print_bytes(message):
	print(', '.join("0x%02x" % d for d in message))

def to_bytes(long, n):
	""" convert 0xffff to [ 0xff, 0xff ] """
	buf = [long]
	while buf[0] > 0xff:
		a0 = buf[0]
		lsb = a0 & 0xff
		msb = a0 >> 8
		buf = [msb, lsb] + buf[1:]
	while len(buf) < n:
		buf.insert(0,0)
	return buf

def bytes_to_long(buf):
	""" convert [ 0x7e, 0x7f ] to 0x7e7f """
	acc = 0x00000000
	for b in buf:
		acc <<= 8
		acc += b
	return acc

def to_nibbles(val, n):
	""" convert 0x0123 to [ 0x00, 0x01, 0x02, 0x03 ] """
	buf = []
	acc = val
	while acc > 0 and len(buf) < n:
		buf.insert(0, acc & 0x0f)
		acc = acc >> 4
	while len(buf) < n:
		buf.insert(0,0)
	return buf

def nibbles_to_long(buf):
	""" convert [ 0x00, 0x01, 0x02, 0x03 ] to 0x0123 """
	acc = 0
	for b in buf:
		acc <<= 4
		acc += b
	return acc

def db_to_long(db):
	""" convert -6.02 to 0x100000 """
	return int(math.pow(10, db/20) * 0x200000)

def long_to_db(long):
	""" convert 0x200000 to 0 """
	ratio = long / 0x200000
	if ratio == 0:
		return -math.inf
	return 20*math.log10(ratio)

def long_to_pan(long):
	return round(100 * (long - 0x4000) / 0x4000)


class Roland():
	capture_sysex = [ 0xf0,0x41,0x10,0x00,0x00,0x6b ]

	def csum(data):
		return (128 - (sum(data) % 128)) & 0x7f

	def sysex(cmd, data):
		csum = Roland.csum(data)
		return Roland.capture_sysex + [cmd] + data + [csum] + [0xf7]

	def receive_data(addr, size):
		return Roland.sysex(0x11, to_bytes(addr, 4) + to_bytes(size, 4))

	def send_data(addr, data):
		return Roland.sysex(0x12, to_bytes(addr, 4) + data)

	def parse_sysex(message):
		if message[0:6] != Roland.capture_sysex:
			return None, None
		if message[6] == 0x12:
			addr = bytes_to_long(message[7:11])
			data = message[11:-2]
		return addr, data

class CaptureView():
	def __init__(self):
		self.setup_mixer()
		self.setup_name_table()
	
	def setup_mixer(self):
		mixer = []
		for monitor in 'a','b','c','d':
			for ch in range(1, 1+16):
				s = "input monitor.%s channel.%d channel." % (monitor, ch)
				segment = []
				if ch % 2 == 1: segment += [ s + "stereo" ]
				segment += [
					s + "mute",
					s + "solo",
					s + "reverb",
					s + "pan",
					s + "volume",
				]
				mixer += segment
			for ch in range(1, 1+12):
				s = "daw_monitor monitor.%s channel.%d channel." % (monitor, ch)
				segment = []
				if ch % 2 == 1: segment += [ s + "stereo" ]
				segment += [
					s + "stereo",
					s + "mute",
					s + "solo",
					s + "pan",
					s + "volume",
				]
				mixer += segment
		mixer += [ "master.reverb_return", "reverb reverb.type" ]
		for type in "echo", "room", "small_hall", "large_hall", "plate":
			for param in "pre_delay", "time":
				mixer += [ "reverb reverb.%s reverb.%s" % (type, param) ]
		self.mixer = mixer

	def print_addrs(self):
		for desc in self.mixer:
			address = Capture.get_addr(desc)
			print(hex(address))

	def add_name_to_table(self, desc):
		address = Capture.get_addr(desc)
		self.name_table[address] = desc

	def setup_name_table(self):
		self.name_table = {}
		for s in self.mixer:
			self.add_name_to_table(s)
	
	def lookup_name(self, addr):
		if addr in self.name_table:
			return self.name_table[addr]
		return hex(addr)

	def format_value(self, desc, value):
		#size = self.get_size(desc)
		formatters = {
			(".volume",".reverb"): lambda x: "%+d" % long_to_db(nibbles_to_long(x)),
			(".pan",): lambda x: "%+d" % long_to_pan(nibbles_to_long(x)),
			("reverb.type",): lambda x: { v:k for (k,v) in Capture.value_map['reverb']['type'].items() }[nibbles_to_long(x)],
		}
		for parts, formatter in formatters.items():
			for part in parts:
				if part in desc:
					return formatter(value)
		return hex(bytes_to_long(value))
		
		

class Capture():
	value_map = {
		'initial_setting': {
			'all':         0x00,
			'mic_pre':     0x01,
			'monitor_mix': 0x02,
			'reverb':      0x03,
			'patchbay':    0x04,
		},
		'patchbay': {
			'monitor_a': 0x00,
			'monitor_b': 0x01,
			'monitor_c': 0x02,
			'monitor_d': 0x03,
			'wave_1-2':  0x05,
			'wave_3-4':  0x06,
			'wave_5-6':  0x07,
			'wave_7-8':  0x08,
			'wave_9-10': 0x09,
		},
		'preamp': {
			'line': {
				'attenuation': {
					'+4':  0x00,
					'-10': 0x01,
					'-20': 0x02,
				},
			},
		},
		'reverb': {
			'type': {
				'off':        0x00,
				'echo':       0x01,
				'room':       0x02,
				'small_hall': 0x03,
				'large_hall': 0x04,
				'plate':      0x05,
			},
		},
	}
	# received on USB connect to Windows machine: f0 41 10 00 00 6b 11  01 00 00 00  00 00 0b 60  14 f7
	memory_map = {
		'initial_setting':    0x00000002,
		'reverb': {
			0:                0x00040000,
			'echo':       0x0100,
			'room':       0x0200,
			'small_hall': 0x0300,
			'large_hall': 0x0400,
			'plate':      0x0500,
			'type':      0x00,
			'pre_delay': 0x01,
			'time':      0x02,
		},
		'patchbay': {
			0:                0x00030000,
			'output': {
				'1-2':  0x00,
				'3-4':  0x01,
				'5-6':  0x02,
				'7-8':  0x03,
				'9-10': 0x04,
			},
		},

		'input':              0x00060000,
		'daw_monitor':        0x00070000,
		'master': {
			'direct_monitor': 0x00080000,
			'link':           0x0008000d,
			'daw_monitor':    0x00090000,
			'reverb_return':  0x00080007,
		},
		'monitor': {
			'a': 0x0000,
			'b': 0x1000,
			'c': 0x2000,
			'd': 0x3000,
		},
		'channel': {
			1: 0x000,  2: 0x100,  3: 0x200,  4: 0x300,  5: 0x400,  6: 0x500,  7: 0x600,  8: 0x700,
			9: 0x800, 10: 0x900, 11: 0xa00, 12: 0xb00, 13: 0xc00, 14: 0xd00, 15: 0xe00, 16: 0xf00,
			'stereo': 0x00, # even channels only
			'solo':   0x02,
			'mute':   0x03,
			'pan':    0x04,
			'volume': 0x08,
			'reverb': 0x0e, # direct monitor only
		},

		'preamp': {
			0:                0x00050000,     # max channel 0x0b00
			1: 0x000,  2: 0x100,  3: 0x200,  4: 0x300,  5: 0x400,  6: 0x500,
			7: 0x600,  8: 0x700,  9: 0x800, 10: 0x900, 11: 0xa00, 12: 0xb00,
			'line': {
				13: 0x1000,
				14: 0x1100,
				15: 0x1200,
				16: 0x1300,
				'stereo':      0x00,
				'attenuation': 0x01,
			},
			'+48':    0x00,
			'lo-cut': 0x01,
			'phase':  0x02,
			'hi-z':   0x03,    # only present on 1-2
			'sens':   0x04,    # max value 0x74
			'stereo': 0x05,
			'bypass':    0x06, # compressor
			'gate':      0x07, # max value 0x32
			'attack':    0x08, # max value 0x7c
			'release':   0x09, # max value 0x7c
			'threshold': 0x0a, # max value 0x28
			'ratio':     0x0b, # max value 0x0d
			'gain':      0x0c, # max value 0x50
			'knee':      0x0d, # max value 0x09
		},
		'meters_active':      0x000a0000, # 0=stop, 1=start
		'load_settings':      0x01000000, # send buffer size of 0x05eb (1515 dec)
	}
	def map_lookup(lookup, desc):
		addr = 0
		words = desc.split(' ')
		for word in words:
			selected = lookup
			for token in word.split('.'):
				if token.isdigit():
					token = int(token)
				selected = selected[token]
			if type(selected) is dict:
				value = selected[0]
			else:
				value = selected
			if addr | value != addr + value:
				raise Exception("%s conflicts with address space" % word)
			addr += value
		return addr

	def addr_to_desc(addr):
		a = addr
		min_diff = 0xffffffff
		found_word = None
		for k,v in Capture.memory_map.items():
			if v - a < min_diff:
				min_diff = v - a
				found_word = k
		return found_word

	def get_value(desc):
		return Capture.map_lookup(Capture.value_map, desc)
	def get_addr(desc):
		return Capture.map_lookup(Capture.memory_map, desc)

	# request size is half of sent size in some cases
	def get_size(desc):
		sizes = {
			2: [ ".pan", ],
			3: [ ".volume", ],
		}
		for size, strings in sizes.items():
			for part in strings:
				if part in desc:
					return size
		return 1

	def vol_addr(mon, ch):
		desc = "input monitor.%s channel.%d channel.volume" % (mon, ch)
		return Capture.get_addr(desc), Capture.get_size(desc)

	def get_volume(mon, ch):
		addr, size = Capture.vol_addr(mon, ch)
		return Roland.receive_data(addr, size)

	def set_volume(mon, ch, vol):
		addr, size = Capture.vol_addr(mon, ch)
		return Roland.send_data(addr, to_nibbles(vol, 2*size))

