from lib.roland import CaptureView
from lib.types import ValueFactory

class Memory(object):
	def __init__(self):
		self.memory = {}
		self.capture_view = CaptureView.instance()

	def get(self, addr):
		if addr not in self.memory: return None
		return self.memory[addr]

	def set(self, addr, value):
		self.memory[addr] = value
	
	def erase(self, addr):
		if addr in self.memory:
			del self.memory[addr]

	def get_value(self, addr):
		name = self.addr_name(addr)
		data = self.get(addr)
		return ValueFactory.from_packed(name, data)

	def set_value(self, addr, value):
		return self.set_long(addr, value.value)

	def get_long(self, addr):
		return self.get_value(addr).value

	def set_long(self, addr, long):
		name = self.addr_name(addr)
		value = ValueFactory.get_class(name)(long)
		data = value.pack()
		self.set(addr, data)
		return data

	def get_formatted(self, addr):
		name = self.addr_name(addr)
		value = self.get_value(addr)
		return self.capture_view.format_value(value, name)

	def increment(self, addr):
		data = self.get(addr)
		name = self.addr_name(addr)
		value = ValueFactory.from_packed(name, data)
		value.increment()
		data = value.pack()
		self.set(addr, data)
		return data

	def decrement(self, addr):
		data = self.get(addr)
		name = self.addr_name(addr)
		value = ValueFactory.from_packed(name, data)
		value.decrement()
		data = value.pack()
		self.set(addr, data)
		return data

	def zero(self, addr):
		name = self.addr_name(addr)
		value = ValueFactory.default_for(name)
		data = value.pack()
		self.set(addr, data)
		return data

	def addr_name(self, addr):
		return self.capture_view.lookup_name(addr)

