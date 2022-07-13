/*
 * Copyright (c) 2022 Casey Tucker
 * 		<dctucker@hotmail.com>
 *
 * TODO License goes here
 */

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/control_external.h>

#include "controls.h"

typedef struct snd_ctl_capmix {
	snd_ctl_ext_t ext;
	char name[64];
	int value;
	int count;
} snd_ctl_capmix_t;

static void capmix_close(snd_ctl_ext_t *ext)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	int i;
	/*
	for (i = 0; i < capmix->num_input_controls; i++) {
		free(capmix->control_info[i].name);
	}
	free(capmix->control_info);
	LADSPAcontrolUnMMAP(capmix->control_data);
	LADSPAunload(capmix->library);
	*/
	free(capmix);
}

static int capmix_elem_count(snd_ctl_ext_t *ext)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	return capmix_ctl_count;
}

static int capmix_elem_list(snd_ctl_ext_t *ext, unsigned int offset,
		snd_ctl_elem_id_t *id)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_device(id, offset);
	if (offset < capmix_ctl_count) {
		snd_ctl_elem_id_set_name(id, capmix_ctl_map[offset].name);
		snd_ctl_elem_id_set_numid(id, capmix_ctl_map[offset].id);
	}
	else {
		SNDERR("unknown element %d", offset);
	}
	return 0;
}

static snd_ctl_ext_key_t capmix_find_elem(snd_ctl_ext_t *ext,
		const snd_ctl_elem_id_t *id)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	const char *name;
	unsigned int i, numid;

	numid = snd_ctl_elem_id_get_numid(id);

	if (0 < numid && numid < capmix_ctl_count) {
		return capmix_ctl_map[numid-1].id;
	}

	name = snd_ctl_elem_id_get_name(id);
	for (i = 0; i < capmix_ctl_count; i++ ) {
		if (!strcmp(name, capmix_ctl_map[i].name)) {
			return capmix_ctl_map[i].id;
		}
	}

	SNDERR("Could not find element %d / %s", numid, name);
	return SND_CTL_EXT_KEY_NOT_FOUND;
}

static int capmix_get_attribute(snd_ctl_ext_t *ext, snd_ctl_ext_key_t key,
	int *type, unsigned int *acc, unsigned int *count)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	*type = SND_CTL_ELEM_TYPE_INTEGER;
	*acc = SND_CTL_EXT_ACCESS_READWRITE |
	       SND_CTL_EXT_ACCESS_TLV_READWRITE |
	       SND_CTL_EXT_ACCESS_TLV_CALLBACK;
	*count = 1;
	return 0;
}

static int capmix_get_integer_info(snd_ctl_ext_t *ext,
	snd_ctl_ext_key_t key, long *imin, long *imax, long *istep)
{
	*istep = 1;
	*imin = -70;
	*imax = 12;
	return 0;
}
static int capmix_read_integer(snd_ctl_ext_t *ext, snd_ctl_ext_key_t key,
		long *value)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	int i;

	if (key != 26)
		return 0;
	if (*value == capmix->value)
		return 0;

	// TODO get from libcapmix
	*value = capmix->value;
	/*
	for(i = 0; i < capmix->control_data->channels; i++) {
		value[i] = ((capmix->control_data->control[key].data[i] -
			capmix->control_info[key].min)/
			(capmix->control_info[key].max-
			capmix->control_info[key].min))*100;
	}
	*/

	return sizeof(long);//*capmix->control_data->channels;
}

static int capmix_write_integer(snd_ctl_ext_t *ext, snd_ctl_ext_key_t key,
		long *value)
{
	snd_ctl_capmix_t *capmix = ext->private_data;
	int i;
	float setting;

	if (key != 26)
		return 0;

	// TODO set via libcapmix
	capmix->value = *value;
	return 1;
	/*
	for(i = 0; i < capmix->control_data->channels; i++) {
		setting = value[i];
		capmix->control_data->control[key].data[i] = (setting/100)*
			(capmix->control_info[key].max-
			capmix->control_info[key].min)+
			capmix->control_info[key].min;
	}
	*/

}
static int capmix_read_event(snd_ctl_ext_t *ext ATTRIBUTE_UNUSED,
		snd_ctl_elem_id_t *id ATTRIBUTE_UNUSED,
		unsigned int *event_mask ATTRIBUTE_UNUSED)
{
	return -EAGAIN;
}

static snd_ctl_ext_callback_t capmix_ext_callback = {
	.close               = capmix_close,
	.elem_count          = capmix_elem_count,
	.elem_list           = capmix_elem_list,
	.find_elem           = capmix_find_elem,
	.get_attribute       = capmix_get_attribute,
	.get_integer_info    = capmix_get_integer_info,
	//.get_enumerated_info = capmix_get_enumerated_info,
	//.get_enumerated_name = capmix_get_enumerated_name,
	.read_integer        = capmix_read_integer,
	.write_integer       = capmix_write_integer,
	//.write_enumerated    = capmix_write_enumerated,
	.read_event          = capmix_read_event,
};

int capmix_tlv_rw(snd_ctl_ext_t *ext,
    snd_ctl_ext_key_t key, int op_flag, unsigned int numid,
    unsigned int *tlv, unsigned int tlv_size)
{
	//unsigned int *p;
	//return snd_tlv_parse_dB_info(tlv, tlv_size, p);
	return 0; // TODO
}

SND_CTL_PLUGIN_DEFINE_FUNC(capmix)
{
	snd_config_iterator_t it, next;
	snd_ctl_capmix_t *capmix;
	int err;

	snd_config_for_each(it, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(it);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0 || strcmp(id, "type") == 0)
			continue;
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}

	capmix = calloc(1, sizeof(*capmix));
	if (capmix == NULL) {
		SNDERR("Out of memory");
		return -ENOMEM;
	}

	capmix->count = 1;
	strncpy(capmix->name         , "Capmix", sizeof(capmix->name) -1);
	capmix->value = 49;

	capmix->ext.version = SND_CTL_EXT_VERSION;
	capmix->ext.card_idx = 0;
	strncpy(capmix->ext.id       , "capmix", sizeof(capmix->ext.id) -1);
	strncpy(capmix->ext.name     , "capmix", sizeof(capmix->ext.name) -1);
	strncpy(capmix->ext.longname , "capmix", sizeof(capmix->ext.longname) -1);
	strncpy(capmix->ext.mixername, "capmix", sizeof(capmix->ext.mixername) -1);
	capmix->ext.poll_fd = -1;
	capmix->ext.callback = &capmix_ext_callback;
	capmix->ext.private_data = capmix;
	capmix->ext.tlv.c = capmix_tlv_rw;

	err = snd_ctl_ext_create(&capmix->ext, name, mode);
	if (err < 0) {
		SNDERR("Unable to ext_create");
		return -1;
	}

	*handlep = capmix->ext.handle;
	return 0;
}
SND_CTL_PLUGIN_SYMBOL(capmix);