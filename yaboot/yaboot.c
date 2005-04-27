/* Yaboot - secondary boot loader for Linux on iMacs.

   Copyright (C) 1999 Benjamin Herrenschmidt

   portions based on poof

   Copyright (C) 1999 Marius Vollmer

   portions based on quik
   
   Copyright (C) 1996 Paul Mackerras.

   Because this program is derived from the corresponding file in the
   silo-0.64 distribution, it is also

   Copyright (C) 1996 Pete A. Zaitcev
   		 1996 Maurizio Plaza
   		 1996 David S. Miller
   		 1996 Miguel de Icaza
   		 1996 Jakub Jelinek

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdarg.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"
#include "prom.h"
#include "file.h"
#include "cfg.h"
#include "cmdline.h"
#include "yaboot.h"
#include "linux/elf.h"
#include "bootinfo.h"

#define CONFIG_FILE_NAME	"yaboot.conf"
#define CONFIG_FILE_MAX		0x8000		/* 32k */
#define SPLASH_SCREEN

#ifdef SPLASH_SCREEN
#include "gui.h"
#endif

/* align addr on a size boundry - adjust address up if needed -- Cort */
#define _ALIGN(addr,size)	(((addr)+size-1)&(~(size-1)))

/* Imported functions */
extern unsigned long reloc_offset(void);
extern int strtol(const char *nptr, char **endptr, int base);
extern long flush_icache_range(unsigned long start, unsigned long stop);

/* Exported functions */
int		yaboot_start(int r3, int r4, int r5);

/* Local functions */
static int	yaboot_main(void);

/* Locals & globals */

int useconf = 0;
char bootdevice[1024];
char *bootpath;
int bootpartition = -1;
int _machine = _MACH_Pmac;

#if DEBUG
static int test_bss;
static int test_data = 0;
#endif
static int pause_after;
static char *pause_message = "Type go<return> to continue.\n";
static char given_bootargs[1024];
static int given_bootargs_by_user = 0;

extern unsigned char linux_logo_red[];
extern unsigned char linux_logo_green[];
extern unsigned char linux_logo_blue[];

#define DEFAULT_TIMEOUT		-1

/* Entry, currently called directly by crt0 (bss not inited) */

extern char* __bss_start;
extern char* _end;

static struct first_info *quik_fip = NULL;

int
yaboot_start (int r3, int r4, int r5)
{
	int result;
	void* malloc_base;
	prom_handle root;
	
 	/* OF seems to do it, but I'm not very confident */
  	memset(&__bss_start, 0, &_end - &__bss_start);
  	
	/* Check for quik first stage bootloader (but I don't think we are
	 * compatible with it anyway, I'll look into backporting to older OF
	 * versions later
	 */
	if (r5 == 0xdeadbeef) {
		r5 = r3;
		quik_fip = (struct first_info *)r4;
	}

  	/* Initialize OF interface */
	prom_init ((prom_entry) r5);
	
	/* Allocate some memory for malloc'ator */
	malloc_base = prom_claim((void *)MALLOCADDR, MALLOCSIZE, 0);
	if (malloc_base == (void *)-1) {
		prom_printf("Can't claim malloc buffer (%d bytes at 0x%08lx)\n",
			MALLOCSIZE, MALLOCADDR);
		return -1;
	}
	malloc_init(malloc_base, MALLOCSIZE);
#if DEBUG
	prom_printf("Malloc buffer allocated at 0x%x (%d bytes)\n",
		malloc_base, MALLOCSIZE);
#endif
		
	/* A few useless printf's */
#if DEBUG
	prom_printf("reloc_offset :  %ld         (should be 0)\n", reloc_offset());
	prom_printf("test_bss     :  %d         (should be 0)\n", test_bss);
	prom_printf("test_data    :  %d         (should be 0)\n", test_data);
	prom_printf("&test_data   :  0x%08lx\n", &test_data);
	prom_printf("&test_bss    :  0x%08lx\n", &test_bss);
	prom_printf("linked at    :  0x%08lx\n", TEXTADDR);
#endif	
	/* ask the OF info if we're a chrp or pmac */
	/* we need to set _machine before calling finish_device_tree */
	root = prom_finddevice("/");
	if (root != 0) {
		static char model[256];
		if (prom_getprop(root, "device_type", model, 256 ) > 0 &&
			!strncmp("chrp", model, 4))
			_machine = _MACH_chrp;
		else {
			if (prom_getprop(root, "model", model, 256 ) > 0 &&
				!strncmp(model, "IBM", 3))
				_machine = _MACH_chrp;
		}
	}
	
#if DEBUG
	prom_printf("Running on _machine = %d\n", _machine);
#endif

	/* Call out main */
	result = yaboot_main();

	/* Get rid of malloc pool */
	malloc_dispose();
	prom_release(malloc_base, MALLOCSIZE);
#if DEBUG
	prom_printf("Malloc buffer released. Exiting with code %d\n",
		result);

#endif

	/* Return to OF */
	prom_exit();
	
	return result;
	
}

