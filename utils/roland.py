from functools import reduce
import operator
import math

def render_bytes(message):
	return ', '.join("0x%02x" % d for d in message)

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

def pan_to_long(pan):
	return int(0x4000 + (0x3fff * (pan / 100))) & 0x7fff

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
		self.name_table = {}

	def instance():
		global capture_view
		if 'capture_view' not in globals():
			capture_view = CaptureView()
		return capture_view
	
	def render_addrs(self):
		for desc in self.mixer:
			address = Capture.get_addr(desc)
			return hex(address)

	def add_name_to_table(self, desc):
		address = Capture.get_addr(desc)
		self.name_table[address] = desc

	def setup_name_table(self):
		self.name_table = {}
		for s in self.mixer:
			self.add_name_to_table(s)

	def lookup_name(self, addr):
		""" convert 0x00060008 into "input_monitor.a.channel.1.volume" """
		if addr in self.name_table:
			return self.name_table[addr]
		return hex(addr)

	def format_value(self, value, desc):
		if value is None: return "?"
		formatters = {
			(".mute",)   : lambda x: "MUTE"   if x else "m",
			(".solo",)   : lambda x: "SOLO"   if x else "s",
			(".stereo",) : lambda x: "STEREO" if x else "--mono--",
			('.+48',)    : lambda x: "+48V"   if x else "0VDC",
			('.lo-cut',) : lambda x: "LO CUT" if x else "low",
			('.phase',)  : lambda x: "-PHASE" if x else "+",
			('.bypass',) : lambda x: "BYPASS" if x else "comp",
			('.hi-z',)   : lambda x: "HI-Z"   if x else "lo-z",
		}
		for parts, formatter in formatters.items():
			for part in parts:
				if part in desc:
					return formatter(value.value)
		return value.format()

