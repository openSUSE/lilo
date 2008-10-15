/*
 *  Yaboot - secondary boot loader for Linux on PowerPC. 
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
 *
 *  Copyright (C) 1999, 2000, 2001 Benjamin Herrenschmidt
 *  
 *  IBM CHRP support
 *  
 *  Copyright (C) 2001 Peter Bergner
 *
 *  portions based on poof
 *  
 *  Copyright (C) 1999 Marius Vollmer
 *  
 *  portions based on quik
 *  
 *  Copyright (C) 1996 Paul Mackerras.
 *  
 *  Because this program is derived from the corresponding file in the
 *  silo-0.64 distribution, it is also
 *  
 *  Copyright (C) 1996 Pete A. Zaitcev
 *                1996 Maurizio Plaza
 *                1996 David S. Miller
 *                1996 Miguel de Icaza
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

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <prom.h>
#include <file.h>
#include <errors.h>
#include <cfg.h>
#include <cmdline.h>
#include <yaboot.h>
#include <elf.h>
#include <debug.h>
#include <partition.h>
#include <version.h>
#include <hardcoded_bootpath.h>
#include <config.h>
#include <md5.h>

static char *hard_coded_bootpath(char *bootpath)
{
#ifdef HARD_CODED_BOOTPATH
	const char path[] = HARD_CODED_BOOTPATH;
	prom_printf("original bootpath: '%s'\n using hardcoded bootpath: '%s'\n", bootpath, path);
	bootpath = malloc(strlen(path) + 1);
	if (bootpath)
		sprintf(bootpath, path);
#endif
	return bootpath;
}

#define CONFIG_FILE_MAX		0x8000	/* 32k */


/* align addr on a size boundry - adjust address up if needed -- Cort */
#define _ALIGN(addr,size)	(((addr)+size-1)&(~(size-1)))
#define MAX_HEADERS	32

#define SLES9_ZIMAGE_BASE ((4 * 1024 * 1024))	/* the zImage header used in SLES8/9 is not relocatable */
#define SLES9_ZIMAGE_SIZE ((7 * 1024 * 1024))	/* its a binary blob from 2.4 kernel source ... */
#define MALLOCADDR ((2 * 1024 * 1024) + (512 * 1024))
#define MALLOCSIZE ((1 * 1024 * 1024) + (512 * 1024))
#define CLAIM_END (128 * 1024 * 1024)	/* FIXME: look at /memory/reg */

typedef struct {
	union {
		Elf32_Ehdr elf32hdr;
		Elf64_Ehdr elf64hdr;
	} elf;
	void *base;
	unsigned long memsize;
	unsigned long filesize;
	unsigned long offset;
	unsigned long load_loc;
	unsigned long entry;
} loadinfo_t;

typedef void (*kernel_entry_t) (void *, unsigned long, prom_entry, unsigned long, unsigned long);

/* Imported functions */
extern long flush_icache_range(unsigned long start, unsigned long stop);
extern int identify_cpu(void);

/* Locals & globals */
int useconf;
static char *password;
static void *sles9_base;
static struct path_description default_device;
static int _cpu;

extern char __bss_start[];
extern char _start[];
extern char _end[];
extern char _yaboot_conf_start[];
extern char _yaboot_conf_end[];

static const char config_file_name_ether_mac_template[] = "yaboot.conf-%02x-%02x-%02x-%02x-%02x-%02x";
static char config_file_name_ether_mac[] = "yaboot.conf-xx-xx-xx-xx-xx-xx";
static const char conf_token[] = "conf=";
static const char *config_file_names_net[] = {
	config_file_name_ether_mac,
	"yaboot.conf",
	NULL
};
static const char *config_file_names_block[] = {
	"yaboot.cnf",
	"yaboot.conf",
	"/etc/yaboot.conf",
	NULL
};

#ifdef CONFIG_COLOR_TEXT
/* Color values for text ui */
static struct ansi_color_t {
	char *name;
	int index;
	int value;
} ansi_color_table[] = {
	{
	"black", 2, 30}, {
	"blue", 0, 31}, {
	"green", 0, 32}, {
	"cyan", 0, 33}, {
	"red", 0, 34}, {
	"purple", 0, 35}, {
	"brown", 0, 36}, {
	"light-gray", 0, 37}, {
	"dark-gray", 1, 30}, {
	"light-blue", 1, 31}, {
	"light-green", 1, 32}, {
	"light-cyan", 1, 33}, {
	"light-red", 1, 34}, {
	"light-purple", 1, 35}, {
	"yellow", 1, 36}, {
	"white", 1, 37}, {
NULL, 0, 0},};

/* Default colors for text ui */
static int fgcolor = 15;
static int bgcolor;

/*
 * Validify color for text ui
 */
static int check_color_text_ui(char *color)
{
	int i = 0;
	while (ansi_color_table[i].name) {
		if (!strcmp(color, ansi_color_table[i].name))
			return i;
		i++;
	}
	return -1;
}
#endif				/* CONFIG_COLOR_TEXT */

static void print_message_file(const char *filename, const struct path_description *default_device)
{
	char *msg;
	int result;
	struct boot_file_t file;
	struct path_description msgfile;

	imagepath_to_path_description(filename, &msgfile, default_device);

	result = open_file(&msgfile, &file);
	if (result != FILE_ERR_OK) {
		msg = path_description_to_string(&msgfile);
		if (msg) {
			prom_perror(result, msg);
			free(msg);
		}
		return;
	}

	msg = malloc(2001);
	if (msg) {
		memset(msg, 0, 2001);
		if (file.fs->read(&file, 2000, msg) > 0)
			prom_printf("%s", msg);
		free(msg);
	}
	file.fs->close(&file);
}

