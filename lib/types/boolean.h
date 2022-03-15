
/// @addtogroup Boolean
/// @ingroup ValueTypes
///@{
static capmix_unpacked_t capmix_boolean_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	return capmix_UnpackedInt(value == 0 ? 0 : 1);
}
static capmix_unpacked_t capmix_boolean_parse  (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	if( strcmp(str, "0")==0 || strcasecmp(str, "off")==0 || strcasecmp(str,"false")==0 )
		return capmix_UnpackedInt(0);
	else if( strcmp(str,"1")==0 || strcasecmp(str, "on")==0 || strcasecmp(str,"true")==0 )
		return capmix_UnpackedInt(1);
	return capmix_UnsetInt;
}
static int               capmix_boolean_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	if( unpacked.discrete )
		sprintf(str, "ON");
	else
		sprintf(str, "off");
	return strlen(str);
}
static int               capmix_boolean_pack   (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) ///< @sa capmix_packer_t
{
	buf[0] = unpacked.discrete == 0 ? 0 : 1;
	return type_info->size;
}
///@}

