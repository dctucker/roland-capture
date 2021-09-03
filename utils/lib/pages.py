
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
	controls = "mute", "solo", None, "pan", "volume"
	def __init__(self, mixer, monitor):
		Page.__init__(self, mixer)
		self.monitor = monitor

	def get_labels(self):
		return ['Stereo'] + [ control.capitalize() if control else None for control in self.controls ]

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
				if control is None:
					desc = None
				else:
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
		controls[3][11] = "master.direct_monitor.a.reverb_return"
		return controls

class PreampPage(Page):
	controls = "+48","lo-cut","phase","sens","bypass","gate"
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
		return ['Stereo','Impedance','Phantom','Low Cut','Polarity','Sensitivity']

	def get_header(self):
		return [str(ch+1) for ch in range(0,12)]

class CompressorPage(Page):
	controls = "bypass","gate","threshold","ratio","attack","release","gain","knee"
	spacing = 7
	def get_controls(self):
		page = "preamp"
		controls = []
		row = []
		for control in self.controls:
			row = []
			for ch in range(0, 12):
				desc = "%s.channel.%d.%s" % (page, ch+1, control)
				row += [desc]
			controls += [row]
		return controls

	def get_labels(self):
		return ['Compressor','Gate','Threshold','Ratio','Attack','Release','Gain','Knee']

	def get_header(self):
		return [str(ch+1) for ch in range(0,12)]

class ReverbPage(Page):
	spacing = 15
	def get_controls(self):
		page = "reverb"
		controls = [
			['reverb.type', None, 'master.direct_monitor.a.reverb_return']
		]
		for verb in 'echo', 'room', 'small_hall', 'large_hall', 'plate':
			row = [None]
			for control in 'pre_delay','time':
				row += [
					'reverb.%s.%s' % (verb, control),
				]
			#row += [None]
			controls += [row]
		return controls

	def get_labels(self):
		labels = ['','Echo','Room','Small Hall','Large Hall','Plate', 'Return Volume']
		verb = self.mixer.get_memory_value("reverb.type")
		v = verb.value if verb and verb.value else 0
		labels[v] = "*%s" % labels[v]
		return labels

	def get_header(self):
		return ["Type", "Pre delay [ms]", "Time [s]"]

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
			[None],
			["patchbay.1-2"],
			["patchbay.3-4"],
			["patchbay.5-6"],
			["patchbay.7-8"],
			["patchbay.9-10"],
		]

	def get_labels(self):
		return [ "Output", "1-2", "3-4", "5-6", "7-8", "9-10" ]

	def get_header(self):
		return ['Source']

class ChannelPage(Page):
	spacing = 7
	def __init__(self, mixer, channel):
		Page.__init__(self, mixer)
		self.channel = channel
	def get_controls(self):
		preamp_controls = PreampPage.controls
		line_controls = ['attenuation']
		input_controls = InputPage.controls
		ch = self.channel

		if ch > 12:
			prefix = "line"
			controls = [
				[ None,   None, None,   None, None,      None,      None],
				[ None,  None,  None,   None,     None,      None,      None],
				[ None, None,     None,   None,  None,  None, None],
				[ None,   None,     None, None,   None, None,    None],
				[ "attenuation",   None,     None,   None,     None,      None,      None],
			]
		else:
			prefix = "preamp"
			hi_z = "hi-z" if ch <= 2 else None
			controls = [
				[ hi_z,   "lo-cut", None,   "bypass", None,      None,      None],
				[ "+48",  "phase",  None,   None,     None,      None,      None],
				[ "sens", None,     None,   "ratio",  "attack",  "threshold", None],
				[ None,   None,     "gate", "knee",   "release", "gain",    None],
				[ None,   None,     None,   None,     None,      None,      None],
			]
		for i, row in enumerate(controls):
			for j, control in enumerate(row):
				if control:
					controls[i][j] = "%s.channel.%d.%s" % (prefix, ch, control)
		
		for i, control in enumerate(input_controls):
			row = []
			for mon in ['a','b','c','d']:
				row += ["input_monitor.%s.channel.%d.%s" % (mon, ch, control)]
			controls[i] += row
		return controls
	def get_header(self):
		return ['Preamp','','','Compressor','','','','A','B','C','D']
	def get_labels(self):
		return [
			'Mute','Solo','Reverb','Pan','Volume',
		]

