from roland import Capture, CaptureView, Memory, Bool

class Cursor(object):
	def __init__(self):
		self.x = 0
		self.y = 0
	def yx(self):
		return self.y, self.x

class Page(object):
	def __init__(self, mixer):
		self.mixer = mixer
	def get_controls(self):
		raise Exception("Not implemented")
	def get_spacing(self):
		return 5
	def get_labels(self):
		return None

class InputPage(Page):
	controls = "mute","solo","reverb","pan","volume"
	def __init__(self, mixer, monitor):
		Page.__init__(self, mixer)
		self.monitor = monitor

	def get_header(self):
		return [str(ch+1) for ch in range(0,16)]

	def get_labels(self):
		return ['Stereo'] + [ control.capitalize() for control in self.controls ]

	def get_controls(self):
		page = "input_monitor." + self.monitor
		controls = []
		row = []
		for ch in range(0,16,2):
			row += [
				"%s.channel.%d.stereo" % (page, ch+1)
			]
		controls += [row]
		for control in self.controls:
			row = []
			for ch in range(0, 16):
				desc = "%s.channel.%d.%s" % (page, ch+1, control)
				row += [desc]
			controls += [row]
		return controls


class OutputPage(Page):
	controls = "mute", "solo", "pan", "volume"
	def __init__(self, mixer, monitor):
		Page.__init__(self, mixer)
		self.monitor = monitor

	def get_labels(self):
		return ['Stereo'] + [ control.capitalize() for control in self.controls ]

	def get_header(self):
		return [str(ch+1) for ch in range(0,10)] + ["INL","INR","DAWL","DAWR"]

	def get_controls(self):
		page = "daw_monitor." + self.monitor
		controls = []
		row = []
		for ch in range(0,10,2):
			row += ["%s.channel.%d.stereo" % (page, ch+1)]
		row += ["master.direct_monitor.%s.left.stereo" % self.monitor, "master.daw_monitor.%s.left.stereo" % self.monitor]
		controls += [row]
		for control in self.controls:
			row = []
			for ch in range(0, 10):
				desc = "%s.channel.%d.%s" % (page, ch+1, control)
				row += [desc]
			if control == 'volume':
				row += [
					"master.direct_monitor.%s.left.volume" % self.monitor,
					"master.direct_monitor.%s.right.volume" % self.monitor,
					"master.daw_monitor.%s.left.volume" % self.monitor,
					"master.daw_monitor.%s.right.volume" % self.monitor,
				]
			else:
				row += [None, None, None, None]
			controls += [row]
		return controls

class PreampPage(Page):
	controls = "+48","lo-cut","phase","sens","bypass","gate","threshold","ratio","attack","release","gain","knee"
	def get_controls(self):
		page = "preamp"
		controls = []
		row = []
		for ch in range(0,12,2):
			row += ["%s.channel.%d.stereo" % (page, ch+1)]
		controls += [row]
		controls += [["%s.channel.%d.hi-z" % (page, ch+1) if ch < 2 else None for ch in range(0,12)]]
		for control in self.controls:
			row = []
			for ch in range(0, 12):
				desc = "%s.channel.%d.%s" % (page, ch+1, control)
				row += [desc]
			controls += [row]
		return controls

	def get_spacing(self):
		return 7

	def get_labels(self):
		return ['Stereo','Impedance','Phantom','Low Cut','Polarity','Sensitivity','Compressor','Gate','Threshold','Ratio','Attack','Release','Gain','Knee']

	def get_header(self):
		return [str(ch+1) for ch in range(0,12)]

class ReverbPage(Page):
	def get_controls(self):
		page = "reverb"
		controls = [
			['reverb.type', None, None, 'master.direct_monitor.a.reverb_return']
		]
		for verb in 'echo', 'room', 'small_hall', 'large_hall', 'plate':
			row = [None]
			for control in 'pre_delay','time':
				row += [
					'reverb.%s.%s' % (verb, control),
				]
			row += [None]
			controls += [row]
		return controls

	def get_labels(self):
		labels = ['','Echo','Room','Small Hall','Large Hall','Plate']
		verb = self.mixer.get_memory_value("reverb.type")
		v = verb.value if verb and verb.value else 0
		labels[v] = "\033[0;4m%s\033[24m" % labels[v]
		return labels

	def get_spacing(self):
		return 14

	def get_header(self):
		return ["Type", "Pre delay [ms]", "Time [s]", "Return"]

