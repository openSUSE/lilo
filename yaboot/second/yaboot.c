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

#include "stdarg.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"
#include "prom.h"
#include "file.h"
#include "errors.h"
#include "cfg.h"
#include "cmdline.h"
#include "yaboot.h"
#include "linux/elf.h"
#include "debug.h"

#define CONFIG_FILE_MAX		0x8000		/* 32k */

#ifdef USE_MD5_PASSWORDS
#include "md5.h"
#endif /* USE_MD5_PASSWORDS */

/* align addr on a size boundry - adjust address up if needed -- Cort */
#define _ALIGN(addr,size)	(((addr)+size-1)&(~(size-1)))

/* Addresses where the PPC32 and PPC64 vmlinux kernels are linked at.
 * These are used to determine whether we are booting a vmlinux, in
 * which case, it will be loaded at KERNELADDR.  Otherwise (eg zImage),
 * we load the binary where it was linked at (ie, e_entry field in
 * the ELF header).
 */
#define KERNEL_LINK_ADDR_PPC32	0xC0000000UL
#define KERNEL_LINK_ADDR_PPC64	0xC000000000000000ULL

typedef struct {
     union {
	  Elf32_Ehdr  elf32hdr;
	  Elf64_Ehdr  elf64hdr;
     } elf;
     void*	    base;
     unsigned long   memsize;
     unsigned long   filesize;
     unsigned long   offset;
     unsigned long   load_loc;
     unsigned long   entry;
} loadinfo_t;

typedef void (*kernel_entry_t)( void *,
                                unsigned long,
                                prom_entry,
                                unsigned long,
                                unsigned long );

/* Imported functions */
extern long flush_icache_range(unsigned long start, unsigned long stop);
extern int identify_cpu(void);

/* Local functions */
static int	yaboot_main(void);
static int	is_elf32(loadinfo_t *loadinfo);
static int	is_elf64(loadinfo_t *loadinfo);
static int      load_elf32(struct boot_file_t *file, loadinfo_t *loadinfo);
static int      load_elf64(struct boot_file_t *file, loadinfo_t *loadinfo);
static void     setup_display(void);

/* Locals & globals */

int useconf;
static char bootdevice[1024];
static char *password;
static struct boot_fspec_t boot;
static struct default_device default_device;
static int _cpu;

#ifdef CONFIG_COLOR_TEXT

/* Color values for text ui */
static struct ansi_color_t {
     char*	name;
     int	index;
     int	value;
} ansi_color_table[] = {
     { "black",		2, 30 },
     { "blue",		0, 31 },
     { "green",		0, 32 },
     { "cyan",		0, 33 },
     { "red",		0, 34 },
     { "purple",		0, 35 },
     { "brown",		0, 36 },
     { "light-gray", 	0, 37 },
     { "dark-gray",		1, 30 },
     { "light-blue",		1, 31 },
     { "light-green",	1, 32 },
     { "light-cyan",		1, 33 },
     { "light-red",		1, 34 },
     { "light-purple",	1, 35 },
     { "yellow",		1, 36 },
     { "white",		1, 37 },
     { NULL,			0, 0 },
};

/* Default colors for text ui */
static int fgcolor = 15;
static int bgcolor;
#endif /* CONFIG_COLOR_TEXT */

#define DEFAULT_TIMEOUT		-1

/* Entry, currently called directly by crt0 (bss not inited) */

extern char __bss_start[];
extern char _start[];
extern char _end[];

