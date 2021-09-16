/// @addtogroup Pan
/// @ingroup ValueTypes
///@{

/**
 * @brief convert a signed floating point value representing left-right pan into a fixed integer value
 * @param db the floating point value to be converted ranging from -100 on the left to +100 on the right
 * @return a capmix_fixed integer where Left 100% is represented as `0x0000`, Center as `0x4000`, and Right 100% as `0x8000`
 */
float             capmix_fixed_to_pan     (capmix_fixed fx)
{
	return round(100. * (fx - 16384.) / 16384.);
}
/**
 * @brief convert a fixed integer value into a signed floating point value representing left-right pan
 * @param fx the capmix_fixed integer to be converted where L 100% is represented as 0x0000, Center as 0x4000, and Right 100% as 0x8000
 * @return a signed floating point value ranging from -100 on the left to +100 on the right
 */
capmix_fixed      capmix_pan_to_fixed     (float pan)
{
	return (int)(16384. + (16384. * (pan / 100.))) & 0xffff;
}

static capmix_unpacked_t capmix_pan_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	float db = capmix_fixed_to_pan(value);
	return capmix_UnpackedFloat(db);
}
static capmix_unpacked_t capmix_pan_parse (capmix_type_info_t *type_info, const char *str) ///< @sa capmix_parser_t
{
	char side;
	float value;
	if( sscanf(str, "%c%f", &side, &value) > 0 )
	{
		if( side == 'l' || side == 'L' )
			return capmix_UnpackedFloat(-value);
		else if( side == 'r' || side == 'R' )
			return capmix_UnpackedFloat(fabs(value));
		else if( side == 'c' || side == 'C' )
			return capmix_UnpackedFloat(0);
		return capmix_UnsetInt;
	}
	return capmix_UnsetInt;
}
static int               capmix_pan_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	float pan = unpacked.continuous;
	if( pan < 0 )
		sprintf(str, "L%1.0f", fabs(pan));
	else if( pan > 0 )
		sprintf(str, "R%1.0f", pan);
	else
		sprintf(str, "C");
	return strlen(str);
}
static int               capmix_pan_pack    (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, uint8_t *buf) ///< @sa capmix_packer_t
{
	float value = unpacked.continuous;
	capmix_fixed fx = capmix_pan_to_fixed(value);
	capmix_fixed_to_nibbles(fx, 4, buf);
	return type_info->size;
}

///@}
