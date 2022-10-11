import sys
import time
from lib.roland import render_bytes, Roland, Capture
from app.controller import Controller
from app.comm import *

class Listener(object):
	def __init__(self, app):
		self.app = app
		self.addr_listeners = {}

	def __call__(self, event, data=None):
		message, deltatime = event
		#self.app.debug("RECV " + render_bytes(message))

		addr, data = Roland.parse_sysex(message)
		self.app.mixer.memory.set(addr, data)
		name = self.app.mixer.memory.addr_name(addr)
		value = self.app.mixer.memory.get_formatted(addr)
		self.app.debug("listener hears 0x%08x=%s; %s %s" % (addr, render_bytes(data), name, value))

		self.app.mixer.memory.set(addr, data)
		self.dispatch(addr, value)
		self.app.interface.notify_control(name)

	def register_addr(self, addr, handler):
		self.addr_listeners[addr] = handler

	def dispatch(self, addr, value):
		if addr in self.addr_listeners:
			self.addr_listeners[addr](addr, value)
			del self.addr_listeners[addr]

class App(object):
	def __init__(self):
		self.port = None
		self.interactive = True
		self.graphical = None
		self.verbose = False
		self.controller = Controller(self)

	def quit(self):
		self.interface.quit()

	def get_mixer_value(self, desc, handler=None):
		addr = Capture.get_addr(desc)
		size = Capture.get_size(desc)
		message = Roland.receive_data(addr, size)
		if handler is not None:
			self.listener.register_addr(addr, handler)
		return self.send(message)

	def set_mixer_value(self, addr, data):
		if addr is None or data is None:
			return
		message = Roland.send_data(addr, data)
		self.send(message)

	def send(self, message):
		if self.midi.output:
			self.debug("SEND " + render_bytes(message))
			self.midi.output.send_message(message)
			return message

	def load_mixer_values(self):
		self.interface.block()
		i = 0
		for row in self.mixer.controls:
			for control in row:
				if control is None: continue
				if i % 8 == 0:
					time.sleep(0.02)
				self.get_mixer_value(control)
				i += 1
		self.interface.unblock()

	def setup_midi(self):
		self.listener = Listener(self)
		if self.port is None:
			self.port = "STUDIO-CAPTURE:STUDIO-CAPTURE MIDI 2"

		midi = MIDI(self.port)
		midi.listener = self.listener
		port_name = midi.setup_output()
		if port_name:
			self.debug("Opened %s for output" % port_name)

		port_name =  midi.setup_input()
		if port_name:
			self.debug("Opened %s for input" % port_name)

		if midi.input is None or midi.output is None:
			message = "Unable to open MIDI ports"
			if not self.interactive:
				print(message)
				sys.exit(2)
			else:
				self.debug(message)

		self.midi = midi

	def setup_interface(self):
		self.mixer = None
		self.interface = None
		if self.interactive:
			if self.graphical:
				#try:
				from ui.gui import GraphicalMixer, MainGraphical
				#except:
				#	self.debug("Unable to setup graphical interface, falling back to terminal")
				#	self.graphical = False
				#	return False
				self.mixer = GraphicalMixer()
				self.interface = MainGraphical(self.controller, self.mixer)
			else:
				from ui.tui import TerminalMixer,  MainTerminal
				self.mixer = TerminalMixer()
				self.interface = MainTerminal(self.controller, self.mixer)
		else: # cli
			from ui.cli import MainCli
			from lib.mixer import Mixer
			self.mixer = Mixer()
			self.interface = MainCli(self.controller, self.mixer)
		return True

	def memory_names(self):
		return Capture.memory_names()

	def main(self):
		self.setup_interface() or self.setup_interface()
		self.setup_midi()

		self.interface.refresh()
		self.load_mixer_values()
		self.interface.present()
		self.cleanup()

	def list_midi(self):
		ports = MIDI.list_ports()
		if ports:
			print("Available MIDI ports use with (-p)")
			for p in ports:
				print("  \"%s\"" % p)
		else:
			print("No MIDI ports available")

	def list_controls(self):
		for control in Capture.memory_names():
			if control.count('.') < 1: continue
			print(control)

	def get(self, control):
		self.setup_interface()
		self.setup_midi()

		self.get_mixer_value(control)
		value = self.interface.wait_for_value(control)
		print(value.format())
		self.cleanup()

	def set(self, control, new_value):
		self.setup_interface()
		self.setup_midi()

		addr = Capture.get_addr(control)
		value = self.interface.parse_value(control, new_value)
		if value is None:
			print("Unrecognized value for %s: %s" % (control.split('.')[-1], new_value))
		else:
			self.set_mixer_value(addr, value.pack())
			value = self.interface.wait_for_value(control)
			print(value.format())
		self.cleanup()


	def cleanup(self):
		self.midi.cleanup()

	def on_keyboard(self, key):
		return self.interface.on_keyboard(key)

	def set_page(self, page):
		self.interface.block()
		self.mixer.set_page(page)
		self.interface.unblock()
		self.load_mixer_values()
		self.interface.refresh()

	def set_monitor(self, monitor):
		self.mixer.set_monitor(monitor)
		self.load_mixer_values()
		self.interface.refresh()

	def set_channel(self, ch):
		self.mixer.set_channel(ch)
		self.load_mixer_values()
		self.interface.refresh()

	def debug(self, message, end="\n"):
		if self.verbose:
			self.interface.debug(message, end)

