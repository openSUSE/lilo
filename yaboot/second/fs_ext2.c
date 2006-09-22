/*
 *  fs_ext2.c - an implementation for the Ext2/Ext3 filesystem
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
 *
 *  Copyright (C) 1999 Benjamin Herrenschmidt
 *
 *  Adapted from quik/silo
 *
 *  Copyright (C) 1996 Maurizio Plaza
 *                1996 Jakub Jelinek
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

#define FAST_VERSION
#define MAX_READ_RANGE	256
#undef VERBOSE_DEBUG

#include <ext2fs.h>
static io_manager linux_io_manager;

/* Currently, we have a mess between what is in the file structure
 * and what is stored globally here. I'll clean this up later
 */
static int opened;		/* We can't open twice ! */
static unsigned int bs;		/* Blocksize */
static unsigned long long doff;	/* Byte offset where partition starts */
static u32 root, cwd;
static ext2_filsys fs;
static struct boot_file_t *cur_file;
static char *block_buffer;

#ifdef FAST_VERSION
static unsigned long read_range_start;
static unsigned long read_range_count;
static unsigned long read_last_logical;
static unsigned long read_total;
static unsigned long read_max;
static struct boot_file_t *read_cur_file;
static long read_result;
static char *read_buffer;

static int read_dump_range(void);
static int read_iterator(ext2_filsys fs, u32 * blocknr, int lg_block, void *private);
#else				/* FAST_VERSION */
static struct ext2_inode cur_inode;
#endif				/* FAST_VERSION */

void com_err(const char *a, long i, const char *fmt, ...)
{
	prom_printf((char *)fmt);
}

static int ext2_open(struct boot_file_t *file, const char *dev_name, struct partition_t *part, const char *file_name)
{
	int result = 0;
	int error = FILE_ERR_NOTFOUND;
	char buffer[1024];
	int ofopened = 0;

	DEBUG_ENTER;
	DEBUG_OPEN;

	if (opened) {
		DEBUG_LEAVE(FILE_ERR_FSBUSY);
		return FILE_ERR_FSBUSY;
	}

	fs = NULL;

	/* We don't care too much about the device block size since we run
	 * thru the deblocker. We may have to change that if we plan to be
	 * compatible with older versions of OF
	 */
	bs = 1024;
	doff = 0;
	doff = (unsigned long long)(part->part_start) * part->blocksize;
	cur_file = file;

	DEBUG_F("partition offset: %Lu\n", doff);

	/* Open the OF device for the entire disk */
	sprintf(buffer, "%s:0", dev_name);

	DEBUG_F("<%s>\n", buffer);

	file->of_device = prom_open(buffer);

	DEBUG_F("file->of_device = %p\n", file->of_device);

	if (file->of_device == PROM_INVALID_HANDLE) {

		DEBUG_F("Can't open device %p\n", file->of_device);
		DEBUG_LEAVE(FILE_IOERR);
		return FILE_IOERR;
	}
	ofopened = 1;

	/* Open the ext2 filesystem */
	result = ext2fs_open(buffer, EXT2_FLAG_RW, 0, 0, linux_io_manager, &fs);
	if (result) {

		if (result == EXT2_ET_BAD_MAGIC) {
			DEBUG_F("ext2fs_open returned bad magic loading file %p\n", file);
		} else {
			DEBUG_F("ext2fs_open error #%d while loading file %s\n", result, file_name);
		}
		error = FILE_ERR_BAD_FSYS;
		goto bail;
	}

	/* Allocate the block buffer */
	block_buffer = malloc(fs->blocksize * 2);
	if (!block_buffer) {

		DEBUG_F("ext2fs: can't alloc block buffer (%d bytes)\n", fs->blocksize * 2);
		error = FILE_IOERR;
		goto bail;
	}

	/* Lookup file by pathname */
	root = cwd = EXT2_ROOT_INO;
	result = ext2fs_namei_follow(fs, root, cwd, file_name, &file->inode);
	if (result) {

		DEBUG_F("ext2fs_namei error #%d while loading file %s\n", result, file_name);
		if (result == EXT2_ET_SYMLINK_LOOP)
			error = FILE_ERR_SYMLINK_LOOP;
		else if (result == EXT2_ET_FILE_NOT_FOUND)
			error = FILE_ERR_NOTFOUND;
		else
			error = FILE_IOERR;
		goto bail;
	}
#if 0
	result = ext2fs_follow_link(fs, root, cwd, file->inode, &file->inode);
	if (result) {

		DEBUG_F("ext2fs_follow_link error #%d while loading file %s\n", result, file_name);
		error = FILE_ERR_NOTFOUND;
		goto bail;
	}
#endif

#ifndef FAST_VERSION
	result = ext2fs_read_inode(fs, file->inode, &cur_inode);
	if (result) {

		DEBUG_F("ext2fs_read_inode error #%d while loading file %s\n", result, file_name);
		if (result == EXT2_ET_FILE_TOO_BIG)
			error = FILE_ERR_LENGTH;
		else if (result == EXT2_ET_LLSEEK_FAILED)
			error = FILE_CANT_SEEK;
		else if (result == EXT2_ET_FILE_NOT_FOUND)
			error = FILE_ERR_NOTFOUND;
		else
			error = FILE_IOERR;
		goto bail;
	}
#endif				/* FAST_VERSION */
	file->pos = 0;

	opened = 1;
      bail:
	if (!opened) {
		if (fs)
			ext2fs_close(fs);
		fs = NULL;
		if (ofopened)
			prom_close(file->of_device);
		if (block_buffer)
			free(block_buffer);
		block_buffer = NULL;
		cur_file = NULL;

		DEBUG_LEAVE_F(error);
		return error;
	}

	DEBUG_LEAVE(FILE_ERR_OK);
	return FILE_ERR_OK;
}