int
yaboot_start (unsigned long r3, unsigned long r4, unsigned long r5, void *sp)
{
     int result;
     void* malloc_base;
     prom_handle cpus[1];

     /* OF seems to do it, but I'm not very confident */
     memset(__bss_start, 0, _end - __bss_start);
  	
     /* Initialize OF interface */
     prom_init ((prom_entry) r5);

     prom_printf("\nyaboot starting: loaded at 0x%p-0x%p (0x%lx/0x%lx/0x%08lx;0x%p)\n",
				_start, _end, r3, r4, r5, sp);

     /* the executable memrange may not be claimed by firmware */
     if (prom_claim(_start, _end - _start, 0) == _start)
	     prom_printf("brokenfirmware did not claim executable memory, fixed it myself\n");
	
     /* Allocate some memory for malloc'ator */
     malloc_base = prom_claim((void *)MALLOCADDR, MALLOCSIZE, 0);
     if (malloc_base == (void *)-1) {
	  prom_printf("Can't claim malloc buffer (%d bytes at 0x%08x)\n",
		      MALLOCSIZE, MALLOCADDR);
	  return -1;
     }
     malloc_init(malloc_base, MALLOCSIZE);
     DEBUG_F("Malloc buffer allocated at %p (%d bytes)\n",
	     malloc_base, MALLOCSIZE);
		
     cpus[0] = 0;
     find_type_devices(cpus, "cpu", sizeof(cpus)/sizeof(prom_handle));
     if (cpus[0]) {
	     if (prom_getprop(cpus[0], "64-bit", NULL, 0) >= 0)
		     _cpu = 64;
	     else
		     _cpu = identify_cpu();
     } else 
	     return -1;
	
     DEBUG_F("Running on %d-bit\n", _cpu);

     /* Call out main */
     result = yaboot_main();

     /* Get rid of malloc pool */
     malloc_dispose();
     prom_release(malloc_base, MALLOCSIZE);
     DEBUG_F("Malloc buffer released. Exiting with code %d\n",
	     result);

     /* Return to OF */
     prom_exit();
	
     return result;
	
}

#ifdef CONFIG_COLOR_TEXT
/*
 * Validify color for text ui
 */
static int
check_color_text_ui(char *color)
{
     int i = 0;
     while(ansi_color_table[i].name) {
	  if (!strcmp(color, ansi_color_table[i].name))
	       return i;
	  i++;
     }
     return -1;
}      
#endif /* CONFIG_COLOR_TEXT */


static void print_message_file(const char *filename, const struct boot_fspec_t *b, const struct default_device *d)
{
     char *msg; 
     int result;
     struct boot_file_t file;
     struct boot_fspec_t msgfile;

     parse_file_to_load_path(filename, &msgfile, b, d);

     result = open_file(&msgfile, &file);
     if (result != FILE_ERR_OK) {
	  prom_printf("%s:%d,", msgfile.device, msgfile.part);
	  prom_perror(result, msgfile.filename);
	  return;
     }

     msg = malloc(2001);
     if (!msg)
	  goto done;

      memset(msg, 0, 2001);

     if (file.fs->read(&file, 2000, msg) <= 0)
	  goto done;
     prom_printf("%s", msg);
     free(msg);
done:
     file.fs->close(&file);
}