/* Currently, the config file must be at the root of the filesystem.
 * todo: recognize the full path to myself and use it to load the
 * config file. Handle the "\\" (blessed system folder)
 */
static int
load_config_file(char *device, char* path, int partition)
{
    char *conf_file = NULL, *p;
    struct boot_file_t file;
    int sz, opened = 0, result = 0;
    char conf_path[512];
    struct boot_fspec_t fspec;

    /* Allocate a buffer for the config file */
    conf_file = malloc(CONFIG_FILE_MAX);
    if (!conf_file) {
    	prom_printf("Can't alloc config file buffer\n");
    	goto bail;
    }

    /* Build the path to the file */
    if (path)
    	strcpy(conf_path, path);
    else
    	conf_path[0] = 0;
    strcat(conf_path, CONFIG_FILE_NAME);

    /* Open it */
    fspec.dev = device;
    fspec.file = conf_path;
    fspec.part = partition;
    result = open_file(&fspec, &file);
    if (result != FILE_ERR_OK) {
    	prom_printf("Can't open config file, err: %d\n", result);
	goto bail;
    }
    opened = 1;

    /* Read it */
    sz = file.read(&file, CONFIG_FILE_MAX, conf_file);
    if (sz <= 0) {
    	prom_printf("Error, can't read config file\n");
    	goto bail;
    }
    prom_printf("Config file read, %d bytes\n", sz);

    /* Close the file */
    if (opened)
    	file.close(&file);
    opened = 0;

    /* Call the parsing code in cfg.c */
    if (cfg_parse(conf_path, conf_file, sz) < 0) {
	prom_printf ("Syntax error or read error config\n");
	goto bail;
    }

#if DEBUG
    prom_printf("Config file successfully parsed\n", sz);
#endif

    /* Now, we do the initialisations stored in the config file */
    p = cfg_get_strg(0, "init-code");
    if (p)
	prom_interpret(p);
    p = cfg_get_strg(0, "init-message");
    if (p)
	prom_printf("%s\n", p);
#if 0
    p = cfg_get_strg(0, "message");
    if (p)
	print_message_file(p);
#endif		

    result = 1;
    
bail:

    if (opened)
    	file.close(&file);
    
    if (result != 1 && conf_file)
    	free(conf_file);
    	
    return result;
}

void maintabfunc (char *cbuff)
{
    if (useconf) {
	cfg_print_images();
	prom_printf("boot: %s", cbuff);
    }
}

void
word_split(char **linep, char **paramsp)
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

