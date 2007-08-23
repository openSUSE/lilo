/*
 *  mac-part.h - Structure of Apple partition tables
 *
 *  Copyright (C) 1996 Paul Mackerras
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

#define MAC_PARTITION_MAGIC	0x504d

/* type field value for A/UX or other Unix partitions */
#define APPLE_AUX_TYPE	"Apple_UNIX_SVR2"

struct mac_partition {
    u16	signature;	/* expected to be MAC_PARTITION_MAGIC */
    u16	res1;
    u32	map_count;	/* # blocks in partition map */
    u32	start_block;	/* absolute starting block # of partition */
    u32	block_count;	/* number of blocks in partition */
    char	name[32];	/* partition name */
    char	type[32];	/* string type description */
    u32	data_start;	/* rel block # of first data block */
    u32	data_count;	/* number of data blocks */
    u32	status;		/* partition status */
    u32	boot_start;	/* logical start block no. of bootstrap */
    u32	boot_size;	/* no. of bytes in bootstrap */
    u32	boot_load;	/* bootstrap load address in memory */
    u32	boot_load2;	/* reserved for extension of boot_load */
    u32	boot_entry;	/* entry point address for bootstrap */
    u32	boot_entry2;	/* reserved for extension of boot_entry */
    u32	boot_cksum;
    char	processor[16];	/* name of processor that boot is for */
};

/* Bit in status field */
#define STATUS_BOOTABLE	8	/* partition is bootable */

#define MAC_DRIVER_MAGIC	0x4552

/* Driver descriptor structure, in block 0 */
struct mac_driver_desc {
    u16	signature;	/* expected to be MAC_DRIVER_MAGIC */
    u16	block_size;
    u32	block_count;
    u16 dev_type;
    u16 dev_id;
    u32 data;
    u16 driver_count;
    u8 driverlist[488];
    u16 padding[3];
};