static int find_and_load_config_file(const struct path_description *b, char *conf_file_buf)
{
	const char **names;
	struct boot_file_t file;
	int sz = 0, opened = 0, result;
	int i;
	struct path_description config_fspec;

	switch (path_type(b)) {
	case TYPE_NET:
		names = config_file_names_net;
		for (i = 0; i < 6; i++)
			if (b->u.n.mac[i]) {
				sprintf(config_file_name_ether_mac, config_file_name_ether_mac_template,
					b->u.n.mac[0], b->u.n.mac[1], b->u.n.mac[2], b->u.n.mac[3], b->u.n.mac[4], b->u.n.mac[5]);
				break;
			}
		break;
	case TYPE_ISCSI:
	case TYPE_BLOCK:
		names = config_file_names_block;
		break;
	default:
		prom_printf("type '%d' not handled\n", path_type(b));
		goto bail;
		break;
	}
	for (i = 0; names[i]; i++) {
		if (!imagepath_to_path_description(names[i], &config_fspec, b)) {
			prom_printf("can not parse '%s'\n", names[i]);
			continue;
		}
		result = open_file(&config_fspec, &file);
		if (result != FILE_ERR_OK)
			continue;
		opened = 1;
		break;
	}

	if (!opened)
		goto bail;

	/* Read it */
	sz = file.fs->read(&file, CONFIG_FILE_MAX, conf_file_buf);
	file.fs->close(&file);
	if (sz <= 0)
		prom_printf("Error, can't read config file\n");
	else
		prom_printf("Config file '%s' read, %d bytes\n", names[i], sz);

      bail:
	return sz;
}

static int load_config_file(const char *configfile, char *conf_file_buf, const struct path_description *b)
{
	int sz = 0;
	struct boot_file_t file;
	struct path_description config_fspec;

	if (imagepath_to_path_description(configfile, &config_fspec, b)) {
		if (open_file(&config_fspec, &file) == FILE_ERR_OK) {
			sz = file.fs->read(&file, CONFIG_FILE_MAX, conf_file_buf);
			file.fs->close(&file);
			if (sz <= 0)
				prom_printf("Error, can't read config file\n");
			else
				prom_printf("Config file '%s' read, %d bytes\n", configfile, sz);
		}
	}
	return sz;
}

static void process_configfile(void)
{
	char *p;
#ifndef DEBUG
	int i;
#endif
	/* Now, we do the initialisations stored in the config file */
	p = cfg_get_strg(NULL, "init-code");
	if (p)
		prom_interpret(p);

	set_default_device(cfg_get_strg(NULL, "device"), cfg_get_strg(NULL, "partition"), &default_device);

	password = cfg_get_strg(NULL, "password");

#ifdef CONFIG_COLOR_TEXT
	p = cfg_get_strg(NULL, "fgcolor");
	if (p) {
		DEBUG_F("fgcolor=%s\n", p);
		fgcolor = check_color_text_ui(p);
		if (fgcolor == -1) {
			prom_printf("Invalid fgcolor: \"%s\".\n", p);
		}
	}
	p = cfg_get_strg(NULL, "bgcolor");
	if (p) {
		DEBUG_F("bgcolor=%s\n", p);
		bgcolor = check_color_text_ui(p);
		if (bgcolor == -1)
			prom_printf("Invalid bgcolor: \"%s\".\n", p);
	}
	if (bgcolor >= 0) {
		char temp[64];
		sprintf(temp, "%x to background-color", bgcolor);
		prom_interpret(temp);
#ifndef DEBUG
		prom_printf("\xc\n");
#endif				/* !DEBUG */
	}
	if (fgcolor >= 0) {
		char temp[64];
		sprintf(temp, "%x to foreground-color", fgcolor);
		prom_interpret(temp);
	}
#endif				/* CONFIG_COLOR_TEXT */

#ifndef DEBUG
	if (stdout_is_screen)
		for (i = 0; i < 10; i++)
			prom_printf("\n");
#endif

	p = cfg_get_strg(NULL, "init-message");
	if (p)
		prom_printf("%s\n", p);

	p = cfg_get_strg(NULL, "message");
	if (p)
		print_message_file(p, &default_device);
}

static int is_elf32(loadinfo_t * loadinfo)
{
	Elf32_Ehdr *e = &(loadinfo->elf.elf32hdr);

	return (e->e_ident[EI_MAG0] == ELFMAG0 &&
		e->e_ident[EI_MAG1] == ELFMAG1 &&
		e->e_ident[EI_MAG2] == ELFMAG2 &&
		e->e_ident[EI_MAG3] == ELFMAG3 &&
		e->e_ident[EI_CLASS] == ELFCLASS32 && e->e_ident[EI_DATA] == ELFDATA2MSB && (e->e_type == ET_EXEC || e->e_type == ET_DYN) && e->e_machine == EM_PPC);
}

static int is_elf64(loadinfo_t * loadinfo)
{
	Elf64_Ehdr *e = &(loadinfo->elf.elf64hdr);

	return (e->e_ident[EI_MAG0] == ELFMAG0 &&
		e->e_ident[EI_MAG1] == ELFMAG1 &&
		e->e_ident[EI_MAG2] == ELFMAG2 &&
		e->e_ident[EI_MAG3] == ELFMAG3 &&
		e->e_ident[EI_CLASS] == ELFCLASS64 &&
		e->e_ident[EI_DATA] == ELFDATA2MSB && (e->e_type == ET_EXEC || e->e_type == ET_DYN) && e->e_machine == EM_PPC64);
}

