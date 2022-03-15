//1000 0011 0000 0001
//0001 1100 0000 1000
//01 0c 00 08 : channels 1, 7, 8, 16 have clipped

static int               capmix_clipmask_format (capmix_type_info_t *type_info, capmix_unpacked_t unpacked, char *str) ///< @sa capmix_formatter_t
{
	sprintf(str, "");
	for(int ch=0; ch < 4; ch++)
	{
		if( unpacked.discrete & (1 << ch) )
			strcat(str, "!");
		else
			strcat(str, ".");
	}
}
