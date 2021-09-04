typedef uint8_t u8;
typedef uint32_t u32;
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
