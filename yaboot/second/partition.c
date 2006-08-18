/*
 *  partition.c - partition table support
 *
 *  Copyright (C) 2004 Sven Luther
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
 *
 *  Copyright (C) 1999 Benjamin Herrenschmidt
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 *  Todo: Add disklabel (or let OF do it ?). Eventually think about
 *        fixing CDROM handling by directly using the ATAPI layer.
 */

#include "ctype.h"
#include "types.h"
#include "stddef.h"
#include "stdlib.h"
#include "mac-part.h"
#include "fdisk-part.h"
#include "amiga-part.h"
#include "partition.h"
#include "prom.h"
#include "string.h"
#include "linux/iso_fs.h"
#include "debug.h"
#include "errors.h"
#include "byteorder.h"

#define MAX_BLOCK_SIZE	2048
static char block_buffer[MAX_BLOCK_SIZE];

static void
add_new_partition(struct partition_t **list, int part_number,
		  unsigned long part_start, unsigned long part_size,
		  enum disk_label label, unsigned short part_blocksize, int sys_ind)
{
	struct partition_t *part;
	part = (struct partition_t *)malloc(sizeof(struct partition_t));

	if (part) {
		part->part_number = part_number;
		part->part_start = part_start;
		part->part_size = part_size;
		part->blocksize = part_blocksize;
		part->label = label;
		part->sys_ind = sys_ind;

		/* Tack this entry onto the list */
		part->next = *list;
		*list = part;
	}
}

/* Note, we rely on partitions being dev-block-size aligned,
 * I have to check if it's true. If it's not, then things will get
 * a bit more complicated
 */
static int mac_magic_present(char *block_buffer)
{
	struct mac_driver_desc *desc = (struct mac_driver_desc *)block_buffer;
	return desc->signature == MAC_DRIVER_MAGIC;
}

static void partition_mac_lookup(prom_handle disk, struct partition_t **list)
{
	int block, map_size;

	/* block_buffer contains block 0 from the partitions_lookup() stage */
	struct mac_partition *part = (struct mac_partition *)block_buffer;
	unsigned short ptable_block_size = ((struct mac_driver_desc *)block_buffer)->block_size;

	map_size = 1;
	for (block = 1; block < map_size + 1; block++) {
		if (prom_readblocks(disk, block, 1, block_buffer) != 1) {
			prom_printf("Can't read partition %d\n", block);
			break;
		}
		if (part->signature != MAC_PARTITION_MAGIC) {
#if 0
			prom_printf("Wrong partition %d signature\n", block);
#endif
			break;
		}
		if (block == 1)
			map_size = part->map_count;

		/* We use the partition block size from the partition table.
		 * The filesystem implmentations are responsible for mapping
		 * to their own fs blocksize */
		add_new_partition(list,	/* partition list */
				  block,	/* partition number */
				  part->start_block + part->data_start,	/* start */
				  part->data_count,	/* size */
				  LABEL_MAC, ptable_block_size, 0);
	}
}

/* 
 * Same function as partition_mac_lookup(), except for fdisk
 * partitioned disks.
 */
static int msdos_magic_present(char *buffer)
{
	return (buffer[510] == 0x55) && (buffer[511] == 0xaa);
}

static int msdos_is_linux_partition(unsigned char sys_ind)
{
	return (sys_ind == LINUX_NATIVE || sys_ind == LINUX_RAID);
}

static int msdos_is_extended_partition(unsigned char sys_ind)
{
	return (EXTENDED == sys_ind || WIN98_EXTENDED == sys_ind || LINUX_EXTENDED == sys_ind);
}

