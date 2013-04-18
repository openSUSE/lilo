/*
 *  gpt-part.h - Structure of gpt partition table
 *
 *  Copyright (C) 2013 Dinar Valeev
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

#include <byteorder.h>

#define GPT_LINUX_NATIVE \
((gpt_part_type_t){ cpu_to_le32(0x0FC63DAF), cpu_to_le16(0x8483), cpu_to_le16(0x4772),0x8E, 0x79, \
	{0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4}})
#define GPT_LINUX_RAID \
((gpt_part_type_t){ cpu_to_le32(0xa19d880f), cpu_to_le16(0x05fc), cpu_to_le16(0x4d3b), 0xa0, 0x06, \
	{ 0x74, 0x3f, 0x0f, 0x84, 0x91, 0x1e }})
#define GPT_BASIC_DATA \
((gpt_part_type_t){ cpu_to_le32 (0xebd0a0a2), cpu_to_le16 (0xb9e5), cpu_to_le16 (0x4433), 0x87, 0xc0, \
	{ 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xC7 }})


#define GPT_HEADER_SIGNATURE 0x5452415020494645LL

typedef struct
{
 u32 data1;
 u16 data2;
 u16 data3;
 u8  data4;
 u8  data5;
 u8  data6[6];
} gpt_part_type_t;

struct gpt_partition {
	gpt_part_type_t type;           /* Partition type GUID */
	u8  guid[16];                   /* Unique partition GUID */
	u64 start;                      /* First partition LBA */
	u64 end;                        /* End LBA */
	u64 flags;                      /* Attribute flags */
	char name[72];                  /* Partition name */
} __attribute__ ((packed));

struct gpt_header {
	u64 signature;
	u32 revision;
	u32 header_size;
	u32 header_crc32;
	u32 reserved;
	u64 primary;
	u64 backup;
	u64 start;
	u64 end;
	u8  dguid[16];
	u64 partitions;
	u32 maxpart;
	u32 partentry_size;
	u32 partentry_crc32;
} __attribute__ ((packed));