static int load_elf32(struct boot_file_t *file, loadinfo_t * loadinfo)
{
	int i, j;
	Elf32_Ehdr *e = &(loadinfo->elf.elf32hdr);
	Elf32_Phdr *p, *ph;
	int size = sizeof(Elf32_Ehdr) - sizeof(Elf_Ident);
	unsigned long addr, loadaddr;

	/* Read the rest of the Elf header... */
	j = (*(file->fs->read)) (file, size, &e->e_version);
	if (j < size) {
		prom_printf("\nCan't read Elf32 image header: %08x/%08x\n", j, size);
		return 0;
	}

	DEBUG_F("Elf32 header:\n");
	DEBUG_F(" e.e_type      = %d\n", (int)e->e_type);
	DEBUG_F(" e.e_machine   = %d\n", (int)e->e_machine);
	DEBUG_F(" e.e_version   = %d\n", (int)e->e_version);
	DEBUG_F(" e.e_entry     = 0x%08x\n", (int)e->e_entry);
	DEBUG_F(" e.e_phoff     = 0x%08x\n", (int)e->e_phoff);
	DEBUG_F(" e.e_shoff     = 0x%08x\n", (int)e->e_shoff);
	DEBUG_F(" e.e_flags     = %d\n", (int)e->e_flags);
	DEBUG_F(" e.e_ehsize    = 0x%08x\n", (int)e->e_ehsize);
	DEBUG_F(" e.e_phentsize = 0x%08x\n", (int)e->e_phentsize);
	DEBUG_F(" e.e_phnum     = %d\n", (int)e->e_phnum);

	loadinfo->entry = e->e_entry;

	if (e->e_phnum > MAX_HEADERS) {
		prom_printf("Can only load kernels with one program header\n");
		return 0;
	}

	ph = (Elf32_Phdr *) malloc(sizeof(Elf32_Phdr) * e->e_phnum);
	if (!ph) {
		prom_printf("Malloc error\n");
		return 0;
	}

	/* Now, we read the section header */
	if ((*(file->fs->seek)) (file, e->e_phoff) != FILE_ERR_OK) {
		prom_printf("seek error\n");
		return 0;
	}
	j = (*(file->fs->read)) (file, sizeof(Elf32_Phdr) * e->e_phnum, ph);
	if (j != sizeof(Elf32_Phdr) * e->e_phnum) {
		prom_printf("read error: %08x/%08x\n", j, sizeof(Elf32_Phdr) * e->e_phnum);
		return 0;
	}

	/* Scan through the program header
	 */
	loadinfo->memsize = loadinfo->filesize = loadinfo->offset = 0;
	p = ph;
	for (i = 0; i < e->e_phnum; ++i, ++p) {
		if (p->p_type != PT_LOAD || p->p_offset == 0)
			continue;
		loadinfo->offset = p->p_offset;
		loadinfo->memsize = p->p_memsz;
		loadinfo->filesize = p->p_filesz;
		loadinfo->load_loc = p->p_vaddr;
	}

	if (loadinfo->memsize == 0) {
		prom_printf("Can't find a loadable segment !\n");
		return 0;
	}

	/* leave some room (1Mb) for boot infos */
	loadinfo->memsize = _ALIGN(loadinfo->memsize, (1 << 20)) + 0x100000;

	/* it seems that the B50 has trouble with a location between
	 * real-base and 32M. The kernel crashes at random places in
	 * prom_init, usually in opening display. */
	if (64 == _cpu)
		loadaddr = 0;
	else
		loadaddr = 32 * 1024 * 1024;

	if (sles9_base && loadinfo->memsize <= SLES9_ZIMAGE_SIZE)
		loadinfo->base = sles9_base;
	else {
		for (addr = loadaddr; addr < CLAIM_END; addr += 0x100000) {
			loadinfo->base = prom_claim((void *)addr, loadinfo->memsize, 0);
			if (loadinfo->base != (void *)-1)
				break;
		}
		if (loadinfo->base == (void *)-1) {
			prom_printf("Claim error, can't allocate kernel memory\n");
			return 0;
		}
	}
	prom_printf("Allocated %08lx bytes for executable @ %p\n", loadinfo->memsize, loadinfo->base);

	/* Load the program segments... */
	p = ph;
	for (i = 0; i < e->e_phnum; ++i, ++p) {
		unsigned long offset;
		if (p->p_type != PT_LOAD || p->p_offset == 0)
			continue;

		/* Now, we skip to the image itself */
		if ((*(file->fs->seek)) (file, p->p_offset) != FILE_ERR_OK) {
			prom_printf("Seek error\n");
			prom_release(loadinfo->base, loadinfo->memsize);
			return 0;
		}
		offset = p->p_vaddr - loadinfo->load_loc;
		j = (*(file->fs->read)) (file, p->p_filesz, loadinfo->base + offset);
		if (j != p->p_filesz) {
			prom_printf("Read failed: %08x/%08x\n", j, p->p_filesz);
			prom_release(loadinfo->base, loadinfo->memsize);
			return 0;
		}
	}

	free(ph);

	/* Return success at loading the Elf32 kernel */
	return 1;
}

