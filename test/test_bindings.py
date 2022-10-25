#!/usr/bin/env python3

from bindings import capmix as cm
mix = cm.capmix
mix.set_model(4)
x = mix.parse_addr("input_monitor.a.channel.1.volume")
print("0x%08x" % x)
def listener(event):
	print(event)
mix.connect(listener)
ev = mix.get(x)
print(ev.value())
