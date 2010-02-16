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

struct option {
	const char *name;
	unsigned int flag;
};

/* Device Support Extensions */
static struct option interface_options[] = {
	{.name = "duplex",.flag = FLAG_MAYBE_EQUALSIGN},
	{.name = "speed",.flag = FLAG_MAYBE_EQUALSIGN},
	{.name = "promiscuous"},
	{}
};

/* do TFTP unless one command is given */
static struct option command_keywords[] = {
	{.name = "iscsi",.flag = FLAG_COMMAND | FLAG_ISCSI},
	{}
};

static struct option qualifier_options[] = {
	{.name = "rawio",.flag = FLAG_QUALIFIER},
	{.name = "ipv6",.flag = FLAG_QUALIFIER | FLAG_IPV6},
	{.name = "bootp",.flag = FLAG_QUALIFIER},
	{.name = "dhcpv6",.flag = FLAG_QUALIFIER | FLAG_MAYBE_EQUALSIGN},
	{.name = "isns",.flag = FLAG_QUALIFIER | FLAG_MAYBE_EQUALSIGN},
	{.name = "slp",.flag = FLAG_QUALIFIER | FLAG_MAYBE_EQUALSIGN},
	{}
};

static struct option tftp_ipv6_boostrap_options[] = {
	{.name = "dhcpv6",.flag = FLAG_MAYBE_EQUALSIGN},
	{.name = "ciaddr",.flag = FLAG_NEEDS_EQUALSIGN},
	{.name = "giaddr",.flag = FLAG_NEEDS_EQUALSIGN},
	{.name = "siaddr",.flag = FLAG_NEEDS_EQUALSIGN},
	{.name = "filename",.flag = FLAG_NEEDS_EQUALSIGN | FLAG_FILENAME},
	{.name = "tftp-retries",.flag = FLAG_NEEDS_EQUALSIGN},
	{.name = "blksize",.flag = FLAG_NEEDS_EQUALSIGN},
	{}
};

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

static void parse_block_device(struct path_description *result, const char *blockdev_options)
{
	const char *pd, *pf;
	char *p;
	int part;
	size_t len;

	DEBUG_F("options '%s'\n", blockdev_options);
	part = strtol(blockdev_options, &p, 10);
	/* if there is no partition number, ignore it */
	if (part <= 0)
		part = -1;
	path_part(result) = part;
	/* find the comma after the partition number */
	pd = strchr(blockdev_options, ',');
	/* there is a comma, hopefully its not part of a filesystem path */
	if (pd)
		pd++;		/* skip it */
	else
		pd = blockdev_options;	/* path may begin with a directory */
	/* find the last filesystem delimiter, can be either /, or \ for firmware pathnames */
	pf = strrchr(pd, '/');
	if (!pf)
		pf = strrchr(pd, '\\');
	if (pf) {
		len = pf - pd;
		/* the delimiter itself is also required */
		len++;
		p = malloc(len + 1);
		if (p) {
			memcpy(p, pd, len);
			p[len] = '\0';
			path_directory(result) = p;
		}
		/* skip the filesystem delimiter */
		pf++;
	} else
		pf = pd;	/* path contains only the filename */
	path_filename(result) = strdup(pf);
}

/*
 * search options listed in *opt
 * set flags in path for all options found
 * stop at first unknown string, or if stopflag is found
 * return the len of found options at offset
 * FIXME: len still includes the trailing comma
 */
