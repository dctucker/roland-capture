#pragma once

#include <inttypes.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define addr_bytes(ADDR) (ADDR>>24) & 0xff, (ADDR>>16) & 0xff, (ADDR>>8) & 0xff, ADDR & 0xff
#define bytes_addr(ADDR) ((Addr)((ADDR[0] << 24) | (ADDR[1]<<16) | (ADDR[2]<<8) | ADDR[3]))
typedef uint32_t Addr;

typedef struct Boolean {
	u8 value;
} Boolean;
typedef struct Volume {
	u8 value[6];
} Volume;
typedef struct Pan {
	u8 value[4];
} Pan;

typedef union input_channel {
	struct {
		Boolean  stereo;
		u8 _unused;
		Boolean  solo;
		Boolean  mute;
		Pan      pan;
		Volume   volume;
		Volume   reverb;
	};
	u8 bytes[20];
} input_channel;
