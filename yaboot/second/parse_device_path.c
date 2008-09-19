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

#define DHCP_UDP_OVERHEAD	(20 + /* IP header */                   \
				 8)	/* UDP header */
#define DHCP_SNAME_LEN		64
#define DHCP_FILE_LEN		128
#define DHCP_FIXED_NON_UDP	236
#define DHCP_FIXED_LEN		(DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
				/* Everything but options. */
#define DHCP_MTU_MAX		1500
#define DHCP_OPTION_LEN		(DHCP_MTU_MAX - DHCP_FIXED_LEN)

#define BOOTP_MIN_LEN		300
#define DHCP_MIN_LEN		548

struct dhcp_packet {
	u8 op;			/* 0: Message opcode/type */
	u8 htype;		/* 1: Hardware addr type (net/if_types.h) */
	u8 hlen;		/* 2: Hardware addr length */
	u8 hops;		/* 3: Number of relay agent hops from client */
	u32 xid;		/* 4: Transaction ID */
	u16 secs;		/* 8: Seconds since client started looking */
	u16 flags;		/* 10: Flag bits */
	u32 ciaddr;		/* 12: Client IP address (if already in use) */
	u32 yiaddr;		/* 16: Client IP address */
	u32 siaddr;		/* 18: IP address of next server to talk to */
	u32 giaddr;		/* 20: DHCP relay agent IP address */
	unsigned char chaddr[16];	/* 24: Client hardware address */
	char sname[DHCP_SNAME_LEN];	/* 40: Server name */
	char file[DHCP_FILE_LEN];	/* 104: Boot filename */
	unsigned char options[DHCP_OPTION_LEN];
	/* 212: Optional parameters
	   (actual length dependent on MTU). */
};

