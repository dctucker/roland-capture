#!bridge/bin/python3

import os

from model import Model
from view import View, log
from control import Control
from capture import Capture

from queue import Empty
from threading import Thread
import alsa_midi
from bindings.capmix import capmix, EVENT, Value, Type, UNPACKED
import time

from alsa_midi import SequencerClient

first_attempt = True
def main():
	global first_attempt

	model = Model()
	view = View(model)

	capture = Capture()
	capture.model = model
	capture.view = view

	control = Control()
	control.model = model
	control.view = view

	# setup MIDI I/O
	client = SequencerClient("midipi")
	client.set_output_buffer_size(2048)
	client.set_input_buffer_size(2048)
	port = client.create_port("io")

	def check_connections(client, port):
		global first_attempt
		client_ports = client.list_ports()

		control_found = False
		capture_found = False
		for p in client_ports:
			if p.name == control.name:
				control_found = True
				control.ok = True
			if p.name == capture.name:
				capture_found = True

		if not control_found:
			control.ok = False
			log("Control disconnected")

		if not capture_found:
			if capture.ok or first_attempt:
				capture.ok = False
				view.dim(True)
			log("STUDIO-CAPTURE disconnected")

		if capture_found and not capture.ok:
			log("STUDIO-CAPTURE connected")
			capture.ok = capture.connect()
			view.dim(False)
			control.hello()

		control.ping()
		first_attempt = False

		return (control.ok, capture.ok)

	last_ok = 0
	last_attempt = 0
	capture.ok = False

	try:
		while True:
			now = time.time()

			# attempt to reconnect
			if not ( control.ok and capture.ok ):
				if now - last_attempt > 5:
					if not control.ok:
						control.ok = control.connect(client, port)
					if not capture.ok:
						capture.ok = capture.connect()
					last_attempt = now
			if capture.ok:
				x = capture.listen()

			if control.ok:
				control.listen()
				control.sync()

			# check for disconnect
			if capture.ok or control.ok:
				if now - last_ok > 5:
					control.ok, capture.ok = check_connections(client, port)
					last_ok = now

			model.flush()

			if view.logged:
				view.print_monitor_mutes()

	except KeyboardInterrupt:
		pass

	capture.disconnect()
	view.cursor_release()

if __name__ == '__main__':
	main()
