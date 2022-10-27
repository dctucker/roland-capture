type
  Model* = enum
    MNone = 0,
    MDuo, # (pretty sure this does not have a software mixer interface)
    MQuad,
    MOcta,
    MStudio,
  CAddr* = uint32
  Fixed* = uint32
  Unpacked* {.union.}= object
    discrete*   : cuint  # for discrete values such as reverb type
    continuous* : cfloat # for continuous values such as pan and volume

type
  Kind* {.size:4.} = enum
    TValue = 0,   # Unknown or invalid type. Use with caution.
    TByte,        # Simple one-byte value. Parent: `TValue`
    TBoolean,     # An on/off switch. Parent: `TByte`
    TVolume,      # Six-byte logarithmic volume value. Parent: `TValue`
    TPan,         # Four-byte pan value. Parent: `TValue`
    TMeter,       # Two-byte meter value. Parent: `TValue`
    TClipMask,    # Bit-mask indicating which channels have clipped. Parent: `TValue`
    TScaled,      # Single-byte value scaled to fit a range. Parent: `TByte`
    TSens,        # Scaled preamp sensitivity value. Parent: `TScaled`
    TThreshold,   # Scaled compressor threshold value. Parent: `TScaled`
    TGain,        # Scaled compressor post-gain value. Parent: `TScaled`
    TGate,        # Scaled gate threshold value. Parent: `TScaled`
    TEnum,        # Byte value representing one of a list of options. Parent: `TByte`
    TRatio,       # Enumerated compressor ratio. Parent: `TEnum`
    TAttack,      # Enumerated compressor attack time. Parent: `TEnum`
    TRelease,     # Enumerated compressor release time. Parent: `TEnum`
    TKnee,        # Enumerated compressor knee hard/soft values. Parent: `TEnum`
    TAttenuation, # Enumerated line input attenuation {-20, -10, +4}. Parent: `TEnum`
    TReverbType,  # Enumerated value representing reverb type. Parent: `TEnum`
    TPreDelay,    # Enumerated reverb pre-delay time. Parent: `TEnum`
    TPatch,       # Enumerated value representing selected source for a given output. Parent: `TEnum`
    TAutoSens,    # Enumerated value representing auto-sens state. Parent: `TEnum`
    TReverbTime,  # Scaled reverb time. Parent: `TScaled`
    TRawData,     # An arbitrary fields of bytes. Parent: `TValue`
    TOctaLoad,    # Populated by load_settings on OCTA-CAPTURE. Parent: `TRawData`
    TStudioLoad,  # Populated by load_settings on STUDIO-CAPTURE. Parent: `TRawData`
    NTypes,       # dummy value to represent the number of enumerated types
  KindInfo* = object
    kind*   : Kind        # primary key, identifies the type to be described
    parent* : Kind        # the supertype from which this type inherits
    min*    : Unpacked    # minimum unpacked value
    max*    : Unpacked    # maximum unpacked value
    step*   : Unpacked    # granularity of the unpacked value
    size*   : cint        # number of bytes stored in device memory for this type
    unpack* : Unpacker    # pointer to the function that unpacks fixed values for this type
    parse*  : Parser      # pointer to the function that parses strings into unpacked values for this type
    format* : Formatter   # pointer to the function that formats strings from unpacked values for this type
    pack*   : Packer      # pointer to the function that packs bytes from unpacked values for this type
    name*   : cstring     # string representing the type's name
    enum_names*: cstringArray  # array of strings describing discrete values for enumerated types
  Unpacker  = proc(info: KindInfo, fixed: Fixed): Unpacked                    {.cdecl.} # pointer to a function that unpacks fixed values
  Parser    = proc(info: KindInfo, str: cstring): Unpacked                    {.cdecl.} # pointer to a function that parses strings into unpacked values
  Formatter = proc(info: KindInfo, unpacked: Unpacked, str: cstring): cint    {.cdecl.} # pointer to a function that formats strings from unpacked values
  Packer    = proc(info: KindInfo, unpacked: Unpacked, packed: pointer): cint {.cdecl.} # pointer to a function that packs bytes from unpacked values

type
  Event = object
    sysex_data_length : cint         # length in bytes of the SysEx message
    sysex             : pointer      # structure describing the SysEx message
    `addr`            : CAddr        # the device address this message considers
    type_info         : ptr KindInfo # structure describing the type of the data stored at this location in device memory
    unpacked          : Unpacked     # the value stored in device memory
  EventHandler = proc(e: Event): void

{.push cdecl, importc, dynlib: "libcapmix.so".}
proc capmix_unpack_type*  (kind: Kind, packed: pointer)                    : Unpacked
proc capmix_pack_type*    (kind: Kind, unpacked: Unpacked, packed: pointer): cint
proc capmix_type_name*    (kind: Kind): cstring
proc capmix_type_size*    (kind: Kind): cint

proc capmix_memory_get_unpacked* (a: CAddr)                    : Unpacked
proc capmix_memory_set_unpacked* (a: CAddr, unpacked: Unpacked): void

proc capmix_set_model*    (model: Model)                      : void
proc capmix_connect*      (handler: EventHandler)             : cint
proc capmix_listen*       ()                                  : cint
proc capmix_get*          (a: CAddr)                          : Event
proc capmix_put*          (a: CAddr, unpacked: Unpacked)      : Event
proc capmix_disconnect*   ()                                  : void

proc capmix_format_addr*  (a: CAddr, str: cstring)            : void
proc capmix_parse_addr*   (str: cstring)                      : CAddr
proc capmix_addr_type*    (a: CAddr)                          : Kind
proc capmix_addr_suffix*  (a: CAddr)                          : cstring
proc capmix_parse_type*   (kind: Kind, str: cstring)          : Unpacked
proc capmix_format_type*  (kind: Kind, unpacked: Unpacked, str: cstring): cint
proc capmix_type*         (kind: Kind)                        : ptr KindInfo
{.pop.}

