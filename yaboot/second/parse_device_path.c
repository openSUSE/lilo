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
		memmove(result->filename + 2, result->filename + 1, strlen(result->filename + 1) + 1);
		result->filename++;
		result->filename[0] = '\0';
		result->filename++;
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

	result->device = malloc(strlen(imagepath) + 2);
	if (!result->device)
		return 0;
	strcpy(result->device, imagepath);
	result->partition = strchr(result->device, ':');
	if (result->partition) {
		*result->partition++ = '\0';
		result->type = prom_get_devtype(result->device);
		switch (result->type) {
		case TYPE_BLOCK:
			parse_block_device(result);
			break;
		case TYPE_NET:
			parse_net_device(result);
			break;
		default:
			prom_printf("type %d of '%s' not handled\n", result->type, result->device);
			return 0;
		}
	}
	return 1;
}
