from PyQt5.QtGui import QPalette, QColor, QKeyEvent, QPainter, QPen, QColor, QFont, QFontMetrics, QIcon
from PyQt5.QtCore import Qt, QSize, QEvent, pyqtSignal
from PyQt5.QtWidgets import *
from roland import ValueFactory, Bool, Capture, Volume, Enum, Scaled, Pan
from math import inf, log10

with open('style.css') as file:
	css = file.read()

class Label(QLabel):
	def __init__(self, text, alignment=Qt.AlignHCenter):
		QLabel.__init__(self, text, alignment=alignment)

class GridLabel(Label): pass
class ValueLabel(Label): pass
class NameLabel(Label): pass
		
class Widget(QWidget):
	def __init__(self):
		QWidget.__init__(self)
		self.setFocusPolicy(Qt.NoFocus)
		self.setContentsMargins(0,0,0,0)

class Control(Widget):
	def __init__(self, name):
		Widget.__init__(self)
		self.set_name(name)
		self.blocked = False
	def set_name(self, control):
		self.name = control
		self.setAccessibleName(self.name)
		self.setAttribute(Qt.WA_StyledBackground, True)

	def set_value(self, value):
		raise Exception("set_value not implemented for %s" % value.__class__)

	def value(self):
		raise Exception("value not implemented for %s" % self.value.__class__)

	def set_range(self, min_, max_):
		pass

	def update_label(self, value):
		pass
	def populate(self, value):
		pass

	def block(self):
		self.blocked = True
	def unblock(self):
		self.blocked = False

class Space(QLabel, Control):
	def __init__(self):
		QLabel.__init__(self, "       ")

	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)

class PushButton(QPushButton):
	def __init__(self):
		QPushButton.__init__(self)
		self.setCheckable(True)
		self.sizePolicy().setVerticalStretch(1)
		self.sizePolicy().setHorizontalStretch(1)
		self.setFocusPolicy(Qt.NoFocus)
	def minimumSizeHint(self):
		return QSize(40, 20)
	def sizeHint(self):
		return QSize(40, 20)
	def value(self):
		return self.isChecked()

class ToggleButton(Control):
	def __init__(self, name):
		Control.__init__(self, name)
		layout = QVBoxLayout()
		layout.setSpacing(0)
		layout.setContentsMargins(2,3,2,1)

		self.sizePolicy().setVerticalStretch(1)
		self.sizePolicy().setHorizontalStretch(1)

		self.button = PushButton()
		self.button.name = self.name
		color_map = {
			('.bypass',): "#fb9931",
			('.+48','hi-z','phase',): "#ef4389",
		}
		for parts, color in color_map.items():
			for part in parts:
				if part in self.name:
					self.setStyleSheet("QPushButton:checked { background-color: %s }" % color)

		title = ''.join(self.name.split('.')[-1])
		self.header = NameLabel(title)

		self.valueChanged = self.button.clicked
		layout.addWidget(self.header, alignment=Qt.AlignBottom)
		layout.addWidget(self.button, alignment=Qt.AlignTop)
		self.setLayout(layout)

	def set_value(self, value):
		self.button.blockSignals(True)
		self.button.setChecked(value.value)
		self.button.blockSignals(False)
	def value(self):
		return self.button.isChecked()

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

class RadioButton(QRadioButton):
	def minimumSizeHint(self):
		return QSize(20, 20)

