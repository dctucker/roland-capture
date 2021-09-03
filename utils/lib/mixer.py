from lib.memory import Memory
from lib.pages import InputPage, OutputPage, PreampPage, CompressorPage, ReverbPage, LinePage, Patchbay, ChannelPage
from lib.roland import Capture, CaptureView

class Cursor(object):
	def __init__(self):
		self.x = 0
		self.y = 0
	def yx(self):
		return self.y, self.x

class Mixer(object):
	def __init__(self):
		self.capture_view = CaptureView.instance()
		self.memory = Memory()
		self.cursor = Cursor()
		self.setup_pages()
		self.setup_name_table()
		self.monitor = 'a'
		self.channel = 1
		self.set_page('input_monitor.' + self.monitor)

	def setup_pages(self):
		self.pages = {
			"input_monitor.a": InputPage(self, 'a'),
			"input_monitor.b": InputPage(self, 'b'),
			"input_monitor.c": InputPage(self, 'c'),
			"input_monitor.d": InputPage(self, 'd'),
			"daw_monitor.a" : OutputPage(self, 'a'),
			"daw_monitor.b" : OutputPage(self, 'b'),
			"daw_monitor.c" : OutputPage(self, 'c'),
			"daw_monitor.d" : OutputPage(self, 'd'),
			"preamp"        : PreampPage(self),
			"compressor"    : CompressorPage(self),
			"line"          : LinePage(self),
			"reverb"        : ReverbPage(self),
			"patchbay"      : Patchbay(self),
		}
		for ch in range(0, 16):
			self.pages |= { "channel.%d" % (ch+1) : ChannelPage(self, ch+1) }

	def setup_controls(self):
		self.page = self.pages[self.page_name]
		controls = []
		self.header   = self.page.get_header()
		self.controls = self.page.get_controls()

	def setup_name_table(self):
		for name, page in self.pages.items():
			for row in page.get_controls():
				for control in row:
					if control is None:
						continue
					self.capture_view.add_name_to_table(control)

	def height(self):
		return len(self.controls)

	def width(self):
		return len(self.controls[self.cursor.y])

	def set_page(self, page):
		self.page_name = page
		self.setup_controls()
		self.cursor.y = min(self.cursor.y, self.height()-1)
		self.cursor.x = min(self.cursor.x, self.width()-1)

	def set_monitor(self, m):
		self.monitor = m
		if 'monitor.' in self.page_name:
			self.set_page(self.page_name[:-1] + self.monitor)
		else:
			print(self.page_name)

	def set_channel(self, ch):
		self.channel = ch
		if 'channel.' in self.page_name:
			self.set_page("channel.%d" % ch)
		else:
			print(self.page_name)

	def cursor_down(self):
		w = self.width()
		if self.cursor.y + 1 < self.height():
			self.cursor.y += 1
		if self.width() >= w * 2:
			self.cursor.x *= 2

	def cursor_up(self):
		w = self.width()
		if self.cursor.y > 0:
			self.cursor.y -= 1
		if self.width() <= w // 2:
			self.cursor.x //= 2

	def cursor_left(self):
		if self.cursor.x > 0:
			self.cursor.x -= 1

	def cursor_right(self):
		if self.cursor.x + 1 < self.width():
			self.cursor.x += 1

	def get_selected_control(self):
		row = self.cursor.y
		col = self.cursor.x
		return self.controls[row][col]

	def get_selected_addr(self):
		control = self.get_selected_control()
		if control is None:
			return None
		return Capture.get_addr(control)

	def get_memory_value(self, control):
		addr = Capture.get_addr(control)
		return self.memory.get_value(addr)

	def decrement_selected(self):
		addr = self.get_selected_addr()
		data = self.memory.decrement(addr) if addr else None
		return addr, data

	def increment_selected(self):
		addr = self.get_selected_addr()
		data = self.memory.increment(addr) if addr else None
		return addr, data

	def zero_selected(self):
		addr = self.get_selected_addr()
		data = self.memory.zero(addr) if addr else None
		return addr, data

	def set_memory_value(self, name, value):
		addr = Capture.get_addr(name)
		data = self.memory.set_value(addr, value)
		return addr, data
