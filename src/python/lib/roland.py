import operator
from functools import reduce
from math import inf, pow, log10
from lib.types import ReverbType

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
reverb_params = {
	'pre_delay': 0x01,
	'time':      0x02,
}
patchbay_outputs = ["%d-%d" % (ch,ch+1) for ch in range(1,10,2)]

class Capture():
	# received on USB connect to Windows machine: f0 41 10 00 00 6b 11  01 00 00 00  00 00 0b 60  14 f7
	memory_map = {
		'initial_setting': 0x00000002,
		'reverb': { 0: 0x00040000 } | {
			'type': 0x0000,
		} | {
			verb: { 0: (i+1) << 8 } | reverb_params
			for i,verb in enumerate(ReverbType.values[1:])
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
				'a': { 0: 0x0000 } | master_left_right | {
					'reverb_return': 0x0007, 'link': 0x000d
				},
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