class RadioGroup(Control):
	valueChanged = pyqtSignal(int)
	first_row = {}
	def __init__(self, name):
		Control.__init__(self, name)
		self.group = QButtonGroup()
		self.group.setExclusive(True)
		self.group.idClicked.connect(self.valueChanged)

		self.label = None
		self.header = None
		self.buttons = []

		self.layout = QGridLayout()
		#self.layout.setSpacing(0)
		self.layout.setContentsMargins(1,1,1,1)

		#self.sizePolicy().setVerticalStretch(1)
		#self.sizePolicy().setHorizontalStretch(1)
		#self.setSizePolicy(QSizePolicy.Expanding)

	def populate(self, value):
		#for i,v in enumerate(value.values):
		#	button = RadioButton()
		#	button.setCheckable(True)
		#	button.name = self.name
		#	self.group.addButton(button, i)
		#	print("added %d" % i)
		#	self.buttons += [button]

		first_row = None
		for i,v in enumerate(value.values):
			value.value = i
			button = RadioButton()
			button.setFocusPolicy(Qt.NoFocus)
			self.group.addButton(button, i)
			self.buttons += [button]
			self.add_widget(button, i+1, 1)

			t = type(value)
			label = ValueLabel(' '.join(value.format().split(' ')))
			if t in self.first_row.keys():
				label.setStyleSheet("color:transparent");
			else:
				first_row = False
			self.add_widget(label, i+1,0)
			self.add_widget(label, i+1,2)

		if first_row is not None:
			RadioGroup.first_row[type(value)] = first_row

		self.setLayout(self.layout)

	def widget_layout(self):
		raise Exception("Not implemented")
	def outer_layout(self):
		raise Exception("Not implemented")

	def value(self):
		return self.group.checkedId()

	def set_value(self, value):
		v = value.value
		self.group.blockSignals(True)
		self.buttons[v].blockSignals(True)
		self.buttons[v].setChecked(True)
		self.buttons[v].blockSignals(False)
		self.group.blockSignals(False)


class VRadioGroup(RadioGroup):
	def outer_layout(self):
		return QVBoxLayout()
	def inner_layout(self):
		return QHBoxLayout()
	def add_widget(self, widget, i, j):
		self.layout.addWidget(widget, i, j, alignment=Qt.AlignVCenter)
		self.layout.setRowStretch(i, 1)

class HRadioGroup(RadioGroup):
	def outer_layout(self):
		return QHBoxLayout()
	def inner_layout(self):
		return QVBoxLayout()
	def add_widget(self, widget, i, j):
		self.layout.addWidget(widget, j, i, alignment=Qt.AlignHCenter)
		self.layout.setColumnStretch(i, 1)

class TrackControl(Control):
	def __init__(self, name):
		Control.__init__(self, name)
		self.label = None
		self.header = None
	def setup_layout(self):
		layout = QVBoxLayout()
		layout.setSpacing(0)
		layout.setContentsMargins(1,2,1,0)

		if self.header:
			layout.addWidget(self.header, 0, alignment=Qt.AlignBottom | Qt.AlignHCenter)
		layout.addWidget(self.slider, 0)
		if self.label:
			layout.addWidget(self.label, 0, alignment=Qt.AlignTop | Qt.AlignHCenter)

		self.valueChanged = self.slider.valueChanged
		self.setLayout(layout)

	def set_name(self, name):
		self.slider.name = name

	def set_value(self, value):
		self.slider.blockSignals(True)
		self.slider.setTracking(False)
		if value.value == -inf:
			position = -71
		else:
			position = value.value

		if isinstance(value, Scaled):
			position /= value.step

		self.slider.setSliderPosition(position)

		self.slider.setTracking(True)
		self.slider.blockSignals(False)
		self.text = value.format()

	def setRange(self, min_, max_):
		self.slider.setRange(min_, max_)

	def update_label(self, value):
		if self.label:
			self.label.setText(str(value))
		self.slider.text = str(value)

	def set_range(self, min_, max_):
		if min_ == -inf:
			min_ = -71
		self.setRange(min_, max_)