static const char *config_file_names_net[] = {
	"yaboot.conf",
	NULL
};
static const char *config_file_names_block[] = {
	"yaboot.cnf",
	"yaboot.conf",
	"/etc/yaboot.conf",
	NULL
};
static int load_config_file(const struct boot_fspec_t *b)
{
     char *conf_file, *p;
     const char **names;
     struct boot_file_t file;
     int sz, opened = 0, result = 0;
     int i;
     struct boot_fspec_t config_fspec;

     /* Allocate a buffer for the config file */
     conf_file = malloc(CONFIG_FILE_MAX);
     if (!conf_file) {
	  prom_printf("Can't alloc config file buffer\n");
	  goto bail;
     }

     switch (b->type) {
	case TYPE_NET:
		names = config_file_names_net;
		break;
	case TYPE_BLOCK:
		names = config_file_names_block;
		break;
	default:
		prom_printf("type '%d' not handled\n", b->type);
		goto bail;
		break;
     }
     for (i = 0; names[i]; i++) {
	     if (!parse_file_to_load_path(names[i], &config_fspec, b, NULL)) {
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
     sz = file.fs->read(&file, CONFIG_FILE_MAX, conf_file);
     file.fs->close(&file);
     if (sz <= 0) {
	  prom_printf("Error, can't read config file\n");
	  goto bail;
     }
     prom_printf("Config file '%s' read, %d bytes\n", names[i], sz);

     /* Call the parsing code in cfg.c */
     if (cfg_parse(names[i], conf_file, sz, _cpu) < 0) {
	  prom_printf ("Syntax error or read error config\n");
	  goto bail;
     }

     DEBUG_F("Config file successfully parsed, %d bytes\n", sz);

     /* Now, we do the initialisations stored in the config file */
     p = cfg_get_strg(NULL, "init-code");
     if (p)
	  prom_interpret(p);

     set_def_device(cfg_get_strg(NULL, "device"), cfg_get_strg(NULL, "partition"), &default_device);

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
#endif /* !DEBUG */
     }
     if (fgcolor >= 0) {
	  char temp[64];
	  sprintf(temp, "%x to foreground-color", fgcolor); 
	  prom_interpret(temp); 
     }
#endif /* CONFIG_COLOR_TEXT */
   
     result = 1;
    
bail:
     if (conf_file)
	  free(conf_file);
    	
     return result > 0;
}

static void maintabfunc (void)
{
     if (useconf) {
	  cfg_print_images();
	  prom_printf("boot: %s", cbuff);
     }
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

static char * make_params(char *label, char *params)
{
     char *p, *q;
     static char buffer[2048];

     q = buffer;
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
     if (params)
	  strcpy(q, params);

     return buffer;
}

static void check_password(char *str)
{
     int i;

     prom_printf("\n%s", str);
     for (i = 0; i < 3; i++) {
	  prom_printf ("\nPassword: ");
	  passwdbuff[0] = 0;
	  cmdedit ((void (*)(void)) 0, 1);
	  prom_printf ("\n");
#ifdef USE_MD5_PASSWORDS
	  if (!strncmp (password, "$1$", 3)) {
	       if (!check_md5_password(passwdbuff, password))
		    return;
	  } 
	  else if (!strcmp (password, passwdbuff))
	       return;
#else /* !MD5 */
	  if (!strcmp (password, passwdbuff))
	       return;
#endif /* USE_MD5_PASSWORDS */
	  if (i < 2) {
	       prom_sleep(1);
	       prom_printf ("Incorrect password.  Try again.");
	  }
     }
     prom_printf(" ___________________\n< Permission denied >\n -------------------\n"
		 "        \\   ^__^\n         \\  (oo)\\_______\n            (__)\\       )\\/\\\n"
		 "                ||----w |\n                ||     ||\n");
     prom_sleep(4);
     prom_interpret("reset-all");
}

static int get_params(struct boot_param_t* params)
{
     struct default_device img_def_device, *d;
     char *p, *q;
     int c;
     char *imagename = NULL, *label;
     int timeout;
     int beg, end;
     int singlekey = 0;
     int restricted = 0;

     d = &default_device;
     memset(params, 0, sizeof(*params));
     params->args = "";
    
     cmdinit();

	  timeout = DEFAULT_TIMEOUT;
	  if (useconf && (q = cfg_get_strg(NULL, "timeout")) != 0 && *q != 0)
	       timeout = simple_strtol(q, NULL, 0);

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
	  else if (c != '\n' && c != '\t' && c != '\r' && c != '\b' ) {
	       cbuff[0] = c;
	       cbuff[1] = 0;
	  }
     }

     if (c != -1 && c != '\n' && c != '\r') {
	  if (c == '\t') {
	       maintabfunc ();
	  }  else if (c >= ' ') {
	       cbuff[0] = c;
	       cbuff[1] = 0;
	       if ((cfg_get_flag (cbuff, "single-key")) && useconf) {
		    imagename = cbuff;
		    singlekey = 1;
		    prom_printf("%s\n", cbuff);
	       }
	  }
     }

     if (c == '\n' || c == '\r') {
	  if (!imagename)
	       imagename = cfg_get_default();
	  if (imagename)
	       prom_printf("%s", imagename);
	  if (params->args)
	       prom_printf(" %s", params->args);
	  prom_printf("\n");
     } else if (!singlekey) {
	  cmdedit(maintabfunc, 0);
	  prom_printf("\n");
	  imagename = cbuff;
	  word_split(&imagename, &params->args);
     }

     /* chrp gets this wrong, force it -- Cort */
     if ( useconf && (!imagename || imagename[0] == 0 ))
	  imagename = cfg_get_default();

     label = 0;

     if (useconf) {
	  set_def_device(cfg_get_strg(NULL, "device"), cfg_get_strg(NULL, "partition"), d);
	  if (cfg_get_flag(NULL, "restricted"))
	       restricted = 1;
	  p = cfg_get_strg(imagename, "image");
	  if (p && *p) {
	       label = imagename;
	       imagename = p;
	       if (set_def_device(cfg_get_strg(label, "device"), cfg_get_strg(label, "partition"), &img_def_device))
		       d = &img_def_device;
	       if (cfg_get_flag(label, "restricted"))
		    restricted = 1;
	       if (label) {
		    if (params->args && password && restricted)
			 check_password ("To specify arguments for this image "
					 "you must enter the password.");
		    else if (password && !restricted)
			 check_password ("This image is restricted.");
	       }
	       params->args = make_params(label, params->args);
	  }
     }

     if (imagename == NULL)
	     return 0;

     if (!strcmp (imagename, "help")) {
	  prom_printf(
	       "\nPress the tab key for a list of defined images.\n"
	       "The label marked with a \"*\" is is the default image, "
	       "press <return> to boot it.\n\n"
	       "To boot any other label simply type its name and press <return>.\n\n"
	       "To boot a kernel image which is not defined in the yaboot configuration \n"
	       "file, enter the kernel image name as [[device:][partno],]/path, where \n"
	       "\"device:\" is the OpenFirmware device path to the disk the image \n"
	       "resides on, and \"partno\" is the partition number the image resides on.\n"
	       "Note that the comma (,) is only required if you specify an OpenFirmware\n"
	       "device, if you only specify a filename you should not start it with a \",\"\n\n"
	       "If you omit \"device:\" and \"partno\" yaboot will use the values of \n"
	       "\"device=\" and \"partition=\" in yaboot.conf, right now those are set to: \n"
	       "device=%s\n"
	       "partition=%d\n\n",
	       default_device.device ? default_device.device : boot.device,
	       default_device.part > 0 ? default_device.part : boot.part);
	  return 0;
     }

     if (!strcmp (imagename, "halt")) {
	  if (password)
	       check_password ("Restricted command.");
	  prom_pause();
	  return 0;
     }
     if (!strcmp (imagename, "bye")) {
	  if (password)
	       check_password ("Restricted command.");
	  return 1; 
     }

     if (imagename[0] == '$') {
	  /* forth command string */
	  if (password)
	       check_password ("OpenFirmware commands are restricted.");
	  prom_interpret(imagename+1);
	  return 0;
     }

     if (!label && password)
	  check_password ("To boot a custom image you must enter the password.");

     if (!parse_file_to_load_path(imagename, &params->kernel, &boot, d)) {
	  prom_printf("%s: Unable to parse\n", imagename);
	  return 0;
     }

     if (useconf) {
	  p = cfg_get_strg(label, "initrd");
	  if (p && *p) {
	       DEBUG_F("Parsing initrd path <%s>\n", p);
	       if (!parse_file_to_load_path(p, &params->rd, &boot, d)) {
		       prom_printf("%s: Unable to parse\n", p);
		    return 0;
	       }
	  }
     }
     return 0;
}

/* This is derived from quik core. To be changed to first parse the headers
 * doing lazy-loading, and then claim the memory before loading the kernel
 * to it
 * We also need to add initrd support to this whole mecanism
 */
static void yaboot_text_ui(void)
{
#define MAX_HEADERS	32

     struct boot_file_t	file;
     int			result;
     static struct boot_param_t	params;
     void 		*claim_base;
     void		*initrd_base;
     unsigned long	initrd_size;
     kernel_entry_t      kernel_entry;
     loadinfo_t          loadinfo;
     void                *initrd_more,*initrd_want;
     unsigned long       initrd_read;
    
     loadinfo.load_loc = 0;

     for (;;) {
	  initrd_size = 0;
	  initrd_base = NULL;
    	
	  if (get_params(&params))
	       return;
	  if (!params.kernel.filename)
	       continue;
	
	  prom_printf("Please wait, loading kernel...\n");

	  result = open_file(&params.kernel, &file);
	  if (result != FILE_ERR_OK) {
	       prom_printf("%s:%d,", params.kernel.device, params.kernel.part);
	       prom_perror(result, params.kernel.filename);
	       continue;
	  }

	  /* Read the Elf e_ident, e_type and e_machine fields to
	   * determine Elf file type
	   */
	  if (file.fs->read(&file, sizeof(Elf_Ident), &loadinfo.elf) < sizeof(Elf_Ident)) {
	       prom_printf("\nCan't read Elf e_ident/e_type/e_machine info\n");
	       file.fs->close(&file);
	       continue;
	  }

	  if (is_elf32(&loadinfo)) {
	       if (!load_elf32(&file, &loadinfo)) {
		    file.fs->close(&file);
		    continue;
	       }
	       prom_printf("   Elf32 kernel loaded...\n");
	  } else if (is_elf64(&loadinfo)) {
	       if (!load_elf64(&file, &loadinfo)) {
		    file.fs->close(&file);
		    continue;
	       }
	       prom_printf("   Elf64 kernel loaded...\n");
	  } else {
	       prom_printf ("%s: Not a valid ELF image\n", params.kernel.filename);
	       file.fs->close(&file);
	       continue;
	  }
	  file.fs->close(&file);

	  /* If ramdisk, load it (only if booting a vmlinux).  For now, we
	   * can't tell the size it will be so we claim an arbitrary amount
	   * of 4Mb.
	   */
	  if (params.rd.filename) {
	       prom_printf("Loading ramdisk...\n");
	       result = open_file(&params.rd, &file);
	       if (result != FILE_ERR_OK) {
		    prom_printf("%s:%d,", params.rd.device, params.rd.part);
		    prom_perror(result, params.rd.filename);
	       }
	       else {
#define INITRD_CHUNKSIZE 0x400000
		    claim_base = loadinfo.base + loadinfo.memsize;
		    for (result = 0; result < 42; result++) {
			    initrd_base = prom_claim(claim_base, INITRD_CHUNKSIZE, 0);
			    if (initrd_base != (void *)-1)
				    break;
			    claim_base += (unsigned long)claim_base + (1*1024*1024);
		    }
		    if (initrd_base == (void *)-1) {
			 prom_printf("Claim failed for initrd memory\n");
			 initrd_base = NULL;
		    } else {
			 initrd_size = file.fs->read(&file, INITRD_CHUNKSIZE, initrd_base);
			 if (initrd_size == 0)
			      initrd_base = NULL;
			 initrd_read = initrd_size;
			 initrd_more = initrd_base;
			 while (initrd_read == INITRD_CHUNKSIZE ) { /* need to read more? */
			      initrd_want = (void *)((unsigned long)initrd_more+INITRD_CHUNKSIZE);
			      initrd_more = prom_claim(initrd_want, INITRD_CHUNKSIZE, 0);
			      if (initrd_more != initrd_want) {
				   prom_printf("Claim failed for initrd memory at %p rc=%p\n",initrd_want,initrd_more);
				   break;
			      }
			      initrd_read = file.fs->read(&file, INITRD_CHUNKSIZE, initrd_more);
			      DEBUG_F("  block at %p rc=%lu\n",initrd_more,initrd_read);
			      initrd_size += initrd_read;
			 }
		    }
		    file.fs->close(&file);
	       }
	       if (initrd_base)
		    prom_printf("ramdisk loaded at %p, size: %lu Kbytes\n",
				initrd_base, initrd_size >> 10);
	       else {
		    prom_printf("ramdisk load failed !\n");
		    prom_pause();
	       }
	  }

	  DEBUG_F("setting kernel args to: %s\n", params.args);
	  prom_setargs(params.args);
	  DEBUG_F("flushing icache...");
	  flush_icache_range ((long)loadinfo.base, (long)loadinfo.base+loadinfo.memsize);
	  DEBUG_F(" done\n");

          /* compute the kernel's entry point. */
	  kernel_entry = loadinfo.base + loadinfo.entry - loadinfo.load_loc;

	  DEBUG_F("Kernel entry point = %p\n", kernel_entry);
	  DEBUG_F("kernel: arg1 = %p,\n"
		  "        arg2 = 0x%08lx,\n"
		  "        prom = %p,\n"
		  "        arg4 = %d,\n"
		  "        arg5 = %d\n\n",
		  initrd_base + loadinfo.load_loc, initrd_size, prom, 0, 0);

	  DEBUG_F("Entering kernel...\n");

          /* call the kernel with our stack. */
	  kernel_entry(initrd_base + loadinfo.load_loc, initrd_size, prom, 0, 0);
     }
}

static int
load_elf32(struct boot_file_t *file, loadinfo_t *loadinfo)
{
     int			i;
     Elf32_Ehdr		*e = &(loadinfo->elf.elf32hdr);
     Elf32_Phdr		*p, *ph;
     int			size = sizeof(Elf32_Ehdr) - sizeof(Elf_Ident);
     unsigned long	addr, loadaddr;

     /* Read the rest of the Elf header... */
     if ((*(file->fs->read))(file, size, &e->e_version) < size) {
	  prom_printf("\nCan't read Elf32 image header\n");
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
	  prom_printf ("Can only load kernels with one program header\n");
	  return 0;
     }

     ph = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * e->e_phnum);
     if (!ph) {
	  prom_printf ("Malloc error\n");
	  return 0;
     }

     /* Now, we read the section header */
     if ((*(file->fs->seek))(file, e->e_phoff) != FILE_ERR_OK) {
	  prom_printf ("seek error\n");
	  return 0;
     }
     if ((*(file->fs->read))(file, sizeof(Elf32_Phdr) * e->e_phnum, ph) !=
	 sizeof(Elf32_Phdr) * e->e_phnum) {
	  prom_printf ("read error\n");
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
     loadinfo->memsize = _ALIGN(loadinfo->memsize,(1<<20)) + 0x100000;
     /* Claim OF memory */
     DEBUG_F("Before prom_claim, mem_sz: 0x%08lx\n", loadinfo->memsize);

     /* Determine whether we are trying to boot a vmlinux or some
      * other binary image (eg, zImage).  We load vmlinux's at
      * KERNELADDR and all other binaries at their e_entry value.
      */
     if (e->e_entry == KERNEL_LINK_ADDR_PPC32) {
          loadaddr = KERNELADDR;
     } else {
          loadaddr = loadinfo->load_loc;
     }

     /* On some systems, loadaddr may already be claimed, so try some
      * other nearby addresses before giving up.
      */
     for(addr=loadaddr; addr <= loadaddr * 8 ;addr+=0x100000) {
	  loadinfo->base = prom_claim((void *)addr, loadinfo->memsize, 0);
	  if (loadinfo->base != (void *)-1) break;
     }
     if (loadinfo->base == (void *)-1) {
	  prom_printf("Claim error, can't allocate kernel memory\n");
	  return 0;
     }	

     DEBUG_F("After ELF parsing, load base: %p, mem_sz: 0x%08lx\n",
	     loadinfo->base, loadinfo->memsize);
     DEBUG_F("    wanted load base: 0x%08lx, mem_sz: 0x%08lx\n",
	     loadaddr, loadinfo->memsize);

     /* Load the program segments... */
     p = ph;
     for (i = 0; i < e->e_phnum; ++i, ++p) {
	  unsigned long offset;
	  if (p->p_type != PT_LOAD || p->p_offset == 0)
	       continue;

	  /* Now, we skip to the image itself */
	  if ((*(file->fs->seek))(file, p->p_offset) != FILE_ERR_OK) {
	       prom_printf ("Seek error\n");
	       prom_release(loadinfo->base, loadinfo->memsize);
	       return 0;
	  }
	  offset = p->p_vaddr - loadinfo->load_loc;
	  if ((*(file->fs->read))(file, p->p_filesz, loadinfo->base+offset) != p->p_filesz) {
	       prom_printf ("Read failed\n");
	       prom_release(loadinfo->base, loadinfo->memsize);
	       return 0;
	  }
     }

     free(ph);
    
     /* Return success at loading the Elf32 kernel */
     return 1;
}

static int
load_elf64(struct boot_file_t *file, loadinfo_t *loadinfo)
{
     int			i;
     Elf64_Ehdr		*e = &(loadinfo->elf.elf64hdr);
     Elf64_Phdr		*p, *ph;
     int			size = sizeof(Elf64_Ehdr) - sizeof(Elf_Ident);
     unsigned long	addr, loadaddr;

     /* Read the rest of the Elf header... */
     if ((*(file->fs->read))(file, size, &e->e_version) < size) {
	  prom_printf("\nCan't read Elf64 image header\n");
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
	  prom_printf ("Can only load kernels with one program header\n");
	  return 0;
     }

     ph = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr) * e->e_phnum);
     if (!ph) {
	  prom_printf ("Malloc error\n");
	  return 0;
     }

     /* Now, we read the section header */
     if ((*(file->fs->seek))(file, e->e_phoff) != FILE_ERR_OK) {
	  prom_printf ("Seek error\n");
	  return 0;
     }
     if ((*(file->fs->read))(file, sizeof(Elf64_Phdr) * e->e_phnum, ph) !=
	 sizeof(Elf64_Phdr) * e->e_phnum) {
	  prom_printf ("Read error\n");
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
     loadinfo->memsize = _ALIGN(loadinfo->memsize,(1<<20)) + 0x100000;
     /* Claim OF memory */
     DEBUG_F("Before prom_claim, mem_sz: 0x%08lx\n", loadinfo->memsize);

     /* Determine whether we are trying to boot a vmlinux or some
      * other binary image (eg, zImage).  We load vmlinux's at
      * KERNELADDR and all other binaries at their e_entry value.
      */
     if (e->e_entry == KERNEL_LINK_ADDR_PPC64) {
          loadaddr = KERNELADDR;
     } else {
          loadaddr = e->e_entry;
     }

     /* On some systems, loadaddr may already be claimed, so try some
      * other nearby addresses before giving up.
      */
     for(addr=loadaddr; addr <= loadaddr * 8 ;addr+=0x100000) {
	  loadinfo->base = prom_claim((void *)addr, loadinfo->memsize, 0);
	  if (loadinfo->base != (void *)-1) break;
     }
     if (loadinfo->base == (void *)-1) {
	  prom_printf("Claim error, can't allocate kernel memory\n");
	  return 0;
     }	

     DEBUG_F("After ELF parsing, load base: %p, mem_sz: 0x%08lx\n",
	     loadinfo->base, loadinfo->memsize);
     DEBUG_F("    wanted load base: 0x%08lx, mem_sz: 0x%08lx\n",
	     loadaddr, loadinfo->memsize);

     /* Load the program segments... */
     p = ph;
     for (i = 0; i < e->e_phnum; ++i, ++p) {
	  unsigned long offset;
	  if (p->p_type != PT_LOAD || p->p_offset == 0)
	       continue;

	  /* Now, we skip to the image itself */
	  if ((*(file->fs->seek))(file, p->p_offset) != FILE_ERR_OK) {
	       prom_printf ("Seek error\n");
	       prom_release(loadinfo->base, loadinfo->memsize);
	       return 0;
	  }
	  offset = p->p_vaddr - loadinfo->load_loc;
	  if ((*(file->fs->read))(file, p->p_filesz, loadinfo->base+offset) != p->p_filesz) {
	       prom_printf ("Read failed\n");
	       prom_release(loadinfo->base, loadinfo->memsize);
	       return 0;
	  }
     }

     free(ph);
    
     /* Return success at loading the Elf64 kernel */
     return 1;
}

