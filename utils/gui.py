#!/usr/bin/env python3

import sys

from mixer import Mixer
from roland import ValueFactory, Bool, Capture, Volume, Enum
from qt import *

class GraphicalMixer(Mixer):
	page_titles = {
		'input_monitor.a': 'Input\nA',
		'input_monitor.b': 'Input\nB',
		'input_monitor.c': 'Input\nC',
		'input_monitor.d': 'Input\nD',
		'daw_monitor.a': 'Output\nA',
		'daw_monitor.b': 'Output\nB',
		'daw_monitor.c': 'Output\nC',
		'daw_monitor.d': 'Output\nD',
	}
	def page_title(self, page_name):
		if page_name in self.page_titles:
			return self.page_titles[page_name]
		return page_name.title()

class MainGraphical(QApplication):
	def __init__(self, controller, mixer):
		QApplication.__init__(self, sys.argv)
		self.setWindowIcon(QIcon('./mixer-256.png'))
		self.controller = controller
		self.controller.cursors = (
			(Qt.Key_Up,),
			(Qt.Key_Down,),
			(Qt.Key_Left,),
			(Qt.Key_Right,),
		)
		self.controller.setup_keyboard_map()
		self.controller.keyboard_map[Qt.Key_Tab,] = self.controller.keyboard_map['\t',]
		self.mixer = mixer

		self.controller = controller
		self.setWheelScrollLines(1)
		self.window = MainWindow(self.controller, self.mixer)

	def notify(self, receiver, event):
		if event.type() == QEvent.KeyPress:
			self.on_keyboard(event)
			return True
		return QApplication.notify(self, receiver, event)

	def present(self):
		self.window.show()
		self.refresh()
		self.update()
		return self.exec_()

	def block(self):
		self.window.block()
		self.window.setEnabled(False)

	def unblock(self):
		self.window.setEnabled(True)
		self.window.unblock()

	def on_keyboard(self, event):
		if self.controller.on_keyboard((event.text(), event.key())):
			self.refresh()
			self.update()

	def debug(self, message, end="\n"):
		print(message, end=end)

	def refresh(self):
		selected = self.mixer.page_name
		shown = self.window.current_page_name()
		if shown != selected:
			widget = self.window.pages[selected]
			self.window.tab.setCurrentWidget(widget)

	def update(self):
		self.window.update_focus()

	def notify_control(self, control):
		widget = self.window.get_control(control)
		if not widget: return

		addr = Capture.get_addr(control)
		value = self.mixer.memory.get_value(addr)

		self.debug("notify_control " + control + " to " + value.format())
		widget.block()
		widget.set_value(value)
		widget.unblock()
		widget.update_label(value.format())

	def quit(self):
		self.window.close()
		self.debug("Exit.")

class AppStub():
	def quit(self):
		self.interface.quit()

def main():
	from controller import NullController
	app = AppStub()
	mixer = GraphicalMixer()
	controller = NullController(app)
	interface = MainGraphical(controller, mixer)
	return interface.present()

if __name__ == '__main__':
	sys.exit(main())
