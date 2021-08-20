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
	def vol_addr(ch):
		return 0x00060008 + ((ch & 0x3f) << 8)

	def get_volume(ch):
		return Roland.receive_data(Capture.vol_addr(ch), 3)

	def set_volume(ch, vol):
		return Roland.send_data(Capture.vol_addr(ch), to_nibbles(vol,6))

