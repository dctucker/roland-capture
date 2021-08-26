#!/usr/bin/env python3

import jinja2
from jinja2 import Template, Environment
from roland import Capture, ReverbType
from struct import Struct
from math import log, ceil

env = Environment(lstrip_blocks = True, trim_blocks = True)

mem = Capture.memory_map

def print_section(section, section_name):
	print("# %s\n" % section_name)
	if type(section) is dict:
		for group_name,group in section.items():
			if group_name != 0:
				print("## %s\n" % group_name)


def render_bytes(long):
	return ' '.join("{:02x}".format(x) for x in Struct('>I').pack(*[long]))

def get_addr(control):
	try:
		return Capture.get_addr(control)
	except:
		return None


#for section_name,section in mem.items():
#	print_section(section, section_name)

#reverb = mem['reverb']
#print_section(reverb, 'Reverb')


def reverb_page():
	reverb = Capture.get_subtree('reverb').keys()
	params = Capture.get_subtree('reverb.echo')

	tmpl = env.from_string("""
Reverb is organized so that the timing parameters for each type of reverb are independent.

Base address: {{ hex(base) }} H

Reverb types:

{% for value, verb in types %}
{% set offset = get_addr('reverb.'+verb) %}
{{ "%02x" % value }}: {{ verb.ljust(10) }}  {{ "memory offset: " + hex(offset) if offset else "" }}
{% endfor %}

Parameters:

{% for param, offset in params %}
{% if param != 0 %}
{{ "%02x" % offset }}: {{ str(param).ljust(10) }}
{% endif %}
{% endfor %}
	""")
	return tmpl.render(
		hex=render_bytes,
		types = enumerate(ReverbType().values()),
		params = params.items(),
		str = str,
		get_addr = get_addr,
		base = Capture.get_addr('reverb')
	)

#print(reverb_page())


def print_memory_tree(title, tree, base = 0):
	def print_memory_item(k,v):
		s = ""
		s += render_bytes(v)
		s += "\t"
		s += str(k)
		print(s)
	for k,v in tree.items():
		if k == 0:
			base += v
			print_memory_item(title, v)
			continue
		if type(v) == dict:
			print_memory_tree(k, v, base)
			continue

def print_addr_description(desc, mask):
	addr = Capture.get_addr(desc)
	b = render_bytes(addr)
	s = desc.split('.')
	title = desc #s[-1]
	indent = "" # "  " * (len(s)-1)
	print("%s\t%s%s"% (b, indent, title))


def get_mask(tree, quant=4):
	if type(tree) is not dict:
		return None
	base = tree[0] if 0 in tree else 0
	def gmask(tree):
		mask = 0
		for k,v in tree.items():
			if type(v) is dict:
				mask |= gmask(v)
				continue
			mask |= v
		return mask
	mask = gmask(tree) & ~base
	#print(hex(mask))
	mask = ( 1 << quant*ceil(log(mask, 2)/quant) ) - 1
	return mask

def nonzero(arr):
	return [e for e in arr if e != 0]

def prefixed(desc):
	return ['%s.%s' % (desc, x) for x in nonzero(Capture.get_subtree(desc).keys())]

#reverb = Capture.get_subtree('reverb')
#print_memory_tree('reverb', reverb)

#reverb = Capture.get_subtree('input_monitor.a')
#mask = get_mask(reverb)
#print(hex(mask))

def print_memory_table(elements):
	masks = [0,0,0,0,0]
	mask = None

	for a in elements:
		dots = a.count('.')
		mask = get_mask( Capture.get_subtree(a) )
		if mask:
			masks[dots] = mask
		else:
			mask = (masks[dots-1] ^ 0xffffffff) #or (pmask ^ 0xffffffff)
			#mask = None
		print("%08x" % mask if mask is not None else "-\t", end=" ")
		print_addr_description(a, mask)

#elements = ['reverb'] + prefixed('reverb') + prefixed('reverb.echo')
#elements = ['input_monitor'] + prefixed('input_monitor') + prefixed('input_monitor.a.channel') + prefixed('input_monitor.a.channel.1')
#elements = ['daw_monitor'] + prefixed('daw_monitor') + prefixed('daw_monitor.a.channel') + prefixed('daw_monitor.a.channel.1')
#elements = ['preamp'] + prefixed('preamp') + prefixed('preamp.channel') + prefixed('preamp.channel.1') + ['line'] + prefixed('line.channel') + prefixed('line.channel.13')
#elements = ['master.direct_monitor'] + prefixed('master.direct_monitor') + prefixed('master.direct_monitor.a') + prefixed('master.direct_monitor.a.left')
#print_memory_table(elements)


mem = ["%s\t%s" % (render_bytes(Capture.get_addr(a)), a) for a in Capture.memory_names() ]
mem.sort()
for m in mem:
	print(m)
