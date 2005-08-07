/* $Id$ */
#include <stdio.h>
#include <string.h>

#define prom_printf printf
#define simple_strtol strtol
# define DEBUG_F(fmt, args...)\
{\
    prom_printf( "    %s - ", __PRETTY_FUNCTION__ );\
    prom_printf( fmt, ## args );\
}

enum device_type {
	TYPE_UNSET = 0,
	TYPE_UNKNOWN,
	TYPE_INVALID,
	TYPE_BLOCK,
	TYPE_NET
};

struct default_device {
	enum device_type type;
	char *device;
	int part;
};

struct boot_fspec_t {
	char*	dev;		/* OF device path */
	char*	file;		/* File path */

	int part;

	enum device_type type;

	char *device;

	char *partition;
	char *directory;

	char *ip_before_filename;
	char *ip_after_filename;

	char *filename;
};

/*
 * '\\' is the blessed system folder on a HFS partition
 * ':tbxi' is a file with type 'tbxi' on a HFS partition
 */

static const char *block_paths[] = {
#if 0
	NULL,
#endif
	"hd:,\\\\:tbxi",
	"hd:9,\\\\:tbxi",
	"hd:9,\\\\yaboot",
	"hd:9,yaboot",
	"hd:9,\\yaboot",
	"hd:9,\\suseboot\\yaboot",
	"/pci@f2000000/pci-bridge@d/ADPT,2930CU@3/disk@0,0:7,vmlinux",
	"/pci@f2000000/pci-bridge@d/ADPT,2930CU@3/disk@0,0:7,/vmlinux",
	"/pci@f2000000/pci-bridge@d/ADPT,2930CU@3/disk@0,0:7,/boot/vmlinux",
	"&device;:10,/boot/vmlinux",
	"hd:yaboot",
	"hd:\\yaboot",
	"hd:\\\\yaboot",
	"hd:suseboot\\yaboot",
	"hd:\\suseboot\\yaboot",
	NULL,
};

static const char *net_paths[] = {
#if 0
	NULL,
#endif
	"enet:0",
	"enet:0,yaboot",
	"enet:bootp",
	"enet:bootp,1.2.3.4,yaboot",
	"enet:,yaboot",
	"enet:1.2.3.4,yaboot",
	"enet:1.2.3.4,yaboot,1.2.3.42",
	"enet:4.3.2.1,yaboot,1.2.3.42;255.255.0.0,;1.2.1.1",
	"network:1.2.3.4,yaboot,1.2.3.42",
	"network:4.3.2.1,yaboot,1.2.3.42,1.2.1.1",
	"network:speed=auto,duplex=auto,4.3.2.1,yaboot,1.2.3.42,1.2.1.1",
	"network:speed=100,duplex=full,4.3.2.1,yaboot,1.2.3.42,1.2.1.1",
	"network:000.000.000.000,,000.000.000.000,000.000.000.000,00",
	NULL,
};
static const char *file_paths[] = {
#if 0
	NULL,
#endif
	"vmlinux",
	"/vmlinux",
	"/boot/vmlinux",
	"boot/vmlinux",
	"&device;:9,/boot/vmlinux",
	"/pci@f2000000/pci-bridge@d/ADPT,2930CU@3/disk@0,0:7,/boot/vmlinux",
	NULL,
};

static enum device_type current_devtype;

static enum device_type prom_get_devtype(const char *device)
{
	device = device;
	return current_devtype;
}

#define DEVPATH_TEST 1
#include "../second/parse_device_path.c"

#define P(m) printf("\t"#m " '%s' ", p->m);

static void print_boot(const struct boot_fspec_t *p) {
		P(device);
		P(partition);
		P(directory);
		printf("\n");
		P(filename);
		printf("\n");
		P(ip_before_filename);
		P(ip_after_filename);
		printf("\n");
}

static void print_file_to_load(const struct boot_fspec_t *b, const char *p) {
	int i;
	struct boot_fspec_t f;
	for (i = 0; file_paths[i]; i++) {
		memset(&f, 0, sizeof(struct boot_fspec_t));
		new_parse_file_to_load_path(file_paths[i], &f, b, NULL);
		printf("\tfile %d to load from /chosen/bootpath '%s': '%s'\n", i, p, file_paths[i]);
		print_boot(&f);
	}
}
int main(void)
{
	int i;
	struct boot_fspec_t p;

	current_devtype = TYPE_BLOCK;
	for (i = 0; block_paths[i]; i++) {
		memset(&p, 0, sizeof(struct boot_fspec_t));
		new_parse_device_path(block_paths[i], &p);
		printf("path %d: '%s'\n", i, block_paths[i]);
		print_boot(&p);
		print_file_to_load(&p, block_paths[i]);
	}

	current_devtype = TYPE_NET;
	for (i = 0; net_paths[i]; i++) {
		memset(&p, 0, sizeof(struct boot_fspec_t));
		new_parse_device_path(net_paths[i], &p);
		printf("path %d: '%s'\n", i, net_paths[i]);
		print_boot(&p);
		print_file_to_load(&p, net_paths[i]);
	}
	return 0;
}