size_t parse_options(const char *string, size_t offset, struct path_description *path, struct option *opt, unsigned int stopflag)
{
	struct option *opt_save;
	char b[64], *eq;
	const char *pos;
	int match;
	size_t ol, sl, new_offset;

	if (!string || !*string)
		return 0;

	ol = sl = 0;
	new_offset = offset;

	opt_save = opt;
	do {
		match = 0;

		/* check how long the current option is */
		pos = strchr(string + new_offset, ',');
		if (pos)
			sl = pos - (string + new_offset);
		else
			sl = strlen(string + new_offset);

		/* its too long or there is nothing up to the next comma */
		if (!sl || sl > (sizeof(b) - 1))
			continue;

		memcpy(b, string + new_offset, sl);
		b[sl] = '\0';
		eq = strchr(b, '=');

		while (opt->name) {
			ol = strlen(opt->name);
			/* option must be at least as long as the string */
			if (sl >= ol) {
				/* truncate at the equal sign to simplify matching */
				if (eq)
					*eq = '\0';
				match = strcmp(opt->name, b) == 0;
				if (eq)
					*eq = '=';
				if (match) {
					/* skip if option needs an equalsign */
					if (!(!eq && opt->flag & FLAG_NEEDS_EQUALSIGN)) {
						path_flags(path) |= opt->flag;
						if (opt->flag & stopflag) {
							/* forward offset up to the = */
							new_offset += ol + 1;
							/* get out of the loop */
							match = 0;
						} else {
							if (pos) {
								new_offset += sl;	/* up to the end of the option */
								new_offset++;	/* skip the comma after current match */
							}
						}
						break;
					}
				}
			}
			opt++;
		}
		opt = opt_save;
	} while (match && pos);	/* one or more match, or no more data */
	/* return the difference */
	return new_offset - offset;
}

/*
 * When the bootpath is on an "iscsi" device,
 * the firmware needs to be passed the contents of the * "nas-bootdevice" property.
 * It points to a block device with options.
 * keep this path+options as the full devicepath to a block device
 */
static const char nas[] = "nas-bootdevice";
static void get_iscsi_options(struct path_description *path, const char *netdev_options, size_t offset)
{
	char *p;
	int size;

	path_part(path) = -1;	/* Scan all partitions */
	path_type(path) = TYPE_ISCSI;
	size = prom_getproplen_chosen(nas);
	if (size <= 0) {
		prom_printf("'%s' property missing for iSCSI boot\n", nas);
		return;
	}
	p = malloc(size + 2);	/* FIXME: fix the additional 2 */
	if (p) {
		prom_get_chosen(nas, p, size);
		DEBUG_F("%s: <%s>\n", nas, p);
	} else
		p = "";
	path_device(path) = p;
}

static void get_tftp_ipv6_options(struct path_description *path, const char *netdev_options, size_t offset)
{
	char *p, *pf;
	size_t len;

	/* stop parsing at filename= */
	len = parse_options(netdev_options, offset, path, &tftp_ipv6_boostrap_options[0], FLAG_FILENAME);
	/*
	 * need SOME data to let firmware download from IPv6 TFTP server
	 * relies on parser to return the len of filename= itself
	 */
	if (!len)
		return;

	/* forward offset to include all options including filename= */
	offset += len;
	p = malloc(offset + 1);
	if (p) {
		memcpy(p, netdev_options, offset);
		p[offset] = '\0';
		path_net_before(path) = p;
	}
	/* find len of filename value */
	p = strchr(netdev_options + offset, ',');
	if (!p) {
		/* no more options, use filename value */
		pf = malloc(strlen(netdev_options + offset) + 1);
		if (pf) {
			strcpy(pf, netdev_options + offset);
			path_filename(path) = pf;
		}
	} else {
		len = p - (netdev_options + offset);
		pf = malloc(len + 1);
		if (pf) {
			memcpy(pf, netdev_options + offset, len);
			pf[len] = '\0';
			path_filename(path) = pf;
		}
		/* skip the comma */
		p++;
		/* use all whats left */
		path_net_after(path) = strdup(p);
	}
}

