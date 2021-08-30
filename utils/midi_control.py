#!/usr/bin/env python3

import os
import sys
import time
from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from roland import render_bytes, Roland, Capture, CaptureView, Memory
from controller import Controller

usage = """Usage: %s [-l|-p <port_name>] [-g|-t]
Roland STUDIO-CAPTURE control

  -g, --graphical       launches in graphical mode
  -l, --list            lists the available MIDI ports
  -p, --port port_name  uses port_name instead of the default
  -t, --terminal        launches in terminal mode""" % sys.argv[0]

def get_mixer_port(api, port_name):
	found = None
	for port, name in enumerate(api.get_ports()):
		if port_name in name:
			found = port
			break
	return found

class Listener(object):
	def __init__(self, app):
		self.app = app
		self.addr_listeners = {}

	def __call__(self, event, data=None):
		message, deltatime = event
		#self.app.debug("< " + render_bytes(message))

		addr, data = Roland.parse_sysex(message)
		self.app.mixer.memory.set(addr, data)
		name = self.app.capture_view.lookup_name(addr)
		value = self.app.mixer.memory.get_formatted(addr)
		self.app.debug("0x%08x=%s; %s %s" % (addr, render_bytes(data), name, value))

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
	capture_view = CaptureView.instance()

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
		if self.midi_out:
			self.debug("> " + render_bytes(message))
			self.midi_out.send_message(message)
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
		if self.port is None:
			self.port = "STUDIO-CAPTURE:STUDIO-CAPTURE MIDI 2"

		apis = get_compiled_api()

		midi_in = None
		midi_out = None
		try:
			api_out = MidiOut()
			api_in = MidiIn()

			port_out = get_mixer_port(api_out, self.port)
			midi_out, port_name = open_midioutput(port_out, interactive=False)
			self.debug("Opened %s for output" % port_name)

			api_in.ignore_types(sysex=False)
			port_in  = get_mixer_port(api_in, self.port)
			midi_in, port_name = open_midiinput(port_in, interactive=False)
			self.debug("Opened %s for input" % port_name)
			midi_in.ignore_types(sysex=False)
			midi_in.set_callback(self.listener)
		except:
			message = "Unable to open MIDI ports"
			if self.interactive:
				self.debug(message)
			else:
				print(message)
				sys.exit(2)

		self.midi_in = midi_in
		self.midi_out = midi_out
		self.api_in = api_in
		self.api_out = api_out

	def setup_interface(self):
		self.mixer = None
		self.interface = None
		if self.interactive:
			if self.graphical:
				try:
					from gui import GraphicalMixer, MainGraphical
				except:
					self.debug("Unable to setup graphical interface, falling back to terminal")
					self.graphical = False
					return False
				self.mixer = GraphicalMixer()
				self.interface = MainGraphical(self.controller, self.mixer)
			else:
				from tui import TerminalMixer,  MainTerminal
				self.mixer = TerminalMixer()
				self.interface = MainTerminal(self.controller, self.mixer)
		else: # cli
			from cli import MainCli
			from mixer import Mixer
			self.mixer = Mixer()
			self.interface = MainCli(self.controller, self.mixer)
		return True

	def main(self):
		self.setup_interface() or self.setup_interface()
		self.listener = Listener(self)

		self.setup_midi()
		self.interface.refresh()
		self.load_mixer_values()
		self.interface.present()
		self.cleanup()

	def list_midi(self):
		midi_in = None
		midi_out = None
		try:
			api_out = MidiOut(API_LINUX_ALSA)
			api_in = MidiIn(API_LINUX_ALSA)
			ports = api_in.get_ports()
			print("Available MIDI ports use with (-p)")
			for p in ports:
				print("  \"%s\"" % p)
		except:
			print("Unable to open MIDI ports")
		return

	def list_controls(self):
		for control in Capture.memory_names():
			if control.count('.') < 1: continue
			print(control)

	def get(self, control):
		self.setup_midi()
		self.get_mixer_value(control)
		self.cleanup()

	def cleanup(self):
		if self.midi_in : self.midi_in.close_port()
		if self.midi_out: self.midi_out.close_port()
		del self.midi_in, self.api_in, self.midi_out, self.api_out

	def on_keyboard(self, key):
		return self.interface.on_keyboard(key)

	def set_page(self, page):
		self.mixer.set_page(page)
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

def main(argv):
	if '-h' in argv:
		print(usage)
		return

	app = App()

	if '-g' in argv:
		app.graphical = True
		app.interactive = True
	if '-t' in argv:
		app.interactive = True
		app.graphical = False
	if app.interactive and app.graphical is None:
		app.graphical = len(os.environ.get('DISPLAY', "")) > 0
	
	if '-l' in argv:
		app.interactive = False
		return app.list_midi()

	if '-c' in argv:
		return app.list_controls()

	if '-p' in argv:
		_p = argv.index('-p')
		if _p and _p + 1 < len(argv):
			app.port = argv[_p+1]
			
	if '-v' in argv:
		app.verbose = True

	if argv[-1] in Capture.memory_names():
		app.interactive = False
		control = argv[-1]
		return app.get(control)

	app.main()

if __name__ == '__main__':
	main(sys.argv)