#ifdef FAST_VERSION

static int read_dump_range(void)
{
	int count = read_range_count;
	int size;

#ifdef VERBOSE_DEBUG
	DEBUG_F("   dumping range: start: 0x%lx count: 0x%lx\n", read_range_count, read_range_start);
#endif
	/* Check if we need to handle a special case for the last block */
	if ((count * bs) > read_max)
		count--;
	if (count) {
		size = count * bs;
		read_result = io_channel_read_blk(fs->io, read_range_start, count, read_buffer);
		if (read_result)
			return BLOCK_ABORT;
		read_buffer += size;
		read_max -= size;
		read_total += size;
		read_cur_file->pos += size;
		read_range_count -= count;
		read_range_start += count;
		read_last_logical += count;
	}
	/* Handle remaining block */
	if (read_max && read_range_count) {
		read_result = io_channel_read_blk(fs->io, read_range_start, 1, block_buffer);
		if (read_result)
			return BLOCK_ABORT;
		memcpy(read_buffer, block_buffer, read_max);
		read_cur_file->pos += read_max;
		read_total += read_max;
		read_max = 0;
	}
	read_range_count = read_range_start = 0;

	return (read_max == 0) ? BLOCK_ABORT : 0;
}

static int read_iterator(ext2_filsys fs, u32 * blocknr, int lg_block, void *private)
{
#ifdef VERBOSE_DEBUG
	DEBUG_F("read_it: p_bloc: 0x%x, l_bloc: 0x%x, f_pos: 0x%Lx, rng_pos: 0x%lx   ",
		*blocknr, lg_block, read_cur_file->pos, read_last_logical);
#endif
	if (lg_block < 0) {
#ifdef VERBOSE_DEBUG
		DEBUG_F(" <skip lg>\n");
#endif
		return 0;
	}

	/* If we have not reached the start block yet, we skip */
	if (lg_block < read_cur_file->pos / bs) {
#ifdef VERBOSE_DEBUG
		DEBUG_F(" <skip pos>\n");
#endif
		return 0;
	}

	/* If block is contiguous to current range, just extend range,
	 * exit if we pass the remaining bytes count to read
	 */
	if (read_range_start && read_range_count < MAX_READ_RANGE && (*blocknr == read_range_start + read_range_count)
	    && (lg_block == read_last_logical + read_range_count)) {
#ifdef VERBOSE_DEBUG
		DEBUG_F(" block in range\n");
#endif
		++read_range_count;
		return ((read_range_count * bs) >= read_max) ? BLOCK_ABORT : 0;
	}

	/* Range doesn't match. Dump existing range */
	if (read_range_start) {
#ifdef VERBOSE_DEBUG
		DEBUG_F(" calling dump range \n");
#endif
		if (read_dump_range())
			return BLOCK_ABORT;
	}

	/* Here we handle holes in the file */
	if (lg_block && lg_block != read_last_logical) {
		unsigned long nzero;
#ifdef VERBOSE_DEBUG
		DEBUG_F(" hole from lg_bloc 0x%lx\n", read_last_logical);
#endif
		if (read_cur_file->pos % bs) {
			int offset = read_cur_file->pos % bs;
			int size = bs - offset;
			if (size > read_max)
				size = read_max;
			memset(read_buffer, 0, size);
			read_max -= size;
			read_total += size;
			read_buffer += size;
			read_cur_file->pos += size;
			++read_last_logical;
			if (read_max == 0)
				return BLOCK_ABORT;
		}
		nzero = (lg_block - read_last_logical) * bs;
		if (nzero) {
			if (nzero > read_max)
				nzero = read_max;
			memset(read_buffer, 0, nzero);
			read_max -= nzero;
			read_total += nzero;
			read_buffer += nzero;
			read_cur_file->pos += nzero;
			if (read_max == 0)
				return BLOCK_ABORT;
		}
		read_last_logical = lg_block;
	}

	/* If we are not aligned, handle that case */
	if (read_cur_file->pos % bs) {
		int offset = read_cur_file->pos % bs;
		int size = bs - offset;
#ifdef VERBOSE_DEBUG
		DEBUG_F(" handle unaligned start\n");
#endif
		read_result = io_channel_read_blk(fs->io, *blocknr, 1, block_buffer);
		if (read_result)
			return BLOCK_ABORT;
		if (size > read_max)
			size = read_max;
		memcpy(read_buffer, block_buffer + offset, size);
		read_cur_file->pos += size;
		read_max -= size;
		read_total += size;
		read_buffer += size;
		read_last_logical = lg_block + 1;
		return (read_max == 0) ? BLOCK_ABORT : 0;
	}

	/* If there is still a physical block to add, then create a new range */
	if (*blocknr) {
#ifdef VERBOSE_DEBUG
		DEBUG_F(" new range\n");
#endif
		read_range_start = *blocknr;
		read_range_count = 1;
		return (bs >= read_max) ? BLOCK_ABORT : 0;
	}
#ifdef VERBOSE_DEBUG
	DEBUG_F("\n");
#endif
	return 0;
}

