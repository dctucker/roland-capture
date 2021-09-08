
def chunks(l, n):
	# looping till length l
	for i in range(0, len(l), n): 
		yield l[i:i + n]

def process_chunk(p):
	if p[0] == 0x14:
		return p[1:4]
	elif p[0] == 0x15:
		return [p[1]]
	elif p[0] == 0x16:
		return [p[1], p[2]]
	elif p[0] == 0x17:
		return [p[1], p[2], p[3]]
	else:
		print("unknown command: 0x%02x" % p[0])
		return ["a"]

def process_sysex(s):
	dev = s[2]
	mod = s[3:6]
	cmd = s[6]
	mem = s[7:11]
	data = s[11:-2]
	csum = s[-2]
	print("dev: %02x, mod: %02x%02x%02x, cmd: %02x, addr: %02x%02x%02x%02x, data: %s" % (dev, mod[0], mod[1], mod[2], cmd, mem[0], mem[1], mem[2], mem[3], ' '.join("%02x" % b for b in data)))

def process_packets(packets):
	sysex_chunk = []
	for p in packets:
		if len(p) < 4:
			continue

		for ch in chunks(p,4):
			sysex = process_chunk(ch)
			sysex_chunk.extend(sysex)
			if sysex_chunk[-1] == 0xf7:
				#print(sysex_chunk)
				process_sysex(sysex_chunk)
				sysex_chunk = []