static void ipv4_to_ascii(char *buf, u32 ip)
{
	if (buf)
		sprintf(buf, "%u.%u.%u.%u", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
}

static u32 ascii_to_ipv4(char *str)
{
	int i;
	long tmp;
	u32 ip = 0;
	char *ptr=str, *endptr;

	if (str == NULL)
		return 0;

	for (i=0; i<4; i++, ptr = ++endptr) {
		tmp = strtol(ptr, &endptr, 10);
		if ((tmp & 0xff) != tmp)
			return 0;

		/* If we reach the end of the string but we're not in the 4th
		 * octet we have an invalid IP */
		if (*endptr == '\x0' && i!=3)
			return 0;

		/* If we have anything other than a NULL or '.' we have an
		 * invlaid IP */
		if (*endptr != '\x0' && *endptr != '.')
			return 0;

		ip += (tmp << (24-(i*8)));
	}
	return ip;
}

static void get_dhcp_ipv4_options(const struct dhcp_packet *dp, int size, u32 * netmask, u32 * gateway)
{
	unsigned char *p, value, len;
	u32 i;
	/* add 4 byte cookie offset */
	i = offsetof(struct dhcp_packet, options) + 4;
	*netmask = 0;
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

/*
 * If booted via bootp/dhcp, all IPv4 addresses may be zero
 * The new IBM CAS firmware expects that we construct a new bootpath based on the bootp response packet
 * Assume all data was supplied manually if the property does not exist
 */

static int get_tftp_ipv4_ibm_CAS(struct path_description *path, const char *netdev_options, size_t offset)
{
	const char *p;
	char *p1, *pf, buf[sizeof("000.000.000.000")];
	char gw[sizeof("000.000.000.000")];
	int bootp_retry, tftp_retry, tftp_blocksize;
	struct dhcp_packet *dp;
	u32 netmask_ipv4 = 0, gateway_ipv4 = 0;
	int size, i;

	size = prom_getproplen_chosen("bootp-response");
	if (size <= 0)
		return 0;
	/* IPv4 data via DHCP */
	dp = malloc(size);
	if (!dp)
		return 0;
	prom_get_chosen("bootp-response", dp, size);

	bootp_retry = tftp_retry = 5;
	tftp_blocksize = 512;

	p = netdev_options + offset;
	/* find comma after server IPv4 */
	p = strchr(p, ',');
	if (p) {
		p++;
		/* find comma after filename */
		p1 = strchr(p, ',');
		if (p1) {
			i = p1 - p;
			pf = malloc(i + 1);
			if (pf) {
				memcpy(pf, p, i);
				pf[i] = '\0';
				path_filename(path) = pf;
			}
			p1++;	/* skip comma */
			p = p1;

			/* Skip the client IPv4, snarf the IPv4 gateway */
			p = strchr(p, ',');
			if (p) {
				p++; /* skip comma */
				p1 = strchr(p, ',');
				if (p1) {
					/* copy the gw IP string */
					i = p1 - p;
					memcpy(gw, p, i);
					p = p1;
					p++;

					gw[i] = '\x0';
					gateway_ipv4 = ascii_to_ipv4(&gw[0]);
				}
			}
		} else {
			/* nothing after filename */
			path_filename(path) = strdup(p);
		}
	}
	if (p) {
		bootp_retry = strtol(p, &p1, 10);
		if (bootp_retry == 0 && (p == p1))
			bootp_retry = 5;
		if (p1)
			p = p1;
		/* find comma after bootp-retry */
		p = strchr(p, ',');
		if (p) {
			p++;
			tftp_retry = strtol(p, &p1, 10);
			if (tftp_retry == 0 && (p == p1))
				tftp_retry = 5;
			if (p1)
				p = p1;
			/* find comma after tftp-retry */
			p = strchr(p, ',');
			if (p) {
				p++;
				/* find comma after netmask */
				p = strchr(p, ',');
				if (p) {
					p++;
					tftp_blocksize = strtol(p, &p1, 10);
					if (tftp_blocksize == 0 && (p == p1))
						tftp_blocksize = 512;

				}
			}
		}
	}
	/* FIXME: We propably should check for subnetmask in the options, we're
	 * okay as long as it's also in the vendor oprions */
	get_dhcp_ipv4_options(dp, size, &netmask_ipv4, &gateway_ipv4);
	ipv4_to_ascii(buf, dp->siaddr);
	i = offset + 1 + strlen(buf) + 1;	/* options + comma + server IPv4 + null */
	p1 = malloc(i);
	if (p1) {
		memcpy(p1, netdev_options, offset);	/* FIXME: this includes trailing comma */
		memcpy(p1 + offset, buf, strlen(buf) + 1);	/* this includes trailing NUL */
		path_net_before(path) = p1;
	}
	i = 3 * sizeof(buf) + 3 * sizeof(buf) + 5 + 1;	/* 3 * IPv4 + 3 * int + 5 comma + null */
	p1 = malloc(i);
	if (p1) {
		ipv4_to_ascii(buf, dp->yiaddr);
		i = strlen(buf);
		memcpy(p1, buf, i);
		p1[i] = ',';
		i++;

		ipv4_to_ascii(buf, gateway_ipv4);
		memcpy(p1 + i, buf, strlen(buf));
		i += strlen(buf);
		p1[i] = ',';
		i++;

		ipv4_to_ascii(buf, netmask_ipv4);

		sprintf(p1 + i, "%d,%d,%s,%d", bootp_retry, tftp_retry, buf, tftp_blocksize);

		path_net_after(path) = p1;
	}
	return 1;
}

static void get_tftp_ipv4_options(struct path_description *path, const char *netdev_options, size_t offset)
{
	char *p, *pf;
	size_t i;

	if (netdev_options[offset]) {
		if (get_tftp_ipv4_ibm_CAS(path, netdev_options, offset) == 0) {
			/* find comma after server IPv4 */
			p = strchr(netdev_options + offset, ',');
			DEBUG_F("comma after server IPv4 '%s' -> '%s'\n", netdev_options + offset, p);
			if (p) {
				offset = p - netdev_options;
				p = malloc(offset + 1);
				if (p) {
					/* keep everything up to the filename */
					memcpy(p, netdev_options, offset);
					p[offset] = '\0';
					path_net_before(path) = p;
				}
				offset++;	/* skip the comma before the filename */
				p = strchr(netdev_options + offset, ',');	/* comma after the filename */
				DEBUG_F("comma after filename '%s'\n", p);
				if (p) {
					i = p - (netdev_options + offset);
					pf = malloc(i + 1);
					if (pf) {
						memcpy(pf, netdev_options + offset, i);
						pf[i] = '\0';
						path_filename(path) = pf;
					}
					offset = p - netdev_options;
					offset++;	/* skip the comma after the filename */
					i = strlen(netdev_options + offset);
					p = malloc(i + 1);
					if (p) {
						strcpy(p, netdev_options + offset);
						path_net_after(path) = p;
					}
				} else {
					/* nothing after filename */
					path_filename(path) = strdup(netdev_options + offset);
				}
			} else {
				/* no filename was provided */
				i = strlen(netdev_options + offset);
				p = malloc(i + 1);
				if (p) {
					strcpy(p, netdev_options + offset);
					path_net_before(path) = p;
				}
			}
		}
	} else {
		/* maybe pmac, booted without any options */
		p = malloc(offset + 1);
		if (p) {
			strcpy(p, netdev_options);
			path_net_before(path) = p;
		}
	}
}

static void get_tftp_options(struct path_description *path, const char *netdev_options, size_t offset)
{
	DEBUG_F("ipv%d tftp: netdev_options '%s' offset '%s'\n", path_flags(path) & FLAG_IPV6 ? 6 : 4, netdev_options, netdev_options + offset);
	if (path_flags(path) & FLAG_IPV6)
		get_tftp_ipv6_options(path, netdev_options, offset);
	else
		get_tftp_ipv4_options(path, netdev_options, offset);
}

static void parse_net_device(struct path_description *path, const char *netdev_options)
{
	size_t offset, newlen;

	offset = 0;
	newlen = parse_options(netdev_options, offset, path, &interface_options[0], 0);
	if (newlen)
		offset += newlen;

	newlen = parse_options(netdev_options, offset, path, &command_keywords[0], 0);
	if (newlen)
		offset += newlen;

	newlen = parse_options(netdev_options, offset, path, &qualifier_options[0], 0);
	if (newlen)
		offset += newlen;

	if (path_flags(path) & FLAG_ISCSI)
		get_iscsi_options(path, netdev_options, offset);
	else
		get_tftp_options(path, netdev_options, offset);
}

static void get_mac_address(struct path_description *result)
{
	phandle dev = prom_finddevice(path_device(result));
	if (dev != PROM_INVALID_HANDLE) {
		if (prom_getprop(dev, "mac-address", &result->u.n.mac, 6) == -1)
			prom_getprop(dev, "local-mac-address", &result->u.n.mac, 6);
		prom_printf("MAC for %s: %02x:%02x:%02x:%02x:%02x:%02x\n", path_device(result),
			    result->u.n.mac[0], result->u.n.mac[1], result->u.n.mac[2], result->u.n.mac[3], result->u.n.mac[4], result->u.n.mac[5]);
	}
}

/*
 * split the full image path into device and options part
 * parse options depending on the device type
 */
static int parse_device_path(const char *imagepath, struct path_description *result)
{
	char *colon, *p;
	size_t offset;
	DEBUG_F("imagepath '%s'\n", imagepath);
	if (!imagepath)
		return 0;

	offset = 0;
	if (path_type(result) == TYPE_UNSET) {
		colon = strchr(imagepath, ':');
		if (colon)
			offset = colon - imagepath;
		else
			offset = strlen(imagepath);
		p = malloc(offset + 1);
		if (!p)
			return 0;
		memcpy(p, imagepath, offset);
		p[offset] = '\0';
		if (colon)
			offset++;	/* skip the colon */
		path_device(result) = p;

		path_type(result) = prom_get_devtype(path_device(result));
	}
	switch (path_type(result)) {
	case TYPE_BLOCK:
		parse_block_device(result, imagepath + offset);
		break;
	case TYPE_NET:
		get_mac_address(result);
		parse_net_device(result, imagepath + offset);
		break;
	case TYPE_INVALID:
		prom_printf("firmware said the path '%s' is invalid\n", path_device(result));
		return 0;
	default:
		prom_printf("type %d of '%s' not handled\n", path_type(result), path_device(result));
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
	switch (path_type(input)) {
	case TYPE_ISCSI:
	case TYPE_BLOCK:
		if (path_part(input) > 0)
			sprintf(part, "%d,", path_part(input));
		else {
			part[0] = ',';
			part[1] = '\0';
		}
		len += strlen(part);
		if (path_directory(input))
			len += strlen(path_directory(input));
		path = malloc(len);
		if (path)
			sprintf(path, "%s:%s%s%s", path_device(input), part, path_directory(input) ? path_directory(input) : "", path_filename(input));
		break;
	case TYPE_NET:
		len += strlen(path_net_before(input));
		if (path_net_after(input)) {
			len++;
			len += strlen(path_net_after(input));
		}
		path = malloc(len);
		if (path)
			sprintf(path, "%s:%s%s%s%s%s", path_device(input),
				path_net_before(input),
				path_flags(input) & FLAG_IPV6 ? "" : ",",
				path_filename(input), path_net_after(input) ? "," : "", path_net_after(input) ? path_net_after(input) : "");
		break;
	default:
		break;
	}
	return path;
}

/*
 * Take the full or relative imagepath and build a full path
 * If its a relative path, use the device yaboot was loaded from as a base
 * Handle special string '&device;' as device yaboot was loaded from
 * returns 0 if something cant be parsed
 */
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
	switch (path_type(default_device)) {
	case TYPE_ISCSI:
		path_part(result) = path_part(default_device);
		path_device(result) = strdup(path_device(default_device));
		if (!path_device(result))
			return 0;
		path_filename(result) = strdup(imagepath);
		if (!path_filename(result)) {
			free(path_device(result));
			return 0;
		}
#if defined(DEBUG) || defined(DEVPATH_TEST)
		prom_printf("hardcoded iscsi path '%s' '%d' '%s'\n", path_device(result), path_part(result), path_filename(result));
#endif
		/* do not call parse_device_path */
		return 1;
	case TYPE_BLOCK:
		part[0] = '\0';
		/* parse_device_path will look for a partition number */
		if (past_device)
			len = strlen(path_device(default_device)) + 1 + strlen(past_device);
		else {
			if (path_part(default_device) > 0)
				sprintf(part, "%d", path_part(default_device));
			comma = ",";
			if (imagepath[0] != '/' && imagepath[0] != '\\' && path_directory(default_device))
				dir = path_directory(default_device);
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
			sprintf(pathname, "%s:%s%s%s%s%s", path_device(default_device),
				path_net_before(default_device) ? path_net_before(default_device) : "",
				path_flags(default_device) & FLAG_IPV6 ? "" : ",",
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
		path_type(default_device) = prom_get_devtype(path_device(default_device));
	}

	if (partition) {
		n = simple_strtol(partition, &endp, 10);
		if (endp != partition && *endp == 0) {
			if (TYPE_UNSET == path_type(default_device))
				path_type(default_device) = TYPE_BLOCK;
			path_part(default_device) = n;
		}
	}
}
int yaboot_set_bootpath(const char *imagepath, struct path_description *result)
{
	path_type(result) = TYPE_UNSET;
	return parse_device_path(imagepath, result);
}
