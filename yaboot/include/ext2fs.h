#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#define EXT2_ET_MAGIC_EXT2FS_FILSYS              (2133571329L)
#define EXT2_ET_MAGIC_BADBLOCKS_LIST             (2133571330L)
#define EXT2_ET_MAGIC_BADBLOCKS_ITERATE          (2133571331L)
#define EXT2_ET_MAGIC_INODE_SCAN                 (2133571332L)
#define EXT2_ET_MAGIC_IO_CHANNEL                 (2133571333L)
#define EXT2_ET_MAGIC_IO_MANAGER                 (2133571335L)
#define EXT2_ET_MAGIC_BLOCK_BITMAP               (2133571336L)
#define EXT2_ET_MAGIC_INODE_BITMAP               (2133571337L)
#define EXT2_ET_MAGIC_GENERIC_BITMAP             (2133571338L)
#define EXT2_ET_MAGIC_DBLIST                     (2133571340L)
#define EXT2_ET_MAGIC_E2IMAGE                    (2133571344L)
#define EXT2_ET_BAD_MAGIC                        (2133571347L)
#define EXT2_ET_REV_TOO_HIGH                     (2133571348L)
#define EXT2_ET_RO_FILSYS                        (2133571349L)
#define EXT2_ET_NEXT_INODE_READ                  (2133571361L)
#define EXT2_ET_UNEXPECTED_BLOCK_SIZE            (2133571362L)
#define EXT2_ET_DIR_CORRUPTED                    (2133571363L)
#define EXT2_ET_SHORT_READ                       (2133571364L)
#define EXT2_ET_NO_BLOCK_BITMAP                  (2133571368L)
#define EXT2_ET_BAD_INODE_NUM                    (2133571369L)
#define EXT2_ET_BAD_IND_BLOCK                    (2133571381L)
#define EXT2_ET_BAD_DIND_BLOCK                   (2133571382L)
#define EXT2_ET_BAD_TIND_BLOCK                   (2133571383L)
#define EXT2_ET_BAD_DEVICE_NAME                  (2133571386L)
#define EXT2_ET_MISSING_INODE_TABLE              (2133571387L)
#define EXT2_ET_CORRUPT_SUPERBLOCK               (2133571388L)
#define EXT2_ET_SYMLINK_LOOP                     (2133571392L)
#define EXT2_ET_CALLBACK_NOTHANDLED              (2133571393L)
#define EXT2_ET_BAD_BLOCK_IN_INODE_TABLE         (2133571394L)
#define EXT2_ET_UNSUPP_FEATURE                   (2133571395L)
#define EXT2_ET_RO_UNSUPP_FEATURE                (2133571396L)
#define EXT2_ET_LLSEEK_FAILED                    (2133571397L)
#define EXT2_ET_NO_MEMORY                        (2133571398L)
#define EXT2_ET_INVALID_ARGUMENT                 (2133571399L)
#define EXT2_ET_NO_DIRECTORY                     (2133571402L)
#define EXT2_ET_FILE_NOT_FOUND                   (2133571404L)
#define EXT2_ET_UNIMPLEMENTED                    (2133571408L)
#define EXT2_ET_FILE_TOO_BIG                     (2133571410L)
#define EXT2_ET_NOT_IMAGE_FILE                   (2133571420L)

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS		12
#define EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)
/*
 * Flags for the ext2_filsys structure and for ext2fs_open()
 */
#define EXT2_FLAG_RW			0x01
#define EXT2_FLAG_CHANGED		0x02
#define EXT2_FLAG_DIRTY			0x04
#define EXT2_FLAG_VALID			0x08
#define EXT2_FLAG_IB_DIRTY		0x10
#define EXT2_FLAG_BB_DIRTY		0x20
#define EXT2_FLAG_SWAP_BYTES		0x40
#define EXT2_FLAG_SWAP_BYTES_READ	0x80
#define EXT2_FLAG_SWAP_BYTES_WRITE	0x100
#define EXT2_FLAG_MASTER_SB_ONLY	0x200
#define EXT2_FLAG_FORCE			0x400
#define EXT2_FLAG_SUPER_ONLY		0x800
#define EXT2_FLAG_JOURNAL_DEV_OK	0x1000
#define EXT2_FLAG_IMAGE_FILE		0x2000
/*
 * Return flags for the block iterator functions
 */