monitors = { chr(97+m): m<<12 for m in range(0,4) }
channels_16 = { ch+1: ch<<8 for ch in range(0, 16) }
channels_12 = { ch+1: ch<<8 for ch in range(0, 12) }
input_params = {
	'stereo': 0x00, # even channels only
	'solo':   0x02,
	'mute':   0x03,
	'pan':    0x04,
	'volume': 0x08,
	'reverb': 0x0e, # direct monitor only
}
daw_params = { k:v for k,v in input_params.items() if k != 'reverb' }
preamp_params = {
	'+48':    0x00,
	'lo-cut': 0x01,
	'phase':  0x02,
	#'hi-z':   0x03,    # only present on 1-2
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
}
lines = { ch+13: ch<<8 for ch in range(0,4) }
master_params = {
	'stereo':   0x00,
	'volume':   0x01,
}
master_left_right = {
	'left' : { 0: 0x0000 } | master_params,
	'right': { 0: 0x0100 } | master_params,
}
reverb_types = ['echo', 'room', 'small_hall', 'large_hall', 'plate']
reverb_params = {
	'pre_delay': 0x01,
	'time':      0x02,
}
patchbay_outputs = ["%d-%d" % (ch,ch+1) for ch in range(1,10,2)]


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
			'mix_a': 0x00,
			'mix_b': 0x01,
			'mix_c': 0x02,
			'mix_d': 0x03,
			'wave_1-2':  0x04,
			'wave_3-4':  0x05,
			'wave_5-6':  0x06,
			'wave_7-8':  0x07,
			'wave_9-10': 0x08,
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
		'initial_setting': 0x00000002,
		'reverb': { 0: 0x00040000 } | {
			'type': 0x0000,
		} | {
			verb: { 0: (i+1) << 8 } | reverb_params
			for i,verb in enumerate(reverb_types)
		},
		'patchbay': { 0: 0x00030000 } | {
			output: i
			for i,output in enumerate(patchbay_outputs)
		},

		'input_monitor': { 0: 0x00060000 } | {
			m: { 0: mon,
				'channel': {
					ch: { 0: channel } | input_params
					for ch,channel in channels_16.items()
				}
			}
			for m,mon in monitors.items()
		},
		'daw_monitor': { 0: 0x00070000 } | {
			m: { 0: mon,
				'channel': {
					ch: { 0: channel } | daw_params
					for ch,channel in channels_12.items()
				}
			}
			for m,mon in monitors.items()
		},
		'preamp': { 0: 0x00050000 } | {
			'channel': {
				ch: { 0: channel, 'hi-z': 0x03 } | preamp_params
				for ch,channel in channels_12.items() if ch <= 2
			} | {
				ch: { 0: channel } | preamp_params
				for ch,channel in channels_12.items() if ch >= 3
			}
		},
		'line': { 0: 0x00051000 } | {
			'channel': {
				ch: { 0: channel,
					'stereo':      0x00,
					'attenuation': 0x01,
				}
				for ch,channel in lines.items()
			}
		},
		'master': {
			'direct_monitor': { 0: 0x00080000 } | {
				'a': { 0: 0x0000 } | master_left_right | { 'reverb_return': 0x0007, 'link': 0x000d },
				'b': { 0: 0x1000 } | master_left_right | { 'link': 0x000d },
				'c': { 0: 0x2000 } | master_left_right | { 'link': 0x000d },
				'd': { 0: 0x3000 } | master_left_right | { 'link': 0x000d },
			},
			'daw_monitor': { 0: 0x00090000 } | {
				'a': { 0: 0x0000 } | master_left_right,
				'b': { 0: 0x1000 } | master_left_right,
				'c': { 0: 0x2000 } | master_left_right,
				'd': { 0: 0x3000 } | master_left_right,
			},
		},
		'meters_active': 0x000a0000, # 0=stop, 1=start
		'load_settings': 0x01000000, # send buffer size of 0x05eb (1515 dec)
	}

	def get_subtree(desc):
		maplist = [ int(word) if word.isdigit() else word for word in desc.split('.') ]
		return reduce(operator.getitem, maplist, Capture.memory_map)

	# new and improved
	def map_lookup(lookup, desc):
		addr = 0
		words = desc.split(' ')
		for word in words:
			selected = lookup
			value = 0
			for token in word.split('.'):
				if token.isdigit():
					token = int(token)
				try:
					selected = selected[token]
				except:
					raise Exception("Failed to lookup " + desc)
				if type(selected) is dict:
					if 0 in selected.keys():
						value += selected[0]
			if type(selected) is not dict:
				value += selected
			if addr | value != addr + value:
				raise Exception("%s conflicts with address space" % word)
			addr += value
		return addr

	def memory_names(prefix="", start=None):
		if start is None: start = Capture.memory_map
		ret = []
		for k,v in start.items():
			if type(v) is dict:
				ret += Capture.memory_names("%s%s." % (prefix, str(k)), v)
			else:
				if k != 0:
					ret += ["%s%s" % (prefix, k)]
		return ret

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
			3: [ ".volume", ".reverb" ],
		}
		for size, strings in sizes.items():
			for part in strings:
				if part in desc:
					return size
		return 1

class Value(object):
	def __init__(self, value=None):
		self.value = value
	def set_packed(self, data):
		self.value = self.unpack(data)

	@classmethod
	def from_packed(cls, data):
		ret = cls()
		ret.set_packed(data)
		return ret

	def format(self):
		if self.value is None:
			return '?'
		return str(self.value)

	def unpack(self, data):
		return data

	def increment(self):
		pass
	def decrement(self):
		pass
	def pack(self):
		if self.value is None: return None
		return [self.value]

	@classmethod
	def default(cls):
		return 0

class Byte(Value):
	def __init__(self, value=0, maxval=0x7f):
		self.value = value
		self.max = maxval
	def increment(self):
		if self.pack()[0] < self.max:
			self.value += 1
	def decrement(self):
		if self.pack()[0] > 0:
			self.value -= 1

	def unpack(self, data):
		return data[0]
	def pack(self):
		return [self.value & 0x7f]

class Bool(Value):
	def increment(self):
		self.value = 1
	def decrement(self):
		self.value = 0

	def unpack(self, data):
		return data[0] != 0
	def pack(self):
		return [0x1 if self.value else 0x0]

	def format(self):
		if self.value:
			return "ON"
		return "off"

