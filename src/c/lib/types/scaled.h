/// @addtogroup Scaled
/// @ingroup ValueTypes
static capmix_unpacked_t capmix_scaled_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	return capmix_UnpackedFloat(type_info->min.continuous + type_info->step.continuous * value);
}
static capmix_unpacked_t capmix_scaled_parse  (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	return capmix_UnpackedFloat(NAN); // # TODO
}
static int               capmix_scaled_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	if( type_info->step.continuous == 1. )
		sprintf(str, "%1.0f", unpacked.continuous);
	else
		sprintf(str, "%1.1f", unpacked.continuous);
	return strlen(str);
}
static int               capmix_scaled_pack   (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) ///< function header for a packer ///< @sa capmix_packer_t
{
	uint8_t value = (unpacked.continuous - type_info->min.continuous) / type_info->step.continuous;
	*buf = value;
	return type_info->size;
}
///@}