#define BLOCK_CHANGED	1
#define BLOCK_ABORT	2
#define BLOCK_ERROR	4
/*
 * Special inode numbers
 */
#define EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_ACL_IDX_INO	 3	/* ACL inode */
#define EXT2_ACL_DATA_INO	 4	/* ACL inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT2_RESIZE_INO		 7	/* Reserved group descriptors inode */
#define EXT2_JOURNAL_INO	 8	/* Journal inode */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11

/*
 * The second extended file system magic number
 */
#define EXT2_SUPER_MAGIC	0xEF53

typedef struct struct_io_manager *io_manager;
typedef struct struct_io_channel *io_channel;
typedef struct struct_ext2_filsys *ext2_filsys;
typedef struct ext2fs_struct_generic_bitmap *ext2fs_inode_bitmap;
typedef struct ext2fs_struct_generic_bitmap *ext2fs_block_bitmap;
typedef struct ext2_struct_u32_list *ext2_badblocks_list;
typedef struct ext2_struct_dblist *ext2_dblist;

struct ext2_super_block {
	u32 s_inodes_count;	/* Inodes count */
	u32 s_blocks_count;	/* Blocks count */
	u32 s_r_blocks_count;	/* Reserved blocks count */
	u32 s_free_blocks_count;	/* Free blocks count */
	u32 s_free_inodes_count;	/* Free inodes count */
	u32 s_first_data_block;	/* First Data Block */
	u32 s_log_block_size;	/* Block size */
	s32 s_log_frag_size;	/* Fragment size */
	u32 s_blocks_per_group;	/* # Blocks per group */
	u32 s_frags_per_group;	/* # Fragments per group */
	u32 s_inodes_per_group;	/* # Inodes per group */
	u32 s_mtime;		/* Mount time */
	u32 s_wtime;		/* Write time */
	u16 s_mnt_count;	/* Mount count */
	s16 s_max_mnt_count;	/* Maximal mount count */
	u16 s_magic;		/* Magic signature */
	u16 s_state;		/* File system state */
	u16 s_errors;		/* Behaviour when detecting errors */
	u16 s_minor_rev_level;	/* minor revision level */
	u32 s_lastcheck;	/* time of last check */
	u32 s_checkinterval;	/* max. time between checks */
	u32 s_creator_os;	/* OS */
	u32 s_rev_level;	/* Revision level */
	u16 s_def_resuid;	/* Default uid for reserved blocks */
	u16 s_def_resgid;	/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32 s_first_ino;	/* First non-reserved inode */
	u16 s_inode_size;	/* size of inode structure */
	u16 s_block_group_nr;	/* block group # of this superblock */
	u32 s_feature_compat;	/* compatible feature set */
	u32 s_feature_incompat;	/* incompatible feature set */
	u32 s_feature_ro_compat;	/* readonly-compatible feature set */
	u8 s_uuid[16];		/* 128-bit uuid for volume */
	char s_volume_name[16];	/* volume name */
	char s_last_mounted[64];	/* directory where last mounted */
	u32 s_algorithm_usage_bitmap;	/* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	u8 s_prealloc_blocks;	/* Nr of blocks to try to preallocate */
	u8 s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u16 s_reserved_gdt_blocks;	/* Per group table for online growth */
	/*
	 * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	u8 s_journal_uuid[16];	/* uuid of journal superblock */
	u32 s_journal_inum;	/* inode number of journal file */
	u32 s_journal_dev;	/* device number of journal file */
	u32 s_last_orphan;	/* start of list of inodes to delete */
	u32 s_hash_seed[4];	/* HTREE hash seed */
	u8 s_def_hash_version;	/* Default hash version to use */
	u8 s_jnl_backup_type;	/* Default type of journal backup */
	u16 s_reserved_word_pad;
	u32 s_default_mount_opts;
	u32 s_first_meta_bg;	/* First metablock group */
	u32 s_mkfs_time;	/* When the filesystem was created */
	u32 s_jnl_blocks[17];	/* Backup of the journal inode */
	u32 s_reserved[172];	/* Padding to the end of the block */
};
struct ext2_group_desc {
	u32 bg_block_bitmap;	/* Blocks bitmap block */
	u32 bg_inode_bitmap;	/* Inodes bitmap block */
	u32 bg_inode_table;	/* Inodes table block */
	u16 bg_free_blocks_count;	/* Free blocks count */
	u16 bg_free_inodes_count;	/* Free inodes count */
	u16 bg_used_dirs_count;	/* Directories count */
	u16 bg_pad;
	u32 bg_reserved[3];
};
struct ext2_inode {
	u16 i_mode;		/* File mode */
	u16 i_uid;		/* Low 16 bits of Owner Uid */
	u32 i_size;		/* Size in bytes */
	u32 i_atime;		/* Access time */
	u32 i_ctime;		/* Creation time */
	u32 i_mtime;		/* Modification time */
	u32 i_dtime;		/* Deletion Time */
	u16 i_gid;		/* Low 16 bits of Group Id */
	u16 i_links_count;	/* Links count */
	u32 i_blocks;		/* Blocks count */
	u32 i_flags;		/* File flags */
	union {
		struct {
			u32 l_i_reserved1;
		} linux1;
		struct {
			u32 h_i_translator;
		} hurd1;
		struct {
			u32 m_i_reserved1;
		} masix1;
	} osd1;			/* OS dependent 1 */
	u32 i_block[EXT2_N_BLOCKS];	/* Pointers to blocks */
	u32 i_generation;	/* File version (for NFS) */
	u32 i_file_acl;		/* File ACL */
	u32 i_dir_acl;		/* Directory ACL */
	u32 i_faddr;		/* Fragment address */
	union {
		struct {
			u8 l_i_frag;	/* Fragment number */
			u8 l_i_fsize;	/* Fragment size */
			u16 i_pad1;
			u16 l_i_uid_high;	/* these 2 fields    */
			u16 l_i_gid_high;	/* were reserved2[0] */
			u32 l_i_reserved2;
		} linux2;
		struct {
			u8 h_i_frag;	/* Fragment number */
			u8 h_i_fsize;	/* Fragment size */
			u16 h_i_mode_high;
			u16 h_i_uid_high;
			u16 h_i_gid_high;
			u32 h_i_author;
		} hurd2;
		struct {
			u8 m_i_frag;	/* Fragment number */
			u8 m_i_fsize;	/* Fragment size */
			u16 m_pad1;
			u32 m_i_reserved2[2];
		} masix2;
	} osd2;			/* OS dependent 2 */
};
struct struct_io_channel {
	long magic;
	io_manager manager;
	char *name;
	int block_size;
	long (*read_error) (io_channel channel, unsigned long block, int count, void *data, size_t size, int actual_bytes_read,
			    long error);
	long (*write_error) (io_channel channel, unsigned long block, int count, const void *data, size_t size,
			     int actual_bytes_written, long error);
	int refcount;
	int flags;
	int reserved[14];
	void *private_data;
	void *app_data;
};
struct struct_io_manager {
	long magic;
	const char *name;
	long (*open) (const char *name, int flags, io_channel * channel);
	long (*close) (io_channel channel);
	long (*set_blksize) (io_channel channel, int blksize);
	long (*read_blk) (io_channel channel, unsigned long block, int count, void *data);
	long (*write_blk) (io_channel channel, unsigned long block, int count, const void *data);
	long (*flush) (io_channel channel);
	long (*write_byte) (io_channel channel, unsigned long offset, int count, const void *data);
	long (*set_option) (io_channel channel, const char *option, const char *arg);
	int reserved[14];
};
struct struct_ext2_filsys {
	long magic;
	io_channel io;
	int flags;
	char *device_name;
	struct ext2_super_block *super;
	unsigned int blocksize;
	int fragsize;
	u32 group_desc_count;
	unsigned long desc_blocks;
	struct ext2_group_desc *group_desc;
	int inode_blocks_per_group;
	ext2fs_inode_bitmap inode_map;
	ext2fs_block_bitmap block_map;
	long (*get_blocks) (ext2_filsys fs, u32 ino, u32 * blocks);
	long (*check_directory) (ext2_filsys fs, u32 ino);
	long (*write_bitmaps) (ext2_filsys fs);
	long (*read_inode) (ext2_filsys fs, u32 ino, struct ext2_inode * inode);
	long (*write_inode) (ext2_filsys fs, u32 ino, struct ext2_inode * inode);
	ext2_badblocks_list badblocks;
	ext2_dblist dblist;
	u32 stride;		/* for mke2fs */
	struct ext2_super_block *orig_super;
	struct ext2_image_hdr *image_header;
	u32 umask;
	/*
	 * Reserved for future expansion
	 */
	u32 reserved[8];

