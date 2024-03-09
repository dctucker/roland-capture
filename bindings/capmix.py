from ctypes import *

class UNPACKED(Union):
	_fields_ = [
		("discrete"  , c_uint ),
		("continuous", c_float),
	]

class TYPE(Structure):
	_fields_ = [
		("type"       , c_uint   ),
		("parent"     , c_uint   ),
		("min"        , UNPACKED ),
		("max"        , UNPACKED ),
		("step"       , UNPACKED ),
		("size"       , c_int    ),
		("unpack"     , c_void_p ),
		("parse"      , c_void_p ),
		("format"     , c_void_p ),
		("pack"       , c_void_p ),
		("name"       , c_char_p ),
		("_enum_names", POINTER(c_char_p) ),
	]
	def enum_names(self):
		return [ x.decode() for x in self._enum_names[0:self.max.discrete+1] ]


class EVENT(Structure):
	_fields_ = [
		("sysex_data_length", c_int    ),
		("sysex"            , c_void_p ),
		("addr"             , c_uint   ),
		("type_info"        , POINTER(TYPE)),
		("unpacked"         , UNPACKED ),
	]
	def type(self):
		return self.type_info.contents.type
	def type_name(self):
		return self.type_info.contents.name.decode()
	def value(self):
		return Value(self.type(), self.unpacked)

@CFUNCTYPE(None, EVENT)
def listener(event):
	capmix._listener(event)

class Capmix(object):
	def __init__(self):
		lib = cdll.LoadLibrary("obj/lib/libcapmix.so")
		self.setup_returns(lib)
		#self.connect             = lib.capmix_connect
		self.listen              = lib.capmix_listen
		self.get                 = lib.capmix_get
		self.put                 = lib.capmix_put
		self.disconnect          = lib.capmix_disconnect
		self.memory_get_unpacked = lib.capmix_memory_get_unpacked
		self.memory_set_unpacked = lib.capmix_memory_set_unpacked
		self.addr_type           = lib.capmix_addr_type
		self.unpack_type         = lib.capmix_unpack_type
		self.parse_type          = lib.capmix_parse_type
		self.lib                 = lib

	@staticmethod
	def setup_returns(lib):
		lib.capmix_type.restype = POINTER(TYPE)
		lib.capmix_get.restype  = EVENT
		lib.capmix_put.restype  = EVENT
		lib.capmix_memory_get_unpacked = UNPACKED

	def set_model(self, m):
		self.lib.capmix_set_model(m)

	def connect(self, l):
		self._listener = l
		return self.lib.capmix_connect(listener)

	def format_addr(self, addr):
		ret = create_string_buffer(b'\000' * 64)
		self.lib.capmix_format_addr(addr, ret)
		return ret.value.decode()

	def format_type(self, ty, unpacked):
		ret = create_string_buffer(b'\000' * 64)
		self.lib.capmix_format_type(ty, unpacked, ret)
		return ret.value.decode()

	def pack_type(self, ty, unpacked):
		ret = create_string_buffer(b'\000' * 8)
		self.lib.capmix_pack_type(ty, unpacked, ret)
		return ret.value.decode()

	def parse_addr(self, desc):
		ret = self.lib.capmix_parse_addr(desc.encode())
		return ret

	def type(self, ty):
		ret = self.lib.capmix_type(ty).contents
		return ret

	def addr_suffix(self, addr):
		return self.lib.capmix_addr_suffix(addr).decode()

capmix = Capmix()

class Value(object):
	def __init__(self, ty, unp):
		self.type = ty
		self.unpacked = unp
	def __str__(self):
		return str(capmix.format_type(self.type, self.unpacked))

	@classmethod
	def parse(cls, ty, str):
		ret = Value(ty, capmix.parse_type(ty, str))
		return ret

