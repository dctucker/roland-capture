#!/usr/bin/env python3
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtWidgets import *
import sys

from math import inf
from mixer import InputPage
from roland import ValueFactory

class Control(QWidget):
	def set_name(self, control):
		self.name = control
		self.setAccessibleName(self.name)
	def set_range(self, min_, max_):
		t = type(self)
		if t is VolSlider or t is Knob:
			if min_ == -inf:
				min_ = -71
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
	def __init__(self, range_=(-100,100)):
		QDial.__init__(self)
		self.setRange(*range_)
		self.text = "1"
	def minimumSizeHint(self):
		return QSize(20,20)

class ControlFactory():
	def get_class(control):
		classes = {
			('.volume',): VolSlider,
			('.mute','.solo','.stereo',): ToggleButton,
			('.reverb','.pan'): Knob,
		}
		for parts, cls in classes.items():
			for part in parts:
				if part in control:
					return cls
	def make(control):
		value_class = ValueFactory.get_class(control)

		widget_class = ControlFactory.get_class(control)
		widget = widget_class()
		widget.set_name(control)
		widget.set_range(value_class.min, value_class.max)
		return widget

class MainWindow(QWidget):
	def __init__(self):
		QWidget.__init__(self)
		self.resize(1260,475)
		self.center()

		#layout = QHBoxLayout()
		#for i in range(0,8):
		#	layout.addWidget(InputStereoChannel())
		#self.setLayout(layout)

		#grid = QGridLayout()
		#cw = self.connect_widget
		#for ch in range(0, 16):
		#	i = 0
		#	j = ch
		#	if ch % 2 == 0:
		#		grid.addWidget(cw(ToggleButton('STEREO')), i, j, 1, 2)
		#	grid.addWidget(cw(ToggleButton('SOLO')), i+1, j)
		#	grid.addWidget(cw(ToggleButton('MUTE')), i+2, j)
		#	grid.addWidget(cw(Knob((-71,12))), i+3, j)
		#	grid.addWidget(cw(Knob()), i+4, j)
		#	grid.addWidget(cw(VolSlider()), i+5, j, alignment=Qt.AlignHCenter)
		#	grid.setRowStretch(i+5, 10)
		#self.setLayout(grid)

		self.input_page = InputPage(None, 'a')
		controls = self.input_page.get_controls()
		grid = QGridLayout()
		cw = self.connect_widget
		max_columns = max([len(row) for row in controls])
		for j, header in enumerate(self.input_page.get_header()):
			widget = QLabel(header)
			grid.addWidget(widget, 0, j, alignment=Qt.AlignHCenter)
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				widget = ControlFactory.make(control)
				if len(row) <= max_columns/ 2:
					grid.addWidget(cw(widget), i+1, j*2, 1, 2, alignment=Qt.AlignHCenter)
				else:
					grid.addWidget(cw(widget), i+1, j, alignment=Qt.AlignHCenter)

		for i, label in enumerate(self.input_page.get_labels()):
			widget = QLabel(label)
			grid.addWidget(widget, i+1, max_columns)

		self.setLayout(grid)

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

def main():
	app = QApplication(sys.argv)
	app.setWheelScrollLines(1)
	window = MainWindow()
	window.show()

	sys.exit(app.exec_())

if __name__ == '__main__':
	main()
