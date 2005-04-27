/*
    Partition map management

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

#ifndef PARTITION_H
#define PARTITION_H

#include "types.h"
#include "stddef.h"
#include "prom.h"

#define MAX_PARTITIONS	32
#define MAX_PART_NAME	32

enum {
	partition_unknown,	/* Try with OF */
	partition_machfs,	/* Apple_HFS */
	partition_ext2,		/* Apple_UNIX_SVR2 */
	partition_macboot,	/* HFS bootstrap */
	partition_iso,		/* ISO9660 (CD ?) */
	partition_ufs		/* UFS (NetBSD) */
};

struct partition_t {
	struct partition_t*	next;
	int			kind;
	int			part_number;
	char			part_name[MAX_PART_NAME];
	unsigned int		part_start;
	unsigned int		part_size;
	unsigned int		blksize;
};

extern struct partition_t*	partitions_lookup(const char *device);
extern void			partitions_free(struct partition_t* list);



#endif