	/*
	 * Reserved for the use of the calling application.
	 */
	void *priv_data;

	/*
	 * Inode cache
	 */
	struct ext2_inode_cache *icache;
	io_channel image_io;
};
struct ext2fs_struct_generic_bitmap {
	long magic;
	ext2_filsys fs;
	u32 start, end;
	u32 real_end;
	char *description;
	char *bitmap;
	long base_error_code;
	u32 reserved[7];
};
#define io_channel_read_blk(c,b,n,d)	((c)->manager->read_blk((c),b,n,d))
extern long ext2fs_block_iterate(ext2_filsys fs, u32 ino, int flags, char *block_buf,
				 int (*func) (ext2_filsys fs, u32 * blocknr, int blockcnt, void *priv_data), void *priv_data);
extern long ext2fs_open(const char *name, int flags, int superblock, unsigned int block_size, io_manager manager,
			ext2_filsys * ret_fs);
extern long ext2fs_close(ext2_filsys fs);
long ext2fs_namei_follow(ext2_filsys fs, u32 root, u32 cwd, const char *name, u32 * inode);

/* required stuff from e2fsprogs headers */
typedef u32 ext2_ino_t;
typedef u32 blk_t;
typedef u32 dgrp_t;
typedef s64 e2_blkcnt_t;
typedef long errcode_t;
typedef struct ext2_struct_u32_list *ext2_u32_list;
typedef struct ext2_struct_u32_iterate *ext2_u32_iterate;
typedef struct ext2_struct_u32_iterate *ext2_badblocks_iterate;
#define EXT2FS_ATTR(x)
/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

