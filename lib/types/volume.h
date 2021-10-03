/// @addtogroup Volume
/// @ingroup ValueTypes
///@{

/**
 * @brief convert a signed floating point value representing dB into a fixed integer value
 * @param db the value in decibels to be converted ranging from -inf to +12
 * @return a capmix_fixed integer where +12 dB is represented as `0x800000`, +0 dB as `0x200000`, -3 dB as `0x100000`, and -INFINITY as `0x000000`
 */
capmix_fixed      capmix_db_to_fixed      (float db)
{
	// convert -6.02 to 0x100000
	return (int)(pow(10., db/20.) * 0x200000);
}
/**
 * @brief convert a fixed integer value into a floating point value representing dB
 * @param fx the fixed value to be converted
 * @return a signed floating point value representing decibels ranging from -inf to +12
 */
float             capmix_fixed_to_db      (capmix_fixed fx)
{
	// convert 0x200000 to 0
	float ratio = fx / 2097152.;
	if( ratio == 0 ) return -INFINITY;
	return 20.*log10(ratio);
}

static capmix_unpacked_t capmix_volume_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	float db = capmix_fixed_to_db(value);
	return capmix_UnpackedFloat(db);
}
static capmix_unpacked_t capmix_volume_parse (capmix_type_info_t *type_info, const char *str)     ///< @sa capmix_parser_t
{
	float value;
	if( sscanf(str, "%f", &value) > 0 )
		return capmix_UnpackedFloat(value);
	return capmix_UnsetInt;
}
static int               capmix_volume_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	sprintf(str, "%+1.0f", unpacked.continuous);
	return strlen(str);
}
static int               capmix_volume_pack    (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) /// @sa capmix_packer_t
{
	float value = unpacked.continuous;
	capmix_fixed fx = capmix_db_to_fixed(value);
	if( fx > 0x7fffff ) fx = 0x7fffff;
	if( value != -INFINITY && round(value) == 0 )
		fx = 0x200000;
	capmix_fixed_to_nibbles(fx, 6, buf);
	return type_info->size;
}
///@}
