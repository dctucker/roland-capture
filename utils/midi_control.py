#!/usr/bin/env python3

import os
import sys
from app.app import App

def main(argv):
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('-g', '--graphical', action="store_true",                help='launch in graphical mode')
	parser.add_argument('-t', '--terminal',  action="store_true",                help='launch in terminal mode')
	parser.add_argument('-l', '--list',      action="store_true",                help='list the available MIDI ports')
	parser.add_argument('-c', '--controls',  action="store_true",                help='list the controllable parameter names')
	parser.add_argument('-p', '--port',      type=str,                           help='uses port_name instead of the default')
	parser.add_argument('-v', '--verbose',   action="store_true", default=False, help='increase debug output')
	parser.add_argument('-M', '--memory-map',action="store_true", default=False, help='list the memory addresses')
	parser.add_argument('control',           default=None, type=str, nargs='?',  help='control name')
	parser.add_argument('value',             default=None,           nargs='?',  help='value to set')
	args = parser.parse_args()

	app = App()

	if args.graphical:
		app.graphical = True
		app.interactive = True
	if args.terminal:
		app.interactive = True
		app.graphical = False
	if app.interactive and app.graphical is None:
		app.graphical = len(os.environ.get('DISPLAY', "")) > 0
	
	if args.list:
		app.interactive = False
		return app.list_midi()

	if args.controls:
		return app.list_controls()

	if args.memory_map:
		from app import docs
		return

	app.port = args.port
	app.verbose = args.verbose

	if args.control:
		app.interactive = False
		if args.control not in Capture.memory_names():
			print("Unknown control name: %s" % args.control)
			sys.exit(3)
		if args.value:
			return app.set(args.control, args.value)
		else:
			return app.get(args.control)

	app.main()

if __name__ == '__main__':
	main(sys.argv)

