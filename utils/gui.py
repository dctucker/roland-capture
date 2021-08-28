#!/usr/bin/env python3

from PyQt5.QtCore import Qt, QSize
from PyQt5.QtWidgets import *
import sys

from math import inf
from mixer import Mixer
from roland import ValueFactory, Bool, Capture, Volume

css = """
QPushButton:checked {
	background-color: #6cbde8
}
QSlider::groove:vertical {
	border: 1px solid #333333;
	width: 2px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */
	background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #616161, stop:1 #747474);
	margin: 0 2px;
}

QSlider::handle:vertical {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
		stop:0    #e8e9e9,
		stop:0.20 #c1c5c8,
		stop:0.33 #1a1b1f,
		stop:0.35 #5c6063,
		stop:0.45 #404145,
		stop:0.5  #2b3034,
		stop:0.55 #8e8e8e,
		stop:0.79 #c9ced1,
		stop:0.80 #28292b,
		stop:1    #171719
	);
	border: 2px outset #5c5c5c;
	height: 30px;
	margin: 0 -8px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */
}
"""

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

	def set_value(self, value):
		raise Exception("set_value not implemented for %s" % value.__class__)

	def update_label(self, value):
		pass

class VolSlider(Control):
	def __init__(self):
		QWidget.__init__(self)
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
		self.text = "1"
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
			raise Exception("Unable to set widget range: %s" % control)
		return widget

class MainWindow(QWidget):
	def __init__(self, mixer):
		QWidget.__init__(self)
		self.mixer = mixer
		self.widgets = { page: {} for page in self.mixer.pages }
		self.layouts = { page: self.setup_page_layout(page) for page in self.mixer.pages }

		#self.resize(1260,475)
		self.center()

		self.setLayout(self.layouts[self.mixer.page_name])
		self.setStyleSheet(css)

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
				widget = cw(ControlFactory.make(control))
				if len(row) <= max_columns/ 2:
					grid.addWidget(widget, i+1, j*2, 1, 2, alignment=Qt.AlignHCenter)
				else:
					grid.addWidget(widget, i+1, j, alignment=Qt.AlignHCenter)
				self.widgets[page_name][control] = widget

		for i, label in enumerate(page.get_labels()):
			widget = QLabel(label)
			grid.addWidget(widget, i+1, max_columns)
		return grid

	def control_change(self):
		sender = self.sender()
		value = sender.value()
		control = sender.name
		page_name = self.mixer.page_name

		widget = self.widgets[page_name][control]
		v = ValueFactory.get_class(control)(value)
		if type(v) is Volume:
			if v.value < -70:
				v.value = -inf

		widget.update_label(v.format())
		self.controller.call_app('assign', (control, v))
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
		self.window = MainWindow(self.mixer)
		self.window.controller = controller

	def present(self):
		self.window.show()
		return self.app.exec_()

	def block(self):
		self.window.setEnabled(False)

	def unblock(self):
		self.window.setEnabled(True)

	def on_keyboard(self, key):
		pass # TODO

	def debug(self, message, end="\n"):
		print(message, end=end)

	def refresh(self):
		pass # TODO

	def notify(self, control):
		page_name = self.mixer.page_name
		if control in self.window.widgets[page_name].keys():
			widget = self.window.widgets[page_name][control]

			addr = Capture.get_addr(control)
			value = self.mixer.memory.get_value(addr)

			print("setting " + control + " to " + value.format())
			widget.set_value(value)
			widget.update_label(value.format())

def main():
	mixer = GraphicalMixer()
	controller = None
	return MainGraphical(controller, mixer).present()

if __name__ == '__main__':
	sys.exit(main())
