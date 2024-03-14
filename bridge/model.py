from bindings.capmix import capmix, Type, Value
from queue import Queue

class Model:
	def __init__(self):
		self.num_channels = 16
		self.monitors = ['a','b','c','d']
		self.mutes  = { (ch+1): { mon: 0 for mon in self.monitors } for ch in range(0, self.num_channels) }
		self.solos  = { (ch+1): { mon: 0 for mon in self.monitors } for ch in range(0, self.num_channels) }
		self.stereo = { (ch+1): 0 for ch in range(0,16) }
		self.pans   = { (ch+1): Value.parse(Type.Pan, "C") for ch in range(0, 16) }
		self.capture_hash = {}
		self.cache = {}
		self.queue = Queue()

	def reset(self):
		for c in range(0, self.num_channels):
			for m in self.monitors:
				self.mutes[c+1][m] = 0
				self.solos[c+1][m] = 0
				self.stereo[c+1] = 0

	def send(self, param, value):
		self.capture_hash[capmix.parse_addr(param)] = value

	def flush(self):
		for k,v in self.capture_hash.items():
			capmix.put(k, v)
		self.capture_hash = {}

	def get_cache(self, k):
		if k in self.cache:
			return self.cache[k]
	
	def set_cache(self, k, v):
		self.cache[k] = v

	def value(self, param):
		addr = capmix.parse_addr(param)
		val = self.get_cache(addr)
		if val is None:
			val = Value(Type.Value, 0)
		ty = capmix.addr_type(addr)
		return Value(ty, val.unpacked)