static int
is_elf32(loadinfo_t *loadinfo)
{
     Elf32_Ehdr *e = &(loadinfo->elf.elf32hdr);

     return (e->e_ident[EI_MAG0]  == ELFMAG0	    &&
	     e->e_ident[EI_MAG1]  == ELFMAG1	    &&
	     e->e_ident[EI_MAG2]  == ELFMAG2	    &&
	     e->e_ident[EI_MAG3]  == ELFMAG3	    &&
	     e->e_ident[EI_CLASS] == ELFCLASS32  &&
	     e->e_ident[EI_DATA]  == ELFDATA2MSB &&
	     e->e_type            == ET_EXEC	    &&
	     e->e_machine         == EM_PPC);
}

static int
is_elf64(loadinfo_t *loadinfo)
{
     Elf64_Ehdr *e = &(loadinfo->elf.elf64hdr);

     return (e->e_ident[EI_MAG0]  == ELFMAG0	    &&
	     e->e_ident[EI_MAG1]  == ELFMAG1	    &&
	     e->e_ident[EI_MAG2]  == ELFMAG2	    &&
	     e->e_ident[EI_MAG3]  == ELFMAG3	    &&
	     e->e_ident[EI_CLASS] == ELFCLASS64  &&
	     e->e_ident[EI_DATA]  == ELFDATA2MSB &&
	     e->e_type            == ET_EXEC	    &&
	     e->e_machine         == EM_PPC64);
}

