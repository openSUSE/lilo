/* $Id$ */
#include <stdio.h>
#include <string.h>
#include <prom.h>

#undef DEBUG

static int nc, ncmax;

static char ptype[32];
#if 0
static char ptp[1024];
static char scsiid[] = "01234567";
static char scsi2id[] = "0123456789abcdef";
static char ata[] = "01";

int inspect_block_node(phandle node, char *devs)
{
	phandle parent_node;
	ihandle handle;
	int i, splen, ret = 0;
	char *sp, *p;

	parent_node = of1275_parent(node);
	of1275_getprop(parent_node, "device_type", ptype, sizeof(ptype));
	if (strcmp("scsi-2", ptype) == 0)
		sp = scsi2id;
	else if (strcmp("scsi", ptype) == 0)
		sp = scsiid;
	else if (strcmp("ata", ptype) == 0)
		sp = ata;
	else if (strcmp("ide", ptype) == 0)
		sp = ata;
	else {
		printf("block host_type '%s' unknown\n\r", ptype);
		return -1;
	}

	p = NULL;
	ptp[0] = '\0';
	of1275_package_to_path(parent_node, ptp, sizeof(ptp));
#ifdef DEBUG
	printf("parent %p '%s' ", parent_node, ptp);
	printf("'%s'\n\r", ptype);
#endif
	of1275_package_to_path(node, ptp, sizeof(ptp));
	i = strlen(ptp);

	if (i > 0) {
		i--;
		if (ptp[i] >= '0' && ptp[i] <= '9')
			while (i > 0)
				if ('@' == ptp[i--])
					break;

		p = &ptp[i + 1];
		*p = '@';
		p++;
	}
	if (p) {
		p[1] = '\0';
		splen = strlen(sp) - 1;
		for (i = 0; i < splen; i++) {
			*p = sp[i];
			devs[i] = '\0';
			handle = of1275_open(ptp);
			if (!handle)
				continue;
#ifdef DEBUG
			printf("opened:'%s' '%p'\n\r", ptp, handle);
#endif
			devs[i] = sp[i];
			of1275_close(handle);
		}
		ret = 1;
	}
	return ret;
}

void show_block_devices(void)
{
	phandle block_phandle[64];

	block_phandle[0] = 0;
	find_type_devices(block_phandle, "block",
			  sizeof(block_phandle) / sizeof(phandle));
	if (block_phandle[0]) {
		char devs[16];
		int i, j;
		for (i = 0; block_phandle[i]
		     && i < sizeof(block_phandle) / sizeof(phandle); i++) {
			memset(devs, 0, sizeof(devs));
			inspect_block_node(block_phandle[i], devs);
			of1275_package_to_path(block_phandle[i], ptp,
					       sizeof(ptp));
			for (j = 0; j < sizeof(devs); j++)
				if (devs[j]) {
					printf("'%s' devs: ", ptp);
					break;
				}
			for (j = 0; j < sizeof(devs); j++)
				if (devs[j])
					printf("'%c' ", devs[j]);
			printf("\n\r");
		}
	}
}
#endif

static void walk_dev_tree(phandle root, const char *type, phandle * nodes)
{
	phandle node;

	node = of1275_child(root);
	while (node) {
		of1275_getprop(node, "device_type", ptype, sizeof(ptype));
		if (strcmp(type, ptype) == 0) {
			nodes[nc] = node;
			if (nc == ncmax)
				return;
			nc++;
			nodes[nc] = 0;
		}
		walk_dev_tree(node, type, nodes);
		node = of1275_peer(node);
	}
}

void find_type_devices(phandle * nodes, const char *type, int max)
{
	phandle root = of1275_peer(0);
	nc = 0;
	ncmax = max;
	walk_dev_tree(root, type, nodes);
}