char *
make_params(char *label, char *params)
{
    char *p, *q;
    static char buffer[2048];

    q = buffer;
    *q = 0;

    p = cfg_get_strg(label, "literal");
    if (p) {
	strcpy(q, p);
	q = strchr(q, 0);
	if (*params) {
	    if (*p)
		*q++ = ' ';
	    strcpy(q, params);
	}
	return buffer;
    }

    p = cfg_get_strg(label, "root");
    if (p) {
	strcpy (q, "root=");
	strcpy (q + 5, p);
	q = strchr (q, 0);
	*q++ = ' ';
    }
    if (cfg_get_flag(label, "read-only")) {
	strcpy (q, "ro ");
	q += 3;
    }
    if (cfg_get_flag(label, "read-write")) {
	strcpy (q, "rw ");
	q += 3;
    }
    p = cfg_get_strg(label, "ramdisk");
    if (p) {
	strcpy (q, "ramdisk=");
	strcpy (q + 8, p);
	q = strchr (q, 0);
	*q++ = ' ';
    }
    p = cfg_get_strg(label, "initrd-size");
    if (p) {
	strcpy (q, "ramdisk_size=");
	strcpy (q + 13, p);
	q = strchr (q, 0);
	*q++ = ' ';
    }
    if (cfg_get_flag(label, "novideo")) {
	strcpy (q, "video=ofonly");
	q = strchr (q, 0);
	*q++ = ' ';
    }
    p = cfg_get_strg (label, "append");
    if (p) {
	strcpy (q, p);
	q = strchr (q, 0);
	*q++ = ' ';
    }
    *q = 0;
    pause_after = cfg_get_flag (label, "pause-after");
    p = cfg_get_strg(label, "pause-message");
    if (p)
	pause_message = p;
    if (*params)
	strcpy(q, params);

    return buffer;
}

