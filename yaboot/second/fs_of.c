/*
 *  fs_of.c - an implementation for OpenFirmware supported filesystems
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
 * BrokenFirmware cannot "read" from the network. We use tftp "load"
 * method for network boot for now, we may provide our own NFS
 * implementation in a later version. That means that we allocate a
 * huge block of memory for the entire file before loading it. We use
 * the location where the kernel puts RTAS, it's not used by the
 * bootloader and if freed when the kernel is booted.  This will have
 * to be changed if we plan to instanciate RTAS in the bootloader
 * itself
 */

#include <ctype.h>
#include <types.h>
#include <stddef.h>
#include <stdlib.h>
#include <file.h>
#include <prom.h>
#include <string.h>
#include <partition.h>
#include <msdos-part.h>
#include <fs.h>
#include <errors.h>
#include <debug.h>

#define LOAD_BUFFER_BASE (24*1024*1024)
#define LOAD_BUFFER_SIZE (32*1024*1024)

static int of_open(struct boot_file_t *file, const char *dev_name, struct partition_t *part, const char *file_name)
{
	char buffer[1024], pn[11 + 1 + 1];
	char *p;

	DEBUG_ENTER;
	DEBUG_OPEN;

	if (part) {
		if (part->label == LABEL_MSDOS && part->sys_ind != MSDOS_FAT16) {
			DEBUG_LEAVE(FILE_ERR_BAD_FSYS);
			return FILE_ERR_BAD_FSYS;
		}
		if (part->label == LABEL_AMIGA) {
			DEBUG_LEAVE(FILE_ERR_BAD_FSYS);
			return FILE_ERR_BAD_FSYS;
		}
		sprintf(pn, "%d%s", part->part_number, (file_name && *file_name) ? "," : "");
	} else {
		/* boot iso images have a msdos or mac partition table, but no usable partition will be found */
		pn[0] = '\0';
	}
	sprintf(buffer, "%s:%s%s", dev_name, pn, (file_name && *file_name) ? file_name : "");
	p = strchr(buffer, ':');
	if (p) {
		for (p++; *p; p++)
			if (*p == '/')
				*p = '\\';
	}

	DEBUG_F("opening: \"%s\"\n", buffer);

	file->of_device = prom_open(buffer);

	DEBUG_F("file->of_device = %p\n", file->of_device);

	file->pos = 0;
	file->buffer = NULL;
	if ((file->of_device == PROM_INVALID_HANDLE) || (file->of_device == 0)) {
		DEBUG_LEAVE(FILE_ERR_BAD_FSYS);
		return FILE_ERR_BAD_FSYS;
	}

	DEBUG_LEAVE(FILE_ERR_OK);
	return FILE_ERR_OK;
}

static int of_net_download(unsigned char **buffer, ihandle of_device)
{
	int ms;
	int ret = FILE_IOERR;
	unsigned char *p, *mem = (unsigned char *)LOAD_BUFFER_BASE;

	DEBUG_ENTER;

	p = prom_claim_chunk_top(LOAD_BUFFER_SIZE, 0);
	if (p == (void *)-1) {
		DEBUG_F("Can't claim memory for TFTP download\n");
		goto out;
	}

	memset(p, 0, LOAD_BUFFER_SIZE);
	ms = prom_getms();

	DEBUG_F("TFTP...\n");
	ret = prom_loadmethod(of_device, p);
	DEBUG_F("result: %d\n", ret);
	if (ret > 0) {
		*buffer = p;
		ms = prom_getms() - ms;
		if (ms < 1000) {
			ms = 1000 / ms;
			ms *= ret;
		} else {
			ms /= 1000;
			ms = ret / ms;
		}
		ms /= 1000;
		prom_printf("yaboot downloaded %08x bytes via network, download rate %d KB/s.\n", ret, ms);
	} else
		prom_release(p, LOAD_BUFFER_SIZE);
      out:
	DEBUG_LEAVE_F(ret);
	return ret;
}

