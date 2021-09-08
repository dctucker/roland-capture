# Patchbay

## Address

`0x00030000`~`0x00030004`

## Description

Address as `0x000300rr` where `rr` is the Output row.
Data as `0`~`8`, where `0`~`3` are Monitor, and `4`~`8` are Wave Out.

## Visual of the patchbay

```
Patchbay

---- Monitor ----     ------- Wave Out -------
A    B    C    D      1-2  3-4  5-6  7-8  9-10
                                                Output
[ ]  [ ]  [ ]  [ ]    [ ]  [ ]  [ ]  [ ]  [ ]   1-2
[ ]  [ ]  [ ]  [ ]    [ ]  [ ]  [ ]  [ ]  [ ]   3-4
[ ]  [ ]  [ ]  [ ]    [ ]  [ ]  [ ]  [ ]  [ ]   5-6
[ ]  [ ]  [ ]  [ ]    [ ]  [ ]  [ ]  [ ]  [ ]   7-8
[ ]  [ ]  [ ]  [ ]    [ ]  [ ]  [ ]  [ ]  [ ]   9-10
```