static void msdos_parse_extended(prom_handle disk, struct partition_t **list, unsigned int start, unsigned int size)
{
	int i, partition = 5;
	unsigned int partition_start = start;
	unsigned int partition_size = size;
	unsigned int offset, length, next;
	char buffer[MAX_BLOCK_SIZE];
	struct fdisk_partition *part;
	while (1) {
		if (partition_start >= start + size)
			return;
		if (!prom_readblocks(disk, partition_start, 1, buffer))
			return;
		if (!msdos_magic_present(buffer))
			return;
		part = (struct fdisk_partition *)(buffer + 0x1be);
		for (i = 0; i < 4; i++, part++) {
			offset = le32_to_cpu(part->start);
			length = le32_to_cpu(part->size);
			next = partition_start + offset;
			if (!le32_to_cpu(part->size) || msdos_is_extended_partition(part->sys_ind))
				continue;
			if (i >= 2) {
				if (offset + length > partition_size)
					continue;
				if (next < partition_start)
					continue;
				if (next + length > partition_start + partition_size)
					continue;
			}
			if (msdos_is_linux_partition(part->sys_ind))
				add_new_partition(list, partition, next, length, LABEL_MSDOS, 512, part->sys_ind);
			partition++;
		}
		part -= 4;
		for (i = 0; i < 4; i++, part++)
			if (le32_to_cpu(part->size) && msdos_is_extended_partition(part->sys_ind))
				break;
		if (i == 4)
			break;
		partition_start = start + le32_to_cpu(part->start);
		partition_size = le32_to_cpu(part->size);
	}
}

static void partition_fdisk_lookup(prom_handle disk, struct partition_t **list)
{
	int partition;

	/* fdisk partition tables start at offset 0x1be
	 * from byte 0 of the boot drive.
	 */
	struct fdisk_partition *part = (struct fdisk_partition *)(block_buffer + 0x1be);

	for (partition = 1; partition <= 4; partition++, part++) {
		if (msdos_is_linux_partition(part->sys_ind))
			add_new_partition(list, partition, le32_to_cpu(part->start), le32_to_cpu(part->size), LABEL_MSDOS, 512,
					  part->sys_ind);
		else if (msdos_is_extended_partition(part->sys_ind))
			msdos_parse_extended(disk, list, le32_to_cpu(part->start), le32_to_cpu(part->size));
	}
}

/* I don't know if it's possible to handle multisession and other multitrack
 * stuffs with the current OF disklabel package. This can still be implemented
 * with direct calls to atapi stuffs.
 * Currently, we enter this code for any device of block size 0x2048 who lacks
 * a MacOS partition map signature.
 */
static int identify_iso_fs(ihandle device, unsigned int *iso_root_block)
{
	int block;

	for (block = 16; block < 100; block++) {
		struct iso_volume_descriptor *vdp;

		if (prom_readblocks(device, block, 1, block_buffer) != 1) {
			prom_printf("Can't read volume desc block %d\n", block);
			break;
		}

		vdp = (struct iso_volume_descriptor *)block_buffer;

		/* Due to the overlapping physical location of the descriptors, 
		 * ISO CDs can match hdp->id==HS_STANDARD_ID as well. To ensure 
		 * proper identification in this case, we first check for ISO.
		 */
		if (strncmp(vdp->id, ISO_STANDARD_ID, sizeof vdp->id) == 0) {
			*iso_root_block = block;
			return 1;
		}
	}

	return 0;
}

/* 
 * Detects and read amiga partition tables.
 */

static int _amiga_checksum(unsigned int blk_size)
{
	unsigned int sum;
	int i, end;
	unsigned int *amiga_block = (unsigned int *)block_buffer;

	sum = amiga_block[0];
	end = amiga_block[AMIGA_LENGTH];

	if (end > blk_size)
		end = blk_size;

	for (i = 1; i < end; i++)
		sum += amiga_block[i];

	return sum;
}

static int _amiga_find_rdb(prom_handle disk, unsigned int prom_blksize)
{
	int i;
	unsigned int *amiga_block = (unsigned int *)block_buffer;

	for (i = 0; i < AMIGA_RDB_MAX; i++) {
		if (i != 0) {
			if (prom_readblocks(disk, i, 1, block_buffer) != 1) {
				prom_printf("Can't read boot block %d\n", i);
				break;
			}
		}
		if ((amiga_block[AMIGA_ID] == AMIGA_ID_RDB) && (_amiga_checksum(prom_blksize) == 0))
			return 1;
	}
	/* Amiga partition table not found, let's reread block 0 */
	if (prom_readblocks(disk, 0, 1, block_buffer) != 1) {
		prom_printf("Can't read boot blocks\n");
		return 0;	/* TODO: something bad happened, should fail more verbosely */
	}
	return 0;
}

