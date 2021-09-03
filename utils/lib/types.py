from math import inf, pow, log10

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
	return int(pow(10, db/20) * 0x200000)

def long_to_db(long):
	""" convert 0x200000 to 0 """
	ratio = long / 0x200000
	if ratio == 0:
		return -inf
	return 20*log10(ratio)

def long_to_pan(long):
	return round(100 * (long - 0x4000) / 0x4000)

def pan_to_long(pan):
	return int(0x4000 + (0x3fff * (pan / 100))) & 0x7fff

class Value(object):
	size = None
	min, max = None, None
	step = None
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
		return nibbles_to_long(data)

	def increment(self):
		pass
	def decrement(self):
		pass
	def pack(self):
		if self.value is None: return None
		return to_nibbles(self.value, self.size)

	def get_size(self):
		return self.size

	@classmethod
	def default(cls):
		return 0

	def __eq__(self, other):
		assert type(self) == type(other)
		return self.value == other.value

class Byte(Value):
	size = 1
	min, max = 0x0, 0x7f
	step = 0x1

	def __init__(self, value=0):
		self.value = value

	def increment(self):
		if self.value == -inf:
			self.value = self.min
		elif self.value < self.max:
			self.value += self.step

	def decrement(self):
		if self.value == inf:
			self.value = self.max
		elif self.value > self.min:
			self.value -= self.step

	def unpack(self, data):
		return data[0] + self.min
	def pack(self):
		return [(self.value - self.min) & 0x7f]

class Bool(Byte):
	min, max = 0, 1
	step = 1

	def unpack(self, data):
		return data[0] != 0
	def pack(self):
		return [0x1 if self.value else 0x0]

	def format(self):
		if self.value:
			return "ON"
		return "off"

class Volume(Value):
	size = 6
	min, max = -inf, 12
	step = 1

	def increment(self):
		if self.value == -inf:
			self.value = -71
		if self.value < self.max:
			self.value += self.step

	def decrement(self):
		self.value -= 1
		if self.value < -71:
			self.value = -inf

	def unpack(self, data):
		return long_to_db(nibbles_to_long(data))
	def pack(self):
		long = db_to_long(self.value)
		long = min(0x7fffff, long)
		if self.value != -inf and round(self.value) == 0:
			long = 0x200000
		return to_nibbles(long, self.size)

	def format(self):
		if self.value == -inf:
			return "-∞"
		return "%+d" % round(self.value)

class Pan(Value):
	size = 4
	min, max = -100, 100
	step = 1

	def increment(self):
		if self.value < 100:
			self.value += 1
	def decrement(self):
		if self.value > -100:
			self.value -= 1
	def unpack(self, data):
		return long_to_pan(nibbles_to_long(data))
	def pack(self):
		return to_nibbles(pan_to_long(self.value), self.size)

	def format(self):
		if self.value < 0:
			return 'L%d' % -self.value
		elif self.value > 0:
			return "R%d" % self.value
		else:
			return "C"

class Scaled(Byte):
	min, max = None, None
	def __init__(self, value=0):
		return Byte.__init__(self, value)

class Sens(Scaled):
	size = 1
	min, max = 0.0, 58.0
	step = 0.5

	def increment(self):
		if self.value < self.max:
			self.value += self.step
	def decrement(self):
		if self.value > self.min:
			self.value -= self.step
	def unpack(self, data):
		return data[0] * self.step
	def pack(self):
		return [int(self.value / self.step)]

class Threshold(Scaled):
	min, max = -40, 0 #max = 0x28
	def __init__(self, value=-12):
		return Scaled.__init__(self, value)

class Gain(Scaled):
	min, max = -40, 40 #max = 0x50
	def __init__(self, value=-12):
		return Scaled.__init__(self, value)
	def format(self):
		return Volume.format(self)

class Gate(Scaled):
	min, max = -70, -20 #max = 0x32
	def __init__(self, value=-70):
		return Byte.__init__(self, value)
	def format(self):
		if self.value <= self.min:
			return '-∞'
		return Volume.format(self)

class Enum(Byte):
	def __init__(self, value=0):
		Byte.__init__(self, value)
		self.min, self.max = 0, len(self.values)-1
	def lookup(self):
		values = self.values
		v = self.value
		return values[v] if v < len(values) else None
	def format(self):
		lookup = self.lookup()
		return str(lookup) if lookup is not None else "?"

class Ratio(Enum):
	values = [1, 1.12, 1.25, 1.4, 1.6, 1.8, 2, 2.5, 3.2, 4, 5.6, 8, 16, inf]

class Attack(Enum):
	values = [
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

class Release(Enum):
	values = [int(attack * 10) for attack in Attack.values]

class Knee(Enum):
	values = ['HARD'] + [('SOFT%d' % k) for k in range(1,10)]

class Attenuation(Enum):
	values = [-20, -10, 4]
	def format(self):
		lookup = self.lookup()
		return "%+d" % lookup if lookup else "?"

class ReverbType(Enum):
	values = ['off', 'echo', 'room', 'small_hall', 'large_hall', 'plate']
	def format(self):
		return self.lookup().upper().replace('_',' ')

class PreDelay(Enum):
	values = [0.0, 0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4, 10, 20, 40, 80, 160]

class Patch(Enum):
	values = [
		'MIX A', 'MIX B', 'MIX C', 'MIX D',
		'WAVE 1/2', 'WAVE 3/4', 'WAVE 5/6', 'WAVE 7/8', 'WAVE 9/10',
	]

class ReverbTime(Value):
	size = 1
	min, max = 0.1, 5.0
	step = 0.1
	def increment(self):
		if self.value < self.max:
			self.value += self.step
	def decrement(self):
		if self.value > self.min:
			self.value -= self.step
	def pack(self):
		return [int(self.value * 10) - 1]
	def unpack(self, data):
		return (data[0] + 1) * 0.1
	def format(self):
		return "%0.1f" % self.value

class ValueFactory:
	classes = {
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
	def get_class(desc):
		for parts, cls in ValueFactory.classes.items():
			for part in parts:
				if part in desc:
					return cls
		return Value
	def from_packed(desc, data):
		if data is None or data == []: return Value()
		cls = ValueFactory.get_class(desc)
		return cls.from_packed(data)

	def default_for(desc):
		cls = ValueFactory.get_class(desc)
		return cls(cls.default())