static int load_elf64(struct boot_file_t *file, loadinfo_t * loadinfo)
{
	int i, j;
	Elf64_Ehdr *e = &(loadinfo->elf.elf64hdr);
	Elf64_Phdr *p, *ph;
	int size = sizeof(Elf64_Ehdr) - sizeof(Elf_Ident);
	unsigned long addr, loadaddr;

	/* Read the rest of the Elf header... */
	j = (*(file->fs->read)) (file, size, &e->e_version);
	if (j < size) {
		prom_printf("\nCan't read Elf64 image header: %08x/%08x\n", j, size);
		return 0;
	}

	DEBUG_F("Elf64 header:\n");
	DEBUG_F(" e.e_type      = %d\n", (int)e->e_type);
	DEBUG_F(" e.e_machine   = %d\n", (int)e->e_machine);
	DEBUG_F(" e.e_version   = %d\n", (int)e->e_version);
	DEBUG_F(" e.e_entry     = 0x%016lx\n", (long)e->e_entry);
	DEBUG_F(" e.e_phoff     = 0x%016lx\n", (long)e->e_phoff);
	DEBUG_F(" e.e_shoff     = 0x%016lx\n", (long)e->e_shoff);
	DEBUG_F(" e.e_flags     = %d\n", (int)e->e_flags);
	DEBUG_F(" e.e_ehsize    = 0x%08x\n", (int)e->e_ehsize);
	DEBUG_F(" e.e_phentsize = 0x%08x\n", (int)e->e_phentsize);
	DEBUG_F(" e.e_phnum     = %d\n", (int)e->e_phnum);

	loadinfo->entry = e->e_entry;

	if (e->e_phnum > MAX_HEADERS) {
		prom_printf("Can only load kernels with one program header\n");
		return 0;
	}

	ph = (Elf64_Phdr *) malloc(sizeof(Elf64_Phdr) * e->e_phnum);
	if (!ph) {
		prom_printf("Malloc error\n");
		return 0;
	}

	/* Now, we read the section header */
	if ((*(file->fs->seek)) (file, e->e_phoff) != FILE_ERR_OK) {
		prom_printf("Seek error\n");
		return 0;
	}
	j = (*(file->fs->read)) (file, sizeof(Elf64_Phdr) * e->e_phnum, ph);
	if (j != sizeof(Elf64_Phdr) * e->e_phnum) {
		prom_printf("Read error: %08x/%08x\n", j, sizeof(Elf64_Phdr) * e->e_phnum);
		return 0;
	}

	/* Scan through the program header
	 */
	loadinfo->memsize = loadinfo->filesize = loadinfo->offset = 0;
	p = ph;
	for (i = 0; i < e->e_phnum; ++i, ++p) {
		if (p->p_type != PT_LOAD || p->p_offset == 0)
			continue;
		loadinfo->offset = p->p_offset;
		loadinfo->memsize = p->p_memsz;
		loadinfo->filesize = p->p_filesz;
		loadinfo->load_loc = p->p_vaddr;
		break;
	}

	if (loadinfo->memsize == 0) {
		prom_printf("Can't find a loadable segment !\n");
		return 0;
	}

	/* leave some room (1Mb) for boot infos */
	loadinfo->memsize = _ALIGN(loadinfo->memsize, (1 << 20)) + 0x100000;

	loadaddr = 0;

	/* On some systems, loadaddr may already be claimed, so try some
	 * other nearby addresses before giving up.
	 */
	for (addr = loadaddr; addr < CLAIM_END; addr += 0x100000) {
		loadinfo->base = prom_claim((void *)addr, loadinfo->memsize, 0);
		if (loadinfo->base != (void *)-1)
			break;
	}
	if (loadinfo->base == (void *)-1) {
		prom_printf("Claim error, can't allocate kernel memory\n");
		return 0;
	}
	prom_printf("Allocated %08lx bytes for kernel @ %p\n", loadinfo->memsize, loadinfo->base);

	/* Load the program segments... */
	p = ph;
	for (i = 0; i < e->e_phnum; ++i, ++p) {
		unsigned long offset;
		if (p->p_type != PT_LOAD || p->p_offset == 0)
			continue;

		/* Now, we skip to the image itself */
		if ((*(file->fs->seek)) (file, p->p_offset) != FILE_ERR_OK) {
			prom_printf("Seek error\n");
			prom_release(loadinfo->base, loadinfo->memsize);
			return 0;
		}
		offset = p->p_vaddr - loadinfo->load_loc;
		j = (*(file->fs->read)) (file, p->p_filesz, loadinfo->base + offset);
		if (j != p->p_filesz) {
			prom_printf("Read failed: %08x/%08llx\n", j, p->p_filesz);
			prom_release(loadinfo->base, loadinfo->memsize);
			return 0;
		}
	}

	free(ph);

	/* Return success at loading the Elf64 kernel */
	return 1;
}

static void setup_display(void)
{
#ifdef CONFIG_SET_COLORMAP
	static unsigned char default_colors[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0xaa,
		0x00, 0xaa, 0x00,
		0x00, 0xaa, 0xaa,
		0xaa, 0x00, 0x00,
		0xaa, 0x00, 0xaa,
		0xaa, 0x55, 0x00,
		0xaa, 0xaa, 0xaa,
		0x55, 0x55, 0x55,
		0x55, 0x55, 0xff,
		0x55, 0xff, 0x55,
		0x55, 0xff, 0xff,
		0xff, 0x55, 0x55,
		0xff, 0x55, 0xff,
		0xff, 0xff, 0x55,
		0xff, 0xff, 0xff
	};
	int i;
	for (i = 0; i < 16; i++)
		prom_set_color(prom_stdout, i, default_colors[i * 3], default_colors[i * 3 + 1], default_colors[i * 3 + 2]);

	prom_printf("\x1b[1;37m\x1b[2;40m");
#ifdef COLOR_TEST
	for (i = 0; i < 16; i++) {
		prom_printf("\x1b[%d;%dm\x1b[1;47m%s \x1b[2;40m %s\n",
			    ansi_color_table[i].index, ansi_color_table[i].value, ansi_color_table[i].name, ansi_color_table[i].name);
		prom_printf("\x1b[%d;%dm\x1b[1;37m%s \x1b[2;30m %s\n",
			    ansi_color_table[i].index, ansi_color_table[i].value + 10, ansi_color_table[i].name, ansi_color_table[i].name);
	}
	prom_printf("\x1b[1;37m\x1b[2;40m");
#endif				/* COLOR_TEST */

#ifndef DEBUG
	prom_printf("\xc\n");
#endif				/* !DEBUG */

#endif				/* CONFIG_SET_COLORMAP */
}

static char *check_manual_config_filepath(char *bootargs)
{
	char *c2, *c1 = strstr(bootargs, conf_token);
	if (!c1)
		return NULL;
	do {
		c1 += strlen(conf_token);
		c2 = strstr(c1, conf_token);
		if (!c2)
			break;
		c2 += strlen(conf_token);
		c1 = strstr(c2, conf_token);
	} while (c1);
	if (!c1)
		c1 = c2;
	else
		c2 = c1;
	while (*c2) {
		if (' ' == *c2) {
			*c2 = '\0';
			break;
		}
		c2++;
	}
	return *c1 ? c1 : NULL;
}

static void word_split(char **linep, char **paramsp)
{
	char *p;

	*paramsp = 0;
	p = *linep;
	if (p == 0)
		return;
	while (*p == ' ')
		++p;
	if (*p == 0) {
		*linep = 0;
		return;
	}
	*linep = p;
	while (*p != 0 && *p != ' ')
		++p;
	while (*p == ' ')
		*p++ = 0;
	if (*p != 0)
		*paramsp = p;
}

