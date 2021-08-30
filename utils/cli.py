from term import Term

class MainCli():
	def __init__(self, controller, mixer):
		self.controller = controller
		self.mixer = mixer
		self.term = None
		self.quitting = True

	def present(self):
		try:
			while True:
				if self.quitting:
					break
		except KeyboardInterrupt:
			print('')

	def notify_control(self, control):
		self.update(False)

	def refresh(self):
		pass

	def block(self):
		pass
	def unblock(self):
		pass

	def quit(self):
		self.quitting = True
