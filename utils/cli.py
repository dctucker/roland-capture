class MainCli():
	def __init__(self, controller, mixer):
		self.controller = controller
		self.mixer = mixer
		self.term = Term()
		self.quitting = False

	def present(self):
		try:
			while True:
				if self.quitting:
					break
		except KeyboardInterrupt:
			print('')

	def notify_control(self, control):
		self.update(False)