class Dial(QDial):
	def __init__(self):
		QDial.__init__(self)

		self.dragging = False
		self.mouse_press_point = None
		self.base_value = 0
		self.scale_factor = 0.2
		self.text = ""
		self.pan_mode = False
		self.setContentsMargins(0,0,0,0)
		self.sizePolicy().setVerticalStretch(1)
		self.sizePolicy().setHorizontalStretch(1)

		self.fm = QFontMetrics(self.font())

	def minimumSizeHint(self):
		return QSize(42,42)
	def sizeHint(self):
		return QSize(42,42)

	def mousePressEvent(self, event):
		self.mouse_press_point = event.pos()
		self.dragging = True
		self.base_value = self.value()

	def mouseReleaseEvent(self, event):
		self.dragging = False

	def mouseMoveEvent(self, event):
		dx = self.mouse_press_point.x() - event.x()
		dy = self.mouse_press_point.y() - event.y()
		distance = abs(dy)
		sign = (1.0 if dy > 0 else -1.0)
		new_value = self.base_value + self.scale_factor * distance * sign
		self.setSliderPosition(new_value)

	def paintEvent(self, event):
		o = 4
		p = QPainter(self)
		p.setRenderHint(QPainter.Antialiasing)
		pen0 = QPen(QColor(0,0,0), 4, Qt.SolidLine)
		pen1 = QPen(QColor(108,189,232), 3, Qt.SolidLine)
		pen2 = QPen(QColor(248,248,248))
		tw = self.fm.width(self.text)
		th = self.fm.height()
		s = self.size()
		d = 40  #max(s.height(), s.width())
		x = s.width()/2 - d/2
		min_angle = 240
		max_angle = -300
		center_angle = 90

		if self.pan_mode:
			angle = max_angle * self.value() / self.maximum()
			p.setPen(pen0)
			p.drawArc(x+o, 0+o, d-o-o-x, d-o-o-x, min_angle * 16, max_angle * 16)
			p.setPen(pen1)
			p.drawArc(x+o, 0+o, d-o-o-x, d-o-o-x, center_angle * 16, angle/2 * 16)
			p.setPen(pen2)
		else:
			angle = max_angle * (self.value() - self.minimum()) / (self.maximum() - self.minimum())
			p.setPen(pen0)
			p.drawArc(x+o, 0+o, d-o-o-x, d-o-o-x, min_angle * 16, max_angle * 16)
			p.setPen(pen1)
			p.drawArc(x+o, 0+o, d-o-o-x, d-o-o-x, min_angle * 16, angle * 16)
			p.setPen(pen2)

		p.drawText(o/2 + s.width()/2 - tw/2, s.height()/2, self.text)

class Knob(TrackControl):
	def __init__(self, name):
		self.slider = Dial()
		TrackControl.__init__(self, name)
		title = self.slider.name.split('.')[-1].replace('_',' ')
		if len(title) > 9:
			title = title.replace(' ','\n')
		self.header = NameLabel(title)
		self.setup_layout()
		self.slider.scale_factor = 0.5

	def set_range(self, min_, max_):
		TrackControl.set_range(self, min_, max_)
		if min_ == -inf:
			min_ = -71
		self.slider.scale_factor = 0.2 * (max_ - min_)/60.0

	def populate(self, value):
		if isinstance(value, Enum):
			range_factor = len(value.values) / 60.0
			self.slider.scale_factor = 0.2 * range_factor
		if isinstance(value, Pan):
			self.slider.pan_mode = True

class VolSlider(TrackControl):
	def __init__(self, name):
		slider = QSlider()
		slider.setTickPosition(QSlider.TicksBothSides)
		slider.setTickInterval(10)
		slider.setPageStep(1)
		slider.setSingleStep(1)
		slider.setMinimum(-71)
		slider.setMaximum(12)
		slider.sizePolicy().setVerticalStretch(0)
		slider.sizePolicy().setHorizontalStretch(0)
		slider.setFocusPolicy(Qt.NoFocus)
		slider.setTracking(False)
		self.slider = slider
		TrackControl.__init__(self, name)
		self.label = ValueLabel("?")
		self.setup_layout()

	def minimumSizeHint(self):
		return QSize(15, 180)


