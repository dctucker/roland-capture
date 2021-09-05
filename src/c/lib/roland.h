#include "types.h"

typedef struct roland_sysex {
	u8 header[6];
	u8 cmd;
	u8 addr[4];
	u8 data[];
} RolandSysex;

void make_sysex(u8 *buffer, u8 cmd, u8 *data, int len);
RolandSysex * parse_sysex(u8 *buffer, int len);