struct ext2_dir_entry {
	u32 inode;		/* Inode number */
	u16 rec_len;		/* Directory entry length */
	u16 name_len;		/* Name length */
	char name[EXT2_NAME_LEN];	/* File name */
};

/*
 * Inode cache structure
 */

struct ext2_inode_cache_ent {
	ext2_ino_t ino;
	struct ext2_inode inode;
};

struct ext2_inode_cache {
	void *buffer;
	blk_t buffer_blk;
	int cache_last;
	int cache_size;
	int refcount;
	struct ext2_inode_cache_ent *cache;
};

/*
 * ext2_dblist structure and abstractions (see dblist.c)
 */
struct ext2_db_entry {
	ext2_ino_t ino;
	blk_t blk;
	int blockcnt;
};

/*
 * Directory block iterator definition
 */
struct ext2_struct_dblist {
	int magic;
	ext2_filsys fs;
	ext2_ino_t size;
	ext2_ino_t count;
	int sorted;
	struct ext2_db_entry *list;
};

/*
 * Badblocks list
 */
struct ext2_struct_u32_list {
	int magic;
	int num;
	int size;
	u32 *list;
	int badblocks_flags;
};

struct ext2_struct_u32_iterate {
	int magic;
	ext2_u32_list bb;
	int ptr;
};

/*
 * Return flags for the directory iterator functions
 */
#define DIRENT_CHANGED	1
#define DIRENT_ABORT	2
#define DIRENT_ERROR	3

/*
 * Block interate flags
 *
 * BLOCK_FLAG_APPEND, or BLOCK_FLAG_HOLE, indicates that the interator
 * function should be called on blocks where the block number is zero.
 * This is used by ext2fs_expand_dir() to be able to add a new block
 * to an inode.  It can also be used for programs that want to be able
 * to deal with files that contain "holes".
 * 
 * BLOCK_FLAG_TRAVERSE indicates that the iterator function for the
 * indirect, doubly indirect, etc. blocks should be called after all
 * of the blocks containined in the indirect blocks are processed.
 * This is useful if you are going to be deallocating blocks from an
 * inode.
 *
 * BLOCK_FLAG_DATA_ONLY indicates that the iterator function should be
 * called for data blocks only.
 *
 * BLOCK_FLAG_NO_LARGE is for internal use only.  It informs
 * ext2fs_block_iterate2 that large files won't be accepted.
 */
#define BLOCK_FLAG_APPEND	1
#define BLOCK_FLAG_HOLE		1
#define BLOCK_FLAG_DEPTH_TRAVERSE	2
#define BLOCK_FLAG_DATA_ONLY	4

#define BLOCK_FLAG_NO_LARGE	0x1000

/*
 * Magic "block count" return values for the block iterator function.
 */
