from PyQt5.QtGui import QPalette, QColor, QKeyEvent
from PyQt5.QtCore import Qt, QSize, QEvent
from PyQt5.QtWidgets import *
from roland import ValueFactory, Bool, Capture, Volume, Enum
from math import inf

with open('style.css') as file:
	css = file.read()

class Widget(QWidget):
	def __init__(self):
		QWidget.__init__(self)
		self.setFocusPolicy(Qt.NoFocus)

class Control(Widget):
	def __init__(self, name):
		Widget.__init__(self)
		self.set_name(name)
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

	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)

class ToggleButton(QPushButton, Control):
	def __init__(self, name):
		QPushButton.__init__(self)
		Control.__init__(self, name)
		self.setText(''.join(self.name.split('.')[-1]))
		self.setCheckable(True)
		self.sizePolicy().setVerticalStretch(1)
		self.sizePolicy().setHorizontalStretch(1)
		self.setFocusPolicy(Qt.NoFocus)
		self.valueChanged = self.clicked
	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)
	def value(self):
		return self.isChecked()

	def set_value(self, value):
		self.setChecked(value.value)

class Option(QComboBox, Control):
	def __init__(self, name):
		QComboBox.__init__(self)
		Control.__init__(self, name)
		self.valueChanged = self.currentIndexChanged
		#Control.__init__(self)
	def populate(self, value):
		for i, v in enumerate(value.values):
			value.value = i
			title = value.format()
			self.addItem(title)
	def value(self):
		return self.currentIndex()
		
class Knob(QDial, Control):
	def __init__(self, name):
		QDial.__init__(self)
		Control.__init__(self, name)

		self.dragging = False
		self.mouse_press_point = None
		self.base_value = 0
		self.scale_factor = 1.0
	def minimumSizeHint(self):
		return QSize(40,40)
	def sizeHint(self):
		return QSize(40,40)

	def mousePressEvent(self, event):
		self.mouse_press_point = event.pos()
		self.dragging = True
		self.base_value = self.value()

	def mouseReleaseEvent(self, event):
		self.dragging = False

	def mouseMoveEvent(self, event):
		new_value = self.base_value + self.scale_factor * (self.mouse_press_point.y() - event.y())
		self.setSliderPosition(new_value)

	def set_value(self, value):
		self.setTracking(False)
		if value.value == -inf:
			self.setSliderPosition(-71)
		elif value.size == 1:
			self.setSliderPosition(value.pack()[0])
		else:
			self.setSliderPosition(value.value)
		self.setTracking(True)

class VolSlider(Control):
	def __init__(self, name):
		slider = QSlider()
		slider.setTickPosition(QSlider.TicksBothSides)
		slider.setTickInterval(10)
		slider.setPageStep(1)
		slider.setSingleStep(1)
		slider.setMinimum(-71)
		slider.setMaximum(12)
		slider.setFocusPolicy(Qt.NoFocus)
		slider.setTracking(False)
		self.slider = slider

		Control.__init__(self, name)
		layout = QVBoxLayout()

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
		self.slider.setTracking(False)
		if value.value == -inf:
			self.slider.setSliderPosition(-71)
		else:
			self.slider.setSliderPosition(value.value)
		self.slider.setTracking(True)

class ControlFactory():
	bools = list(ValueFactory.classes.keys())[list(ValueFactory.classes.values()).index(Bool)]
	def get_class(control):
		classes = {
			ControlFactory.bools: ToggleButton,
			('.reverb','.pan','.sens','.gate','.attack','.release','.knee','.gain','.threshold','.ratio','.time','.pre_delay',): Knob,
			('.volume',): VolSlider,
			('.attenuation',): VolSlider,
			('patchbay.','reverb.type',): Option,
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
		widget = widget_class(control)
		if isinstance(value, Enum) and isinstance(widget, Option):
			widget.populate(value)

		try:
			widget.set_range(min_, max_)
		except:
			raise Exception("Unable to set widget range: %s" % control)
		return widget

class TabBar(QTabBar):
	def wheelEvent(self, event):
		event.ignore()

class MainWindow(QMainWindow):
	def __init__(self, controller, mixer):
		QMainWindow.__init__(self)
		self.controller = controller
		self.mixer = mixer
		self.controls = { page: {} for page in self.mixer.pages }
		self.layouts = { page: self.setup_page_layout(page) for page in self.mixer.pages }
		self.previous_cursor = None

		#self.resize(1260,475)
		self.center()

		tab_layout = self.setup_tabs()
		#self.setLayout(tab_layout)
		self.setCentralWidget(self.tab)
		self.setStyleSheet(css)
		#self.grabKeyboard()
		self.installEventFilter(self)
	
	def eventFilter(self, obj, event):
		if event.type() is QEvent.KeyPress:
			self.controller.app.debug('key')
			return False
		return super(MainWindow, self).eventFilter(obj, event)
		
	def setup_tabs(self):
		self.tab = QTabWidget()
		self.tab.setTabBar(TabBar())
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
			widget.sizePolicy().setVerticalStretch(0)
			widget.sizePolicy().setHorizontalStretch(0)
			align = None
			if j < max_columns:
				align = Qt.AlignHCenter | Qt.AlignTop
			grid.addWidget(widget, 0, j, alignment=align or Qt.AlignTop)
		sliders = False
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				#if control is None:
				widget = cw(ControlFactory.make(control)) if control else Space()
				if len(row) <= max_columns/ 2:
					grid.addWidget(widget, i+1, j*2, 1, 2, alignment=Qt.AlignHCenter)
				else:
					grid.addWidget(widget, i+1, j, alignment=Qt.AlignHCenter)
				self.controls[page_name][control] = widget
				if type(widget) is VolSlider:
					sliders = True

		if not sliders:
			grid.setRowStretch(i+2, 1)
		for i, label in enumerate(page.get_labels()):
			widget = QLabel(label)
			grid.addWidget(widget, i+1, max_columns, alignment=Qt.AlignLeft)
			if max_columns <= 4:
				grid.setColumnStretch(max_columns, 1)
		return grid

	def get_control(self, control):
		page_name = self.mixer.page_name
		if control not in self.controls[page_name].keys():
			self.controller.app.debug("control not found on active page: %s" % control)
			return None
		return self.controls[page_name][control]

	def control_change(self):
		sender = self.sender()
		value = sender.value()
		control = sender.name

		widget = self.get_control(control)
		if not widget: return
		widget.setFocus()
		v = ValueFactory.get_class(control)(value)
		if type(v) is Volume:
			if v.value < -70:
				v.value = -inf

		widget.update_label(v.format())
		self.controller.call_app('assign', (control, v))
		self.controller.app.debug("%s = %s" % (control, value))

	def tab_change(self):
		self.controller.call_app('set_page', self.current_page_name())

	def get_cursor_widget(self, cursor=None):
		if cursor is None:
			cursor = self.mixer.cursor
		layout = self.layouts[self.mixer.page_name].itemAtPosition(cursor.y+1, cursor.x)
		return layout.widget()

	def update_focus(self):
		control = self.mixer.get_selected_control()
		#widget = get_cursor_widget()
		if control is None:
			widget = self.get_cursor_widget(self.previous_cursor)
		else:
			widget = self.get_control(control)

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
	
	#def keyPressEvent(self, event):
	#	self.on_keyboard(event)

