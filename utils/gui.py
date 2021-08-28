#!/usr/bin/env python3

from PyQt5.QtCore import Qt, QSize
from PyQt5.QtWidgets import *
import sys

from math import inf
from mixer import Mixer
from roland import ValueFactory, Bool

class GraphicalMixer(Mixer):
	pass

class Control(QWidget):
	def set_name(self, control):
		self.name = control
		self.setAccessibleName(self.name)
	def set_range(self, min_, max_):
		t = type(self)
		if t is VolSlider or t is Knob:
			if min_ == -inf:
				min_ = -71
			print(min_, max_) # TODO floats are getting passed here, needs fix
			self.setRange(min_, max_)

class VolSlider(QSlider, Control):
	def __init__(self):
		QSlider.__init__(self)
		self.setTickPosition(QSlider.TicksBothSides)
		self.setTickInterval(10)
		self.setPageStep(1)
		self.setSingleStep(1)
		self.setMinimum(-71)
		self.setMaximum(12)
	def minimumSizeHint(self):
		return QSize(10, 100)

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

class Knob(QDial, Control):
	def __init__(self):
		QDial.__init__(self)
		self.text = "1"
	def minimumSizeHint(self):
		return QSize(20,20)

class ControlFactory():
	bools = list(ValueFactory.classes.keys())[list(ValueFactory.classes.values()).index(Bool)]
	def get_class(control):
		classes = {
			ControlFactory.bools: ToggleButton,
			('.reverb','.pan','.sens','.gate','.attack','.release','.knee','.gain','.threshold','.ratio','.time','.pre-delay'): Knob,
			('.volume',): VolSlider,
			('.attenuation'): VolSlider,
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
		#print(widget_class)
		widget = widget_class()
		widget.set_name(control)
		try:
			widget.set_range(min_, max_)
		except:
			raise Exception("Widget is None: %s" % control)
		return widget

class MainWindow(QWidget):
	def __init__(self):
		QWidget.__init__(self)
		self.mixer = GraphicalMixer()
		self.layouts = { page: self.setup_page_layout(page) for page in self.mixer.pages }

		#self.resize(1260,475)
		self.center()

		self.setLayout(self.layouts['preamp'])

	def setup_page_layout(self, page_name=None):
		if page_name is None:
			page_name = self.mixer.page_name
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
				if control is None: continue
				widget = ControlFactory.make(control)
				if len(row) <= max_columns/ 2:
					grid.addWidget(cw(widget), i+1, j*2, 1, 2, alignment=Qt.AlignHCenter)
				else:
					grid.addWidget(cw(widget), i+1, j, alignment=Qt.AlignHCenter)

		for i, label in enumerate(page.get_labels()):
			widget = QLabel(label)
			grid.addWidget(widget, i+1, max_columns)
		return grid

	def control_change(self):
		sender = self.sender()
		value = sender.value()
		control = sender.name
		print("%s = %s" % (control, value))

	def connect_widget(self, widget):
		#if type(widget) is ToggleButton
		widget.valueChanged.connect(self.control_change)
		return widget

	def center(self):
		frame_geom = self.frameGeometry()
		desk_center = QDesktopWidget().availableGeometry().center()
		frame_geom.moveCenter(desk_center)
		self.move(frame_geom.topLeft())


class MainGraphical():
	def __init__(self, controller, mixer):
		self.controller = controller
		self.mixer = mixer

		self.app = QApplication(sys.argv)
		self.app.setWheelScrollLines(1)
		self.window = MainWindow()

	def present(self):
		self.window.show()
		return self.app.exec_()
		

def main():
	mixer = GraphicalMixer()
	controller = None
	return MainGraphical(controller, mixer).present()

if __name__ == '__main__':
	sys.exit(main())
