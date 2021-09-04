modes:
	multiple monitor
		on or off
driver settings:
	sample rate:
		44100, 48000, 96000, 192000
	audio buffer size
		[44100/48000]:
			1,2,3,4,5,6,7,8,9,10
			1 = 32 samples, favors real time
			2 = 48 samples
			3 = 64
			4 = 96
			5 = 128
			6 = 256 samples, balanced
			7 = 384 samples
			8 = 512
			9 = 1024
			10 = 2048 samples, favors stability
		[96000]:
			audio buffer double size
		[192000]:
			audio buffer quadruple size
	ASIO direct monitor
	Low Latency Mode
	VS Expand :
	Match with the ASIO sample rate

device settings:
	auto-sens
		auto or manual
		margin [dB]
			0 to 12 in increments of 1
	sync
		auto or internal
	input selector (input 15/16)
		auto or analog
	solo
		dim solo [dB]
			off, -6, -12, -18
	stereo-link
		individual or paired
patchbay:
	output 1-2, 3-4, 5-6, 7-8, 9-10
		monitor a,b,c,d or wave out 1-2,3-4,5-6,7-8,9-10
	presets
		init
			A, 3-4, 5-6, 7-8, 9-10
		multiple monitor
			A, B, C, D, A
		direct wave out
			1-2, 3-4, 5-6, 7-8, 9-10
preamp:
	channels 1-12:
		stereo
		+48V
		Hi-Z
		Lo-Cut
		Phase
		sens
			0.0 to 58.0 in increments of 0.5
		compressor:
			bypass
			gate [dB]
				-inf, -69.0 to -20.0 in increments of 1.0
			threshold [dB]
				-40.0 to 0.0 in increments of 1.0
			ratio [1:x]
				1.0, 1.12, 1.25, 1.40, 1.60, 1.80, 2.0, 2.50, 3.20, 4.00, 5.60, 8.00, 16.00, +inf
			gain [dB]
				-40.0 to +40.0
			attack [ms]
				0.0 to 800 in varying increments
			release [ms]
				0.0 to 8000 in varying increments
			knee
				hard 0 to soft 9
	line in 13-16:
		stereo
		attenuation [dBu]
			-20, -10, +4
monitor a,b,c,d:
	master:
		reverb: (monitor A only)
			type:
				off / echo / room / small hall / large hall / plate
				pre delay [ms]
					0.0, 0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4, 10, 20, 40, 80 160
				time [sec]
					0.1 to 5.0 in increments of 0.1
			return
		link
		output jack
		direct monitor:
			stereo
			volume l/r
		daw monitor:
			stereo
			volume l/r
	channels 1-16:
		stereo
		mute
		solo
		reverb
		pan
		volume
	daw monitor 1-10:
		stereo
		mute
		solo
		pan
		volume

