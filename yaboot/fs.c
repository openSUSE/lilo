/*
    FileSystems common definitions

    Copyright (C) 1999 Benjamin Herrenschmidt

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "types.h"
#include "stddef.h"
#include "prom.h"
#include "partition.h"
#include "fs.h"

extern const struct fs_t	of_filesystem;
extern const struct fs_t	of_net_filesystem;
extern const struct fs_t	ext2_filesystem;
//extern const struct fs_t	iso_filesystem;

const struct fs_t* filesystems[fs_count] = {
	&of_filesystem,		/* OF */
	&of_net_filesystem,	/* OF (network - no seek) */
	&of_filesystem,		/* HFS/HFS+ */
	&ext2_filesystem,	/* ext2 */
	&of_filesystem,		/* ISO9660 */
	&of_filesystem,		/* UFS */
	&of_filesystem		/* UDF */
};

const int part_2_fs_map[] = 
{
	fs_of,		// partition_unknown
	fs_hfs,		// partition_machfs
	fs_ext2,	// partition_ext2
	fs_hfs,		// partition_macboot
	fs_iso,		// partition_iso
	fs_ufs		// partition_ufs
};