static void
setup_display(void)
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
     int i, result;
     prom_handle scrn = PROM_INVALID_HANDLE;

     /* Try Apple's mac-boot screen ihandle */
     result = (int)call_prom_return("interpret", 1, 2,
				    "\" _screen-ihandle\" $find if execute else 0 then", &scrn);
     DEBUG_F("Trying to get screen ihandle, result: %d, scrn: %p\n", result, scrn);

     if (scrn == 0 || scrn == PROM_INVALID_HANDLE) {
	  char type[32];
	  /* Hrm... check to see if stdout is a display */
	  scrn = call_prom ("instance-to-package", 1, 1, prom_stdout);
	  DEBUG_F("instance-to-package of stdout is: %p\n", scrn);
	  if (prom_getprop(scrn, "device_type", type, 32) > 0 && !strncmp(type, "display", 7)) {
	       DEBUG_F("got it ! stdout is a screen\n");
	       scrn = prom_stdout;
	  } else {
	       /* Else, we try to open the package */
	       scrn = (prom_handle)call_prom( "open", 1, 1, "screen" );
	       DEBUG_F("Open screen result: %p\n", scrn);
	  }
     }
  	
     if (scrn == PROM_INVALID_HANDLE) {
	  prom_printf("No screen device found !/n");
	  return;
     }
     for(i=0;i<16;i++) {
	  prom_set_color(scrn, i, default_colors[i*3],
			 default_colors[i*3+1], default_colors[i*3+2]);
     }
     prom_printf("\x1b[1;37m\x1b[2;40m");	