#define BLOCK_COUNT_IND		(-1)
#define BLOCK_COUNT_DIND	(-2)
#define BLOCK_COUNT_TIND	(-3)
#define BLOCK_COUNT_TRANSLATOR	(-4)
/*
 * For checking structure magic numbers...
 */

#define EXT2_CHECK_MAGIC(struct, code) \
	  if ((struct)->magic != (code)) return (code)

/*
 * Where the master copy of the superblock is located, and how big
 * superblocks are supposed to be.  We define SUPERBLOCK_SIZE because
 * the size of the superblock structure is not necessarily trustworthy
 * (some versions have the padding set up so that the superblock is
 * 1032 bytes long).
 */
#define SUPERBLOCK_OFFSET	1024
#define SUPERBLOCK_SIZE 	1024

extern errcode_t ext2fs_dir_iterate(ext2_filsys fs, ext2_ino_t dir, int flags, char *block_buf,
				    int (*func) (struct ext2_dir_entry * dirent, int offset, int blocksize, char *buf,
						 void *priv_data), void *priv_data);
extern errcode_t ext2fs_open2(const char *name, const char *io_options, int flags, int superblock, unsigned int block_size,
			      io_manager manager, ext2_filsys * ret_fs);
extern int ext2fs_process_dir_block(ext2_filsys fs, blk_t * blocknr, e2_blkcnt_t blockcnt, blk_t ref_block, int ref_offset,
				    void *priv_data);

extern void ext2fs_free_inode_bitmap(ext2fs_inode_bitmap bitmap);
extern void ext2fs_free_block_bitmap(ext2fs_block_bitmap bitmap);
extern void ext2fs_badblocks_list_free(ext2_badblocks_list bb);
extern void ext2fs_u32_list_free(ext2_u32_list bb);
extern void ext2fs_free_dblist(ext2_dblist dblist);
#define i_size_high	i_dir_acl
#define EXT2_FEATURE_INCOMPAT_META_BG           0x0010
#define IO_FLAG_RW     1

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX		0
#define EXT2_OS_HURD		1
#define EXT2_OS_MASIX		2
#define EXT2_OS_FREEBSD		3
#define EXT2_OS_LITES		4

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV	0	/* The good old (original) format */
#define EXT2_DYNAMIC_REV	1	/* V2 format w/ dynamic inode sizes */

#define EXT2_CURRENT_REV	EXT2_GOOD_OLD_REV
#define EXT2_MAX_SUPP_REV	EXT2_DYNAMIC_REV

#define EXT2_GOOD_OLD_INODE_SIZE 128

/*
 * Journal inode backup types
 */
#define EXT3_JNL_BACKUP_BLOCKS	1

/*
 * Feature set definitions
 */

#define EXT2_HAS_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_compat & (mask) )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_ro_compat & (mask) )
#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_incompat & (mask) )

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES	0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR		0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INODE	0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX		0x0020

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
/* #define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004 not used */

#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004	/* Needs recovery */
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008	/* Journal device */
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT3_FEATURE_INCOMPAT_EXTENTS		0x0040

/*
 * The last ext2fs revision level that this version of the library is
 * able to support.
 */
#define EXT2_LIB_CURRENT_REV	EXT2_DYNAMIC_REV
#define EXT2_LIB_FEATURE_INCOMPAT_SUPP	(EXT2_FEATURE_INCOMPAT_FILETYPE|\
					 EXT3_FEATURE_INCOMPAT_JOURNAL_DEV|\
					 EXT2_FEATURE_INCOMPAT_META_BG|\
					 EXT3_FEATURE_INCOMPAT_RECOVER)
#define EXT2_LIB_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER|\
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE)
struct ext2_image_hdr {
	u32 magic_number;	/* This must be EXT2_ET_MAGIC_E2IMAGE */
	char magic_descriptor[16];	/* "Ext2 Image 1.0", w/ null padding */
	char fs_hostname[64];	/* Hostname of machine of image */
	char fs_netaddr[32];	/* Network address */
	u32 fs_netaddr_type;	/* 0 = IPV4, 1 = IPV6, etc. */
	u32 fs_device;		/* Device number of image */
	char fs_device_name[64];	/* Device name */
	char fs_uuid[16];	/* UUID of filesystem */
	u32 fs_blocksize;	/* Block size of the filesystem */
	u32 fs_reserved[8];

