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

#include "ctype.h"
#include "types.h"
#include "stddef.h"
#include "stdlib.h"
#include "file.h"
#include "prom.h"
#include "string.h"
#include "partition.h"
#include "fs.h"
#include "errors.h"
#include "debug.h"

static int file_block_open(struct boot_file_t *file, const struct boot_fspec_t* spec)
{
     struct partition_t*	parts;
     struct partition_t*	p;
     struct partition_t*	found;
     char f[1024];
     int partition = spec->part;
	
     parts = partitions_lookup(spec->device);
     found = NULL;
     sprintf(f, "%s%s", spec->directory, spec->filename);
     DEBUG_F("filename '%s'\n", f);

#if DEBUG
     if (parts)
	  prom_printf("partitions:\n");
     else
	  prom_printf("no partitions found.\n");
#endif
     for (p = parts; p && !found; p=p->next) {
	  DEBUG_F("number: %02d, start: 0x%08lx, length: 0x%08lx\n",
		  p->part_number, p->part_start, p->part_size );
	  if (partition == -1) {
	       file->fs = fs_open(file, spec->device, p, f);
	       if (file->fs == NULL || fserrorno != FILE_ERR_OK)
		    continue;
	       else {
		    partition = p->part_number;
		    goto done;
	       }
	  }
	  if ((partition >= 0) && (partition == p->part_number))
	       found = p;
#if DEBUG
	  if (found)
	       prom_printf(" (match)\n");
#endif						
     }

     /* Note: we don't skip when found is NULL since we can, in some
      * cases, let OF figure out a default partition.
      */
     DEBUG_F( "Using OF defaults.. (found = %p)\n", found );
     file->fs = fs_open(file, spec->device, found, f);

done:
     if (parts)
	  partitions_free(parts);

     return fserrorno;
}

static int
file_net_open(	struct boot_file_t*	file,
		const struct boot_fspec_t *spec)
{
     file->fs = fs_of_netboot;
     return fs_of_netboot->new_open(file, spec);
}

static int
default_read(	struct boot_file_t*	file,
		unsigned int		size,
		void*			buffer)
{
     prom_printf("WARNING ! default_read called !\n");
     return FILE_ERR_EOF;
}

static int
default_seek(	struct boot_file_t*	file,
		unsigned int		newpos)
{
     prom_printf("WARNING ! default_seek called !\n");
     return FILE_ERR_EOF;
}

static int
default_close(	struct boot_file_t*	file)
{
     prom_printf("WARNING ! default_close called !\n");
     return FILE_ERR_OK;
}

static struct fs_t fs_default =
{
	.name = "defaults",
	.read = default_read,
	.seek = default_seek,
	.close = default_close
};


int open_file(const struct boot_fspec_t* spec, struct boot_file_t* file)
{
     memset(file, 0, sizeof(struct boot_file_t));
     file->fs        = &fs_default;
     file->dev_type = prom_get_devtype(spec->device);

     switch(file->dev_type) {
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
 * c-basic-offset: 5
 * End:
 */
