import unittest
from roland import Capture
from mixer import Mixer

class TestCapture(unittest.TestCase):
	def test_memory_lookup(self):
		self.assertEqual(0x00061208, Capture.get_addr("input_monitor.b.channel.3.volume"))
		self.assertEqual(0x0006230e, Capture.get_addr("input_monitor.c.channel.4.reverb"))
		with self.assertRaises(Exception) as ctx:
			Capture.get_addr("daw_monitor.c.channel.4.reverb")
		self.assertEqual(0x00050b07, Capture.get_addr("preamp.channel.12.gate"))
		self.assertEqual(0x00050004, Capture.get_addr("preamp.channel.1.hi-z"))
		self.assertEqual(0x00050104, Capture.get_addr("preamp.channel.2.hi-z"))
		with self.assertRaises(Exception) as ctx:
			Capture.get_addr("preamp.channel.3.hi-z")

		self.assertEqual(0x00050104, Capture.get_addr("preamp.channel.2.hi-z"))
		self.assertEqual(0x00030001, Capture.get_addr("patchbay.3-4"))
		self.assertEqual(0x00040000, Capture.get_addr("reverb.type"))
		self.assertEqual(0x00040101, Capture.get_addr("reverb.echo.pre_delay"))

	def test_mixer(self):
		Mixer()

	def test_all_memory(self):
		names = Capture.memory_names()
		self.assertIsInstance(names, list)
		{ name:Capture.get_addr(name) for name in names }
