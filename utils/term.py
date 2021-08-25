import os, sys, atexit

def is_nt():
	return os.name == 'nt'

try:
   import colorama
   colorama.init()
except:
   try:
       import tendo.ansiterm
   except:
       pass

if is_nt():
	import msvcrt
else:
	import termios, fcntl

class Term:
	KEY_LEFT  = ('\033[D', '\xe0K')
	KEY_RIGHT = ('\033[C', '\xe0M')
	KEY_DOWN  = ('\033[B', '\xe0P')
	KEY_UP    = ('\033[A',)

	def __init__(self):
		self.blocked = False
		if is_nt():
			self.init_nt()
		else:
			self.init_posix()

	def init_posix(self):
		os.system('clear')
		self.fd = sys.stdin.fileno()

		self.oldterm = termios.tcgetattr(self.fd)
		self.newattr = termios.tcgetattr(self.fd)
		self.newattr[3] = self.newattr[3] & ~termios.ICANON & ~termios.ECHO
		termios.tcsetattr(self.fd, termios.TCSANOW, self.newattr)

		self.oldflags = fcntl.fcntl(self.fd, fcntl.F_GETFL)
		fcntl.fcntl(self.fd, fcntl.F_SETFL, self.oldflags | os.O_NONBLOCK)

		atexit.register(self.cleanup_posix)

	def block(self):
		self.blocked = True
		if is_nt():
			return
		termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.oldterm)
		fcntl.fcntl(self.fd, fcntl.F_SETFL, self.oldflags)

	def unblock(self):
		self.blocked = False
		if is_nt():
			return
		termios.tcsetattr(self.fd, termios.TCSANOW, self.newattr)
		fcntl.fcntl(self.fd, fcntl.F_SETFL, self.oldflags | os.O_NONBLOCK)

	def cleanup_posix(self):
		self.block()
		print("\033[?25h")

	def init_nt(self):
		os.system('cls')
		atexit.register(self.cleanup_nt)

	def cleanup_nt(self):
		print("\033[?25h")
	
	def getch(self):
		ret = ""
		c = " "
		if is_nt():
			while c != "":
				if msvcrt.kbhit():
					c = msvcrt.getch()
					ret += ''.join(chr(x) for x in c)
				else:
					c = ""
		else:
			while c != "":
				c = sys.stdin.read(1)
				ret += c
		return ret

	def display(self, s):
		self.block()
		self.clear()
		print(str(s))
		self.unblock()

	def clear(self):
		print('\033[?25l\033[2J\033[H',end="")

	def size(self):
		return os.get_terminal_size()
