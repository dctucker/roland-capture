from rtmidi import (API_LINUX_ALSA, API_MACOSX_CORE, API_RTMIDI_DUMMY,
		API_UNIX_JACK, API_WINDOWS_MM, MidiIn, MidiOut,
		get_compiled_api)
from rtmidi.midiutil import open_midiinput, open_midioutput
from rtmidi._rtmidi import NoDevicesError, InvalidPortError

class MIDI:
	def __init__(self, port):
		self.port = port
		self.apis = get_compiled_api()
		self.output = None
		self.input = None
		self.api_out = None
		self.api_in = None

	def get_mixer_port(self, api):
		found = None
		for port, name in enumerate(api.get_ports()):
			if self.port in name:
				found = port
				break
		return found

	def setup_output(self):
		api_out = MidiOut()
		port_out = self.get_mixer_port(api_out)
		try:
			midi_out, port_name = open_midioutput(port_out, interactive=False)
		except InvalidPortError:
			return
		except NoDevicesError:
			return
		
		self.api_out = api_out
		self.output = midi_out
		return port_name

	def setup_input(self):
		api_in = MidiIn()
		api_in.ignore_types(sysex=False)
		port_in  = self.get_mixer_port(api_in)
		try:
			midi_in, port_name = open_midiinput(port_in, interactive=False)
		except InvalidPortError:
			return
		except NoDevicesError:
			return

		midi_in.ignore_types(sysex=False)
		midi_in.set_callback(self.listener)

		self.api_in = api_in
		self.input = midi_in
		return port_name

	def cleanup(self):
		if self.input :
			self.input.close_port()
		if self.output:
			self.output.close_port()
		del self.input, self.api_in, self.output, self.api_out

	@staticmethod
	def list_ports():
		midi_in = None
		midi_out = None
		try:
			api_out = MidiOut()
			api_in = MidiIn()
			ports = api_in.get_ports()
			return ports
		except:
			print("Unable to open MIDI ports")
		return
