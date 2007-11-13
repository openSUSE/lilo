/* $Id$ */
#ifndef DEVPATH_TEST
#include <errors.h>
#include <file.h>
#include <debug.h>
#else
#define dump_path_description(p) do { __dump_path_description(__FUNCTION__,__LINE__,p); } while(0)
#endif
#include <string.h>
#include <stdlib.h>

#define I(m) DEBUG_F(" %p "#m " '%d' \n", &p->m, p->m);
#define S(m) DEBUG_F(" %p "#m " %p '%s' \n", &p->m, p->m, p->m ? p->m : "");
void __dump_path_description(const char *fn, int l, const struct path_description *p)
{
	DEBUG_F("called from '%s(%u)' '%p'\n", fn, l, p);
	if (!p)
		return;
	I(part);
	I(type);
	S(device);
	S(u.d.s1);
	S(u.d.s2);
	S(filename);
}

#undef S
#undef I

static void parse_block_device(struct path_description *result)
{
	char *ip;
#if 0
	prom_printf("%s\n", __FUNCTION__);
#endif
	if (!result->u.b.partition)
		return;

	result->part = strtol(result->u.b.partition, &ip, 10);
	DEBUG_F("part '%d', partition '%s', ip '%s'\n", result->part, result->u.b.partition, ip);
	if (result->part)
		*ip++ = '\0';
	else {
		result->part = -1;
		result->u.b.partition = "";
	}
	if (',' == ip[0])
		ip++;
	result->u.b.directory = ip;
	result->filename = strrchr(result->u.b.directory, '/');
	if (!result->filename)
		result->filename = strrchr(result->u.b.directory, '\\');
	if (result->filename) {
		memmove(result->filename + 2, result->filename + 1, strlen(result->filename + 1) + 1);
		result->filename++;
		result->filename[0] = '\0';
		result->filename++;
	} else {
		result->filename = result->u.b.directory;
		result->u.b.directory = "";
	}
}

/**
 * reset_device_to_iscsi
 * When the bootpath is on an "iscsi" device,
 * the firmware needs to be passed the contents of the
 * "nas-bootdevice" property.
 */
static void reset_device_to_iscsi(struct path_description *result)
{
	int size = prom_getproplen_chosen("nas-bootdevice");

	if (size > 0) {
		result->device = malloc(size + 2);
		if (result->device) {
			prom_get_chosen("nas-bootdevice", result->device, size);
			DEBUG_F("nas-bootdevice: <%s>\n", result->device);
		}
	}
}

/*
 * options for a "network" device, according to "4.3.2 iSCSI Bootstrap"
 *
 * iscsi,[dev=block],[ipv6,][dhcp[=diaddr],][bootp,][slp[=SLP­server],][isns=iSNS­server,][itname=init­name,]
 * [ichapid=init­chapid,][ichappw=init­chappw,]ciaddr=init­addr,[giaddr=gateway­addr,][subnet­mask=net­mask,]
 * siaddr=target­server,[iport=target­port,]iname=target­name,[ilun=target­lun,][chapid=target­chapid,][chappw=target­chappw,]disk­label args
 *
 * There will be no disk-label args because the firmware is unable to load the bootfile from a specified partition.
 * Only the first 0x41 PReP Boot is considered, /chosen/bootpath will not contain partition, directory and filename
 * Clear partition and directory string, invalidate partition numer
 */
static void parse_iscsi_device(struct path_description *result)
{
	result->part = -1;
	result->u.b.directory = result->u.b.partition = "";
	reset_device_to_iscsi(result);
	return;
}

