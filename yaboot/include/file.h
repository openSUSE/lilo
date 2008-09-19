/*
 *  file.h - Filesystem related interfaces
 *
 *  Copyright (C) 2001 Ethan Benson
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

#include <types.h>
#include <stddef.h>
#include <prom.h>
#include <errors.h>

struct boot_file_t;
struct path_description;
#include <fs.h>

#define FILE_MAX_PATH		1024

struct default_device {
	enum device_type type;
	char *device;
	int part;
};

enum flags {
	may_have_equal_sign,
	needs_equal_sign,
	got_COMMAND,
	got_QUALIFIER,
	got_FILENAME,
	do_ISCSI,
	do_IPV6,
	do_something,
};

#define FLAG_MAYBE_EQUALSIGN    (1 << may_have_equal_sign)
#define FLAG_NEEDS_EQUALSIGN    (1 << needs_equal_sign)
#define FLAG_COMMAND    (1 << got_COMMAND)
#define FLAG_QUALIFIER  (1 << got_QUALIFIER)
#define FLAG_FILENAME   (1 << got_FILENAME)
#define FLAG_ISCSI      (1 << do_ISCSI)
#define FLAG_IPV6       (1 << do_IPV6)
#define FLAG_SOMETHING  (1 << do_something)

/* describes individual parts of a firmware path
 * block:   <device>:<partition>,<directory/><filename>
 * network: <device>:<before_filename>,<filename>,<after_filename>
 */
struct path_description {
	int part;		/* Partition number or -1 */

	enum device_type type;
#define path_flags(x) (x)->flags
	unsigned int flags;

#define path_device(x) (x)->device
	char *device;

	union {
		struct {
#define path_partition(x) (x)->u.b.partition
			char *partition;
#define path_directory(x) (x)->u.b.directory
			char *directory;
		} b;
		struct {
#define path_net_before(x) (x)->u.n.ip_before_filename
			char *ip_before_filename;
#define path_net_after(x) (x)->u.n.ip_after_filename
			char *ip_after_filename;
			unsigned char mac[6];
			char *dev_options;
			u32 client_ip;
			u32 server_ip;
			u32 gateway_ip;
			u32 netmask;
			u32 bootp_retry;
			u32 tftp_retry;
			u32 tftp_blocksize;
		} n;
		struct {
			char *s1;
			char *s2;
		} d;
	} u;

#define path_filename(x) (x)->filename
	char *filename;
};

struct boot_file_t {

	/* File access methods */
	const struct fs_t *fs;

	/* Filesystem private (to be broken once we have a
	 * better malloc'ator)
	 */

	enum device_type dev_type;
	ihandle of_device;
	u32 inode;
	u64 pos;
	unsigned char *buffer;
	u64 len;
//      unsigned int    dev_blk_size;
//      unsigned int    part_start;
//      unsigned int    part_count;
};

extern int open_file(const struct path_description *spec, struct boot_file_t *file);

char *path_description_to_string(const struct path_description *input);
int imagepath_to_path_description(const char *imagepath, struct path_description *result, const struct path_description *default_device);
void set_default_device(const char *dev, const char *partition, struct path_description *default_device);
extern int yaboot_set_bootpath(const char *imagepath, struct path_description *default_device);
#define dump_path_description(p) do { __dump_path_description(__FUNCTION__,__LINE__,p); } while(0)
void __dump_path_description(const char *fn, int l, const struct path_description *p);

#endif
