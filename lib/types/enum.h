/// @addtogroup Enum
/// @ingroup ValueTypes
///@{
//static capmix_unpacked_t capmix_enum_unpack (capmix_type_info_t *type_info, capmix_fixed value)
static capmix_unpacked_t capmix_enum_parse (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	for(int i=0; i < type_info->max.discrete; i++)
	{
		const char *name = type_info->enum_names[i];
		if( strcmp(str, name) == 0 )
			return capmix_UnpackedInt(i);
	}
	return capmix_UnpackedInt(capmix_Unset);
}
static int               capmix_enum_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	uint32_t v = unpacked.discrete;
	if( v > type_info->max.discrete )
	{
		//warn("\nvalue too large 0x%x\n", v);
		sprintf(str, "?");
		return 1;
	}
	const char *name = type_info->enum_names[v];
	sprintf(str, "%s", name);
	return strlen(str);
}
//static int               capmix_enum_pack    (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf)
///@}