static void parse_net_device(struct path_description *result)
{
	char *p;
#if 0
	prom_printf("%s\n", __FUNCTION__);
#endif
	if (!result->u.n.ip_before_filename)
		return;

	p = result->u.n.ip_before_filename;

	if (strncmp("bootp", p, 5) == 0) {
		p = strchr(p, ',');
		if (!p) {
			result->u.n.ip_before_filename[5] = ',';
			result->u.n.ip_before_filename[6] = '\0';
			goto out;
		}
		p++;
	}
	if (strncmp("promiscuous", p, 11) == 0) {
		p = strchr(p, ',');
		if (!p)
			goto out;
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
		goto out;
	*p = '\0';
	p++;
	result->filename = p;
	p = strchr(p, ',');
	if (p) {
		*p = '\0';
		p++;
		result->u.n.ip_after_filename = p;
	}
      out:
	return;
}

static void get_mac_address(struct path_description *result)
{
	phandle dev = prom_finddevice(result->device);
	if (dev != PROM_INVALID_HANDLE) {
		if (prom_getprop(dev, "mac-address", &result->u.n.mac, 6) == -1)
			prom_getprop(dev, "local-mac-address", &result->u.n.mac, 6);
		prom_printf("MAC for %s: %02x:%02x:%02x:%02x:%02x:%02x\n",
				result->device,
				result->u.n.mac[0], result->u.n.mac[1],
				result->u.n.mac[2], result->u.n.mac[3],
				result->u.n.mac[4], result->u.n.mac[5]
				);
	}
}

int parse_device_path(const char *imagepath, struct path_description *result)
{
	char *colon;
	DEBUG_F("imagepath '%s'\n", imagepath);
	if (!imagepath)
		return 0;

	result->device = malloc(strlen(imagepath) + 2);
	if (!result->device)
		return 0;
	strcpy(result->device, imagepath);
	result->u.d.s1 = strchr(result->device, ':');
	if (result->u.d.s1) {
		colon = result->u.d.s1;
		*result->u.d.s1 = '\0';
		result->u.d.s1++;
	} else
		colon = NULL;

	result->type = prom_get_devtype(result->device);
	switch (result->type) {
	case TYPE_BLOCK:
		parse_block_device(result);
		break;
	case TYPE_NET:
		if (strncmp("iscsi,", result->u.d.s1, 6) == 0) {
			result->type = TYPE_ISCSI;
			*colon = ':';
			result->u.d.s1 = NULL;
			parse_iscsi_device(result);
		} else {
			parse_net_device(result);
			get_mac_address(result);
		}
		break;
	case TYPE_INVALID:
		prom_printf("firmware said the path '%s' is invalid\n", result->device);
		return 0;
	default:
		prom_printf("type %d of '%s' not handled\n", result->type, result->device);
		return 0;
	}
	dump_path_description(result);
	return 1;
}

char *path_description_to_string(const struct path_description *input)
{
	int len;
	char part[42], *path;

	if (!input)
		return NULL;
	dump_path_description(input);
	path = NULL;
	len = strlen(input->device);
	len += strlen(input->filename);
	len += 1 + 1 + 1;	/* : , \0 */
	switch (input->type) {
	case TYPE_ISCSI:
	case TYPE_BLOCK:
		if (input->part > 0)
			sprintf(part, "%d,", input->part);
		else {
			part[0] = ',';
			part[1] = '\0';
		}
		len += strlen(part);
		len += strlen(input->u.b.directory);
		path = malloc(len);
		if (path)
			sprintf(path, "%s:%s%s%s", input->device, part, input->u.b.directory, input->filename);
		break;
	case TYPE_NET:
		len += strlen(input->u.n.ip_before_filename);
		if (input->u.n.ip_after_filename) {
			len++;
			len += strlen(input->u.n.ip_after_filename);
		}
		path = malloc(len);
		if (path)
			sprintf(path, "%s:%s,%s%s%s", input->device,
				input->u.n.ip_before_filename, input->filename,
				input->u.n.ip_after_filename ? "," : "",
				input->u.n.ip_after_filename ? input->u.n.ip_after_filename : "");
		break;
	default:
		break;
	}
	return path;
}

int imagepath_to_path_description(const char *imagepath, struct path_description *result,
				  const struct path_description *default_device)
{
	char *past_device, *comma, *dir, *pathname;
	char part[42];
	int len;
	DEBUG_F("imagepath '%s'\n", imagepath);

	if (!imagepath)
		return 0;

	memset(result, 0, sizeof(*result));
	past_device = strchr(imagepath, ':');
	if (past_device) {
		if (strncmp("&device;:", imagepath, 9) != 0) {
#if defined(DEBUG) || defined(DEVPATH_TEST)
			prom_printf("parsing full path '%s'\n", imagepath);
#endif
			return parse_device_path(imagepath, result);
		}
		past_device++;
	}
	comma = dir = "";
	pathname = NULL;
	switch (default_device->type) {
	case TYPE_ISCSI:
		result->part = default_device->part;
		pathname = malloc(strlen(default_device->device) + 1);
		if (!pathname)
			return 0;
		strcpy(pathname, default_device->device);
		result->device = pathname;
		pathname = malloc(strlen(imagepath) + 1);
		if (!pathname) {
			free(result->device);
			return 0;
		}
		strcpy(pathname, imagepath);
		result->filename = pathname;
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("hardcoded iscsi path '%s' '%d' '%s'\n", result->device, result->part, result->filename);
#endif
		/* do not call parse_device_path */
		return 1;
	case TYPE_BLOCK:
		part[0] = '\0';
		/* parse_device_path will look for a partition number */
		if (past_device)
			len = strlen(default_device->device) + 1 + strlen(past_device);
		else {
			if (default_device->part > 0)
				sprintf(part, "%d", default_device->part);
			comma = ",";
			if (imagepath[0] != '/' && imagepath[0] != '\\' && default_device->u.b.directory)
				dir = default_device->u.b.directory;
			len = strlen(default_device->device) + 1 + strlen(part) + 1 + strlen(dir) + strlen(imagepath);
		}
		len += 2;
		pathname = malloc(len);
		if (pathname)
			sprintf(pathname, "%s:%s%s%s%s", default_device->device, part, comma, dir,
				past_device ? past_device : imagepath);
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("parsing block path '%s'\n", pathname);
#endif
		break;
	case TYPE_NET:
		if (past_device)
			len = strlen(default_device->device) + 1 + strlen(past_device);
		else {
			len = strlen(default_device->device) + 1;
			if (default_device->u.n.ip_before_filename)
				len += strlen(default_device->u.n.ip_before_filename);
			len++;
			len += strlen(imagepath);
			if (default_device->u.n.ip_after_filename) {
				len += 1 + strlen(default_device->u.n.ip_after_filename);
				comma = ",";
			}
		}
		len += 2;
		pathname = malloc(len);
		if (pathname)
			sprintf(pathname, "%s:%s,%s%s%s", default_device->device,
				default_device->u.n.ip_before_filename ? default_device->u.n.ip_before_filename : "",
				past_device ? past_device : imagepath, comma,
				default_device->u.n.ip_after_filename ? default_device->u.n.ip_after_filename : "");
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("parsing net path '%s'\n", pathname);
#endif
		break;
	default:
		;
	}
	len = parse_device_path(pathname, result);
	if (pathname)
		free(pathname);
	return len;
}

void set_default_device(const char *dev, const char *partition, struct path_description *default_device)
{
	int n;
	char *endp;

	DEBUG_F("dev '%s' part '%s'\n", dev, partition);

	if (dev) {
		endp = strdup(dev);
		if (!endp)
			return;
		default_device->device = endp;
		endp = strchr(default_device->device, ':');
		if (endp)
			endp[0] = '\0';
		default_device->type = prom_get_devtype(default_device->device);
	}

	if (partition) {
		n = simple_strtol(partition, &endp, 10);
		if (endp != partition && *endp == 0) {
			if (TYPE_UNSET == default_device->type)
				default_device->type = TYPE_BLOCK;
			default_device->part = n;
		}
	}
}
