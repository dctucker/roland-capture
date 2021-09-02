from term import Term
from mixer import Mixer
from roland import Capture, Bool
import sys
import time

class MainTerminal():
	def __init__(self, controller, mixer):
		self.debug_string = ""
		self.controller = controller
		self.controller.cursors = (
			Term.KEY_UP,
			Term.KEY_DOWN,
			Term.KEY_LEFT,
			Term.KEY_RIGHT,
		)
		self.controller.setup_keyboard_map()
		self.mixer = mixer
		self.term = Term()
		self.quitting = False

	def block(self):
		self.term.blocked = True
	def unblock(self):
		self.term.blocked = False

	def on_keyboard(self, key):
		self.remember_debug()
		self.clear_debug()
		ret = self.controller.on_keyboard((key,))
		self.recall_debug()
		return ret

	def refresh(self):
		self.term.clear()

	def present(self):
		self.term.clear()
		self.update(False)
		try:
			while True:
				key = self.term.getch()
				if key == "":
					time.sleep(0.01)
					continue
				if self.on_keyboard(key):
					self.update(False)
				elif key in ('q',"\033"):
					break
				if self.quitting:
					break
		except KeyboardInterrupt:
			print('')
		finally:
			term_width, term_height = self.term.size()
			print("\033[r\033["+str(term_height-1)+"HExit.", end="")

	def notify_control(self, control):
		self.update(False)

	def update(self, clear_debug=True):
		if self.term.blocked: return
		self.block()
		rendered = self.controller.app.mixer.render()
		term_width, term_height = self.term.size()
		CL = Term.CLEAR_LINE
		debug_out = CL + "\n" + CL
		debug_out += "\033[20;%dr\033[%d;1H" % (term_height, term_height)
		debug_out += "\033[38;5;24m%s" % self.debug_string
		debug_out += "\033[0m\033[r"
		self.term.display(rendered + debug_out)
		if clear_debug:
			self.controller.clear_debug()
		self.unblock()

	def debug(self, message, end=""):
		self.debug_string += Term.CLEAR_LINE + message + end

	def remember_debug(self):
		self.saved_debug_string = self.debug_string

	def recall_debug(self):
		self.debug_string = self.saved_debug_string

	def clear_debug(self):
		self.debug_string = ""

	def quit(self):
		self.quitting = True

class TerminalMixer(Mixer):
	def render(self):
		ret = ""
		selected_control = ""
		spacing = self.page.spacing
		max_width = max(len(row) for row in self.controls)
		ret += "\033[2K"
		ret += "%s\033[0m" % TerminalMixer.highlight(self.legend(), self.get_highlight_for_page())
		ret += " \033[4m %s \033[0m" % self.page_name
		ret += "\n\033[2K\n\033[2K"
		ret += '\033[1;30m'
		ret += ''.join(h[0:spacing].center(spacing) if i < max_width else h for i,h in enumerate(self.header))
		ret += '\033[0m'
		ret += "\n\033[2K"
		labels = self.page.get_labels()
		for r, row in enumerate(self.controls):
			w = spacing*int(max_width/len(row))
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
				label = labels[r]
				if len(label) > 0 and label[0] == "*":
					ret += " \033[0;4m%s\033[24m" % label[1:]
				else:
					ret += " \033[1;30m%s\033[0m" % label
			ret += "\n\033[2K"

		ret += "\n\033[2K%s\n" % (selected_control or '')
		return ret

	def highlight(s, positions):
		ret = "\033[1;30m"
		for i,v in enumerate(s):
			if i in positions:
				ret += "\033[36m%s\033[1;30m" % v
			else:
				ret += v
		return ret

	def get_highlight_for_page(self):
		legend = self.legend()
		mon = ord(self.monitor) - 97
		if 'input' in self.page_name:
			i = legend.find('i')
			m = legend.find('[') + mon + 1
			return (i, m)
		if 'daw_monitor' in self.page_name:
			o = legend.find('o')
			m = legend.find('[') + mon + 1
			return (o, m)
		if 'patchbay' in self.page_name:
			return (legend.find('y'),)
		return (legend.find(self.page_name[0]),)

	def legend(self):
		return 'i o [abcd] k l p r y'

