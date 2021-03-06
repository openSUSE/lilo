/*
 *  partition.h - partition table support
 *
 *  Copyright (C) 2001 Ethan Benson
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

#ifndef PARTITION_H
#define PARTITION_H



enum disk_label {
	LABEL_MAC,
	LABEL_MSDOS,
	LABEL_AMIGA,
	LABEL_ISO9660,
	LABEL_GPT,
};

struct partition_t {
	struct partition_t*	next;
	int			part_number;
	unsigned long long	part_start; /* In blocks */
	unsigned long long	part_size; /* In blocks */
	unsigned short		blocksize;
	enum disk_label		label;
	int			sys_ind; /* fs type */
};

extern struct partition_t*	partitions_lookup(const char *device);
extern void			partitions_free(struct partition_t* list);
extern char lilo_once_cmdline[];

#endif