#endif				/* FAST_VERSION */

static int ext2_read(struct boot_file_t *file, unsigned int size, void *buffer)
{
	long retval;

#ifdef FAST_VERSION
	if (!opened)
		return FILE_IOERR;

	DEBUG_F("ext_read() from pos 0x%Lx, size: 0x%ux\n", file->pos, size);

	read_cur_file = file;
	read_range_start = 0;
	read_range_count = 0;
	read_last_logical = file->pos / bs;
	read_total = 0;
	read_max = size;
	read_buffer = buffer;
	read_result = 0;

	retval = ext2fs_block_iterate(fs, file->inode, 0, 0, read_iterator, 0);
	if (retval == BLOCK_ABORT)
		retval = read_result;
	if (!retval && read_range_start) {
#ifdef VERBOSE_DEBUG
		DEBUG_F("on exit: range_start is 0x%lx, calling dump...\n", read_range_start);
#endif
		read_dump_range();
		retval = read_result;
	}
	if (retval)
		prom_printf("ext2: i/o error %ld in read\n", (long)retval);

	return read_total;

#else				/* FAST_VERSION */
	int status;
	unsigned int read = 0;

	if (!opened)
		return FILE_IOERR;

	DEBUG_F("ext_read() from pos 0x%x, size: 0x%x\n", file->pos, size);

	while (size) {
		u32 fblock = file->pos / bs;
		u32 pblock;
		unsigned int blkorig, s, b;

		pblock = 0;
		status = ext2fs_bmap(fs, file->inode, &cur_inode, block_buffer, 0, fblock, &pblock);
		if (status) {

			DEBUG_F("ext2fs_bmap(fblock:%d) return: %d\n", fblock, status);
			return read;
		}
		blkorig = fblock * bs;
		b = file->pos - blkorig;
		s = ((bs - b) > size) ? size : (bs - b);
		if (pblock) {
			unsigned long long pos = ((unsigned long long)pblock) * (unsigned long long)bs;
			pos += doff;
			prom_seek(file->of_device, pos);
			status = prom_read(file->of_device, block_buffer, bs);
			if (status != bs) {
				prom_printf("ext2: io error in read, ex: %d, got: %d\n", bs, status);
				return read;
			}
		} else
			memset(block_buffer, 0, bs);

		memcpy(buffer, block_buffer + b, s);
		read += s;
		size -= s;
		buffer += s;
		file->pos += s;
	}
	return read;
#endif				/* FAST_VERSION */
}