	u32 image_device;	/* Device number of image file */
	u32 image_inode;	/* Inode number of image file */
	u32 image_time;		/* Time of image creation */
	u32 image_reserved[8];

	u32 offset_super;	/* Byte offset of the sb and descriptors */
	u32 offset_inode;	/* Byte offset of the inode table  */
	u32 offset_inodemap;	/* Byte offset of the inode bitmaps */
	u32 offset_blockmap;	/* Byte offset of the inode bitmaps */
	u32 offset_reserved[8];
};

/*
 * For directory iterators
 */
struct dir_context {
	ext2_ino_t dir;
	int flags;
	char *buf;
	int (*func) (ext2_ino_t dir, int entry, struct ext2_dir_entry * dirent, int offset, int blocksize, char *buf,
		     void *priv_data);
	void *priv_data;
	errcode_t errcode;
};

/*
 * Directory iterator flags
 */

#define DIRENT_FLAG_INCLUDE_EMPTY	1
#define DIRENT_FLAG_INCLUDE_REMOVED	2

#define DIRENT_DOT_FILE		1
#define DIRENT_DOT_DOT_FILE	2
#define DIRENT_OTHER_FILE	3
#define DIRENT_DELETED_FILE	4
/*
 * Inode scan definitions
 */
typedef struct ext2_struct_inode_scan *ext2_inode_scan;
/*
 * ext2fs_scan flags
 */
#define EXT2_SF_CHK_BADBLOCKS	0x0001
#define EXT2_SF_BAD_INODE_BLK	0x0002
#define EXT2_SF_BAD_EXTRA_BYTES	0x0004
#define EXT2_SF_SKIP_MISSING_ITABLE	0x0008
typedef struct ext2_struct_u32_list *badblocks_list;
/*
 * Permanent part of an large inode on the disk
 */
struct ext2_inode_large {
	u16 i_mode;		/* File mode */
	u16 i_uid;		/* Low 16 bits of Owner Uid */
	u32 i_size;		/* Size in bytes */
	u32 i_atime;		/* Access time */
	u32 i_ctime;		/* Creation time */
	u32 i_mtime;		/* Modification time */
	u32 i_dtime;		/* Deletion Time */
	u16 i_gid;		/* Low 16 bits of Group Id */
	u16 i_links_count;	/* Links count */
	u32 i_blocks;		/* Blocks count */
	u32 i_flags;		/* File flags */
	union {
		struct {
			u32 l_i_reserved1;
		} linux1;
		struct {
			u32 h_i_translator;
		} hurd1;
		struct {
			u32 m_i_reserved1;
		} masix1;
	} osd1;			/* OS dependent 1 */
	u32 i_block[EXT2_N_BLOCKS];	/* Pointers to blocks */
	u32 i_generation;	/* File version (for NFS) */
	u32 i_file_acl;		/* File ACL */
	u32 i_dir_acl;		/* Directory ACL */
	u32 i_faddr;		/* Fragment address */
	union {
		struct {
			u8 l_i_frag;	/* Fragment number */
			u8 l_i_fsize;	/* Fragment size */
			u16 i_pad1;
			u16 l_i_uid_high;	/* these 2 fields    */
			u16 l_i_gid_high;	/* were reserved2[0] */
			u32 l_i_reserved2;
		} linux2;
		struct {
			u8 h_i_frag;	/* Fragment number */
			u8 h_i_fsize;	/* Fragment size */
			u16 h_i_mode_high;
			u16 h_i_uid_high;
			u16 h_i_gid_high;
			u32 h_i_author;
		} hurd2;
		struct {
			u8 m_i_frag;	/* Fragment number */
			u8 m_i_fsize;	/* Fragment size */
			u16 m_pad1;
			u32 m_i_reserved2[2];
		} masix2;
	} osd2;			/* OS dependent 2 */
	u16 i_extra_isize;
	u16 i_pad1;
};
#define ENOMEM -1

/*
 * File system states
 */