class Volume(Value):
	def increment(self):
		if self.value == -math.inf:
			self.value = -71
		if self.value < 12:
			self.value += 1
	def decrement(self):
		self.value -= 1
		if self.value < -71:
			self.value = -math.inf

	def unpack(self, data):
		return long_to_db(nibbles_to_long(data))
	def pack(self):
		long = db_to_long(self.value)
		long = min(0x7fffff, long)
		if self.value != -math.inf and round(self.value) == 0:
			long = 0x200000
		return to_nibbles(long, 6)

	def format(self):
		if self.value == -math.inf:
			return "-∞"
		return "%+d" % round(self.value)

class Pan(Value):
	def increment(self):
		if self.value < 100:
			self.value += 1
	def decrement(self):
		if self.value > -100:
			self.value -= 1
	def unpack(self, data):
		return long_to_pan(nibbles_to_long(data))
	def pack(self):
		return to_nibbles(pan_to_long(self.value), 4)

	def format(self):
		if self.value < 0:
			return 'L%d' % -self.value
		elif self.value > 0:
			return "R%d" % self.value
		else:
			return "C"

class Sens(Value):
	def increment(self):
		if self.value < 58.0:
			self.value += 0.5
	def decrement(self):
		if self.value > 0:
			self.value -= 0.5
	def unpack(self, data):
		return data[0] * 0.5
	def pack(self):
		return [int(self.value * 2)]

class Scaled(Byte):
	def __init__(self, value=0, maxval=0x7f):
		return Byte.__init__(self, value, maxval)
	def offset(self):
		pass
	def unpack(self, data):
		return data[0] - self.offset()
	def pack(self):
		return [self.value + self.offset()]

class Threshold(Scaled):
	def __init__(self, value=-12):
		return Scaled.__init__(self, value, 40)
	def offset(self):
		return 40

class Gain(Scaled):
	def __init__(self, value=-12):
		return Scaled.__init__(self, value, 80)
	def offset(self):
		return 40
	def format(self):
		return Volume.format(self)

class Gate(Scaled):
	def __init__(self, value=-70):
		return Byte.__init__(self, value, 50)
	def offset(self):
		return 70
	def format(self):
		if self.value <= -self.offset():
			return '-inf'
		return Volume.format(self)

class Enum(Byte):
	def __init__(self, value=0):
		Byte.__init__(self, value, len(self.values())-1)
	def lookup(self):
		values = self.values()
		return values[self.value] if self.value < len(values) else None
	def format(self):
		lookup = self.lookup()
		return str(lookup) if lookup is not None else "?"
	def values(self):
		raise Exception("Not implemented: Enum.values")

class Ratio(Enum):
	ratios = [1, 1.12, 1.25, 1.4, 1.6, 1.8, 2, 2.5, 3.2, 4, 5.6, 8, 16, math.inf]
	def values(self):
		return self.ratios

class Attack(Enum):
	attacks = [
		 0.0,  0.1,  0.2,  0.3,  0.4,  0.5,  0.6,  0.7,  0.8,  0.9,
		 1.0,  1.1,  1.2,  1.3,  1.4,  1.5,  1.6,  1.7,  1.8,  1.9,
		 2.0,  2.1,  2.2,  2.4,  2.5,  2.7,  2.8,  3.0,  3.2,  3.3,  3.6,  3.8,
		 4.0,  4.2,  4.5,  4.7,  5.0,  5.3,  5.6,  6.0,  6.3,  6.7,  7.1,  7.5,
		 8.0,  8.4,  9.0,  9.4, 10.0, 10.6, 11.2, 12.0, 12.5, 13.3, 14.0,
		15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.4, 23.7, 25.0, 26.6, 28.0,
		30.0, 31.5, 33.5, 35.5, 37.6, 40.0, 42.2, 45.0, 47.3, 50.0, 53.0, 56.0,
		60.0, 63.0, 67.0, 71.0, 75.0, 80.0, 84.0, 90.0, 94.4, 100,  106,  112,
		120,  125,  133,  140,  150,  160,  170,  180,  190,  200,  210,  224,
		237,  250,  266,  280,  300,  315,  335,  355,  376,  400,  422,  450,
		473,  500,  530,  560,  600,  630,  670,  710,  750,  800
	]
	def values(self):
		return self.attacks

