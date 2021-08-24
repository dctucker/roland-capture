#!/usr/bin/env python3

import sys
import time
from term import Term
from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from roland import render_bytes, Roland, Capture, CaptureView, Memory

mixer_port = "STUDIO-CAPTURE:STUDIO-CAPTURE MIDI 2"

def get_mixer_port(api):
	found = None
	for port, name in enumerate(api.get_ports()):
		if mixer_port in name:
			found = port
			break
	return found

class Cursor(object):
	def __init__(self):
		self.x = 0
		self.y = 0
	def yx(self):
		return self.y, self.x

class Mixer(object):
	def __init__(self):
		#self.volume = [ [ None for i in range(0, 16) ] for m in range(0, 4) ]
		self.memory = Memory()
		self.capture_view = CaptureView.instance()
		self.setup_controls("monitor.a")
		self.cursor = Cursor()

	def setup_controls(self, mode):
		self.controls = []
		row = []
		for ch in range(0,16,2):
			row += [
				"input %s channel.%d channel.stereo" % (mode, ch+1)
			]
		self.controls += [row]
		for control in "mute", "solo", "reverb", "pan", "volume":
			row = []
			for ch in range(0, 16):
				desc = "input %s channel.%d channel.%s" % (mode, ch+1, control)
				row += [desc]
			self.controls += [row]

	def height(self):
		return len(self.controls)

	def width(self):
		return len(self.controls[self.cursor.y])

	def render(self):
		ret = ""
		selected_control = ""
		spacing = 5
		for ch in range(0, 16):
			ret += ("%d" % (ch+1)).center(spacing)
		ret += "\n"
		for r, row in enumerate(self.controls):
			w = spacing*int(16/len(row))
			for c, control in enumerate(row):
				value = self.memory.get_formatted( Capture.get_addr(control) )
				if self.cursor.x == c and self.cursor.y == r:
					ret += "\033[7m"
					selected_control = control
				else:
					ret += "\033[0m"
				ret += value.center(w)
			ret += "\033[0m\n"

		ret += "\033[2K%s\n" % selected_control
		return ret

	def decrement_selected(self):
		row = self.cursor.y
		col = self.cursor.x
		addr = Capture.get_addr(self.controls[row][col])
		data = self.memory.decrement(addr)
		return addr, data

	def increment_selected(self):
		row = self.cursor.y
		col = self.cursor.x
		addr = Capture.get_addr(self.controls[row][col])
		data = self.memory.increment(addr)
		return addr, data

	#def set_volume(self, addr, value):
	#	ch = (addr & 0x0f00) >> 8
	#	m = (addr & 0xf000) >> 12
	#	#self.volume[m][ch] = value

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
		message = Roland.send_data(addr, data)
		self.send(message)

	def send(self, message):
		self.midi_out.send_message(message)
		self.debug("> " + render_bytes(message))
		return message

	def main(self):
		apis = get_compiled_api()

		api_out = MidiOut(API_LINUX_ALSA)
		port_out = get_mixer_port(api_out)
		midi_out, port_name = open_midioutput(port_out, interactive=False)
		self.debug("Opened %s for output" % port_name)

		api_in = MidiIn(API_LINUX_ALSA)
		api_in.ignore_types(sysex=False)
		port_in  = get_mixer_port(api_in)
		midi_in, port_name = open_midiinput(port_in, interactive=False)
		self.debug("Opened %s for input" % port_name)
		midi_in.ignore_types(sysex=False)
		midi_in.set_callback(self.listener)

		self.midi_in = midi_in
		self.midi_out = midi_out

		for row in self.mixer.controls:
			for control in row:
				self.get_mixer_value(control)#, self.mixer.set_volume)

		if not hasattr(sys, 'ps1'): # non-interactive mode only
			try:
				while True:
					key = self.term.getch()
					if key in Term.KEY_DOWN:
						if self.mixer.cursor.y + 1 < self.mixer.height():
							self.mixer.cursor.y += 1
						self.display()
					elif key in Term.KEY_UP:
						if self.mixer.cursor.y > 0:
							self.mixer.cursor.y -= 1
						self.display()
					elif key in Term.KEY_LEFT:
						if self.mixer.cursor.x > 0:
							self.mixer.cursor.x -= 1
						self.display()
					elif key in Term.KEY_RIGHT:
						if self.mixer.cursor.x + 1 < self.mixer.width():
							self.mixer.cursor.x += 1
						self.display()
					elif key in ('-','_'):
						addr, data = self.mixer.decrement_selected()
						self.set_mixer_value(addr, data)
						self.display()
					elif key in ('=','+'):
						addr, data = self.mixer.increment_selected()
						self.set_mixer_value(addr, data)
						self.display()
					elif key in ('q',"\033"):
						break
			except KeyboardInterrupt:
				print('')
			finally:
				print("Exit.")
				midi_in.close_port()
				midi_out.close_port()
				del midi_in, api_in, midi_out, api_out

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

