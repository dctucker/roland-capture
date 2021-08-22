import unittest
from roland import Capture

class TestCapture(unittest.TestCase):
	def test_address_map(self):
		self.assertEqual(0x00061208, Capture.get_addr("input monitor.b channel.3 channel.volume"))
		self.assertEqual(0x0007230e, Capture.get_addr("daw_monitor monitor.c channel.4 channel.reverb"))
		self.assertEqual(0x00050b07, Capture.get_addr("preamp channel.12 preamp.gate"))
		self.assertEqual(0x00050b07, Capture.get_addr("preamp preamp.12 preamp.gate"))