class ControlFactory():
	bools = list(ValueFactory.classes.keys())[list(ValueFactory.classes.values()).index(Bool)]
	def get_class(control):
		classes = {
			ControlFactory.bools: ToggleButton,
			('reverb.type',): VRadioGroup,
			('.reverb_return',): Knob,
			('.reverb','.pan','.sens','.gate','.attack','.release','.knee','.gain','.threshold','.ratio','.time','.pre_delay',): Knob,
			('.volume',): VolSlider,
			('.attenuation',): VRadioGroup,
			('patchbay.',): HRadioGroup,
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
		widget.set_range(min_, max_)
		widget.populate(value)

		return widget

class TabBar(QTabBar):
	def wheelEvent(self, event):
		event.ignore()

class MainWindow(QMainWindow):
	def __init__(self, controller, mixer):
		QMainWindow.__init__(self)
		self.blocked = False
		self.controller = controller
		self.mixer = mixer
		self.controls = { page: {} for page in self.mixer.pages }
		self.layouts = { page: self.setup_page_layout(page) for page in self.mixer.pages }
		self.top_tab_names = ['inputs','outputs','channel','preamp','compressor','line','reverb','patchbay']
		self.previous_cursor = None

		tab_layout = self.setup_tabs()
		self.center()
		widget = Widget()
		widget.setLayout(tab_layout)
		self.setCentralWidget(widget)
		self.setStyleSheet(css)
		self.installEventFilter(self)

	def block(self):
		self.blocked = True
	def unblock(self):
		self.blocked = False
	
	def eventFilter(self, obj, event):
		if event.type() is QEvent.KeyPress:
			self.controller.app.debug('key')
			return False
		return super(MainWindow, self).eventFilter(obj, event)
		
	def setup_tabs(self):
		self.tab_bar = TabBar()
		self.tab_bar.setFocusPolicy(Qt.NoFocus)

		self.stack = QStackedWidget()

		self.pages = { page: Widget() for page in self.mixer.pages }
		for page_name in self.mixer.pages.keys():
			widget = Widget()
			widget.sizePolicy().setVerticalStretch(0)
			widget.sizePolicy().setHorizontalStretch(0)
			widget.page_name = page_name
			widget.setLayout(self.layouts[page_name])
			self.pages[page_name] = widget
			title = self.mixer.page_title(page_name)
			self.stack.addWidget(widget)

		#prefixes = { k.split('.')[0]: k for k in reversed(self.pages.keys()) }
		for i,action in enumerate(self.top_tab_names):
			self.tab_bar.addTab(action.title())
			self.tab_bar.setTabData(i, action)

		self.tab_bar.currentChanged.connect(self.top_tab_change)

		self.channel_bar = TabBar()
		for ch in range(0, 16):
			self.channel_bar.addTab("%d" % (ch+1))
			self.channel_bar.setTabData(ch, str(ch+1))
		self.channel_bar.currentChanged.connect(self.channel_change)

		self.monitor_bar = TabBar()
		for m,mon in enumerate(['a','b','c','d']):
			self.monitor_bar.addTab(mon.upper())
			self.monitor_bar.setTabData(m, mon)
		self.monitor_bar.currentChanged.connect(self.monitor_change)

		self.subtab = QStackedWidget()
		self.subtab.addWidget(Widget())
		self.subtab.addWidget(self.monitor_bar)
		self.subtab.addWidget(self.channel_bar)

		tab_layout = QVBoxLayout()
		tab_layout.addWidget(self.tab_bar)
		tab_layout.addWidget(self.subtab)
		tab_layout.addWidget(self.stack)
		return tab_layout

	def hide_subtab_bar(self):
		self.subtab.setCurrentIndex(0)
	def show_monitor_bar(self):
		self.subtab.setCurrentIndex(1)
	def show_channel_bar(self):
		self.subtab.setCurrentIndex(2)

	def top_tab_change(self, index):
		action = self.tab_bar.tabData(index)
		self.controller.call_app(action)

	def monitor_change(self, index):
		action = self.tab_bar.tabData(index)
		self.controller.call_app('monitor', chr(97+index))

	def channel_change(self, index):
		action = self.tab_bar.tabData(index)
		self.controller.call_app('channel', index+1)

	def tab_change(self):
		self.controller.app.debug("tab_change")
		self.controller.call_app('set_page', self.current_page_name())

	def set_current_page(self, selected):
		names = selected.split('.')
		top_name = names[0]
		sub_name = names[1] if len(names) > 1 else ""
		widget = self.pages[selected]
		self.stack.setCurrentWidget(widget)
		
		if top_name == "daw_monitor":
			top_name = "outputs"
		elif top_name == "input_monitor":
			top_name = "inputs"
		top_index = self.top_tab_names.index(top_name)
		self.tab_bar.setCurrentIndex(top_index)

		if '_monitor' in selected:
			self.monitor_bar.setCurrentIndex(ord(sub_name)-97)
			self.show_monitor_bar()
		elif 'channel.' in selected:
			self.channel_bar.setCurrentIndex(int(sub_name)-1)
			self.show_channel_bar()
		else:
			self.hide_subtab_bar()

	def current_page_name(self):
		return self.stack.currentWidget().page_name

	def setup_page_layout(self, page_name):
		page = self.mixer.pages[page_name]
		controls = page.get_controls()
		grid = QGridLayout()
		grid.setSpacing(0)
		cw = self.connect_widget
		max_columns = max([len(row) for row in controls])
		final_row = len(controls) -1
		for j, header in enumerate(page.get_header()):
			widget = GridLabel(header)
			widget.sizePolicy().setVerticalStretch(0)
			widget.sizePolicy().setHorizontalStretch(0)
			align = Qt.AlignTop
			if j < max_columns:
				align |= Qt.AlignHCenter
			grid.addWidget(widget, 0, j, alignment=align)
		sliders = False
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				#if control is None:
				if control is None:
					if grid.itemAtPosition(i+1,j) is not None:
						continue
					widget = Space()
				else:
					widget = cw(ControlFactory.make(control))

				align = Qt.AlignHCenter
				rowspan, colspan = 1, 1
				#if control and '.reverb_return' in control:
				#	rowspan = final_row - i

				if control and 'reverb.type' in control:
					rowspan = 6
					#align |= Qt.AlignVCenter
					#widget.layout.setVerticalSpacing(27)
					align = None

				if align is None:
					grid.addWidget(widget, i+1, j, rowspan, colspan)
				elif len(row) <= max_columns / 2:
					colspan = 2
					grid.addWidget(widget, i+1, j*2, rowspan, colspan, alignment=align)
				else:
					#if i == final_row: align |= Qt.AlignTop
					grid.addWidget(widget, i+1, j, rowspan, colspan, alignment=align)

				self.controls[page_name][control] = widget
				if type(widget) is VolSlider:
					sliders = True

		grid.setRowStretch(i+2, 1)

		j = max_columns #grid.columnCount()
		for i, label in enumerate(page.get_labels()):
			widget = GridLabel(label)
			grid.addWidget(widget, i+1, j, alignment=Qt.AlignLeft | Qt.AlignVCenter)
			if max_columns <= 4:
				grid.setColumnStretch(j, 1)
		return grid

	def get_control(self, control):
		page_name = self.mixer.page_name
		if control not in self.controls[page_name].keys():
			self.controller.app.debug("control not found on active page: %s" % control)
			return None
		return self.controls[page_name][control]

	def control_change(self):
		if self.blocked: return

		sender = self.sender()
		value = sender.value()
		control = sender.name

		widget = self.get_control(control)
		#print(control)
		if not widget or widget.blocked: return

		widget.setFocus()

		v = ValueFactory.get_class(control)()
		if type(v) is Volume:
			if value < -70:
				value = -inf
		elif isinstance(v, Scaled):
			value *= v.step
			value += v.min
		v.value = value

		widget.update_label(v.format())
		self.controller.call_app('assign', (control, v))
		self.controller.app.debug("control_change %s = %s (%s)" % (control, v.format(), str(value)))

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

		if widget is None:
			return
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