class Release(Enum):
	releases = [int(attack * 10) for attack in Attack.attacks]
	def values(self):
		return self.releases

class Knee(Enum):
	knees = ['HARD'] + [('SOFT%d' % k) for k in range(1,10)]
	def values(self):
		return self.knees

class Attenuation(Enum):
	attenuations = [-20, -10, 4]
	def values(self):
		return self.attenuations
	def format(self):
		lookup = self.lookup()
		return "%+d" % lookup if lookup else "?"

class ReverbType(Enum):
	types = [k for k in Capture.value_map['reverb']['type'].keys()]
	def values(self):
		return self.types
	def format(self):
		return self.lookup().upper()

class PreDelay(Enum):
	delays = [0.0, 0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4, 10, 20, 40, 80, 160]
	def values(self):
		return self.delays

class Patch(Enum):
	feeds = [
		'MIX A',
		'MIX B',
		'MIX C',
		'MIX D',
		'WAVE 1/2',
		'WAVE 3/4',
		'WAVE 5/6',
		'WAVE 7/8',
		'WAVE 9/10',
	]
	def values(self):
		return self.feeds

class ReverbTime(Value):
	def increment(self):
		if self.value < 5.0:
			self.value += 0.1
	def decrement(self):
		if self.value > 0.1:
			self.value -= 0.1
	def pack(self):
		return [int(self.value * 10) - 1]
	def unpack(self, data):
		return (data[0] + 1) * 0.1
	def format(self):
		return "%0.1f" % self.value

class ValueFactory:
	def get_class(desc):
		formatters = {
			(".volume",".reverb"): Volume,
			(".pan",): Pan,
			("reverb.type",): ReverbType,
			('.sens',): Sens,
			(".solo",".mute",".stereo",'.hi-z','.+48','.lo-cut','.phase','.bypass'): Bool,
			('.gate',): Byte,
			('.threshold',): Threshold,
			('.gain',): Gain,
			('.ratio',): Ratio,
			('.gate',): Gate,
			('.attenuation',): Attenuation,
			('.attack',): Attack,
			('.release',): Release,
			('.knee',): Knee,
			('.pre_delay',): PreDelay,
			('.time',): ReverbTime,
			('patchbay.',): Patch,
		}
		for parts, formatter in formatters.items():
			for part in parts:
				if part in desc:
					return formatter
		return Value
	def from_packed(desc, data):
		if data is None or data == []: return Value()
		cls = ValueFactory.get_class(desc)
		return cls.from_packed(data)

	def default_for(desc):
		cls = ValueFactory.get_class(desc)
		return cls(cls.default())

class Memory(object):
	def __init__(self):
		self.memory = {}
		self.capture_view = CaptureView.instance()

	def get(self, addr):
		if addr not in self.memory: return None
		return self.memory[addr]

	def set(self, addr, value):
		self.memory[addr] = value
	
	def get_value(self, addr):
		name = self.capture_view.lookup_name(addr)
		value = self.get(addr)
		return ValueFactory.from_packed(name, value)

	def get_long(self, addr):
		return self.get_value(addr).unpack()

	def get_formatted(self, addr):
		name = self.capture_view.lookup_name(addr)
		value = self.get_value(addr)
		return self.capture_view.format_value(value, name)

	def increment(self, addr):
		data = self.get(addr)
		name = self.capture_view.lookup_name(addr)
		value = ValueFactory.from_packed(name, data)
		value.increment()
		data = value.pack()
		self.set(addr, data)
		return data

	def decrement(self, addr):
		data = self.get(addr)
		name = self.capture_view.lookup_name(addr)
		value = ValueFactory.from_packed(name, data)
		value.decrement()
		data = value.pack()
		self.set(addr, data)
		return data

	def zero(self, addr):
		name = self.capture_view.lookup_name(addr)
		value = ValueFactory.default_for(name)
		data = value.pack()
		self.set(addr, data)
		return data