static char *make_params_buffer;
static char *make_params(char *label, char *params)
{
	char *p, *q;

	q = make_params_buffer;
	*q = 0;

	p = cfg_get_strg(label, "literal");
	if (p) {
		strcpy(q, p);
		q = strchr(q, 0);
		if (params) {
			if (*p)
				*q++ = ' ';
			strcpy(q, params);
		}
		return make_params_buffer;
	}

	p = cfg_get_strg(label, "root");
	if (p) {
		strcpy(q, "root=");
		strcpy(q + 5, p);
		q = strchr(q, 0);
		*q++ = ' ';
	}
	if (cfg_get_flag(label, "read-only")) {
		strcpy(q, "ro ");
		q += 3;
	}
	if (cfg_get_flag(label, "read-write")) {
		strcpy(q, "rw ");
		q += 3;
	}
	p = cfg_get_strg(label, "ramdisk");
	if (p) {
		strcpy(q, "ramdisk=");
		strcpy(q + 8, p);
		q = strchr(q, 0);
		*q++ = ' ';
	}
	p = cfg_get_strg(label, "initrd-size");
	if (p) {
		strcpy(q, "ramdisk_size=");
		strcpy(q + 13, p);
		q = strchr(q, 0);
		*q++ = ' ';
	}
	if (cfg_get_flag(label, "novideo")) {
		strcpy(q, "video=ofonly");
		q = strchr(q, 0);
		*q++ = ' ';
	}
	p = cfg_get_strg(label, "append");
	if (p) {
		strcpy(q, p);
		q = strchr(q, 0);
		*q++ = ' ';
	}
	*q = 0;
	if (params)
		strcpy(q, params);

	return make_params_buffer;
}

static void check_password(char *str)
{
	int i;
	char *passwdbuff = passwdinit();

	if (!passwdbuff)
		return;
	prom_printf("\n%s", str);
	for (i = 0; i < 3; i++) {
		prom_printf("\nPassword: ");
		passwordedit(passwdbuff);
		prom_printf("\n");
#ifdef USE_MD5_PASSWORDS
		if (!strncmp(password, "$1$", 3)) {
			if (!check_md5_password(passwdbuff, password))
				return;
		} else if (!strcmp(password, passwdbuff))
			return;
#else				/* !MD5 */
		if (!strcmp(password, passwdbuff))
			return;
#endif				/* USE_MD5_PASSWORDS */
		if (i < 2) {
			prom_sleep(1);
			prom_printf("Incorrect password.  Try again.");
		}
	}
	prom_printf(" ___________________\n< Permission denied >\n -------------------\n"
		    "        \\   ^__^\n         \\  (oo)\\_______\n            (__)\\       )\\/\\\n" "                ||----w |\n                ||     ||\n");
	prom_sleep(4);
	prom_interpret("reset-all");
}

static void print_boot(const char *p)
{
	prom_printf("boot: %s", p ? p : "");
}

static void print_all_labels(void)
{
	if (useconf) {
		cfg_print_images(NULL, 0, 0);
		print_boot(NULL);
	}
}
enum get_params_result {
	GET_PARAMS_FATAL,
	GET_PARAMS_OK,
	GET_PARAMS_STOP,
};

static enum get_params_result get_params(struct boot_param_t *params, enum get_params_result gpr)
{
	struct path_description img_def_device;
	char *p, *q, *cmdbuff, *imagename, *label;
	int c, timeout, restricted;

	cmdbuff = cmdlineinit();
	if (!cmdbuff)
		return GET_PARAMS_FATAL;

	imagename = label = NULL;
	restricted = c = 0;
	memcpy(&img_def_device, &default_device, sizeof(img_def_device));
	memset(params, 0, sizeof(*params));
	params->args = "";

	print_boot(NULL);

	if (lilo_once_cmdline[0]) {
		strcpy(cmdbuff, lilo_once_cmdline);
		lilo_once_cmdline[0] = '\0';
		imagename = cmdbuff;
		prom_printf("lilo once: '%s'\n", imagename);
		word_split(&imagename, &params->args);
	} else {
		if (gpr == GET_PARAMS_OK && useconf && (p = cfg_get_strg(NULL, "timeout")) && *p) {
			timeout = simple_strtol(p, NULL, 0);
			if (timeout > 0) {
				timeout = prom_getms() + 100 * timeout;
				do {
					c = prom_nbgetchar();
				} while (c == -1 && prom_getms() <= timeout);
			}
			if (c == -1 || !c)
				c = '\n';
			else if (!char_is_newline(c) && !char_is_tab(c) && !char_is_backspace(c)) {
				cmdbuff[0] = c = char_to_ascii(c);
				cmdbuff[1] = 0;
			}
		}

		if (char_is_newline(c)) {
			imagename = cfg_get_default();
			if (imagename)
				prom_printf("%s", imagename);
			prom_printf("\n");
		} else {
			if (c >= ' ' && useconf && cfg_get_flag(cmdbuff, "single-key")) {
				imagename = cmdbuff;
				prom_printf("%s\n", cmdbuff);
			} else {
				if (char_is_tab(c))
					print_all_labels();
				cmdlineedit(cmdbuff, print_boot);
				prom_printf("\n");
				imagename = cmdbuff;
				word_split(&imagename, &params->args);
			}
		}
	}

	if (useconf && (!imagename || imagename[0] == 0))
		imagename = cfg_get_default();

	/* Now we have an image or label name and additional cmdline options.
	   Store them in nvram in case the pSeries firmware has to reboot */
	c = strlen(imagename) + 1 + strlen(params->args) + 1;
	p = malloc(c);
	if (p) {
		sprintf(p, "%s %s", imagename, params->args[0] ? params->args : "");
		prom_set_options("boot-last-label", p, c);
		free(p);
	}

