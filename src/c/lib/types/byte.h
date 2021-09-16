
/// @addtogroup Byte
/// @ingroup ValueTypes
///@{
static capmix_unpacked_t capmix_byte_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	return capmix_UnpackedInt(value & 0x7f);
}
static capmix_unpacked_t capmix_byte_parse (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	int value;
	if( sscanf(str, "%d", &value) > 0 )
		return capmix_UnpackedInt(value);
	return capmix_UnsetInt;
}
static int               capmix_byte_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	if( unpacked.discrete == capmix_Unset )
		sprintf(str, "?");
	else
		sprintf(str, "0x%02x", unpacked.discrete);
	return strlen(str);
}
static int               capmix_byte_pack    (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) ///< @sa capmix_packer_t
{
	buf[0] = unpacked.discrete & 0x7f;
	return type_info->size;
}
///@}
