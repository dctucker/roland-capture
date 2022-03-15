import unittest
from math import inf
from lib.memory import Memory
from lib.types import Volume

mem = Memory()

class TestMemory(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		cls.mem = mem
		cls.mem.capture_view.add_name_to_table('input_monitor.a.channel.1.volume')

	def test_memory_get_empty(self):
		self.assertEqual(None, self.mem.get(0))

	def test_memory_set_get_erase(self):
		self.mem.set(0, 1)
		self.assertEqual(mem.get(0), 1)
		self.mem.set(1, 2)
		self.assertEqual(mem.get(0), 1)
		self.assertEqual(mem.get(1), 2)
		self.mem.set(0, 3)
		self.assertEqual(mem.get(0), 3)
		self.assertEqual(mem.get(1), 2)

		self.mem.erase(0)
		self.mem.erase(1)
		self.assertIsNone(self.mem.get(0))
		self.assertIsNone(self.mem.get(1))

	def test_get_value(self):
		addr = 0x00060008
		data = [0x02,0,0,0,0,0]
		self.mem.set(addr, data)
		value = self.mem.get_value(addr)
		self.assertIsInstance(value, Volume)
		self.assertEqual(value.pack(), data)
		self.assertEqual(value.value, 0.0)

	def test_set_value(self):
		addr = 0x00060008
		self.mem.set_value(addr, Volume(0.0))
		got = self.mem.get(addr)
		self.assertEqual(got, [0x02,0,0,0,0,0])

	def test_get_long(self):
		addr = 0x00060008
		self.mem.set_value(addr, Volume(0.0))
		got = self.mem.get_long(addr)
		self.assertEqual(got, 0.0)

	def test_set_long(self):
		addr = 0x00060008
		self.mem.set_long(addr, 0.0)
		got = self.mem.get_value(addr)
		self.assertEqual(got, Volume(0.0))

	def test_get_formatted(self):
		addr = 0x00060008
		self.mem.set_long(addr, 0.0)
		got = self.mem.get_formatted(addr)
		self.assertEqual(got, "+0")
	
	def test_increment(self):
		addr = 0x00060008
		self.mem.set_long(addr, 0.0)
		self.mem.increment(addr)
		got = self.mem.get_formatted(addr)
		self.assertEqual(got, "+1")

	def test_decrement(self):
		addr = 0x00060008
		self.mem.set_long(addr, 0.0)
		self.mem.decrement(addr)
		got = self.mem.get_formatted(addr)
		self.assertEqual(got, "-1")

	def test_zero(self):
		addr = 0x00060008
		self.mem.set_long(addr, -inf)
		self.mem.zero(addr)
		got = self.mem.get_formatted(addr)
		self.assertEqual(got, "+0")