	if (useconf) {
		set_default_device(cfg_get_strg(NULL, "device"), cfg_get_strg(NULL, "partition"), &img_def_device);
		if (cfg_get_flag(NULL, "restricted"))
			restricted = 1;
		p = cfg_get_strg(imagename, "image");
		if (p && *p) {
			label = imagename;
			imagename = p;
			set_default_device(cfg_get_strg(label, "device"), cfg_get_strg(label, "partition"), &img_def_device);
			if (cfg_get_flag(label, "restricted"))
				restricted = 1;
			if (label && password) {
				if (params->args && restricted)
					check_password("To specify arguments for this image you must enter the password.");
				else if (!restricted)
					check_password("This image is restricted.");
			}
			params->args = make_params(label, params->args);
		}
	}

	if (imagename == NULL)
		return GET_PARAMS_STOP;

	if (!strcmp(imagename, "help")) {
		prom_printf("  Press the tab key for a list of defined images.\n"
			    "  The label marked with a \"*\" is the default image, press <return> to boot it.\n\n"
			    "  To boot any other label simply type its name and press <return>.\n"
			    "  It is also possible to expand a label with the tab key.\n\n"
			    "  To boot a kernel image which is not defined in the yaboot configuration \n"
			    "  file, enter the kernel image name as [[device:[partno]],]/path, where \n"
			    "  \"device:\" is the OpenFirmware device path to the disk the image \n"
			    "  resides on, and \"partno\" is the partition number the image resides on.\n"
			    "  Note that the comma (,) is only required if you specify an OpenFirmware\n"
			    "  device, if you only specify a filename you should not start it with a \",\"\n"
			    "  The shortcut '&device;' can be used to specify the OpenFirmware device path\n"
			    "  where yaboot was loaded from.\n\n"
			    "  To load an initrd, specify its path as initrd=[[device:[partno]],]/path\n\n"
			    "  If you omit \"device:\" and \"partno\" yaboot will use the values of \n"
			    "  \"device=\" and \"partition=\" in yaboot.conf, right now those are set to: \n"
			    "  device=%s\n  partition=%d\n\n", default_device.device, default_device.part);
		return GET_PARAMS_STOP;
	}

	if (!strcmp(imagename, "halt")) {
		if (password)
			check_password("Restricted command.");
		prom_pause();
		return GET_PARAMS_FATAL;
	}
	if (!strcmp(imagename, "bye")) {
		if (password)
			check_password("Restricted command.");
		return GET_PARAMS_FATAL;
	}

	if (imagename[0] == '$') {
		/* forth command string */
		if (password)
			check_password("OpenFirmware commands are restricted.");
		prom_interpret(imagename + 1);
		return GET_PARAMS_STOP;
	}

	if (!label && password)
		check_password("To boot a custom image you must enter the password.");

	if (!imagepath_to_path_description(imagename, &params->kernel, &img_def_device)) {
		prom_printf("%s: Unable to parse\n", imagename);
		return GET_PARAMS_STOP;
	}

	p = strstr(params->args, "initrd=");
	if (p) {
		p = strdup(p + sizeof("initrd=") - 1);
		if (p) {
			q = strchr(p, ' ');
			if (q)
				*q = '\0';
		}
	} else if (useconf)
		p = cfg_get_strg(label, "initrd");

	if (p && *p) {
		DEBUG_F("Parsing initrd path <%s>\n", p);
		if (!imagepath_to_path_description(p, &params->rd, &img_def_device))
			prom_printf("%s: Unable to parse\n", p);
	}
	return GET_PARAMS_OK;
}

/* This is derived from quik core. To be changed to first parse the headers
 * doing lazy-loading, and then claim the memory before loading the kernel
 * to it
 * We also need to add initrd support to this whole mecanism
 */
