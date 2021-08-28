from term import Term
import sys
import time

class MainTerminal():
	def __init__(self, controller, term):
		self.controller = controller
		self.term = term

	def on_keyboard(self, key):
		self.controller.remember_debug()
		self.controller.clear_debug()
		keyboard_map = {
			Term.KEY_DOWN:     ('down',),
			Term.KEY_UP:       ('up',),
			Term.KEY_LEFT:     ('left',),
			Term.KEY_RIGHT:    ('right',),
			('a','b','c','d'): ('monitor', key,),
			('-','_'):         ('decrement',),
			('=','+'):         ('increment',),
			('0',):            ('zero',),
			('p','\033[Z'):    ('preamp',),
			('l',):            ('line',),
			('i',):            ('inputs',),
			('o',):            ('outputs',),
			('\t',):           ('toggle_inputs_outputs',),
			('[',):            ('previous_monitor',),
			(']',):            ('next_monitor',),
			('r','v',):        ('reverb',),
			('y','P',):        ('patchbay',),
		}
		for keys, action in keyboard_map.items():
			if key in keys:
				if len(action) == 2:
					name, arg = action
					self.controller.call_app(name, arg)
				else:
					name = action[0]
					self.controller.call_app(name)
				return True
		self.controller.recall_debug()
		return False

	def refresh(self):
		self.term.clear()

	def present(self, app):
		if not hasattr(sys, 'ps1'): # non-interactive mode only
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
			except KeyboardInterrupt:
				print('')
			finally:
				term_width, term_height = self.term.size()
				print("\033[r\033["+str(term_height-1)+"HExit.", end="")
				if app.midi_in: app.midi_in.close_port()
				if app.midi_out: app.midi_out.close_port()
				del app.midi_in, app.api_in, app.midi_out, app.api_out

	def update(self, clear_debug=True):
		if self.term.blocked: return
		self.term.blocked = True
		rendered = self.controller.app.mixer.render()
		term_width, term_height = self.term.size()
		CL = Term.CLEAR_LINE
		debug_out = CL + "\n" + CL + "\033[20;%dr\033[%d;1H\033[38;5;24m%s\033[0m\033[r" % (term_height, term_height, self.controller.get_debug())
		self.term.display(rendered + debug_out)
		if clear_debug:
			self.controller.clear_debug()
		self.term.blocked = False
