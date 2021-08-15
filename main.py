#!/usr/bin/env python3

from sys import argv
from roland_sysex import *

#filename = 'mixer/mutes-all.txt'
#filename = 'mixer/some-solos.txt'
#filename = 'mixer/some-monitors.txt'
#filename = 'mixer/widgets.txt'
#filename = 'mixer/pan-1a.txt'
filename = argv[1]
packets = []
with open(filename) as f:
	for line in f.readlines():
		line = line.strip()
		packet = [int(line[i:i+2],16) for i in range(0, len(line), 2)]
		packets.extend([packet])

#from mixer.device_volume_1a import packets
process_packets(packets)