static int ext2_seek(struct boot_file_t *file, unsigned long long newpos)
{
	if (!opened)
		return FILE_CANT_SEEK;

	file->pos = newpos;
	return FILE_ERR_OK;
}

static int ext2_close(struct boot_file_t *file)
{
	if (!opened)
		return FILE_IOERR;

	if (block_buffer)
		free(block_buffer);
	block_buffer = NULL;

	if (fs)
		ext2fs_close(fs);
	fs = NULL;

	prom_close(file->of_device);
	DEBUG_F("ext2_close called\n");

	opened = 0;

	return 0;
}

static long linux_open(const char *name, int flags, io_channel * channel)
{
	io_channel io;

	if (!name)
		return EXT2_ET_BAD_DEVICE_NAME;
	io = (io_channel) malloc(sizeof(struct struct_io_channel));
	if (!io)
		return EXT2_ET_BAD_DEVICE_NAME;
	memset(io, 0, sizeof(struct struct_io_channel));
	io->magic = EXT2_ET_MAGIC_IO_CHANNEL;
	io->manager = linux_io_manager;
	io->name = (char *)malloc(strlen(name) + 1);
	strcpy(io->name, name);
	io->block_size = bs;
	io->read_error = 0;
	io->write_error = 0;
	*channel = io;

	return 0;
}

static long linux_close(io_channel channel)
{
	free(channel);
	return 0;
}

static long linux_set_blksize(io_channel channel, int blksize)
{
	channel->block_size = bs = blksize;
	if (block_buffer) {
		free(block_buffer);
		block_buffer = malloc(bs * 2);
	}
	return 0;
}

static long linux_read_blk(io_channel channel, unsigned long block, int count, void *data)
{
	int size, size_read;
	unsigned long long tempb;

	if (count == 0)
		return 0;

	tempb = (((unsigned long long)block) * ((unsigned long long)bs)) + (unsigned long long)doff;
	size = (count < 0) ? -count : count * bs;
	prom_seek(cur_file->of_device, tempb);
	size_read = prom_read(cur_file->of_device, data, size);
	if (size_read != size) {
		DEBUG_F("\nRead error on block %lx. expected %x, got %x\n", block, size, size_read);
		return EXT2_ET_SHORT_READ;
	}
	return 0;
}

static long linux_write_blk(io_channel channel, unsigned long block, int count, const void *data)
{
	return 0;
}

static long linux_flush(io_channel channel)
{
	return 0;
}

static struct struct_io_manager struct_linux_manager = {
	.magic = EXT2_ET_MAGIC_IO_MANAGER,
	.name = "linux I/O Manager",
	.open = linux_open,
	.close = linux_close,
	.set_blksize = linux_set_blksize,
	.read_blk = linux_read_blk,
	.write_blk = linux_write_blk,
	.flush = linux_flush,
};

static io_manager linux_io_manager = &struct_linux_manager;

struct fs_t ext2_filesystem = {
	.name = "ext2",
	.open = ext2_open,
	.read = ext2_read,
	.seek = ext2_seek,
	.close = ext2_close
};

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
