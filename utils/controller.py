class Controller():
	def __init__(self, app):
		self.app = app

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
		app.mixer.set_monitor(monitor)
		app.load_mixer_values()
	def decrement(app):
		addr, data = app.mixer.decrement_selected()
		app.set_mixer_value(addr, data)
	def increment(app):
		addr, data = app.mixer.increment_selected()
		app.set_mixer_value(addr, data)
	def zero(app):
		addr, data = app.mixer.zero_selected()
		app.set_mixer_value(addr, data)
	def preamp(app):
		app.set_page("preamp")
	def line(app):
		app.set_page("line")
	def inputs(app):
		app.set_page("input_monitor." + app.mixer.monitor)
	def outputs(app):
		app.set_page("daw_monitor." + app.mixer.monitor)
	def toggle_inputs_outputs(app):
		if 'input' in app.mixer.page:
			app.set_page("daw_monitor." + app.mixer.monitor)
		else:
			app.set_page("input_monitor." + app.mixer.monitor)
	def previous_monitor(app):
		if app.mixer.monitor > 'a':
			app.mixer.set_monitor(chr(ord(app.mixer.monitor)-1))
			app.load_mixer_values()
	def next_monitor(app):
		if app.mixer.monitor < 'd':
			app.mixer.set_monitor(chr(ord(app.mixer.monitor)+1))
			app.load_mixer_values()
	def reverb(app):
		app.set_page('reverb')
	def patchbay(app):
		app.set_page('patchbay')

