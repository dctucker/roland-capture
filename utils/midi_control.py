#!/usr/bin/env python3

import sys
import time
from term import Term
from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from roland import render_bytes, Roland, Capture, CaptureView, Memory
from mixer import Mixer

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
		self.app.debug("< " + render_bytes(message))
		addr, data = Roland.parse_sysex(message)
		self.app.mixer.memory.set(addr, data)
		name = self.app.capture_view.lookup_name(addr)
		value = self.app.mixer.memory.get_formatted(addr)

		self.app.display()
		self.app.debug("%s %s %s\n" % (addr, name, value))

		self.app.mixer.memory.set(addr, data)
		self.dispatch(addr, value)

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
		self.mixer = Mixer()
		self.listener = Listener(self)
		self.term.clear()

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
			self.midi_out.send_message(message)
			self.debug("> " + render_bytes(message))
			return message

	def load_mixer_values(self):
		for row in self.mixer.controls:
			for control in row:
				if control is None: continue
				self.get_mixer_value(control)

	def main(self):
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

		self.load_mixer_values()

		if not hasattr(sys, 'ps1'): # non-interactive mode only
			try:
				while True:
					key = self.term.getch()
					if self.on_keyboard(key):
						self.display()
					elif key in ('q',"\033"):
						break
			except KeyboardInterrupt:
				print('')
			finally:
				print("Exit.")
				if self.midi_in: self.midi_in.close_port()
				if self.midi_out: self.midi_out.close_port()
				del midi_in, api_in, midi_out, api_out

	def on_keyboard(self, key):
		if key in Term.KEY_DOWN:
			self.mixer.cursor_down()
		elif key in Term.KEY_UP:
			self.mixer.cursor_up()
		elif key in Term.KEY_LEFT:
			self.mixer.cursor_left()
		elif key in Term.KEY_RIGHT:
			self.mixer.cursor_right()
		elif key in ('-','_'):
			addr, data = self.mixer.decrement_selected()
			self.set_mixer_value(addr, data)
		elif key in ('=','+'):
			addr, data = self.mixer.increment_selected()
			self.set_mixer_value(addr, data)
		elif key in ('p','\033[Z'):
			self.mixer.page = "preamp"
			self.mixer.setup_controls()
			self.load_mixer_values()
		elif key in ('\t',):
			if 'input' in self.mixer.page:
				self.mixer.set_page("daw_monitor monitor." + self.mixer.monitor)
			else:
				self.mixer.set_page("input monitor." + self.mixer.monitor)
			self.load_mixer_values()
		elif key in ('[',):
			if self.mixer.monitor > 'a':
				self.mixer.set_monitor(chr(ord(self.mixer.monitor)-1))
				self.load_mixer_values()
		elif key in (']',):
			if self.mixer.monitor < 'd':
				self.mixer.set_monitor(chr(ord(self.mixer.monitor)+1))
				self.load_mixer_values()
		else:
			return False
		return True

	def display(self):
		#self.height = 12
		rendered = self.mixer.render()
		#rendered = ""
		#self.height = rendered.count("\n")
		self.term.display(rendered + '\n\033['+str(self.height)+';6H\n' + self.debug_string)
		self.debug_string = ""
	
	def debug(self, message):
		self.debug_string = message + "\n"
		#self.height += message.count("\n") + 1


	#def demo(self):
	#	self.send(Capture.get_volume(0))           #message = Roland.receive_data(0x00060008, 3)
	#	self.send(Capture.set_volume(0, 0x200000)) #Roland.send_data(0x00060008, [2,0,0,0,0,0])

if __name__ == '__main__':
	App().main()

