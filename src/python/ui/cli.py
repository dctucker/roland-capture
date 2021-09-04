import time
from interface.term import Term
from lib.roland import Capture, ValueFactory, Bool

class MainCli():
	def __init__(self, controller, mixer):
		self.controller = controller
		self.mixer = mixer
		self.term = None
		self.quitting = True
		self.debug_string = ""

	def present(self):
		try:
			while True:
				if self.quitting:
					break
		except KeyboardInterrupt:
			print('')

	def notify_control(self, control):
		self.update(False)

	def update(self, clear_debug=True):
		if len(self.debug_string) > 0:
			print(self.debug_string)

	def debug(self, message, end=""):
		self.debug_string += Term.CLEAR_LINE + message + end

	def refresh(self): pass

	def block(self): pass
	def unblock(self): pass

	def parse_value(self, name, new_value):
		value = ValueFactory.get_class(name)(new_value)
		if isinstance(value, Bool):
			if new_value.lower() in ('on','1','true'):
				value.value = True
			elif new_value.lower() in ('off','0','false'):
				value.value = False
			else:
				return None
		else:
			value.value = int(new_value)
		return value

	def wait_for_value(self, control):
		timeout = 10
		waited = 0
		delay = 0.05

		addr = Capture.get_addr(control)
		while waited < timeout:
			value = self.mixer.memory.get_value(addr)
			if value.value is not None:
				break
			time.sleep(delay)
			waited += delay
		return value

	def quit(self):
		self.quitting = True