/* DHCP options */
enum dhcp_options {
	DHCP_PAD = 0,
	DHCP_NETMASK = 1,
	DHCP_ROUTERS = 3,
	DHCP_END = 255,
};

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
	char *ip, *pf;
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
	pf = strrchr(result->u.b.directory, '/');
	if (!pf)
		pf = strrchr(result->u.b.directory, '\\');
	if (pf) {
		memmove(pf + 2, pf + 1, strlen(pf + 1) + 1);
		pf++;
		pf[0] = '\0';
		pf++;
		path_filename(result) = pf;
	} else {
		path_filename(result) = result->u.b.directory;
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
		path_device(result) = malloc(size + 2);
		if (path_device(result)) {
			prom_get_chosen("nas-bootdevice", path_device(result), size);
			DEBUG_F("nas-bootdevice: <%s>\n", path_device(result));
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

/*
 * pmac and older chrp (up to power5) can request any file from the tfp server
 * Newer IBM firmware requires a boot path with static ip addresses and the
 * desired filename. If /chosen/bootpath is reused, the bootfile (yaboot) itself
 * is loaded again and again.
 * pmac uses dhcp-response
 * chrp uses bootpreply-packet
 * power6 CAS firmware use bootp-response
 * build a static IP data from the bootp server response
 * format is <device>:<interfaceoption,interfaceoption,>tftpserver,filename,client,gateway,bootp-retry,tftp-retry,netmask,tftp-blocksize
 */
static void ipv4_to_ascii(char *buf, u32 ip)
{
	if (buf)
		sprintf(buf, "%u.%u.%u.%u", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
}

static void get_dhcp_options(const struct dhcp_packet *dp, int size, u32 * netmask, u32 * gateway)
{
	unsigned char *p, value, len;
	u32 i;

	/* add 4 byte cookie offset */
	i = offsetof(struct dhcp_packet, options) + 4;
	if (size >= i) {
		p = (unsigned char *)dp;
		p += i;
		size -= i;
		while (size > 0) {
			value = *p;
			p++;
			size--;
			if (!value)
				continue;
			if (value == DHCP_END)
				break;
			len = *p;
			p++;
			size--;
			if (len > size)
				break;
			if (value == DHCP_NETMASK)
				memcpy(netmask, p, 4);
			else if (value == DHCP_ROUTERS)
				memcpy(gateway, p, 4);
			p += len;
			size -= len;
		}
	}
}

static void hack_boot_path_for_CAS(struct path_description *result)
{
	char *p, buf[sizeof("000.000.000.000")];
	char *clientip, *gatewayip, *bootp_retry, *tftp_retry, *netmask, *tftp_blocksize;
	struct dhcp_packet *dp;
	int size, s;

	size = prom_getproplen_chosen("bootp-response");
	if (size <= 0)
		return;

	dp = malloc(size);
	if (!dp)
		return;
	clientip = gatewayip = bootp_retry = tftp_retry = netmask = tftp_blocksize = NULL;
	prom_get_chosen("bootp-response", dp, size);

	p = strrchr(path_net_before(result), ',');
	if (p) {
		*p = '\0';
		result->u.n.dev_options = path_net_before(result);
	}

	result->u.n.server_ip = dp->siaddr;
	result->u.n.client_ip = dp->yiaddr;
	result->u.n.gateway_ip = dp->giaddr;
	get_dhcp_options(dp, size, &result->u.n.netmask, &result->u.n.gateway_ip);

	ipv4_to_ascii(buf, dp->siaddr);
	s = 0;
	if (result->u.n.dev_options)
		s += strlen(result->u.n.dev_options) + 1;
	s += strlen(buf) + 1;
	p = malloc(s);
	if (p) {
		path_net_before(result) = p;
		if (result->u.n.dev_options) {
			sprintf(p, "%s,", result->u.n.dev_options);
			p += strlen(p);
		}
		sprintf(p, "%s", buf);
	}

	clientip = path_net_after(result);

	p = strchr(clientip, ',');
	if (!p)
		goto end_parse;
	*p = '\0';
	p++;
	gatewayip = p;

	p = strchr(p, ',');
	if (!p)
		goto end_parse;
	*p = '\0';
	p++;
	bootp_retry = p;

	p = strchr(p, ',');
	if (!p)
		goto end_parse;
	*p = '\0';
	p++;
	tftp_retry = p;

	p = strchr(p, ',');
	if (!p)
		goto end_parse;
	*p = '\0';
	p++;
	netmask = p;

	p = strchr(p, ',');
	if (!p)
		goto end_parse;
	*p = '\0';
	p++;
	tftp_blocksize = p;

      end_parse:
	s = sizeof(buf) + 1 + sizeof(buf) + 1 + strlen(bootp_retry) + 1 + strlen(tftp_retry) + 1 + sizeof(buf) + 1 + strlen(tftp_blocksize) + 1;
	p = malloc(s);
	if (p) {
		path_net_after(result) = p;
		ipv4_to_ascii(buf, result->u.n.client_ip);
		sprintf(p, "%s,", buf);
		p += strlen(p);
		ipv4_to_ascii(buf, result->u.n.gateway_ip);
		sprintf(p, "%s,", buf);
		p += strlen(p);
		sprintf(p, "%s,", bootp_retry ? bootp_retry : "5");
		p += strlen(p);
		sprintf(p, "%s,", tftp_retry ? tftp_retry : "5");
		p += strlen(p);
		ipv4_to_ascii(buf, result->u.n.netmask);
		sprintf(p, "%s,%s", buf, tftp_blocksize ? tftp_blocksize : "512");
	}
	return;
}

static void parse_net_device(struct path_description *result)
{
	char *p;
#if 0
	prom_printf("%s\n", __FUNCTION__);
#endif
	if (!path_net_before(result))
		goto out;

	p = path_net_before(result);

	if (strncmp("bootp", p, 5) == 0) {
		p = strchr(p, ',');
		if (!p) {
			path_net_before(result)[5] = ',';
			path_net_before(result)[6] = '\0';
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
	path_filename(result) = p;
	p = strchr(p, ',');
	if (p) {
		*p = '\0';
		p++;
		path_net_after(result) = p;
	}
      out:
	hack_boot_path_for_CAS(result);
	return;
}

static void get_mac_address(struct path_description *result)
{
	phandle dev = prom_finddevice(path_device(result));
	if (dev != PROM_INVALID_HANDLE) {
		if (prom_getprop(dev, "mac-address", &result->u.n.mac, 6) == -1)
			prom_getprop(dev, "local-mac-address", &result->u.n.mac, 6);
		prom_printf("MAC for %s: %02x:%02x:%02x:%02x:%02x:%02x\n",
			    path_device(result), result->u.n.mac[0], result->u.n.mac[1], result->u.n.mac[2], result->u.n.mac[3], result->u.n.mac[4], result->u.n.mac[5]
		    );
	}
}

static int parse_device_path(const char *imagepath, struct path_description *result)
{
	char *colon;
	DEBUG_F("imagepath '%s'\n", imagepath);
	if (!imagepath)
		return 0;

	path_device(result) = malloc(strlen(imagepath) + 2);
	if (!path_device(result))
		return 0;
	strcpy(path_device(result), imagepath);
	result->u.d.s1 = strchr(path_device(result), ':');
	if (result->u.d.s1) {
		colon = result->u.d.s1;
		*result->u.d.s1 = '\0';
		result->u.d.s1++;
	} else
		colon = NULL;

	result->type = prom_get_devtype(path_device(result));
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
		prom_printf("firmware said the path '%s' is invalid\n", path_device(result));
		return 0;
	default:
		prom_printf("type %d of '%s' not handled\n", result->type, path_device(result));
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
	len = strlen(path_device(input));
	len += strlen(path_filename(input));
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
			sprintf(path, "%s:%s%s%s", path_device(input), part, input->u.b.directory, path_filename(input));
		break;
	case TYPE_NET:
		len += strlen(path_net_before(input));
		if (path_net_after(input)) {
			len++;
			len += strlen(path_net_after(input));
		}
		path = malloc(len);
		if (path)
			sprintf(path, "%s:%s,%s%s%s", path_device(input),
				path_net_before(input), path_filename(input), path_net_after(input) ? "," : "", path_net_after(input) ? path_net_after(input) : "");
		break;
	default:
		break;
	}
	return path;
}

int imagepath_to_path_description(const char *imagepath, struct path_description *result, const struct path_description *default_device)
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
		pathname = malloc(strlen(path_device(default_device)) + 1);
		if (!pathname)
			return 0;
		strcpy(pathname, path_device(default_device));
		path_device(result) = pathname;
		pathname = malloc(strlen(imagepath) + 1);
		if (!pathname) {
			free(path_device(result));
			return 0;
		}
		strcpy(pathname, imagepath);
		path_filename(result) = pathname;
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("hardcoded iscsi path '%s' '%d' '%s'\n", path_device(result), result->part, path_filename(result));
#endif
		/* do not call parse_device_path */
		return 1;
	case TYPE_BLOCK:
		part[0] = '\0';
		/* parse_device_path will look for a partition number */
		if (past_device)
			len = strlen(path_device(default_device)) + 1 + strlen(past_device);
		else {
			if (default_device->part > 0)
				sprintf(part, "%d", default_device->part);
			comma = ",";
			if (imagepath[0] != '/' && imagepath[0] != '\\' && default_device->u.b.directory)
				dir = default_device->u.b.directory;
			len = strlen(path_device(default_device)) + 1 + strlen(part) + 1 + strlen(dir) + strlen(imagepath);
		}
		len += 2;
		pathname = malloc(len);
		if (pathname)
			sprintf(pathname, "%s:%s%s%s%s", path_device(default_device), part, comma, dir, past_device ? past_device : imagepath);
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("parsing block path '%s'\n", pathname);
#endif
		break;
	case TYPE_NET:
		if (past_device)
			len = strlen(path_device(default_device)) + 1 + strlen(past_device);
		else {
			len = strlen(path_device(default_device)) + 1;
			if (path_net_before(default_device))
				len += strlen(path_net_before(default_device));
			len++;
			len += strlen(imagepath);
			if (path_net_after(default_device)) {
				len += 1 + strlen(path_net_after(default_device));
				comma = ",";
			}
		}
		len += 2;
		pathname = malloc(len);
		if (pathname)
			sprintf(pathname, "%s:%s,%s%s%s", path_device(default_device),
				path_net_before(default_device) ? path_net_before(default_device) : "",
				past_device ? past_device : imagepath, comma, path_net_after(default_device) ? path_net_after(default_device) : "");
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
		path_device(default_device) = endp;
		endp = strchr(path_device(default_device), ':');
		if (endp)
			endp[0] = '\0';
		default_device->type = prom_get_devtype(path_device(default_device));
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
int yaboot_set_bootpath(const char *imagepath, struct path_description *result)
{
	result->type = TYPE_UNSET;
	return parse_device_path(imagepath, result);
}
