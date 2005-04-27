/* File related stuff
   
   Copyright (C) 1999 Benjamin Herrenschmidt

   Todo: Add disklabel (or let OF do it ?). Eventually think about
         fixing CDROM handling by directly using the ATAPI layer.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "ctype.h"
#include "types.h"
#include "stddef.h"
#include "stdlib.h"
#include "mac-part.h"
#include "partition.h"
#include "prom.h"
#include "string.h"
#include "linux/iso_fs.h"

#define MAX_BLOCK_SIZE	2048
static char block_buffer[MAX_BLOCK_SIZE];

static void
add_partition(	struct partition_t**	list,
		int			kind,
		int			part_number,
		const char*		part_name,
		unsigned int		part_start,
		unsigned int		part_size,
		unsigned int		blksize)
{
	struct partition_t*	part;
	
	part = (struct partition_t*)malloc(sizeof(struct partition_t));
	
	part->next = *list;
	part->kind = kind;
	part->part_number = part_number;
	strncpy(part->part_name, part_name, MAX_PART_NAME);
	part->part_start = part_start;
	part->part_size = part_size;
	part->blksize = blksize;
	*list = part;
}

/* Note, we rely on partitions being dev-block-size aligned,
 * I have to check if it's true. If it's not, then things will get
 * a bit more complicated
 */
static void
partition_mac_lookup(prom_handle disk, unsigned int blksize,
	struct partition_t** list)
{
	int block, map_size, kind, blkdiv, i;
	struct mac_partition* part = (struct mac_partition *)block_buffer;
	
	map_size = 1;
	blkdiv = blksize / 512;
	for (block=1; block<(map_size+1); block++) {
//#if DEBUG
//		prom_printf("reading partition %d\n", block);
//#endif	
		if (prom_readblocks(disk, block, 1, block_buffer) != 1) {
			prom_printf("Can't read partition %d\n", block);
			break;
		}
		if (part->signature != MAC_PARTITION_MAGIC) {
			prom_printf("Wrong partition %d signature\n", block);
			break;
		}
		if (block == 1)
			map_size = part->map_count;
		
		for (i=0; (i<32)&&(part->name[i] != 0); i++)
			part->name[i] = tolower(part->name[i]);
		for (i=0; (i<32)&&(part->type[i] != 0); i++)
			part->type[i] = tolower(part->type[i]);

//#if DEBUG
//		prom_printf(" ->name: %.32s\n", part->name);
//		prom_printf(" ->type: %.32s\n", part->type);
//#endif	
		
		kind = partition_unknown;
		if (!strcmp(part->type, "apple_partition_map"))
			kind = -1;
		else if (!strcmp(part->type, "apple_unix_svr2"))
			kind = (strcmp(part->name, "swap") == 0) ? -1 : partition_ext2;
		else if (!strncmp(part->type, "linux", 5)) {
			if (!strcmp(part->type, "linux_swap") || !strcmp(part->name, "swap"))
				kind = -1;
			else
				kind = partition_ext2;
		} else if (!strcmp(part->type, "apple_hfs"))
			kind = partition_machfs;
		else if (!strcmp(part->type, "apple_boot"))
			kind = partition_macboot;
		else if (!strcmp(part->type, "apple_bootstrap"))
			kind = partition_macboot;
		else if (!strncmp(part->type, "apple_driver", 12))
			kind = -1;	
		else if (!strcmp(part->type, "apple_patches"))
			kind = -1;
		else if (!strcmp(part->type, "apple_free"))
			kind = -1;
//#if DEBUG		
//		prom_printf(" ->kind: %d\n", kind);
//#endif		
		/* We set the partition block size to 512 since I beleive that's
		 * what is meant by the partition map, but I have to double check
		 * this (HFS CDROMs may have 2048 block size)
		 */
		if (kind > 0)
			add_partition(	list,
					kind,
					block,
					part->name,
					(part->start_block + part->data_start) / blkdiv,
					part->data_count / blkdiv,
					512/*blksize*/);
	}
}

/* I don't know if it's possible to handle multisession and other multitrack
 * stuffs with the current OF disklabel package. This can still be implemented
 * with direct calls to atapi stuffs.
 * Currently, we enter this code for any device of block size 0x2048 who lacks
 * a MacOS partition map signature.
 */
static int
identify_iso_fs(ihandle device, unsigned int *iso_root_block)
{
	int block;

 	for (block = 16; block < 100; block++) {
	    struct iso_volume_descriptor  * vdp;

	    if (prom_readblocks(device, block, 1, block_buffer) != 1) {
		prom_printf("Can't read volume desc block %d\n", block);
		break;
	    }
 		
	    vdp = (struct iso_volume_descriptor *)block_buffer;
	    
	    /* Due to the overlapping physical location of the descriptors, 
	     * ISO CDs can match hdp->id==HS_STANDARD_ID as well. To ensure 
	     * proper identification in this case, we first check for ISO.
	     */
	    if (strncmp (vdp->id, ISO_STANDARD_ID, sizeof vdp->id) == 0) {
	    	*iso_root_block = block;
	    	return 1;
	    }
	}
	
	return 0;
}

struct partition_t*
partitions_lookup(const char *device)
{
	ihandle	disk;
	struct mac_driver_desc *desc = (struct mac_driver_desc *)block_buffer;
	struct partition_t* list = NULL;
	unsigned int blksize, iso_root_block;
	
	strncpy(block_buffer, device, 2040);
	strcat(block_buffer, ":0");
	
	/* Open device */
	disk = prom_open(block_buffer);
	if (disk == NULL) {
		prom_printf("Can't open device <%s>\n", block_buffer);
		goto bail;
	}
	blksize = prom_getblksize(disk);
#if DEBUG
	prom_printf("block size of device is %d\n", blksize);
#endif	
	if (blksize <= 1)
		blksize = 512;
	if (blksize > MAX_BLOCK_SIZE) {
		prom_printf("block_size %d not supported !\n");
		goto bail;
	}
	
	/* Read boot blocs */
	if (prom_readblocks(disk, 0, 1, block_buffer) != 1) {
		prom_printf("Can't read boot blocs\n");
		goto bail;
	}	
	if (desc->signature == MAC_DRIVER_MAGIC)
		partition_mac_lookup(disk, blksize, &list);
	else if (blksize == 2048 && identify_iso_fs(disk, &iso_root_block)) {
		add_partition(	&list,
				partition_iso,
				0,
				"",
				iso_root_block,
				0,
				blksize);
		prom_printf("ISO9660 disk\n");
	} else {
		prom_printf("Not a macintosh-formatted disk !\n");
		goto bail;
	}

bail:
	prom_close(disk);
	
	return list;
}

/* Freed in reverse order of allocation to help malloc'ator */
void
partitions_free(struct partition_t* list)
{
	struct partition_t*	next;
	
	while(list) {
		next = list->next;
		free(list);
		list = next;
	}
}
