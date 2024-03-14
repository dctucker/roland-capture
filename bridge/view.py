import os
from time import strftime

logged = False

def log(*msg):
	global logged
	now = strftime("%H:%M:%S")
	logged = True
	print("\033[1;30m", end='')
	print(now, end=' ')
	print(*msg, end='')
	print("\033[0m")

class View:
	labels = [
		'  Mic ',
		'Guitar',
		'Deluge',
		'Opsix',
		'Phatty',
		' SM10 ',
		'Circuit',
		' JUNO ',
	]

	eighths = {
		0: " ",
		1: "\u258f",
		2: "\u258e",
		3: "\u258d",
		4: "\u258c",
		5: "\u258b",
		6: "\u258a",
		7: "\u2589",
		8: "\u2588",
	}

	enable_dimming = False

	def __init__(self, model):
		self.compact = True
		self.model = model
		size = lambda: None
		size.columns = 99
		size.lines = 27
		self.size = size

		self.dimmed = False
		self.brightness = 48

	@property
	def logged(self):
		global logged
		return logged

	@logged.setter
	def logged(self, l):
		global logged
		logged = l

	def backlight(self, value=None):
		if value is not None:
			b = value * 2
			if b > 15:
				self.brightness = value * 2
		else:
			b = self.brightness
		with open('/sys/class/backlight/10-0045/brightness','w') as f:
			f.write("%d" % b)
			log('backlight: %d' % b)

	def dim(self, dark=True, force=False):
		if self.enable_dimming or force:
			if dark:
				if not self.dimmed:
					self.backlight(0)
				self.model.reset()
			else:
				if self.dimmed:
					self.backlight()
			self.dimmed = dark

	def pan_graphic(self, pan, width=4):
		eighths = self.eighths
		p = int(str(pan).replace('R','').replace('L','-').replace('C','0'))
		x = p * (width / 2) / 100.0
		i = int(abs(x))
		f = int(abs(x * 8)) % 8
		w2 = int(width / 2)
		if p < 0:
			if p == -100:
				return '\u2588\u2588%s ' % (eighths[4])
			return (' ' * max(0, (w2 - i) - 1)) + "\033[7m" + eighths[8-f] + "\033[27m" + ("\u2588" * i) + eighths[4] + (' ' * (w2-1))
		elif p > 0:
			if p == 100:
				return '   \033[7m' + eighths[4] + '\033[27m' + (eighths[8] * 2)
			return '   \033[7m' + eighths[4] + '\033[27m' + (eighths[8] * i) + eighths[f] + (' ' * max(0, (w2 - i) - 1))
		return '  \u2503  '

	def print_pan(self, ch):
		row = 5 if not self.compact else 3
		col = 6 * (ch-1) + 3
		print("\033[?25l\033[%d;%df" % (row, col), end='')
		pan = self.model.pans[ch]
		graphic = self.pan_graphic(pan)
		if ch % 2 == 1 or str(pan) == 'C':
			print(" ", end='')
			#graphic = " " + graphic
		print("\033[33m%s" % graphic, end='')

	#size = os.get_terminal_size()
	def cursor_to_log(self):
		height = 17 if not self.compact else 9
		print("\033[%d;%dr\033[%d;1f\033[?25h" % (height, self.size.lines, self.size.lines-1))

	def print_monitor_mutes(self):
		compact = self.compact
		#print("\033[r",end='')
		stereo = self.model.stereo
		solos = self.model.solos
		mutes = self.model.mutes
		pans = self.model.pans

		print("\033[?25l\033[H\033[2K    ", end='')
		for ch in range(0, 16):
			s = ' /' if stereo[ch+1] else '  '
			if ch % 2 == 0:
				k = str(ch + 1).rjust(2) + ' '
			else:
				k = ' ' + str(ch + 1).ljust(2)
			print("%s%s" % (k, s), end='')
			if ch < 15:
				print(" ", end='')
		print()
		if not compact: print("\033[2K")
		print("\033[2K ", end='')
		for ch in range(0, 16, 2):
			if stereo[ch+1]:
				st = "STEREO"
				sc = "\033[7;36m"
			else:
				st = ""
				sc = "\033[7;1;30m"
			print("   %s%7s  \033[0m" % (sc, st), end='')
		print()
		if not compact: print("\033[2K")
		print("\033[2K  ", end='')
		for ch in range(0, 16):
			pan = pans[ch+1]
			graphic = self.pan_graphic(pan)
			print(" \033[33m%s" % graphic, end='')
		print("\033[0m")

		if not compact: print("\033[2K")
		print("\033[2K", end='')
		print(" S ", end='')
		for ch in range(0, 16):
			value = solos[ch+1]['a']
			if value == 0:
				msg = "     "
				color = "\033[7;2;30m"
			else:
				msg = "  S  "
				color = "\033[7;2;31m"
			print("%s%5s\033[0m " % (color,msg), end='')
		print()

		for mon in self.model.monitors:
			if not compact: print("\033[2K")
			print("\033[2K", end='')
			print("%2s " % (mon.upper()), end='')
			for ch in range(0, 16):
				value = mutes[ch+1][mon]
				if value == 0:
					msg = "     "
					color = "\033[7;2;30m"
				else:
					msg = "  M  "
					color = "\033[7;2;31m"
				print("%s%5s\033[0m " % (color,msg), end='')
			print()
		if not compact: print("\033[2K")
		print("\033[2K ",end='')
		for label in self.labels:
			print("  %8s  " % label, end='')
		print()
		if not compact: print("\033[2K")
		#print("\0338", end='')

		self.cursor_to_log()
		self.logged = False