static void yaboot_text_ui(void)
{

	struct boot_file_t file;
	int result;
	int chunksize;
	enum get_params_result gpr;
	struct boot_param_t params;
	struct path_description *kernel, *rd;
	unsigned long claim_base;
	void *initrd_base;
	unsigned long initrd_size;
	kernel_entry_t kernel_entry;
	loadinfo_t loadinfo;
	void *initrd_more, *initrd_want;
	unsigned long initrd_read;
	char fw_nbr_reboots[4];
	char *msg;

	kernel = &params.kernel;
	rd = &params.rd;
	make_params_buffer = malloc(2048);
	if (!make_params_buffer)
		return;
	loadinfo.load_loc = 0;
	gpr = GET_PARAMS_OK;

	memset(fw_nbr_reboots, 0, sizeof(fw_nbr_reboots));
	if (prom_get_chosen("ibm,client-architecture-support-reboot", &fw_nbr_reboots, sizeof(fw_nbr_reboots)) == -1)
		prom_get_options("ibm,fw-nbr-reboots", &fw_nbr_reboots, sizeof(fw_nbr_reboots));
	result = simple_strtol(fw_nbr_reboots, &msg, 10);
	if (result > 0)
		prom_get_options("boot-last-label", lilo_once_cmdline, 512);

	for (;;) {
		initrd_size = 0;
		initrd_base = NULL;

		gpr = get_params(&params, gpr);
		if (gpr == GET_PARAMS_FATAL)
			return;
		if (gpr == GET_PARAMS_STOP)
			continue;

		prom_printf("Please wait, loading kernel...\n");

		msg = path_description_to_string(kernel);
		result = open_file(kernel, &file);
		if (result != FILE_ERR_OK) {
			if (msg) {
				prom_perror(result, msg);
				free(msg);
			}
			gpr = GET_PARAMS_STOP;
			continue;
		}

		/* Read the Elf e_ident, e_type and e_machine fields to
		 * determine Elf file type
		 */
		if (file.fs->read(&file, sizeof(Elf_Ident), &loadinfo.elf) < sizeof(Elf_Ident)) {
			prom_printf("\nCan't read Elf e_ident/e_type/e_machine info\n");
			file.fs->close(&file);
			gpr = GET_PARAMS_STOP;
			continue;
		}

		if (is_elf32(&loadinfo)) {
			if (!load_elf32(&file, &loadinfo)) {
				file.fs->close(&file);
				gpr = GET_PARAMS_STOP;
				continue;
			}
			prom_printf("   Elf32 kernel loaded...\n");
		} else if (is_elf64(&loadinfo)) {
			if (!load_elf64(&file, &loadinfo)) {
				file.fs->close(&file);
				gpr = GET_PARAMS_STOP;
				continue;
			}
			prom_printf("   Elf64 kernel loaded...\n");
		} else {
			prom_printf("%s: Not a valid ELF image\n", path_filename(kernel));
			file.fs->close(&file);
			gpr = GET_PARAMS_STOP;
			continue;
		}
		file.fs->close(&file);
		if (msg) {
			prom_set_chosen("yaboot,image", msg, strlen(msg) + 1);
			free(msg);
		}

		/* If ramdisk, load it (only if booting a vmlinux).  For now, we
		 * can't tell the size it will be so we claim an arbitrary amount
		 * of 4Mb.
		 */
		if (path_filename(rd)) {
			prom_printf("Loading ramdisk...\n");
			msg = path_description_to_string(rd);
			result = open_file(rd, &file);
			if (result != FILE_ERR_OK) {
				if (msg) {
					prom_perror(result, msg);
					free(msg);
				}
				gpr = GET_PARAMS_STOP;
			} else {
				/* put initrd after the kernels final location */
				/* it seems that the B50 has trouble with a location between
				 * real-base and 32M. The kernel crashes at random places in
				 * prom_init, usually in opening display. */
				if (64 == _cpu)
					claim_base = loadinfo.memsize;
				else
					claim_base = 32 * 1024 * 1024;
				if (file.dev_type == TYPE_NET)
					chunksize = file.len;
				else
					chunksize = 10 * 1024 * 1024; /* FIXME: if we had a stat() ... */
				/* try to claim memory up to 128MB */
				while (claim_base + chunksize < 128 * 1024 * 1024) {
					initrd_base = prom_claim((void *)claim_base, chunksize, 0);
					if (initrd_base != (void *)-1)
						break;
					claim_base += 64 * 1024;
				}
				if (initrd_base == (void *)-1) {
					prom_printf("Claim failed for initrd memory\n");
					initrd_base = NULL;
				} else {
					initrd_size = file.fs->read(&file, chunksize, initrd_base);
					if (initrd_size == 0)
						initrd_base = NULL;
					if (file.dev_type != TYPE_NET) {
						initrd_read = initrd_size;
						initrd_more = initrd_base;
						while (initrd_read == chunksize) {	/* need to read more? */
							initrd_want = (void *)((unsigned long)initrd_more + chunksize);
							initrd_more = prom_claim(initrd_want, chunksize, 0);
							if (initrd_more != initrd_want) {
								prom_printf("Claim failed for initrd memory at %p rc=%p\n", initrd_want, initrd_more);
								break;
							}
							initrd_read = file.fs->read(&file, chunksize, initrd_more);
							DEBUG_F("  block at %p rc=%lu\n", initrd_more, initrd_read);
							initrd_size += initrd_read;
						}
					}
				}
				file.fs->close(&file);
			}
			if (initrd_base) {
				if (msg) {
					prom_set_chosen("yaboot,initrd", msg, strlen(msg) + 1);
					free(msg);
				}
				prom_printf("ramdisk loaded %08lx @ %p\n", initrd_size, initrd_base);
			} else {
				prom_printf("ramdisk load failed !\n");
				gpr = GET_PARAMS_STOP;
				continue;
			}
		}

		DEBUG_F("setting kernel args to: %s\n", params.args);
		prom_set_chosen("bootargs", params.args, strlen(params.args) + 1);
		DEBUG_F("flushing icache...");
		flush_icache_range((long)loadinfo.base, (long)loadinfo.base + loadinfo.memsize);
		DEBUG_F(" done\n");

		/* compute the kernel's entry point. */
		kernel_entry = loadinfo.base + loadinfo.entry - loadinfo.load_loc;

		DEBUG_F("Kernel entry point = %p\n", kernel_entry);
		DEBUG_F("kernel: arg1 = %p,\n" "        arg2 = %08lx,\n" "        prom = %p,\n", initrd_base + loadinfo.load_loc, initrd_size, prom);

		DEBUG_F("Entering kernel...\n");

		/* call the kernel with our stack. */
		kernel_entry(initrd_base + loadinfo.load_loc, initrd_size, prom, 0, 0);
	}
}

static char sysinfo[234];
static const char si_model[] = "model";
static const char si_serial[] = "serial-number";
static const char si_systemid[] = "system-id";
static const char si_partition_name[] = "ibm,partition-name";

static int append_system_info(const char *node, const char *prop, char *s, size_t size, int off, const char *ident, int last, int mac_serial)
{
	char buf[32], *p;
	int len, num;

	len = prom_getprop(prom_finddevice(node), prop, buf, sizeof(buf) - 1);
	if (len > 0 && len < sizeof(buf)) {
		if (last) {
			strncat(s, ",", size - off);
			off = strlen(s);
		}
		strncat(s, " ", size - off);
		off = strlen(s);
		strncat(s, ident, size - off);
		off = strlen(s);
		strncat(s, " '", size - off);
		off = strlen(s);
		p = buf;
		if (mac_serial) {
			num = 10;
			while (num) {
				p = p + strlen(p);
				num--;
				p++;	/* skip null byte */
			}
		}
		strncat(s, p, size - off);
		off = strlen(s);
		strncat(s, "'", size - off);
		off = strlen(s);
	}
	return off;
}
static void get_system_info(char *s, size_t size)
{
	int off;

	sprintf(s, "running");
	off = strlen(s);
	off = append_system_info("/openprom", si_model, s, size, off, "with firmware", 0, 0);
	off = append_system_info("/", si_model, s, size, off, "on model", 0, 0);
	if (prom_getproplen(prom_finddevice("/"), si_serial) > 0)
		off = append_system_info("/", si_serial, s, size, off, "serial", 1, 1);
	else
		off = append_system_info("/", si_systemid, s, size, off, "serial", 1, 0);
	off = append_system_info("/", si_partition_name, s, size, off, "partition", 1, 0);
}