int get_params(struct boot_param_t* params)
{
    int defpart;
    char *defdevice = 0;
    char *p, *q, *endp;
    int c, n;
    char *imagename = 0, *label;
    int timeout = -1;
    int beg = 0, end;
    static int first = 1;
    static char bootargs[1024];
    static char imagepath[1024];
    static char initrdpath[1024];
    static char sysmappath[1024];
#ifdef SPLASH_SCREEN
    static char splashpath[1024];
#endif

    pause_after = 0;
    memset(params, 0, sizeof(*params));
    params->args = "";
    params->kernel.part = -1;
    params->rd.part = -1;
    params->sysmap.part = -1;
    params->splash.part = -1;
    defpart = bootpartition;
    
    if (first) {
	first = 0;
	prom_get_chosen("bootargs", bootargs, sizeof(bootargs));
	imagename = bootargs;
	word_split(&imagename, &params->args);
	timeout = DEFAULT_TIMEOUT;
	if (useconf && (q = cfg_get_strg(0, "timeout")) != 0 && *q != 0)
	    timeout = strtol(q, NULL, 0);
    }

    prom_printf("boot: ");
    c = -1;
    if (timeout != -1) {
	beg = prom_getms();
	if (timeout > 0) {
	    end = beg + 100 * timeout;
	    do {
		c = prom_nbgetchar();
	    } while (c == -1 && prom_getms() <= end);
	}
	if (c == -1)
	    c = '\n';
    }

    if (c == '\n') {
	prom_printf("%s", imagename);
	if (params->args)
	    prom_printf(" %s", params->args);
	prom_printf("\n");

    } else {
	cmdinit();
	cmdedit(maintabfunc, c);
	prom_printf("\n");
	strcpy(given_bootargs, cmd_buffer);
	given_bootargs_by_user = 1;
	imagename = cmd_buffer;
	word_split(&imagename, &params->args);
    }

    /* chrp gets this wrong, force it -- Cort */
    if ( useconf && ((imagename[0] == 0) /*|| is_chrp*/) )
	imagename = cfg_get_default();

    label = 0;
    defdevice = bootdevice;

    if (useconf) {
	defdevice = cfg_get_strg(0, "device");
	p = cfg_get_strg(0, "partition");
	if (p) {
	    n = strtol(p, &endp, 10);
	    if (endp != p && *endp == 0)
		defpart = n;
	}
	p = cfg_get_strg(0, "pause-message");
	if (p)
	    pause_message = p;
	p = cfg_get_strg(imagename, "image");
	if (p && *p) {
	    label = imagename;
	    imagename = p;
	    defdevice = cfg_get_strg(label, "device");
	    if(!defdevice) defdevice=bootdevice;
	    p = cfg_get_strg(label, "partition");
	    if (p) {
		n = strtol(p, &endp, 10);
		if (endp != p && *endp == 0)
		    defpart = n;
	    }
	    params->args = make_params(label, params->args);
	}
    }

    if (!strcmp (imagename, "halt")) {
	prom_pause();
	return 0;
    }
    if (!strcmp (imagename, "bye"))
    	return 1; 

    if (imagename[0] == '$') {
	/* forth command string */
	prom_interpret(imagename+1);
	return 0;
    }
    strncpy(imagepath, imagename, 1024);
    params->kernel.dev = parse_device_path(imagepath, &params->kernel.file,
    	&params->kernel.part);
    if (validate_fspec(&params->kernel, defdevice, defpart) != FILE_ERR_OK) {
	prom_printf(
"Enter the kernel image name as [device:][partno]/path, where partno is a\n"
"number from 0 to 16.  Instead of /path you can type [mm-nn] to specify a\n"
"range of disk blocks (512B)\n");
	return 0;
    }

    if (useconf) {
 	p = cfg_get_strg(label, "initrd");
	if (p && *p) {
#if DEBUG
	    prom_printf("parsing initrd path <%s>\n", p);
#endif	    
	    strncpy(initrdpath, p, 1024);
	    params->rd.dev = parse_device_path(initrdpath,
	    	&params->rd.file, &params->rd.part);
	    validate_fspec(&params->rd, defdevice, defpart);
	}
 	p = cfg_get_strg(label, "sysmap");
	if (p && *p) {
#if DEBUG
	    prom_printf("parsing sysmap path <%s>\n", p);
#endif	    
	    strncpy(sysmappath, p, 1024);
	    params->sysmap.dev = parse_device_path(sysmappath,
	    	&params->sysmap.file, &params->sysmap.part);
	    validate_fspec(&params->sysmap, defdevice, defpart);
	}
#ifdef SPLASH_SCREEN
 	p = cfg_get_strg(label, "splash");
	if (p && *p) {
#if DEBUG
	    prom_printf("parsing splash path <%s>\n", p);
#endif	    
	    strncpy(splashpath, p, 1024);
	    params->splash.dev = parse_device_path(splashpath,
	    	&params->splash.file, &params->splash.part);
	    validate_fspec(&params->splash, defdevice, defpart);
	}
   }
#endif /* SPLASH_SCREEN */
    
    return 0;
}

/* This is derived from quik core. To be changed to first parse the headers
 * doing lazy-loading, and then claim the memory before loading the kernel
 * to it
 * We also need to add initrd support to this whole mecanism
 */
