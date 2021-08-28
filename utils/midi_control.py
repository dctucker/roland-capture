#!/usr/bin/env python3

import os
import time
from term import Term
from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from roland import render_bytes, Roland, Capture, CaptureView, Memory
from mixer import TerminalMixer, GraphicalMixer
from tui import MainTerminal
from controller import Controller

mixer_port = "STUDIO-CAPTURE:STUDIO-CAPTURE MIDI 2"

def get_mixer_port(api):
	found = None
	for port, name in enumerate(api.get_ports()):
		if mixer_port in name:
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

		self.app.interface.update(False)

	def register_addr(self, addr, handler):
		self.addr_listeners[addr] = handler

	def dispatch(self, addr, value):
		if addr in self.addr_listeners:
			self.addr_listeners[addr](addr, value)
			del self.addr_listeners[addr]

class App(object):
	capture_view = CaptureView.instance()

	def __init__(self):
		self.debug_string = ""
		self.height = 12
		self.term = Term()
		self.mixer = TerminalMixer()
		self.controller = Controller(self)
		self.interface = MainTerminal(self.controller, self.term)
		self.listener = Listener(self)
		self.interface.refresh()

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
		self.term.blocked = True
		i = 0
		for row in self.mixer.controls:
			for control in row:
				if control is None: continue
				if i % 8 == 0:
					time.sleep(0.02)
				self.get_mixer_value(control)
				i += 1
		self.term.blocked = False

	def setup_midi(self):
		apis = get_compiled_api()

		midi_in = None
		midi_out = None
		try:
			api_out = MidiOut(API_LINUX_ALSA)
			api_in = MidiIn(API_LINUX_ALSA)

			port_out = get_mixer_port(api_out)
			midi_out, port_name = open_midioutput(port_out, interactive=False)
			self.debug("Opened %s for output" % port_name)

			api_in.ignore_types(sysex=False)
			port_in  = get_mixer_port(api_in)
			midi_in, port_name = open_midiinput(port_in, interactive=False)
			self.debug("Opened %s for input" % port_name)
			midi_in.ignore_types(sysex=False)
			midi_in.set_callback(self.listener)
		except:
			print("Unable to open MIDI ports")

		self.midi_in = midi_in
		self.midi_out = midi_out
		self.api_in = api_in
		self.api_out = api_out

	def main(self):
		self.setup_midi()
		self.load_mixer_values()
		self.interface.present(self)

	def on_keyboard(self, key):
		return self.interface.on_keyboard(key)

	def set_page(self, page):
		self.mixer.set_page(page)
		self.load_mixer_values()
		self.interface.refresh()

	def debug(self, message, end="\n"):
		self.debug_string += Term.CLEAR_LINE + message + end
		#self.height += message.count("\n") + 1


	#def demo(self):
	#	self.send(Capture.get_volume(0))           #message = Roland.receive_data(0x00060008, 3)
	#	self.send(Capture.set_volume(0, 0x200000)) #Roland.send_data(0x00060008, [2,0,0,0,0,0])

if __name__ == '__main__':
	App().main()

