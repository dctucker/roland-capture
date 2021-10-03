/// @addtogroup Unset
/// @ingroup ValueTypes
///@{
static capmix_unpacked_t capmix_unset_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	return capmix_UnsetInt;
}
static capmix_unpacked_t capmix_unset_parse  (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	return capmix_UnsetInt;
}
static int               capmix_unset_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	str[0] = '?'; str[1] = '\0'; return 1;
}
static int               capmix_unset_pack   (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) ///< @sa capmix_packer_t
{
	return 0;
}
///@}
