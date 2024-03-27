from multiprocessing import shared_memory
from multiprocessing.resource_tracker import unregister
from os.path import exists
import numpy as np
from bindings.capmix import capmix, Type, Value
from queue import Queue

class ShmMonitors:
	def __init__(self, data):
		self.data = data

	@classmethod
	def ord(cls, k):
		return (ord(k) - 1) % 4

	def __getitem__(self, k):
		return self.data[self.ord(k)]

	def __setitem__(self, k, v):
		self.data[self.ord(k)] = v

class ShmStereoLink:
	def __init__(self, data):
		self.data = data

	def __getitem__(self, k):
		return self.data[k-1][0]

	def __setitem__(self, k, v):
		self.data[k-1][0] = v

class ShmChannels:
	def __init__(self, data):
		self.data = data
		self.monitors = [ ShmMonitors(data[c]) for c in range(16) ]

	def __getitem__(self, k):
		return self.monitors[k-1]

class Model:
	def __init__(self, create=True):
		self.num_channels = 16
		self.monitors = ['a','b','c','d']
		#self.stereo = { (ch+1): 0 for ch in range(0,16) }
		#self.mutes  = { (ch+1): { mon: 0 for mon in self.monitors } for ch in range(0, self.num_channels) }
		#self.solos  = { (ch+1): { mon: 0 for mon in self.monitors } for ch in range(0, self.num_channels) }
		#self.pans   = { (ch+1): Value.parse(Type.Pan, "C") for ch in range(0, 16) }

		shm_path = 'pimix'
		if create:
			create = not exists('/dev/shm/' + shm_path)
		if create:
			shm = shared_memory.SharedMemory(shm_path, create=True, size=16*self.num_channels)
		else:
			shm = shared_memory.SharedMemory(shm_path, create=False)
		unregister(shm._name, 'shared_memory')

		self.data = np.ndarray(shape=(16,4,4), dtype=np.int8, buffer=shm.buf)
		self.stereo = ShmStereoLink( np.ndarray(shape=(16,4), dtype=np.int8, buffer=shm.buf[0:]) )
		self.mutes  = ShmChannels(   np.ndarray(shape=(16,4), dtype=np.int8, buffer=shm.buf[64:]) )
		self.solos  = ShmChannels(   np.ndarray(shape=(16,4), dtype=np.int8, buffer=shm.buf[128:]) )
		self.pans   = ShmChannels(   np.ndarray(shape=(16,4), dtype=np.int8, buffer=shm.buf[192:]) )
		self.shm = shm

		self.capture_hash = {}
		self.cache = {}
		self.queue = Queue()

	@classmethod
	def pan_to_int(cls, value):
		return int(str(value).replace('R','').replace('L','-').replace('C','0'))

	def close(self):
		print("Closing shared memory")
		self.shm.close()
		print("Closed shared memory")

	def reset(self):
		self.data.fill(0)
		for c in range(0, self.num_channels):
			for m in self.monitors:
				self.mutes[c+1][m] = 0
				self.solos[c+1][m] = 0
				self.stereo[c+1] = 0

	def mix(self, param, value):
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

