/* $Id$ */
#ifndef DEVPATH_TEST
#include <errors.h>
#include <file.h>
#endif
#include <string.h>
#include <stdlib.h>

static void parse_block_device(struct boot_fspec_t *result)
{
	char *ip;
#if 0
	prom_printf("%s\n", __FUNCTION__);
#endif
	result->type = TYPE_BLOCK;
	result->part = strtol(result->partition, &ip, 10);
	if (result->part)
		*ip++ = '\0';
	else
		result->partition = "";
	if (',' == ip[0])
		ip++;
	result->directory = ip;
	result->filename = strrchr(result->directory, '/');
	if (!result->filename)
		result->filename = strrchr(result->directory, '\\');
	if (result->filename) {
		char *p;
		result->filename++;
		p = strdup(result->filename);
		result->filename[0] = '\0';
		result->filename = p;
	} else {
		result->filename = result->directory;
		result->directory = "";
	}
}

static void parse_net_device(struct boot_fspec_t *result)
{
	char *p;
#if 0
	prom_printf("%s\n", __FUNCTION__);
#endif
	result->type = TYPE_NET;
	p = result->partition;

	if (strncmp("bootp", p, 5) == 0) {
		p = strchr(p, ',');
		if (!p)
			goto bootp;
		p++;
	}
	if (strncmp("speed=", p, 6) == 0) {
		p = strchr(p, ',');
		if (!p)
			goto out;
		p++;
	}
	if (strncmp("duplex=", p, 7) == 0) {
		p = strchr(p, ',');
		if (!p)
			goto out;
		p++;
	}
	p = strchr(p, ',');
	if (!p)
		goto bootp;
	*p = '\0';
	p++;
	result->filename = p;
	p = strchr(p, ',');
	if (p) {
		*p = '\0';
		p++;
		result->ip_after_filename = p;
	}
bootp:
	result->ip_before_filename = result->partition;

      out:
	result->partition = NULL;
}

int new_parse_device_path(const char *imagepath, struct boot_fspec_t *result)
{
	if (!imagepath)
		return 0;

	result->device = strdup(imagepath);
	result->partition = strchr(result->device, ':');
	if (result->partition) {
		*result->partition++ = '\0';
		switch (prom_get_devtype(result->device)) {
		case TYPE_BLOCK:
			parse_block_device(result);
			break;
		case TYPE_NET:
			parse_net_device(result);
			break;
		default:
			return 0;
		}
	} else {
		char *p = strrchr(result->device, '/');
		if (p) {
			result->filename = strdup(p);
			p[1] = '\0';
			result->directory = result->device;
			if (!result->directory[1])
				result->directory++;
		} else
			result->filename = result->device;
		result->type = TYPE_UNKNOWN;
		result->device = NULL;
	}
	return 1;
}
