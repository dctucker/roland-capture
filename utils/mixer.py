from roland import Capture, CaptureView, Memory

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
		self.set_page('input monitor.' + self.monitor)

	def setup_pages(self):
		self.pages = {
			"input monitor.a": [],
			"input monitor.b": [],
			"input monitor.c": [],
			"input monitor.d": [],
			"daw_monitor monitor.a": [],
			"daw_monitor monitor.b": [],
			"daw_monitor monitor.c": [],
			"daw_monitor monitor.d": [],
			"preamp": [],
			"line": [],
			"reverb": [],
		}
		for monitor in 'a','b','c','d':
			page = 'input monitor.' + monitor
			controls = []
			row = []
			for ch in range(0,16,2):
				row += [
					"%s channel.%d channel.stereo" % (page, ch+1)
				]
			controls += [row]
			for control in "mute","solo","reverb","pan","volume":
				row = []
				for ch in range(0, 16):
					desc = "%s channel.%d channel.%s" % (page, ch+1, control)
					row += [desc]
				controls += [row]
			self.pages[page] = controls

			page = "daw_monitor monitor." + monitor
			controls = []
			row = []
			for ch in range(0,10,2):
				row += ["%s channel.%d channel.stereo" % (page, ch+1)]
			row += ["master.direct_monitor channel.stereo", "master.daw_monitor channel.stereo"]
			controls += [row]
			for control in "mute", "solo", "pan", "volume":
				row = []
				for ch in range(0, 10):
					desc = "%s channel.%d channel.%s" % (page, ch+1, control)
					row += [desc]
				if control == 'volume':
					row += [
						"master.direct_monitor master.left master.volume",
						"master.direct_monitor master.right master.volume",
						"master.daw_monitor master.left master.volume",
						"master.daw_monitor master.right master.volume",
					]
				else:
					row += [None, None, None, None]
				controls += [row]
			self.pages[page] = controls

		page = "preamp"
		controls = []
		row = []
		for ch in range(0,12,2):
			row += ["%s preamp.%d preamp.stereo" % (page, ch+1)]
		controls += [row]
		for control in "+48","lo-cut","phase","sens","bypass","gate","threshold","ratio","attack","release","gain","knee":
			row = []
			for ch in range(0, 12):
				desc = "%s preamp.%d preamp.%s" % (page, ch+1, control)
				row += [desc]
			controls += [row]
		self.pages[page] = controls

		page = "line"
		controls = [
			["preamp preamp.line.13 preamp.line.stereo", "preamp preamp.line.15 preamp.line.stereo"],
			["preamp preamp.line.%d preamp.line.attenuation" % (ch+1) for ch in range(12,16)]
		]
		self.pages[page] = controls

		page = "reverb"
		controls = [
			['reverb reverb.type', None, None, None, None, None, 'master.reverb_return']
		]
		for control in 'pre_delay','time':
			row = [None]
			for verb in 'echo', 'room', 'small_hall', 'large_hall', 'plate':
				row += [
					'reverb reverb.%s reverb.%s' % (verb, control),
				]
			row += [None]
			controls += [row]
		self.pages[page] = controls

	def setup_controls(self):
		page = self.page
		controls = []
		self.spacing = 5
		if 'input' in page:
			self.header = [str(ch+1) for ch in range(0,16)]
		elif 'daw_monitor' in page:
			self.header = [str(ch+1) for ch in range(0,10)] + ["DIRL","DIRR","DAWL","DAWR"]
		elif 'preamp' in page:
			self.spacing = 7
			self.header = [str(ch+1) for ch in range(0,12)]
		elif 'line' in page:
			self.spacing = 7
			self.header = [str(ch+1) for ch in range(12,16)]
		elif 'reverb' in page:
			self.spacing = 10
			self.header = ['Type','Echo','Room','Sm. Hall','Lg. Hall','Plate','Return']
		self.controls = self.pages[page]

	def setup_name_table(self):
		for page, rows in self.pages.items():
			for row in rows:
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
		spacing = self.spacing
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

	def get_selected_control(self):
		row = self.cursor.y
		col = self.cursor.x
		return self.controls[row][col]

	def get_selected_addr(self):
		control = self.get_selected_control()
		if control is None:
			return None
		return Capture.get_addr(control)

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