#define EXT2_VALID_FS			0x0001	/* Unmounted cleanly */
#define EXT2_ERROR_FS			0x0002	/* Errors detected */
#define io_channel_close(c)            ((c)->manager->close((c)))
#define io_channel_write_blk(c,b,n,d)  ((c)->manager->write_blk((c),b,n,d))
extern errcode_t ext2fs_badblocks_list_add(ext2_badblocks_list bb, blk_t blk);
extern errcode_t ext2fs_read_inode_bitmap(ext2_filsys fs);
extern errcode_t ext2fs_read_inode(ext2_filsys fs, ext2_ino_t ino, struct ext2_inode *inode);
extern errcode_t ext2fs_badblocks_list_create(ext2_badblocks_list * ret, int size);
errcode_t ext2fs_block_iterate2(ext2_filsys fs, ext2_ino_t ino, int flags, char *block_buf,
				int (*func) (ext2_filsys fs, blk_t * blocknr, e2_blkcnt_t blockcnt, blk_t ref_blk, int ref_offset,
					     void *priv_data), void *priv_data);
static inline u32 ext2fs_swab32(u32 val)
{
	return ((val >> 24) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | (val << 24));
}
static inline u16 ext2fs_swab16(u16 val)
{
	return (val >> 8) | (val << 8);
}

/*
 * Flags for directory block reading and writing functions
 */
#define EXT2_DIRBLOCK_V2_STRUCT	0x0001

extern errcode_t ext2fs_read_ind_block(ext2_filsys fs, blk_t blk, void *buf);
extern errcode_t ext2fs_write_ind_block(ext2_filsys fs, blk_t blk, void *buf);
/*
 * Ext2/linux mode flags.  We define them here so that we don't need
 * to depend on the OS's sys/stat.h, since we may be compiling on a
 * non-Linux system.
 */
#define LINUX_S_IFMT	00170000
#define LINUX_S_IFDIR	0040000
#define LINUX_S_IFLNK	0120000
#define LINUX_S_ISDIR(m)       (((m) & LINUX_S_IFMT) == LINUX_S_IFDIR)
#define LINUX_S_ISLNK(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFLNK)
extern errcode_t ext2fs_get_blocks(ext2_filsys fs, ext2_ino_t ino, blk_t * blocks);
extern errcode_t ext2fs_write_inode(ext2_filsys fs, ext2_ino_t ino, struct ext2_inode *inode);
extern errcode_t ext2fs_lookup(ext2_filsys fs, ext2_ino_t dir, const char *name, int namelen, char *buf, ext2_ino_t * inode);
extern int ext2fs_bg_has_super(ext2_filsys fs, int group_block);
extern errcode_t io_channel_set_options(io_channel channel, const char *options);
extern errcode_t io_channel_write_byte(io_channel channel, unsigned long offset, int count, const void *data);
#define io_channel_set_blksize(c,s)	((c)->manager->set_blksize((c),s))
#define io_channel_flush(c) 		((c)->manager->flush((c)))
extern void ext2fs_swap_super(struct ext2_super_block *super);
/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT2_MIN_BLOCK_LOG_SIZE		10	/* 1024 */
#define EXT2_MAX_BLOCK_LOG_SIZE		16	/* 65536 */
#define EXT2_MIN_BLOCK_SIZE	(1 << EXT2_MIN_BLOCK_LOG_SIZE)
#define EXT2_BLOCK_SIZE(s)	(EXT2_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define EXT2_BLOCK_SIZE_BITS(s)	((s)->s_log_block_size + 10)
#define EXT2_MIN_FRAG_SIZE		EXT2_MIN_BLOCK_SIZE
# define EXT2_FRAG_SIZE(s)		(EXT2_MIN_FRAG_SIZE << (s)->s_log_frag_size)
#define EXT2_INODE_SIZE(s)	(((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? \
				 EXT2_GOOD_OLD_INODE_SIZE : (s)->s_inode_size)
/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT2_SB(sb)	(sb)
#define EXT2_INODES_PER_GROUP(s)	(EXT2_SB(s)->s_inodes_per_group)
#define EXT2_BLOCKS_PER_GROUP(s)	(EXT2_SB(s)->s_blocks_per_group)
#define EXT2_INODES_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s)/EXT2_INODE_SIZE(s))
/* limits imposed by 16-bit value gd_free_{blocks,inode}_count */
#define EXT2_MAX_BLOCKS_PER_GROUP(s)	((1 << 16) - 8)
#define EXT2_MAX_INODES_PER_GROUP(s)	((1 << 16) - EXT2_INODES_PER_BLOCK(s))
#define EXT2_DESC_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_group_desc))
extern void ext2fs_swap_group_desc(struct ext2_group_desc *gdp);
extern void ext2fs_free(ext2_filsys fs);
extern errcode_t ext2fs_check_directory(ext2_filsys fs, ext2_ino_t ino);
extern errcode_t ext2fs_read_dir_block(ext2_filsys fs, blk_t block, void *buf);
extern errcode_t ext2fs_write_dir_block(ext2_filsys fs, blk_t block, void *buf);
/* read_bb.c */
extern errcode_t ext2fs_read_bb_inode(ext2_filsys fs, ext2_badblocks_list * bb_list);
extern void ext2fs_swap_inode_full(ext2_filsys fs, struct ext2_inode_large *t, struct ext2_inode_large *f, int hostorder,
				   int bufsize);
