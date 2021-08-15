# Mixer commands

So far it appears mixer commands are reported from the device to the host via Roland System Exclusive commands.

## Memory map

Address `00 06 cc ww` where `cc` is channel and `ww` is the widget being adjusted.

### Widgets

| offset | widget | length | format           ||
|--------|--------|-------:|------------------||
| `00`   | stereo | 1      | 0x00 or 0x01     ||
| `03`   | mute   | 1      | 0x00 or 0x01     ||
| `02`   | solo   | 1      | 0x00 or 0x01     ||
| `0e`   | reverb | 6      | nibbles          ||
| `04`   | pan    | 4      | nibbles          ||
| `08`   | volume | 6      | nibbles          ||

Pan is four masked bytes representing one nibble each, ranging from 0x0000 (left) to 0x8000 (right) and centered at 0x4000

### Volume

- six bytes each ranging from 0x000000000000 to 0x070f0f0f0f0f
- appears to be 0x000000 0x7fffff but encoded as masked to 0x0f

#### Monitor A

1. 0x00060008

### Mutes

- one byte, value of 0x00 or 0x01

#### Monitor A

00060003
00060103
00060203
00060303
00060403
00060503
00060603
00060703
00060803
00060903
00060a03
00060b03
00060c03
00060d03
00060e03
00060f03

#### Monitor B

00061003
00061103
00061203
00061303
00061403
00061503
00061603
00061703
00061803
00061903
00061a03
00061b03
00061c03
00061d03
00061e03
00061f03

#### Monitor C

00062003
00062103
00062203
00062303
00062403
00062503
00062603
00062703
00062803
00062903
00062a03
00062b03
00062c03
00062d03
00062e03
00062f03

#### Monitor D

00063003
00063103
00063203
00063303
00063403
00063503
00063603
00063703
00063803
00063903
00063a03
00063b03
00063c03
00063d03
00063e03
00063f03

