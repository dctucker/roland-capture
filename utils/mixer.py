from roland import Capture, CaptureView, Memory, Bool

class Cursor(object):
	def __init__(self):
		self.x = 0
		self.y = 0
	def yx(self):
		return self.y, self.x

class Page(object):
	spacing = 5
	def __init__(self, mixer):
		self.mixer = mixer
	def get_controls(self):
		raise Exception("Not implemented")
	def get_header(self):
		return ['']
	def get_labels(self):
		return ['']

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
	spacing = 7
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

	def get_labels(self):
		return ['Stereo','Impedance','Phantom','Low Cut','Polarity','Sensitivity','Compressor','Gate','Threshold','Ratio','Attack','Release','Gain','Knee']

	def get_header(self):
		return [str(ch+1) for ch in range(0,12)]

class ReverbPage(Page):
	spacing = 14
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
		labels[v] = "*%s" % labels[v]
		return labels

	def get_header(self):
		return ["Type", "Pre delay [ms]", "Time [s]", "Return"]

class LinePage(Page):
	spacing = 7
	def get_controls(self):
		return [
			["line.channel.13.stereo", "line.channel.15.stereo"],
			["line.channel.%d.attenuation" % (ch+1) for ch in range(12,16)]
		]
	def get_header(self):
		return [str(ch+1) for ch in range(12,16)]

	def get_labels(self):
		return ["Stereo","Attenuation"]

class Patchbay(Page):
	spacing = 14
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

	def get_header(self):
		return ['Source', 'Output']

class ChannelPage(Page):
	spacing = 14
	def __init__(self, mixer, channel):
		Page.__init__(self, mixer)
		self.channel = channel
	def get_controls(self):
		preamp_controls = PreampPage.controls
		line_controls = ['attenuation']
		input_controls = InputPage.controls
		ch = self.channel

		controls = [
			[None, "bypass", "gate", "threshold", "ratio",],
			[None, "attack", "release", "knee", "gain",],
			["+48",],
			["hi-z" if ch <= 2 else None,],
			["lo-cut",],
			["phase",],
			["sens",],
		]
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				if control:
					controls[i][j] = "preamp.channel.%d.%s" % (ch, control)
		
		for i, control in enumerate(input_controls):
			row = []
			for mon in ['a','b','c','d']:
				row += ["input_monitor.%s.channel.%d.%s" % (mon, ch, control)]
			controls[i+2] += row
		return controls
	def get_header(self):
		return ['A','B','C','D']
	def get_labels(self):
		return [
			'Compressor',
			'Timing',
			'Mute','Solo','Reverb','Pan','Volume',
		]

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
			"channel.1"     : ChannelPage(self, 1),
		}

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
		self.set_page("channel.%d" % ch)

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
		data = self.memory.set_value(addr, value.value)
		return addr, data