static void yaboot_main(void)
{
	char *bootpath = NULL, *bootargs = NULL;
	char *conf_file_buf, *configfile = NULL;
	int bootpath_len, bootargs_len;
	int sz;
	if (prom_getprop(call_prom("instance-to-package", 1, 1, prom_stdout), "iso6429-1983-colors", NULL, 0) >= 0) {
		stdout_is_screen = 1;
		setup_display();
	}

	bootpath_len = prom_getproplen_chosen("bootpath");
	bootargs_len = prom_getproplen_chosen("bootargs");

	if (bootpath_len > 0)
		bootpath = malloc(bootpath_len + 1);
	if (bootargs_len > 0)
		bootargs = malloc(bootargs_len + 1);

	if (bootpath) {
		memset(bootpath, 0, bootpath_len + 1);
		prom_get_chosen("bootpath", bootpath, bootpath_len);
		bootpath = hard_coded_bootpath(bootpath);
		DEBUG_F("/chosen/bootpath = %s\n", bootpath);
		if (bootpath[0] == 0) {
			prom_printf("Couldn't determine boot device\n");
			return;
		}

		if (!yaboot_set_bootpath(bootpath, &default_device)) {
			prom_printf("%s: Unable to parse\n", bootpath);
			return;
		}
		prom_set_chosen("yaboot,bootpath", bootpath, strlen(bootpath) + 1);
	}

	if (bootargs) {
		memset(bootargs, 0, bootargs_len + 1);
		prom_get_chosen("bootargs", bootargs, bootargs_len);
		DEBUG_F("/chosen/bootargs = '%s'\n", bootargs);
		if (bootargs[0]) {
			prom_set_chosen("yaboot,bootargs", bootargs, bootargs_len + 1);
			configfile = check_manual_config_filepath(bootargs);
			if (!imagepath_to_path_description(configfile, &default_device, &default_device))
				configfile = NULL;
		}
	}

	/* Allocate a buffer for the config file */
	conf_file_buf = malloc(CONFIG_FILE_MAX);
	if (!conf_file_buf) {
		prom_printf("Can't alloc config file buffer\n");
		return;
	}

	if (configfile)
		sz = load_config_file(configfile, conf_file_buf, &default_device);
	else {
		sz = (int)(_yaboot_conf_end - _yaboot_conf_start);
		if (sz) {
			if (sz > CONFIG_FILE_MAX)
				sz = CONFIG_FILE_MAX - 1;
			memcpy(conf_file_buf, _yaboot_conf_start, sz);
			configfile = "built-in";
		} else
			sz = find_and_load_config_file(&default_device, conf_file_buf);
	}
	if (sz > 0)
		useconf = cfg_parse(conf_file_buf, sz, _cpu);
	if (useconf)
		process_configfile();
	free(conf_file_buf);

	get_system_info(sysinfo, sizeof(sysinfo) - 1);
	prom_printf("Welcome to yaboot version " VERSION "\n");
	prom_printf("booted from '%s'\n", bootpath);
	prom_printf("%s\n", sysinfo);
	if (configfile && sz > 0)
		prom_printf("Using configfile '%s'\n", configfile);
	prom_printf("Enter \"help\" to get some basic usage information\n");
	free(bootargs);
	free(bootpath);

	yaboot_text_ui();

	prom_printf("Bye.\n");
}

void yaboot_start(unsigned long r3, unsigned long r4, unsigned long r5, void *sp)
{
	void *malloc_base;
	prom_handle cpus[1];
	unsigned long addr;

	memset(__bss_start, 0, _end - __bss_start);

	/* Initialize OF interface */
	prom_init((prom_entry) r5);

	prom_printf("\nyaboot starting: loaded at %p %p (%lx/%lx/%08lx; sp: %p)\n", _start, _end, r3, r4, r5, sp);

	/* the executable memrange may not be claimed by firmware */
	if (prom_claim(_start, _end - _start, 0) == _start)
		prom_printf("brokenfirmware did not claim executable memory, fixed it myself\n");

	sles9_base = prom_claim((void *)SLES9_ZIMAGE_BASE, SLES9_ZIMAGE_SIZE, 0);
	if (sles9_base == (void *)-1)
		sles9_base = NULL;
	DEBUG_F("Allocated %08x bytes @ %p for SLES8/9 install file\n", SLES9_ZIMAGE_SIZE, sles9_base);

	if (sles9_base)
		addr = SLES9_ZIMAGE_BASE + SLES9_ZIMAGE_SIZE;
	else
		addr = 64 * 1024;
	for (; addr < CLAIM_END - MALLOCSIZE; addr += 64 * 1024) {
		/* overlap check */
		if ((addr < (unsigned long)_end || addr + MALLOCSIZE < (unsigned long)_end)
		    && (addr >= (unsigned long)_start || addr + MALLOCSIZE >= (unsigned long)_start))
			continue;
		/* Allocate some memory for malloc'ator */
		malloc_base = prom_claim((void *)addr, MALLOCSIZE, 0);
		if (malloc_base && malloc_base != (void *)-1)
			break;
	}
	if (malloc_base == (void *)-1) {
		prom_printf("Can't claim malloc buffer (%x bytes between %08x and %08x)\n", MALLOCSIZE, 64 * 1024, CLAIM_END - MALLOCSIZE);
		goto exit;
	}
	malloc_init(malloc_base, MALLOCSIZE);
	DEBUG_F("Malloc buffer allocated at %p (%x bytes)\n", malloc_base, MALLOCSIZE);

	cpus[0] = 0;
	find_type_devices(cpus, "cpu", sizeof(cpus) / sizeof(prom_handle));
	if (cpus[0]) {
		if (prom_getprop(cpus[0], "64-bit", NULL, 0) >= 0)
			_cpu = 64;
		else
			_cpu = identify_cpu();

		DEBUG_F("Running on %d-bit\n", _cpu);
		yaboot_main();
	}

	/* Get rid of malloc pool */
	malloc_dispose();
	prom_release(malloc_base, MALLOCSIZE);

      exit:
	/* Return to OF */
	while (1)
		prom_exit();
}

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
