from roland import Capture, CaptureView, Memory

class Cursor(object):
	def __init__(self):
		self.x = 0
		self.y = 0
	def yx(self):
		return self.y, self.x

class Mixer(object):
	pages = [
		"input monitor.a",
		"input monitor.b",
		"input monitor.c",
		"input monitor.d",
		"daw_monitor monitor.a",
		"daw_monitor monitor.b",
		"daw_monitor monitor.c",
		"daw_monitor monitor.d",
		"preamp",
	]

	def __init__(self):
		#self.volume = [ [ None for i in range(0, 16) ] for m in range(0, 4) ]
		self.memory = Memory()
		self.capture_view = CaptureView.instance()
		self.monitor = 'a'
		self.page = 'input monitor.' + self.monitor
		self.setup_controls()
		self.cursor = Cursor()

	def setup_controls(self):
		mode = self.page
		self.controls = []
		if 'input' in mode:
			self.header = [str(ch+1) for ch in range(0,16)]
			row = []
			for ch in range(0,16,2):
				row += [
					"%s channel.%d channel.stereo" % (mode, ch+1)
				]
			self.controls += [row]
			for control in "mute","solo","reverb","pan","volume":
				row = []
				for ch in range(0, 16):
					desc = "%s channel.%d channel.%s" % (mode, ch+1, control)
					row += [desc]
				self.controls += [row]
		elif 'daw_monitor' in mode:
			self.header = [str(ch+1) for ch in range(0,10)] + ["DIRL","DIRR","DAWL","DAWR"]
			row = []
			for ch in range(0,10,2):
				row += ["%s channel.%d channel.stereo" % (mode, ch+1)]
			row += ["master.direct_monitor channel.stereo", "master.daw_monitor channel.stereo"]
			self.controls += [row]
			for control in "mute", "solo", "pan", "volume":
				row = []
				for ch in range(0, 10):
					desc = "%s channel.%d channel.%s" % (mode, ch+1, control)
					row += [desc]
				if control == 'volume':
					row += [
						"master.direct_monitor channel.1 channel.volume",
						"master.direct_monitor channel.1 channel.volume", #TODO
						"master.daw_monitor channel.1 channel.volume",
						"master.daw_monitor channel.2 channel.volume", #TODO
					]
				else:
					row += [None, None, None, None]
				self.controls += [row]
		elif 'preamp' in mode:
			self.header = [str(ch+1) for ch in range(0,16)]
			row = []
			for ch in range(0,12,2):
				row += ["%s preamp.%d preamp.stereo" % (mode, ch+1)]
			row += ["preamp preamp.line.13 preamp.line.stereo", "preamp preamp.line.15 preamp.line.stereo"]
			self.controls += [row]
			for control in "+48","lo-cut","phase","sens","gate","threshold","ratio":
				row = []
				for ch in range(0, 12):
					desc = "%s preamp.%d preamp.%s" % (mode, ch+1, control)
					row += [desc]
				if control == "sens":
					for ch in range(12,16):
						row += ["%s preamp.line.%d preamp.line.attenuation" % (mode, ch+1)]
				else:
					row += [None, None, None, None]
				self.controls += [row]
		self.setup_name_table()

	def setup_name_table(self):
		for row in self.controls:
			for control in row:
				if control is None:
					continue
				self.capture_view.add_name_to_table(control)

	def height(self):
		return len(self.controls)

	def width(self):
		return len(self.controls[self.cursor.y])

	def render(self):
		ret = ""
		selected_control = ""
		spacing = 5
		channels = max(len(row) for row in self.controls)
		ret += "\033[2K"
		ret += self.page
		ret += "\n\033[2K\n\033[2K"
		ret += ''.join(h.center(spacing) for h in self.header)
		ret += "\n\033[2K"
		for r, row in enumerate(self.controls):
			w = spacing*int(channels/len(row))
			for c, control in enumerate(row):
				if control is None:
					value = ' '
				else:
					value = self.memory.get_formatted( Capture.get_addr(control) )
				if self.cursor.x == c and self.cursor.y == r:
					ret += "\033[7m"
					selected_control = control
				else:
					ret += "\033[0m"
				ret += value.center(w)
			ret += "\033[0m\n\033[2K"

		ret += "\n\033[2K%s\n" % (selected_control or '')
		return ret

	def set_page(self, page):
		self.page = page
		self.setup_controls()
		self.cursor.y = min(self.cursor.y, self.height()-1)
		self.cursor.x = min(self.cursor.x, self.width()-1)

	def set_monitor(self, m):
		self.monitor = m
		if 'monitor.' in self.page:
			self.set_page(self.page[:-1] + self.monitor)

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

	def decrement_selected(self):
		row = self.cursor.y
		col = self.cursor.x
		control = self.controls[row][col]
		if control is None:
			return None, None
		addr = Capture.get_addr(control)
		data = self.memory.decrement(addr)
		return addr, data

	def increment_selected(self):
		row = self.cursor.y
		col = self.cursor.x
		control = self.controls[row][col]
		if control is None:
			return None, None
		addr = Capture.get_addr(control)
		data = self.memory.increment(addr)
		return addr, data

