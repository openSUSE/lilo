/*
 *  file.c - Filesystem related interfaces
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
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

#include <ctype.h>
#include <types.h>
#include <stddef.h>
#include <stdlib.h>
#include <file.h>
#include <prom.h>
#include <string.h>
#include <partition.h>
#include <fs.h>
#include <errors.h>
#include <debug.h>
#include <stdlib.h>
#include <fs.h>
#include <errors.h>
#include <config.h>

extern const struct fs_t of_filesystem;
extern const struct fs_t of_net_filesystem;
extern const struct fs_t ext2_filesystem;

/* Configurable filesystems */
extern const struct fs_t xfs_filesystem;
extern const struct fs_t reiserfs_filesystem;

/* Filesystem handlers yaboot knows about */
static const struct fs_t *block_filesystems[] = {
	&ext2_filesystem,
#ifdef CONFIG_FS_XFS
	&xfs_filesystem,
#endif
#ifdef CONFIG_FS_REISERFS
	&reiserfs_filesystem,
#endif
	&of_filesystem,		/* HFS/HFS+, ISO9660, UDF, UFS */
	NULL
};

static const struct fs_t *fs_of_netboot = &of_net_filesystem;

static int fs_open(struct boot_file_t *file, const char *dev_name, struct partition_t *part, const char *file_name)
{
	const struct fs_t **fs;
	for (fs = block_filesystems; *fs; fs++)
		if ((*fs)->open(file, dev_name, part, file_name) == FILE_ERR_OK) {
			file->fs = *fs;
			return 1;
		}
	return 0;
}

static int file_block_open(struct boot_file_t *file, const struct path_description *spec)
{
	struct partition_t *parts;
	struct partition_t *p;
	char f[1024];
	int fserrorno;
	int partition = path_part(spec);

	parts = partitions_lookup(path_device(spec));
	sprintf(f, "%s%s", path_directory(spec) ? path_directory(spec) : "" , path_filename(spec));
	DEBUG_F("filename '%s'\n", f);
	fserrorno = FILE_ERR_BADDEV;
	if (parts) {
#ifdef DEBUG
		prom_printf("partitions:\n");
#endif
		for (p = parts; p; p = p->next) {
			DEBUG_F("#%02d, start: %08lx, length: %08lx\n", p->part_number, p->part_start, p->part_size);
			if (partition == -1 || partition == p->part_number) {
				if (fs_open(file, path_device(spec), p, f)) {
					fserrorno = FILE_ERR_OK;
					break;
				}
			}
		}
		partitions_free(parts);
	} else {
#ifdef DEBUG
		prom_printf("no partitions found.\n");
#endif
		fserrorno = of_filesystem.open(file, path_device(spec), NULL, f);
		if (fserrorno == FILE_ERR_OK)
			file->fs = &of_filesystem;
	}
	return fserrorno;
}

static int file_net_open(struct boot_file_t *file, const struct path_description *spec)
{
	file->fs = fs_of_netboot;
	return fs_of_netboot->new_open(file, spec);
}

static int default_read(struct boot_file_t *file, unsigned int size, void *buffer)
{
	prom_printf("WARNING ! default_read called !\n");
	return FILE_ERR_EOF;
}

static int default_seek(struct boot_file_t *file, unsigned long long newpos)
{
	prom_printf("WARNING ! default_seek called !\n");
	return FILE_ERR_EOF;
}

static int default_close(struct boot_file_t *file)
{
	prom_printf("WARNING ! default_close called !\n");
	return FILE_ERR_OK;
}

static struct fs_t fs_default = {
	.name = "defaults",
	.read = default_read,
	.seek = default_seek,
	.close = default_close
};

int open_file(const struct path_description *spec, struct boot_file_t *file)
{
	memset(file, 0, sizeof(struct boot_file_t));
	file->fs = &fs_default;
	file->dev_type = prom_get_devtype(path_device(spec));

	switch (file->dev_type) {
	case TYPE_ISCSI:
		DEBUG_F("iscsi, pray!\n");
	case TYPE_BLOCK:
		DEBUG_F("device is a block device\n");
		return file_block_open(file, spec);
	case TYPE_NET:
		DEBUG_F("device is a network device\n");
		return file_net_open(file, spec);
	default:
		return FILE_ERR_BADDEV;
	}
}

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