static int of_net_open(struct boot_file_t *file, const struct path_description *spec)
{
	char buffer[1024];
	char *p = buffer;
	int ret;

	DEBUG_ENTER;
	DEBUG_OPEN_NEW;

	sprintf(buffer, "%s:%s%s", path_device(spec), path_net_before(spec), path_flags(spec) & FLAG_IPV6 ? "" : ",");
	p = p + strlen(buffer);
	strcat(buffer, path_filename(spec));
	while (*p) {
		if (*p == '/')
			*p = '\\';
		p++;
	}
	if (path_net_after(spec)) {
		strcat(buffer, ",");
		strcat(buffer, path_net_after(spec));
	}

	DEBUG_F("Opening: \"%s\"\n", buffer);

	file->of_device = prom_open(buffer);

	DEBUG_F("file->of_device = %p\n", file->of_device);

	file->pos = 0;
	if ((file->of_device == PROM_INVALID_HANDLE) || (file->of_device == 0)) {
		ret = FILE_ERR_BAD_FSYS;
		goto out;
	}

	ret = FILE_ERR_OK;

	if (file->buffer == NULL) {
		ret = of_net_download(&file->buffer, file->of_device);
		if (ret > 0) {
			file->len = ret;
			ret = FILE_ERR_OK;
		} else {
			prom_printf("download failed: %d\n", ret);
			ret = FILE_IOERR;
			prom_close(file->of_device);
		}
	}

      out:
	DEBUG_LEAVE_F(ret);
	return ret;
}

static int of_read(struct boot_file_t *file, unsigned int size, void *buffer)
{
	unsigned int size_read, i;
	int count;
	unsigned char *p;
	p = buffer;
	i = 5;
	count = size_read = 0;
	do {
		size_read = prom_read(file->of_device, p, size);
		if (size_read == 0) {
			if (i++ > 5)
				break;
		} else {
			if (size_read != size)
				DEBUG_F("short read? expected %08x, got %08x at %p, base %p\n", size, size_read, p, buffer);
			if (size_read > 0 && size_read <= size) {
				i = 0;
				size -= size_read;
				p += size_read;
				count += size_read;
			} else {
				if (i++ > 5)
					break;
			}
		}
	} while (size > 0);
	file->pos += count;
	return count;
}

static int of_net_read(struct boot_file_t *file, unsigned int size, void *buffer)
{
	unsigned int count, av;

	av = file->len - file->pos;
	count = size > av ? av : size;
	memcpy(buffer, file->buffer + file->pos, count);
	file->pos += count;
	return count;
}

static int of_seek(struct boot_file_t *file, unsigned long long newpos)
{
	if (prom_seek(file->of_device, newpos)) {
		file->pos = newpos;
		return FILE_ERR_OK;
	}

	return FILE_CANT_SEEK;
}

static int of_net_seek(struct boot_file_t *file, unsigned long long newpos)
{
	file->pos = (newpos > file->len) ? file->len : newpos;
	return FILE_ERR_OK;
}

static int of_close(struct boot_file_t *file)
{

	DEBUG_ENTER;
	DEBUG_F("<@%p>\n", file->of_device);

	if (file->buffer) {
		prom_release(file->buffer, LOAD_BUFFER_SIZE);
	}
	prom_close(file->of_device);
	DEBUG_F("of_close called\n");

	DEBUG_LEAVE(0);
	return 0;
}

static unsigned int
of_net_ino_size(struct boot_file_t *file)
{
	return file->len;
}

struct fs_t of_filesystem = {
	.name = "built-in",
	.open = of_open,
	.read = of_read,
	.seek = of_seek,
	.close = of_close
};

struct fs_t of_net_filesystem = {
	.name = "built-in network",
	.new_open = of_net_open,
	.read = of_net_read,
	.seek = of_net_seek,
	.close = of_close,
	.ino_size = of_net_ino_size,
};

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
