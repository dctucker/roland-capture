#!/usr/bin/env python3

from PyQt5.QtGui import QPalette, QColor
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtWidgets import *
import sys

from math import inf
from mixer import Mixer
from roland import ValueFactory, Bool, Capture, Volume

with open('style.css') as file:
	css = file.read()

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

class Widget(QWidget):
	def __init__(self):
		QWidget.__init__(self)
	#	self.setFocusPolicy(Qt.NoFocus)
	#def keyPressEvent(self, event):
	#	QWidget.keyPressEvent(self, event)

class Control(Widget):
	def set_name(self, control):
		self.name = control
		self.setAccessibleName(self.name)
		self.setAttribute(Qt.WA_StyledBackground, True)
	def set_range(self, min_, max_):
		t = type(self)
		if t is VolSlider or t is Knob:
			if min_ == -inf:
				min_ = -71
			self.setRange(min_, max_)

	def set_value(self, value):
		raise Exception("set_value not implemented for %s" % value.__class__)

	def update_label(self, value):
		pass

class Space(QLabel, Control):
	def __init__(self):
		QLabel.__init__(self, "       ")
		self.setContentsMargins(0,0,0,0)
		#self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)

	pass

class VolSlider(Control):
	def __init__(self):
		Widget.__init__(self)
		layout = QVBoxLayout()

		slider = QSlider()
		slider.setTickPosition(QSlider.TicksBothSides)
		slider.setTickInterval(10)
		slider.setPageStep(1)
		slider.setSingleStep(1)
		slider.setMinimum(-71)
		slider.setMaximum(12)
		self.slider = slider

		self.label = QLabel("0.0", alignment=Qt.AlignHCenter)

		layout.addWidget(self.slider)
		layout.addWidget(self.label)

		self.valueChanged = self.slider.valueChanged
		self.setLayout(layout)

	def update_label(self, value):
		self.label.setText(str(value))

	def minimumSizeHint(self):
		return QSize(15, 120)

	def set_name(self, name):
		self.slider.name = name

	def setRange(self, min_, max_):
		self.slider.setRange(min_, max_)

	def set_value(self, value):
		if value.value == -inf:
			self.slider.setValue(-71)
		else:
			self.slider.setValue(value.value)

class ToggleButton(QPushButton, Control):
	def __init__(self, title=""):
		QPushButton.__init__(self)
		self.setText(title)
		self.setCheckable(True)
		self.sizePolicy().setVerticalStretch(1)
		self.sizePolicy().setHorizontalStretch(1)
		self.valueChanged = self.clicked
	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)
	def value(self):
		return self.isChecked()

	def set_value(self, value):
		#self.checked = value.value
		self.setChecked(value.value)

class Knob(QDial, Control):
	def __init__(self):
		QDial.__init__(self)
	def minimumSizeHint(self):
		return QSize(40,40)
	def sizeHint(self):
		return QSize(40,40)

	def set_value(self, value):
		if value.size == 1:
			self.setValue(value.pack()[0])
		else:
			if value.value == -inf:
				self.setValue(-71)
			else:
				self.setValue(value.value)

class Option(QComboBox):
	def __init__(self):
		QComboBox.__init__(self)

class ControlFactory():
	bools = list(ValueFactory.classes.keys())[list(ValueFactory.classes.values()).index(Bool)]
	def get_class(control):
		classes = {
			ControlFactory.bools: ToggleButton,
			('.reverb','.pan','.sens','.gate','.attack','.release','.knee','.gain','.threshold','.ratio','.time','.pre_delay',): Knob,
			('.volume',): VolSlider,
			('.attenuation',): VolSlider,
			('patchbay.',): Knob,   # TODO implement dropdown list box
			('reverb.type',): Knob, # TODO implement dropdown list box
		}
		for parts, cls in classes.items():
			for part in parts:
				if part in control:
					return cls
		raise Exception("Control class unknown: %s" % control)

	def make(control):
		value_class = ValueFactory.get_class(control)
		value = value_class()
		if value.size == 1:
			value.value = value.min
			min_ = value.pack()[0]
			value.value = value.max
			max_ = value.pack()[0]
		else:
			min_, max_ = value.min, value.max

		widget_class = ControlFactory.get_class(control)
		#print(control, widget_class)
		widget = widget_class()
		widget.set_name(control)
		try:
			widget.set_range(min_, max_)
		except:
			raise Exception("Unable to set widget range: %s" % control)
		return widget

