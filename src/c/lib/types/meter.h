
float             capmix_fixed_to_meter      (capmix_fixed fx)
{
	// convert 0x3fff to 1.0
	return fx / 16383.;
}

static capmix_unpacked_t capmix_meter_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	float ratio = capmix_fixed_to_meter(value);
	return capmix_UnpackedFloat(ratio);
}

static int               capmix_meter_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	sprintf(str, "%0.2f", unpacked.continuous);
	return strlen(str);
}

