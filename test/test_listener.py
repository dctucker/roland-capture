#!/usr/bin/env python3

from bindings.capmix import capmix, EVENT

def main():
	capmix.set_model(4)

	def listener(event):
		#value = capmix.format_type(event.type(), event.unpacked)
		value = event.value()
		addr = capmix.format_addr(event.addr)
		print("addr=%s type=%s v=%s" % (addr, event.type_name(), value))
		#print("type=%x" % event.type_info)

	ok = capmix.connect(listener)
	try:
		while ok:
			x = capmix.listen()
	except KeyboardInterrupt:
		pass
	capmix.disconnect()
	print("Done.")

if __name__ == '__main__':
	main()
	#t = capmix.type(13)
	#print(t.enum_names())