class MainWindow(QWidget):
	def __init__(self, controller, mixer):
		QWidget.__init__(self)
		self.controller = controller
		self.mixer = mixer
		self.controls = { page: {} for page in self.mixer.pages }
		self.layouts = { page: self.setup_page_layout(page) for page in self.mixer.pages }
		self.previous_cursor = None

		#self.resize(1260,475)
		self.center()

		tab_layout = self.setup_tabs()
		self.setLayout(tab_layout)
		self.setStyleSheet(css)
		self.grabKeyboard()
	
	def setup_tabs(self):
		self.tab = QTabWidget()
		self.tab.setFocusPolicy(Qt.NoFocus)

		self.pages = { page: Widget() for page in self.mixer.pages }
		for page_name in self.mixer.pages.keys():
			widget = Widget()
			widget.page_name = page_name
			widget.setLayout(self.layouts[page_name])
			self.pages[page_name] = widget
			title = self.mixer.page_title(page_name)
			self.tab.addTab(widget, title)

		self.tab.insertTab(8, Widget(), "")
		self.tab.setTabEnabled(8, False)
		self.tab.insertTab(4, Widget(), "")
		self.tab.setTabEnabled(4, False)
		self.tab.currentChanged.connect(self.tab_change)

		tab_layout = QVBoxLayout()
		tab_layout.addWidget(self.tab)
		return tab_layout

	def current_page_name(self):
		return self.tab.currentWidget().page_name

	def setup_page_layout(self, page_name):
		page = self.mixer.pages[page_name]
		controls = page.get_controls()
		grid = QGridLayout()
		cw = self.connect_widget
		max_columns = max([len(row) for row in controls])
		for j, header in enumerate(page.get_header()):
			widget = QLabel(header)
			grid.addWidget(widget, 0, j, alignment=Qt.AlignHCenter)
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				#if control is None:
				widget = cw(ControlFactory.make(control)) if control else Space()
				if len(row) <= max_columns/ 2:
					grid.addWidget(widget, i+1, j*2, 1, 2, alignment=Qt.AlignHCenter)
				else:
					grid.addWidget(widget, i+1, j, alignment=Qt.AlignHCenter)
				self.controls[page_name][control] = widget

		#grid.setRowStretch(i+1, 1)
		for i, label in enumerate(page.get_labels()):
			widget = QLabel(label)
			grid.addWidget(widget, i+1, max_columns)
			if max_columns <= 4:
				grid.setColumnStretch(max_columns, 1)
		return grid

	def control_change(self):
		sender = self.sender()
		value = sender.value()
		control = sender.name
		page_name = self.mixer.page_name

		widget = self.controls[page_name][control]
		v = ValueFactory.get_class(control)(value)
		if type(v) is Volume:
			if v.value < -70:
				v.value = -inf

		widget.update_label(v.format())
		self.controller.call_app('assign', (control, v))
		print("%s = %s" % (control, value))

	def tab_change(self):
		page_name = self.tab.currentWidget().page_name
		self.mixer.set_page(page_name)

	def get_cursor_widget(self, cursor=None):
		if cursor is None:
			cursor = self.mixer.cursor
		layout = self.layouts[self.mixer.page_name].itemAtPosition(cursor.y+1, cursor.x)
		return layout.widget()

	def update_focus(self):
		page_name = self.mixer.page_name
		control = self.mixer.get_selected_control()
		#widget = get_cursor_widget()
		if control is None:
			widget = self.get_cursor_widget(self.previous_cursor)
		else:
			widget = self.controls[page_name][control]

		widget.setFocus()

		if self.previous_cursor:
			w = self.get_cursor_widget(self.previous_cursor)
			w.setAttribute(Qt.WA_StyledBackground, False)
			#w.setStyleSheet('background-color: none;')

		w = self.get_cursor_widget()
		w.setAttribute(Qt.WA_StyledBackground, True)

	def connect_widget(self, widget):
		#if type(widget) is ToggleButton
		widget.valueChanged.connect(self.control_change)
		return widget

	def center(self):
		frame_geom = self.frameGeometry()
		desk_center = QDesktopWidget().availableGeometry().center()
		frame_geom.moveCenter(desk_center)
		self.move(frame_geom.topLeft())
	
	def keyPressEvent(self, event):
		self.on_keyboard(event)

class MainGraphical():
	def __init__(self, controller, mixer):
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

		self.app = QApplication(sys.argv)
		self.app.setWheelScrollLines(1)
		self.window = MainWindow(self.controller, self.mixer)
		self.window.controller = controller
		self.window.on_keyboard = self.on_keyboard

	def present(self):
		self.window.show()
		return self.app.exec_()

	def block(self):
		self.window.setEnabled(False)

	def unblock(self):
		self.window.setEnabled(True)

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

	def notify(self, control):
		page_name = self.mixer.page_name
		if control in self.window.controls[page_name].keys():
			widget = self.window.controls[page_name][control]

			addr = Capture.get_addr(control)
			value = self.mixer.memory.get_value(addr)

			print("setting " + control + " to " + value.format())
			widget.set_value(value)
			widget.update_label(value.format())

	def quit(self):
		self.window.close()
		print("Exit.")

class AppStub():
	def quit(self):
		self.interface.quit()

def main():
	from controller import NullController
	app = AppStub()
	mixer = GraphicalMixer()
	controller = NullController(app)
	interface = MainGraphical(controller, mixer)
	app.interface = interface
	return interface.present()

if __name__ == '__main__':
	sys.exit(main())