void
yaboot_text_ui(void)
{
#define MAX_HEADERS	32

    struct boot_file_t	file;
    int			result;
    void*		base;
    Elf32_Ehdr		e;
    Elf32_Phdr		*p, *ph;
    static struct boot_param_t	params;
    void		*initrd_base, *sysmap_base;
    unsigned int	memsize, filesize, offset, initrd_size, load_loc = 0;
    unsigned int	sysmap_size;
    int 		i;
    struct bi_record*	birec;
    
    for (;;) {
    	initrd_size = 0;
    	initrd_base = 0;
    	
	if (get_params(&params))
	    return;
	if (!params.kernel.file)
	    continue;
	
#ifdef SPLASH_SCREEN
	if (params.splash.file)
        	fxDisplaySplash(&params.splash);
#endif
	prom_printf("Loading kernel...\n");
	result = open_file(&params.kernel, &file);
	if (result != FILE_ERR_OK) {
	    prom_printf("\nImage not found.... try again\n");
	    continue;
	}	
    	if (file.read(&file, sizeof(Elf32_Ehdr), &e) < sizeof(e)) {
	    prom_printf("\nCan't read image header\n");
	    goto next;
	}
	/* By this point the first sector is loaded,
	 * we check if it is an executable elf binary. */
	if (e.e_ident[EI_MAG0] != ELFMAG0
	      || e.e_ident[EI_MAG1] != ELFMAG1
	      || e.e_ident[EI_MAG2] != ELFMAG2
	      || e.e_ident[EI_MAG3] != ELFMAG3
	      || e.e_ident[EI_CLASS] != ELFCLASS32
	      || e.e_ident[EI_DATA] != ELFDATA2MSB
	      || e.e_type != ET_EXEC
	      || e.e_machine != EM_PPC) {
	    prom_printf ("not a valid ELF image\n");
	    goto next;
	}

#if DEBUG
	prom_printf("elf header:\n");
	prom_printf(" e.e_type      = %d\n", (int)e.e_type);
	prom_printf(" e.e_type      = %d\n", (int)e.e_type);
	prom_printf(" e.e_machine   = %d\n", (int)e.e_machine);
	prom_printf(" e.e_version   = %d\n", (int)e.e_version);
	prom_printf(" e.e_entry     = 0x%08x\n", (int)e.e_entry);
	prom_printf(" e.e_phoff     = 0x%08x\n", (int)e.e_phoff);
	prom_printf(" e.e_shoff     = 0x%08x\n", (int)e.e_shoff);
	prom_printf(" e.e_flags     = %d\n", (int)e.e_flags);
	prom_printf(" e.e_ehsize    = 0x%08x\n", (int)e.e_ehsize);
	prom_printf(" e.e_phentsize = 0x%08x\n", (int)e.e_phentsize);
	prom_printf(" e.e_phnum     = %d\n", (int)e.e_phnum);
#endif	    

	if (e.e_phnum > MAX_HEADERS) {
	    prom_printf ("can only load kernels with one program header\n");
	    goto next;
	}
	
	ph = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * e.e_phnum);
	if (!ph) {
	    prom_printf ("malloc error\n");
	    goto next;
	}
		
	/* Now, we read the section header */
	if (file.seek(&file, e.e_phoff) != FILE_ERR_OK) {
	    prom_printf ("seek error\n");
	    goto next;
	}
	if (file.read(&file, sizeof(Elf32_Phdr) * e.e_phnum, ph) !=
		sizeof(Elf32_Phdr) * e.e_phnum) {
	    prom_printf ("read error\n");
	    goto next;
	}

	/* Scan through the program header
	 * HACK:  We must return the _memory size of the kernel image, not the
	 *        file size (because we have to leave room before other boot
	 *	  infos. This code works as a side effect of the fact that
	 *	  we have one section and vaddr == p_paddr
	 */
	memsize = filesize = offset = 0;
	p = ph;
	for (i = 0; i < e.e_phnum; ++i, ++p) {
	    if (p->p_type != PT_LOAD || p->p_offset == 0)
		continue;
	    if (memsize == 0) {
		offset = p->p_offset;
		memsize = p->p_memsz;
		filesize = p->p_filesz;
		load_loc = p->p_vaddr;
	    } else {
		memsize = p->p_offset + p->p_memsz - offset; /* XXX Bogus */
		filesize = p->p_offset + p->p_filesz - offset;
	    }
	}
	free(ph);
	if (memsize == 0) {
	    prom_printf("Can't find a loadable segment !\n");
	    goto next;
	}

	/* leave some room (1Mb) for boot infos */
	memsize = _ALIGN(memsize,(1<<20)) + 0x100000;
	/* Claim OF memory */
	base = prom_claim((void *)KERNELADDR, memsize, 0);
	if (base == (void *)-1) {
	    prom_printf("claim error, can't allocate kernel memory\n");
	    goto next;
	}	
#if DEBUG
	prom_printf("After ELF parsing, load base: 0x%08lx, mem_sz: 0x%08lx, birec: \n",
    		base, memsize);
#endif    
	/* Now, we skip to the image itself */
	if (file.seek(&file, offset) != FILE_ERR_OK) {
	    prom_printf ("seek error\n");
	    prom_release(base, memsize);
	    goto next;
	}
#ifdef SPLASH_SCREEN
        if (fxReadImage(&file, filesize, base) != filesize) {
#else
        if (file.read(&file, filesize, base) != filesize) {
#endif
	    prom_printf ("read failed\n");
	    prom_release(base, memsize);
	    goto next;
	}
#if 0	/* to make editor happy */
	}
#endif	
	file.close(&file);

	/* If sysmap, load it. 
	 */
	if (params.sysmap.file) {
	    prom_printf("Loading System.map ...\n");
	    result = open_file(&params.sysmap, &file);
	    if (result != FILE_ERR_OK)
		prom_printf("\nSystem.map file not found.\n");
	    else {
		sysmap_base = prom_claim(base+memsize, 0x100000, 0);
		if (sysmap_base == (void *)0xffffffff) {
		    prom_printf("claim failed for sysmap memory\n");
		    sysmap_base = 0;
		} else {
	    	    sysmap_size = file.read(&file, 0x400000, sysmap_base);
	    	    if (sysmap_size == 0)
	    	        sysmap_base = 0;
	    	}
	    	file.close(&file);
	    }
	    if (sysmap_base) {
	    	prom_printf("System.map loaded at 0x%08lx, size: %d bytes\n",
	    		sysmap_base, sysmap_size);
	    	memsize += _ALIGN(0x100000, 0x1000);
	    } else {
	    	prom_printf("System.map load failed !\n");
	    	prom_pause();
	    }
	}

	/* If ramdisk, load it. For now, we can't tell the size it will be
	 * so we claim an arbitrary amount of 4Mb
	 */
	if (params.rd.file) {
	    prom_printf("Loading ramdisk...\n");
	    result = open_file(&params.rd, &file);
	    if (result != FILE_ERR_OK)
		prom_printf("\nRamdisk image not found.\n");
	    else {
		initrd_base = prom_claim(base+memsize, 0x400000, 0);
		if (initrd_base == (void *)0xffffffff) {
		    prom_printf("claim failed for initrd memory\n");
		    initrd_base = 0;
		} else {
	    	    initrd_size = file.read(&file, 0x400000, initrd_base);
	    	    if (initrd_size == 0)
	    	        initrd_base = 0;
	    	}
	    	file.close(&file);
	    }
	    if (initrd_base)
	    	prom_printf("ramdisk loaded at 0x%08lx, size: %d bytes\n",
	    		initrd_base, initrd_size);
	    else {
	    	prom_printf("ramdisk load failed !\n");
	    	prom_pause();
	    }
	}

#if DEBUG
	prom_printf("setting kernel args to: %s\n", params.args);
#endif	
	prom_setargs(params.args);
#if DEBUG
	prom_printf("flushing icache...\n");
#endif	
	flush_icache_range ((int)base, (int)base+memsize);

	/* 
	 * Fill mew boot infos
	 *
	 * The birec is low on memory, probably inside the malloc pool, so
	 * we don't write it earlier. At this point, we should not use anything
	 * coming from the malloc pool
	 */
	birec = (struct bi_record *)_ALIGN(filesize+(1<<20)-1,(1<<20));
	/* We make sure it's mapped. We map only 64k for now, it's plenty enough
	 * we don't claim since this precise memory range may already be claimed
	 * by the malloc pool
	 */
	prom_map (birec, birec, 0x10000);
#if DEBUG
	prom_printf("birec at 0x%08lx\n", birec);
	i = prom_getms();
	while((prom_getms() - i) < 2000)
		;
#endif	

	birec->tag = BI_FIRST;
	birec->size = sizeof(struct bi_record);
	birec = (struct bi_record *)((unsigned long)birec + birec->size);
	
	birec->tag = BI_BOOTLOADER_ID;
	sprintf( (char *)birec->data, "yaboot");
	birec->size = sizeof(struct bi_record) + strlen("yaboot") + 1;
	birec = (struct bi_record *)((unsigned long)birec + birec->size);
	
	birec->tag = BI_MACHTYPE;
	birec->data[0] = _machine;
	birec->data[1] = 1;
	birec->size = sizeof(struct bi_record) + sizeof(unsigned long);
	birec = (struct bi_record *)((unsigned long)birec + birec->size);
	
	birec->tag = BI_SYSMAP;
	birec->data[0] = (unsigned long)sysmap_base;
	birec->data[1] = sysmap_size;
	birec->size = sizeof(struct bi_record) + sizeof(unsigned long);
	birec = (struct bi_record *)((unsigned long)birec + birec->size);
	birec->tag = BI_LAST;
	birec->size = sizeof(struct bi_record);
	birec = (struct bi_record *)((unsigned long)birec + birec->size);

	// call the kernel with our stack.
#if DEBUG
	prom_printf("entering kernel...\n");
#endif	
	((void (*)())(base + e.e_entry - load_loc + 0xC)) (
		initrd_base + load_loc, 
		initrd_size,
		prom, 0, 0);
	continue;
next:
	file.close(&file);    
    }
}