static void partition_amiga_lookup(prom_handle disk, unsigned int prom_blksize, struct partition_t **list)
{
	int partition, part;
	unsigned int blockspercyl;
	unsigned int *amiga_block = (unsigned int *)block_buffer;
	unsigned int *used = NULL;
	unsigned int possible;
	int checksum;
	int i;

	blockspercyl = amiga_block[AMIGA_SECT] * amiga_block[AMIGA_HEADS];
	possible = amiga_block[AMIGA_RDBLIMIT] / 32 + 1;

	used = (unsigned int *)malloc(sizeof(unsigned int) * (possible + 1));
	if (!used)
		return;

	for (i = 0; i < possible; i++)
		used[i] = 0;

	for (part = amiga_block[AMIGA_PARTITIONS], partition = 0;
	     part != AMIGA_END; part = amiga_block[AMIGA_PART_NEXT], partition++) {
		if (prom_readblocks(disk, part, 1, block_buffer) != 1) {
			prom_printf("Can't read partition block %d\n", part);
			break;
		}
		checksum = _amiga_checksum(prom_blksize);
		if ((amiga_block[AMIGA_ID] == AMIGA_ID_PART) && (checksum == 0) && ((used[part / 32] & (0x1 << (part % 32))) == 0)) {
			used[part / 32] |= (0x1 << (part % 32));
		} else {
			prom_printf("Amiga partition table corrupted at block %d\n", part);
			if (amiga_block[AMIGA_ID] != AMIGA_ID_PART)
				prom_printf("block type is not partition but %08x\n", amiga_block[AMIGA_ID]);
			if (checksum != 0)
				prom_printf("block checsum is bad : %d\n", checksum);
			if ((used[part / 32] & (0x1 << (part % 32))) != 0)
				prom_printf("partition table is looping, block %d already traveled\n", part);
			break;
		}

		/* We use the partition block size from the partition table.
		 * The filesystem implmentations are responsible for mapping
		 * to their own fs blocksize */
		add_new_partition(list,	/* partition list */
				  partition,	/* partition number */
				  blockspercyl * amiga_block[AMIGA_PART_LOWCYL],	/* start */
				  blockspercyl * (amiga_block[AMIGA_PART_HIGHCYL] - amiga_block[AMIGA_PART_LOWCYL] + 1),	/* size */
				  LABEL_AMIGA, prom_blksize, 0);
	}
}

struct partition_t *partitions_lookup(const char *device)
{
	ihandle disk;
	struct partition_t *list = NULL;
	unsigned int prom_blksize, iso_root_block;

	strncpy(block_buffer, device, 2040);
	strcat(block_buffer, ":0");

	/* Open device */
	disk = prom_open(block_buffer);
	if (disk == NULL) {
		prom_printf("Can't open device <%s>\n", block_buffer);
		goto bail;
	}
	prom_blksize = prom_getblksize(disk);
	DEBUG_F("block size of device is %d\n", prom_blksize);

	if (prom_blksize <= 1)
		prom_blksize = 512;
	if (prom_blksize > MAX_BLOCK_SIZE) {
		prom_printf("block_size %d not supported !\n", prom_blksize);
		goto bail;
	}

	/* Read boot blocs */
	if (prom_readblocks(disk, 0, 1, block_buffer) != 1) {
		prom_printf("Can't read boot blocks\n");
		goto bail;
	}
	if (mac_magic_present(block_buffer)) {
		/* pdisk partition format */
		partition_mac_lookup(disk, &list);
	} else if (msdos_magic_present(block_buffer)) {
		/* fdisk partition format */
		partition_fdisk_lookup(disk, &list);
	} else if (prom_blksize == 2048 && identify_iso_fs(disk, &iso_root_block)) {
		add_new_partition(&list, 0, iso_root_block, 0, LABEL_ISO9660, prom_blksize, 0);
		prom_printf("ISO9660 disk\n");
	} else if (_amiga_find_rdb(disk, prom_blksize) != -1) {
		/* amiga partition format */
		partition_amiga_lookup(disk, prom_blksize, &list);
	} else {
		prom_printf("No supported partition table detected\n");
		goto bail;
	}

      bail:
	prom_close(disk);

	return list;
}

/* Freed in reverse order of allocation to help malloc'ator */
void partitions_free(struct partition_t *list)
{
	struct partition_t *next;

	while (list) {
		next = list->next;
		free(list);
		list = next;
	}
}

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
