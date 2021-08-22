#!/usr/bin/env python3

import sys
import time
from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from roland import print_bytes, Roland, Capture, CaptureView

mixer_port = "STUDIO-CAPTURE:STUDIO-CAPTURE MIDI 2"

def get_mixer_port(api):
	found = None
	for port, name in enumerate(api.get_ports()):
		if mixer_port in name:
			found = port
			break
	return found

capture_view = CaptureView()

class Mixer(object):
	def __init__(self):
		self.volume = [ [ None for i in range(0, 16) ] for m in range(0, 4) ]
	def set_volume(self, addr, value):
		ch = (addr & 0x0f00) >> 8
		m = (addr & 0xf000) >> 12
		self.volume[m][ch] = value
		#print("set volume %s %s" % (capture_view.lookup_name(addr), value))
		self.print_volumes()
	def print_volumes(self):
		for m, monitor in enumerate(self.volume):
			print("Monitor %s" % ['A','B','C','D'][m])
			for volume in monitor:
				print(volume, end="\t")
			print('')


class App(object):
	def get_mixer_value(self, desc, handler):
		addr = Capture.get_addr(desc)
		size = Capture.get_size(desc)
		message = Roland.receive_data(addr, size)
		self.listener.register_addr(addr, handler)
		return self.send(message)

	def send(self, message):
		print(">", end=" ")
		self.midi_out.send_message(message)
		#print(message)
		print_bytes(message)
		return message

	def main(self):
		self.listener = Listener()
		self.mixer = Mixer()
		apis = get_compiled_api()

		api_out = MidiOut(API_LINUX_ALSA)
		port_out = get_mixer_port(api_out)
		midi_out, port_name = open_midioutput(port_out)
		print("Opened %s for output" % port_name)

		api_in = MidiIn(API_LINUX_ALSA)
		api_in.ignore_types(sysex=False)
		port_in  = get_mixer_port(api_in)
		midi_in, port_name = open_midiinput(port_in)
		print("Opened %s for input" % port_name)
		midi_in.ignore_types(sysex=False)
		midi_in.set_callback(self.listener)

		self.midi_in = midi_in
		self.midi_out = midi_out

		for m in 'a','b','c','d':
			for i in range(1, 1+16):
				desc = "input monitor.%s channel.%d channel.volume" % (m, i)
				value = self.get_mixer_value(desc, self.mixer.set_volume)
				#print(desc, value)

		if not hasattr(sys, 'ps1'): # non-interactive mode only
			try:
				while True:
					time.sleep(1)
			except KeyboardInterrupt:
				print('')
			finally:
				print("Exit.")
				midi_in.close_port()
				midi_out.close_port()
				del midi_in, api_in, midi_out, api_out

	#def demo(self):
	#	self.send(Capture.get_volume(0))           #message = Roland.receive_data(0x00060008, 3)
	#	self.send(Capture.set_volume(0, 0x200000)) #Roland.send_data(0x00060008, [2,0,0,0,0,0])

class Listener(object):
	def __init__(self):
		self.addr_listeners = {}

	def __call__(self, event, data=None):
		message, deltatime = event
		print("<", end=" ")
		print_bytes(message)
		addr, data = Roland.parse_sysex(message)
		name = capture_view.lookup_name(addr)
		value = capture_view.format_value(name, data)
		print(addr, name, value)

		self.dispatch(addr, value)

	def register_addr(self, addr, handler):
		self.addr_listeners[addr] = handler

	def dispatch(self, addr, value):
		if addr in self.addr_listeners:
			self.addr_listeners[addr](addr, value)
			del self.addr_listeners[addr]





if __name__ == '__main__':
	App().main()