#define time(x) (0)
extern errcode_t ext2fs_badblocks_list_iterate_begin(ext2_badblocks_list bb, ext2_badblocks_iterate * ret);
extern void ext2fs_badblocks_list_iterate_end(ext2_badblocks_iterate iter);
extern int ext2fs_badblocks_list_iterate(ext2_badblocks_iterate iter, blk_t * blk);
void ext2fs_block_alloc_stats(ext2_filsys fs, blk_t blk, int inuse);
extern int ext2fs_test_block_bitmap(ext2fs_block_bitmap bitmap, blk_t block);
extern errcode_t ext2fs_new_block(ext2_filsys fs, blk_t goal, ext2fs_block_bitmap map, blk_t * ret);
struct ext2_ext_attr_header {
	u32 h_magic;		/* magic number for identification */
	u32 h_refcount;		/* reference count */
	u32 h_blocks;		/* number of disk blocks used */
	u32 h_hash;		/* hash value of all attributes */
	u32 h_reserved[4];	/* zero right now */
};

struct ext2_ext_attr_entry {
	u8 e_name_len;		/* length of name */
	u8 e_name_index;	/* attribute name index */
	u16 e_value_offs;	/* offset in disk block of value */
	u32 e_value_block;	/* disk block attribute is stored on (n/i) */
	u32 e_value_size;	/* size of attribute value */
	u32 e_hash;		/* hash value of name and value */
#if 0
	char e_name[0];		/* attribute name */
#endif
};
/* Magic value in attribute blocks */
#define EXT2_EXT_ATTR_MAGIC_v1		0xEA010000
#define EXT2_EXT_ATTR_MAGIC		0xEA020000

#define EXT2_EXT_ATTR_PAD_BITS		2
#define EXT2_EXT_ATTR_PAD		(1<<EXT2_EXT_ATTR_PAD_BITS)
#define EXT2_EXT_ATTR_ROUND		(EXT2_EXT_ATTR_PAD-1)
#define EXT2_EXT_ATTR_LEN(name_len) \
	(((name_len) + EXT2_EXT_ATTR_ROUND + \
	sizeof(struct ext2_ext_attr_entry)) & ~EXT2_EXT_ATTR_ROUND)
#define EXT2_EXT_ATTR_NEXT(entry) \
	( (struct ext2_ext_attr_entry *)( \
	  (char *)(entry) + EXT2_EXT_ATTR_LEN((entry)->e_name_len)) )
/*
 *  Allocate memory
 */
static inline errcode_t ext2fs_get_mem(unsigned long size, void *ptr)
{
	void **pp = (void **)ptr;

	*pp = malloc(size);
	if (!*pp)
		return EXT2_ET_NO_MEMORY;
	return 0;
}

/*
 * Free memory
 */
static inline errcode_t ext2fs_free_mem(void *ptr)
{
	void **pp = (void **)ptr;

	free(*pp);
	*pp = 0;
	return 0;
}

static inline errcode_t ext2fs_resize_mem(unsigned long EXT2FS_ATTR((unused)) old_size, unsigned long size, void *ptr)
{
	void *op, *np;
	/* Use "memcpy" for pointer assignments here to avoid problems
	 * with C99 strict type aliasing rules. */
	memcpy(&op, ptr, sizeof(op));
	np = malloc(size);
	if (!np)
		return EXT2_ET_NO_MEMORY;
	if (old_size > size)
		memcpy(np, op, size);
	else
		memcpy(np, op, old_size);
	free(op);
	memcpy(ptr, &np, sizeof(np));
	return 0;
}
static inline blk_t ext2fs_inode_data_blocks(ext2_filsys fs, struct ext2_inode *inode)
{
	return inode->i_blocks - (inode->i_file_acl ? fs->blocksize >> 9 : 0);
}