class LinePage(Page):
	def get_controls(self):
		return [
			["line.channel.13.stereo", "line.channel.15.stereo"],
			["line.channel.%d.attenuation" % (ch+1) for ch in range(12,16)]
		]
	def get_spacing(self):
		return 7

	def get_header(self):
		return [str(ch+1) for ch in range(12,16)]

	def get_labels(self):
		return ["Stereo","Attenuation"]

class Patchbay(Page):
	def get_controls(self):
		return [
			["patchbay.1-2"],
			["patchbay.3-4"],
			["patchbay.5-6"],
			["patchbay.7-8"],
			["patchbay.9-10"],
		]

	def get_labels(self):
		return [ "1-2", "3-4", "5-6", "7-8", "9-10" ]

	def get_spacing(self):
		return 14
	
	def get_header(self):
		return ['Source', 'Output']

class Mixer(object):
	def __init__(self):
		self.capture_view = CaptureView.instance()
		self.memory = Memory()
		self.cursor = Cursor()
		self.setup_pages()
		self.setup_name_table()
		self.monitor = 'a'
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
			"line"          : LinePage(self),
			"reverb"        : ReverbPage(self),
			"patchbay"      : Patchbay(self),
		}

	def setup_controls(self):
		page_name = self.page
		page = self.pages[page_name]
		controls = []
		self.spacing  = page.get_spacing()
		self.header   = page.get_header()
		self.controls = page.get_controls()

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

	def highlight(s, positions):
		ret = "\033[1;30m"
		for i,v in enumerate(s):
			if i in positions:
				ret += "\033[36m%s\033[1;30m" % v
			else:
				ret += v
		return ret

	def legend(self):
		return 'i o [abcd] l p r y'

	def get_highlight_for_page(self):
		legend = self.legend()
		mon = ord(self.monitor) - 97
		if 'input' in self.page:
			i = legend.find('i')
			m = legend.find('[') + mon + 1
			return (i, m)
		if 'daw_monitor' in self.page:
			o = legend.find('o')
			m = legend.find('[') + mon + 1
			return (o, m)
		if 'patchbay' in self.page:
			return (legend.find('y'),)
		return (legend.find(self.page[0]),)

	def render(self):
		ret = ""
		selected_control = ""
		spacing = self.spacing
		channels = max(len(row) for row in self.controls)
		ret += "\033[2K"
		ret += "%s\033[0m" % Mixer.highlight(self.legend(), self.get_highlight_for_page())
		ret += " \033[4m %s \033[0m" % self.page
		ret += "\n\033[2K\n\033[2K"
		ret += '\033[1;30m'
		ret += ''.join(h.center(spacing) if i < channels else h for i,h in enumerate(self.header))
		ret += '\033[0m'
		ret += "\n\033[2K"
		labels = self.pages[self.page].get_labels()
		for r, row in enumerate(self.controls):
			w = spacing*int(channels/len(row))
			for c, control in enumerate(row):
				active = False
				if control is None:
					formatted = ' '
				else:
					addr = Capture.get_addr(control)
					value = self.memory.get_value(addr)
					if type(value) is Bool and value.value:
						active = True
					formatted = self.memory.get_formatted(addr)
				if self.cursor.x == c and self.cursor.y == r:
					ret += "\033[7m"
					selected_control = control
				else:
					ret += "\033[0m"
				if active:
					ret += "\033[1m"
				ret += formatted.center(w)
			ret += "\033[0m"
			if labels is not None and r < len(labels):
				ret += " \033[1;30m" + labels[r] + "\033[0m" #.center(spacing)
			ret += "\n\033[2K"

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

class TerminalMixer(Mixer):
	pass

class GraphicalMixer(Mixer):
	pass