#ifdef COLOR_TEST
     for (i=0;i<16; i++) {
	  prom_printf("\x1b[%d;%dm\x1b[1;47m%s \x1b[2;40m %s\n",
		      ansi_color_table[i].index,
		      ansi_color_table[i].value,
		      ansi_color_table[i].name,
		      ansi_color_table[i].name);
	  prom_printf("\x1b[%d;%dm\x1b[1;37m%s \x1b[2;30m %s\n",
		      ansi_color_table[i].index,
		      ansi_color_table[i].value+10,
		      ansi_color_table[i].name,
		      ansi_color_table[i].name);
     }
     prom_printf("\x1b[1;37m\x1b[2;40m");	
#endif /* COLOR_TEST */

#ifndef DEBUG
     prom_printf("\xc\n");
#endif /* !DEBUG */

#endif /* CONFIG_SET_COLORMAP */
}

static int yaboot_main(void)
{
     int i;
     char *p;
     if (prom_getprop(call_prom("instance-to-package", 1, 1, prom_stdout), "iso6429-1983-colors", NULL, 0) >= 0) {
	  stdout_is_screen = 1;
	  setup_display();
     }
	
     if (bootdevice[0] == '\0') {
	     prom_get_chosen("bootpath", bootdevice, sizeof(bootdevice));
	     DEBUG_F("/chosen/bootpath = %s\n", bootdevice);
	     if (bootdevice[0] == 0) {
		     prom_printf("Couldn't determine boot device\n");
		     return -1;
	     }
     }

     if (!parse_device_path(bootdevice, &boot)) {
	  prom_printf("%s: Unable to parse\n", bootdevice);
	  return -1;
     }

     useconf = load_config_file(&boot);

#ifndef DEBUG
     if (stdout_is_screen)
	     for(i = 0; i < 10 ; i++)
		     prom_printf("\n");
#endif

     p = cfg_get_strg(NULL, "init-message");
     if (p)
	  prom_printf("%s\n", p);

     p = cfg_get_strg(NULL, "message");
     if (p)
	  print_message_file(p, &boot, &default_device);


     prom_printf("Welcome to yaboot version " VERSION "\n");
     prom_printf("booted from '%s'\n", bootdevice);
     prom_printf("Enter \"help\" to get some basic usage information\n");

     /* brain damage. censored. */

     yaboot_text_ui();
	
     prom_printf("Bye.\n");
     return 0;
}

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 5
 * End:
 */
