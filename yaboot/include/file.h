/*
 *  file.h - Filesystem related interfaces
 *
 *  Copyright (C) 2001 Ethan Benson
 *
 *  parse_device_path()
 *
 *  Copyright (C) 2001 Colin Walters
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

#ifndef FILE_H
#define FILE_H

#include "types.h"
#include "stddef.h"
#include "prom.h"
#include "errors.h"

struct boot_file_t;
struct boot_fspec_t;
#include "fs.h"

#define FILE_MAX_PATH		1024

struct default_device {
	enum device_type type;
	char *device;
	int part;
};

struct boot_fspec_t {
	char*	dev;		/* OF device path */
	int	part;		/* Partition number or -1 */
	char*	file;		/* File path */

	enum device_type type;

	char *device;

	char *partition;
	char *directory;

	char *ip_before_filename;
	char *ip_after_filename;

	char *filename;
};

struct boot_file_t {

	/* File access methods */
        const struct fs_t *fs;

	/* Filesystem private (to be broken once we have a
	 * better malloc'ator)
	 */

	enum device_type dev_type;
	ihandle		of_device;
	ino_t		inode;
	__u64           pos;
	unsigned char*	buffer;
	__u64   	len;
//	unsigned int	dev_blk_size;
//	unsigned int	part_start;
//	unsigned int	part_count;
};

extern int
open_file(const struct boot_fspec_t*	spec,
	  struct boot_file_t*		file);

extern int
parse_device_path(char *imagepath, char *defdevice, int defpart,
		  char *deffile, struct boot_fspec_t *result);

int new_parse_device_path(const char *imagepath, struct boot_fspec_t *result);
int new_parse_file_to_load_path(const char *imagepath, struct boot_fspec_t *result, const struct boot_fspec_t *b, const struct default_device *d);
int new_set_def_device(const char *dev, const char *partition, struct default_device *def);
#define dump_boot_fspec_t(p) do { __dump_boot_fspec_t(__FUNCTION__,__LINE__,p); } while(0)
void __dump_boot_fspec_t (const char *fn, int l, const struct boot_fspec_t *p);

#endif
