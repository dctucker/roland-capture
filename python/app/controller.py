
class BaseController():
	def __init__(self, app=None):
		self.app = app
		self.cursors = None

	def call_app(app, func, *args, **kwargs):
		raise Exception("BaseController not implemented")

	def setup_keyboard_map(self):
		UP, DOWN, LEFT, RIGHT = self.cursors
		self.keyboard_map = {
			UP:             ('up',),
			DOWN:           ('down',),
			LEFT:           ('left',),
			RIGHT:          ('right',),
			('a',):         ('monitor', 'a',),
			('b',):         ('monitor', 'b',),
			('c',):         ('monitor', 'c',),
			('d',):         ('monitor', 'd',),
			('-','_'):      ('decrement',),
			('=','+'):      ('increment',),
			('z',):         ('zero',),
			('p','\033[Z'): ('preamp',),
			('s','k'):      ('compressor',),
			('l',):         ('line',),
			('i',):         ('inputs',),
			('o',):         ('outputs',),
			('\t',):        ('toggle_inputs_outputs',),
			('[',):         ('previous_subpage',),
			(']',):         ('next_subpage',),
			('r','v',):     ('reverb',),
			('y','P',):     ('patchbay',),
			('q',):         ('quit',),
			('n',):         ('channel',),
			('1',):         ('channel',1),
			('2',):         ('channel',2),
			('3',):         ('channel',3),
			('4',):         ('channel',4),
			('5',):         ('channel',5),
			('6',):         ('channel',6),
			('7',):         ('channel',7),
			('8',):         ('channel',8),
			('9',):         ('channel',9),
			('0',):         ('channel',10),
			('!',):         ('channel',11),
			('@',):         ('channel',12),
			('#',):         ('channel',13),
			('$',):         ('channel',14),
			('%',):         ('channel',15),
			('^',):         ('channel',16),
		}

	def on_keyboard(self, pressed):
		for keys, action in self.keyboard_map.items():
			for press in pressed:
				if press in keys:
					if len(action) == 2:
						name, arg = action
						self.call_app(name, arg)
						self.app.debug("action%s" % str(action))
					else:
						name = action[0]
						self.call_app(name)
						self.app.debug("action%s" % str(action))
					return True
		return False

class NullController(BaseController):
	def call_app(self, func, *args, **kwargs):
		if func == 'quit':
			self.app.quit()

class Controller(BaseController):
	def call_app(self, func, *args, **kwargs):
		f = getattr(Controller, func)
		return f(self.app, *args, **kwargs)

	def down(app):
		app.mixer.cursor_down()
	def up(app):
		app.mixer.cursor_up()
	def left(app):
		app.mixer.cursor_left()
	def right(app):
		app.mixer.cursor_right()
	def monitor(app, monitor):
		app.set_monitor(monitor)
	def decrement(app):
		addr, data = app.mixer.decrement_selected()
		app.set_mixer_value(addr, data)
	def increment(app):
		addr, data = app.mixer.increment_selected()
		app.set_mixer_value(addr, data)
	def assign(app, arg):
		name, value = arg
		addr, data = app.mixer.set_memory_value(name, value)
		app.set_mixer_value(addr, data)
	def zero(app):
		addr, data = app.mixer.zero_selected()
		app.set_mixer_value(addr, data)
	def preamp(app):
		app.set_page("preamp")
	def compressor(app):
		app.set_page("compressor")
	def line(app):
		app.set_page("line")
	def set_page(app, page):
		app.set_page(page)
	def inputs(app):
		app.set_page("input_monitor.%s" % app.mixer.monitor)
	def outputs(app):
		app.set_page("daw_monitor.%s" % app.mixer.monitor)
	def toggle_inputs_outputs(app):
		if 'input' in app.mixer.page_name:
			app.set_page("daw_monitor.%s" % app.mixer.monitor)
		else:
			app.set_page("input_monitor.%s" % app.mixer.monitor)
	def previous_subpage(app):
		if '_monitor.' in app.mixer.page_name:
			if app.mixer.monitor > 'a':
				app.set_monitor(chr(ord(app.mixer.monitor)-1))
		elif 'channel.' in app.mixer.page_name:
			if app.mixer.channel > 1:
				app.set_channel(app.mixer.channel - 1)
	def next_subpage(app):
		if '_monitor.' in app.mixer.page_name:
			if app.mixer.monitor < 'd':
				app.set_monitor(chr(ord(app.mixer.monitor)+1))
		elif 'channel.' in app.mixer.page_name:
			if app.mixer.channel < 16:
				app.set_channel(app.mixer.channel + 1)
	def reverb(app):
		app.set_page('reverb')
	def patchbay(app):
		app.set_page('patchbay')
	def channel(app, ch=None):
		if ch is None:
			app.set_page("channel.%d" % app.mixer.channel)
		else:
			app.set_channel(ch)
	def quit(app):
		app.quit()
