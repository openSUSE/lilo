#include <types.h>

#define EXT2_ET_MAGIC_IO_CHANNEL                 (2133571333L)
#define EXT2_ET_MAGIC_IO_MANAGER                 (2133571335L)
#define EXT2_ET_BAD_MAGIC                        (2133571347L)
#define EXT2_ET_SHORT_READ                       (2133571364L)
#define EXT2_ET_BAD_DEVICE_NAME                  (2133571386L)
#define EXT2_ET_SYMLINK_LOOP                     (2133571392L)
#define EXT2_ET_FILE_NOT_FOUND                   (2133571404L)
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
	__u8 s_uuid[16];	/* 128-bit uuid for volume */
	char s_volume_name[16];	/* volume name */
	char s_last_mounted[64];	/* directory where last mounted */
	u32 s_algorithm_usage_bitmap;	/* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	__u8 s_prealloc_blocks;	/* Nr of blocks to try to preallocate */
	__u8 s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u16 s_reserved_gdt_blocks;	/* Per group table for online growth */
	/*
	 * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	__u8 s_journal_uuid[16];	/* uuid of journal superblock */
	u32 s_journal_inum;	/* inode number of journal file */
	u32 s_journal_dev;	/* device number of journal file */
	u32 s_last_orphan;	/* start of list of inodes to delete */
	u32 s_hash_seed[4];	/* HTREE hash seed */
	__u8 s_def_hash_version;	/* Default hash version to use */
	__u8 s_jnl_backup_type;	/* Default type of journal backup */
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
	u32 i_file_acl;	/* File ACL */
	u32 i_dir_acl;	/* Directory ACL */
	u32 i_faddr;		/* Fragment address */
	union {
		struct {
			__u8 l_i_frag;	/* Fragment number */
			__u8 l_i_fsize;	/* Fragment size */
			u16 i_pad1;
			u16 l_i_uid_high;	/* these 2 fields    */
			u16 l_i_gid_high;	/* were reserved2[0] */
			u32 l_i_reserved2;
		} linux2;
		struct {
			__u8 h_i_frag;	/* Fragment number */
			__u8 h_i_fsize;	/* Fragment size */
			u16 h_i_mode_high;
			u16 h_i_uid_high;
			u16 h_i_gid_high;
			u32 h_i_author;
		} hurd2;
		struct {
			__u8 m_i_frag;	/* Fragment number */
			__u8 m_i_fsize;	/* Fragment size */
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
	long (*read_error) (io_channel channel,
			    unsigned long block, int count, void *data, size_t size, int actual_bytes_read, long error);
	long (*write_error) (io_channel channel,
			     unsigned long block, int count, const void *data, size_t size, int actual_bytes_written, long error);
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
