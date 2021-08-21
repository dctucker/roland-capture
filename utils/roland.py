import math

def print_bytes(message):
	print(', '.join("0x%02x" % d for d in message))

def to_bytes(long, n):
	buf = [long]
	while buf[0] > 0xff:
		a0 = buf[0]
		lsb = a0 & 0xff
		msb = a0 >> 8
		buf = [msb, lsb] + buf[1:]
	while len(buf) < n:
		buf.insert(0,0)
	return buf

def to_nibbles(val, n):
	buf = []
	acc = val
	while acc > 0 and len(buf) < n:
		buf.insert(0, acc & 0x0f)
		acc = acc >> 4
	while len(buf) < n:
		buf.insert(0,0)
	return buf

def db_to_long(db):
	return int(math.pow(10, db/20) * 0x200000)

def long_to_db(long):
	ratio = long / 0x200000
	return 20*math.log10(ratio)


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
	memory_map = {
		'initial_setting':    0x00000002,
		'reverb': {
			0:                0x00040000,
			'type':      0x0000,
			'pre_delay': 0x0101,
			'time':      0x0102,
		},
		'patchbay': {
			0:                0x00030000,
			'output': {
				'1-2': 0x00,
				'3-4': 0x01,
				'5-6': 0x02,
				'7-8': 0x03,
				'9-10':0x04,
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

	def get_value(desc):
		return Capture.map_lookup(Capture.value_map, desc)
	def get_addr(desc):
		return Capture.map_lookup(Capture.memory_map, desc)

	def vol_addr(mon, ch):
		return Capture.get_addr("input monitor.%s channel.%d channel.volume" % (mon, ch))

	def get_volume(mon, ch):
		return Roland.receive_data(Capture.vol_addr(mon, ch), 3)

	def set_volume(mon, ch, vol):
		return Roland.send_data(Capture.vol_addr(mon, ch), to_nibbles(vol,6))

