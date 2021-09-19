
float             capmix_fixed_to_meter      (capmix_fixed fx)
{
	// convert 0x3fff to 0
	float ratio = fx / 16383.;
	if( ratio == 0 ) return -INFINITY;
	return 20. * log10(ratio);
}

static capmix_unpacked_t capmix_meter_unpack (capmix_type_info_t *type_info, capmix_fixed value) ///< @sa capmix_unpacker_t
{
	float ratio = capmix_fixed_to_meter(value);
	return capmix_UnpackedFloat(ratio);
}

static int               capmix_meter_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	if( unpacked.continuous < -70. )
		sprintf(str, "-inf");
	else
		sprintf(str, "%-1.0f", unpacked.continuous);
	return strlen(str);
}