static void
setup_display(void)
{
	static unsigned char default_colors[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0xaa,
		0x00, 0xaa, 0x00,
		0x00, 0xaa, 0xaa,
		0xaa, 0x00, 0x00,
		0xaa, 0x00, 0xaa,
		0xaa, 0xaa, 0x00,
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

	for(i=0;i<32;i++) {
		prom_setcolor(i, default_colors[i*3],
			default_colors[i*3+1], default_colors[i*3+2]);
	}
	prom_printf("\x1b[33m\x1b[40m");
#if !DEBUG
	prom_printf("\xc");
#endif	
}

int
yaboot_main(void)
{
	setup_display();
	
	prom_printf("Welcome to yaboot version " VERSION "\n");
	
	prom_get_chosen("bootpath", bootdevice, sizeof(bootdevice));
#if DEBUG
	prom_printf("/chosen/bootpath = %s\n", bootdevice);
#endif	
	if (bootdevice[0] == 0)
		prom_get_options("boot-device", bootdevice, sizeof(bootdevice));
	if (bootdevice[0] == 0) {
	    prom_printf("Coundn't determine boot device\n");
	    return -1;
    	}
    	parse_device_path(bootdevice, &bootpath, &bootpartition);
#if DEBUG
	prom_printf("after parse_device_path: device:%s, path: %s, partition: %d\n",
		bootdevice, bootpath ? bootpath : NULL, bootpartition);
#endif	
  	if (bootpath) {
		if (!strncmp(bootpath, "\\\\", 2))
			bootpath[2] = 0;
		else {
			char *p, *last;
			p = last = bootpath;
			while(*p) {
			    if (*p == '\\')
			    	last = p;
			    p++;
			}
			if (p)
				*(last) = 0;
			else
				bootpath = NULL;
			if (bootpath && strlen(bootpath))
				strcat(bootpath, "\\");
		}
	}
#if DEBUG
	prom_printf("after path fixup: device:%s, path: %s, partition: %d\n",
		bootdevice, bootpath ? bootpath : NULL, bootpartition);
#endif	
	useconf = load_config_file(bootdevice, bootpath, bootpartition);
	yaboot_text_ui();
	
	prom_printf("Bye.\n");
	return 0;
}
