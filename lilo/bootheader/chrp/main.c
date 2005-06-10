/*  $Id$ */
/*
 * Copyright (C) Paul Mackerras 1997.
 *
 * Updates for PPC64 by Todd Inglett, Dave Engebretsen & Peter Bergner.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <stdarg.h>
#include <stddef.h>
#include <elf.h>
#include <page.h>
#include <string.h>
#include <stdio.h>
#include <prom.h>
#include <zlib/zutil.h>

extern void flush_cache(void *, unsigned long);

#define MSR_IR		(1<<5)		/* Instruction Relocate */
#define MSR_DR		(1<<4)		/* Data Relocate */
#define mfmsr()         ({unsigned long rval; \
		asm volatile("mfmsr %0" : "=r" (rval)); rval;})


/* Value picked to match that used by yaboot */
#define PROG_START	0x01400000
#define RAM_END		(256<<20) // Fixme: use OF */

extern char _start[];
extern char _vmlinuz_start[];
extern char _vmlinuz_end[];
extern char _initrd_start[];
extern char _initrd_end[];

struct addr_range {
	unsigned long addr;
	unsigned long size;
	unsigned long memsize;
	unsigned long offset;
};
static struct addr_range vmlinux;
static struct addr_range vmlinuz;
static struct addr_range initrd;

static char scratch[128<<10];	/* 128kB of scratch space for gunzip */
static unsigned char elfheader[256];

typedef void (*kernel_entry_t)( unsigned long,
                                unsigned long,
                                void *,
				void *);


#undef DEBUG

#define cmdline_start_string   "cmd_line_start"
#define cmdline_end_string     "cmd_line_end"
struct _builtin_cmd_line {
	unsigned char prefer;
	unsigned char cmdling_start_flag[sizeof(cmdline_start_string)-1]; /* without trailing zero */
	unsigned char string[512]; /* COMMAND_LINE_SIZE */
	unsigned char cmdline_end_flag[sizeof(cmdline_end_string)]; /* with trailing zero */
} __attribute__ ((__packed__));

struct _builtin_cmd_line _builtin_cmd_line = {
	.prefer = '0',
	.cmdling_start_flag = cmdline_start_string,
	.string = "",
	.cmdline_end_flag = cmdline_end_string,
};

static int check_elf32(void *p)
{
	Elf32_Ehdr *elf32 = p;
	Elf32_Phdr *elf32ph;

	if (elf32->e_ident[EI_MAG0] != ELFMAG0 ||
	    elf32->e_ident[EI_MAG1] != ELFMAG1 ||
	    elf32->e_ident[EI_MAG2] != ELFMAG2 ||
	    elf32->e_ident[EI_MAG3] != ELFMAG3 ||
	    elf32->e_ident[EI_CLASS] != ELFCLASS32 ||
	    elf32->e_ident[EI_DATA] != ELFDATA2MSB ||
	    elf32->e_type != ET_EXEC || elf32->e_machine != EM_PPC)
		return 0;

	elf32ph = (Elf32_Phdr *) ((unsigned long)elf32 +
				  (unsigned long)elf32->e_phoff);

	vmlinux.memsize = (unsigned long)elf32ph->p_memsz;
	vmlinux.offset = (unsigned long)elf32ph->p_offset;

#ifdef DEBUG
	printf("PPC32 ELF file:\n\r");
	printf("e_ehsize    0x%08x\n\r", elf32->e_ehsize);
	printf("e_phentsize 0x%08x\n\r", elf32->e_phentsize);
	printf("e_phnum     0x%08lx\n\r", elf32->e_phnum);
	printf("e_shentsize 0x%08lx\n\r", elf32->e_shentsize);
	printf("e_shnum     0x%08lx\n\r", elf32->e_shnum);
	printf("e_shstrndx  0x%08lx\n\r", elf32->e_shstrndx);

	printf("p_type   0x%08x\n\r", elf32ph->p_type);
	printf("p_flags  0x%08x\n\r", elf32ph->p_flags);
	printf("p_offset 0x%08lx\n\r", elf32ph->p_offset);
	printf("p_vaddr  0x%08lx\n\r", elf32ph->p_vaddr);
	printf("p_paddr  0x%08lx\n\r", elf32ph->p_paddr);
	printf("p_filesz 0x%08lx\n\r", elf32ph->p_filesz);
	printf("p_memsz  0x%08lx\n\r", elf32ph->p_memsz);
	printf("p_align  0x%08lx\n\r", elf32ph->p_align);
#endif

	return 32;
}

static int check_elf64(void *p)
{
	Elf64_Ehdr *elf64 = p;
	Elf64_Phdr *elf64ph;
	int i;

	if (elf64->e_ident[EI_MAG0] != ELFMAG0 ||
	    elf64->e_ident[EI_MAG1] != ELFMAG1 ||
	    elf64->e_ident[EI_MAG2] != ELFMAG2 ||
	    elf64->e_ident[EI_MAG3] != ELFMAG3 ||
	    elf64->e_ident[EI_CLASS] != ELFCLASS64 ||
	    elf64->e_ident[EI_DATA] != ELFDATA2MSB ||
	    elf64->e_type != ET_EXEC || elf64->e_machine != EM_PPC64)
		return 0;

	elf64ph = (Elf64_Phdr *) ((unsigned long)elf64 +
				  (unsigned long)elf64->e_phoff);

	for (i = 0; i < (unsigned int)elf64->e_phnum; i++, elf64ph++)
		if (elf64ph->p_type == PT_LOAD && elf64ph->p_offset != 0)
			break;

	vmlinux.memsize = (unsigned long)elf64ph->p_memsz;
	vmlinux.offset = (unsigned long)elf64ph->p_offset;

#ifdef DEBUG
	printf("PPC64 ELF file, ph %d\n\r", i);
	printf("p_type   0x%08x\n\r", elf64ph->p_type);
	printf("p_flags  0x%08x\n\r", elf64ph->p_flags);
	printf("p_offset 0x%016llx\n\r", elf64ph->p_offset);
	printf("p_vaddr  0x%016llx\n\r", elf64ph->p_vaddr);
	printf("p_paddr  0x%016llx\n\r", elf64ph->p_paddr);
	printf("p_filesz 0x%016llx\n\r", elf64ph->p_filesz);
	printf("p_memsz  0x%016llx\n\r", elf64ph->p_memsz);
	printf("p_align  0x%016llx\n\r", elf64ph->p_align);
	printf("... skipping 0x%lx bytes of ELF header\n\r",
	       (unsigned long)elf64ph->p_offset);
#endif

	return 64;
}

static unsigned long claim_base = PROG_START;

static void try_map(unsigned long phys, unsigned long virt, unsigned int size)
{
	unsigned long msr = mfmsr();
	if (msr & (MSR_IR|MSR_DR))
		printf("map 0x%08lx@0x%p: %d\n\r", size, phys,
			map(phys, virt, size));
}

static unsigned long try_claim(unsigned long size)
{
	unsigned long addr = 0;

	for(; claim_base < RAM_END; claim_base += 0x100000) {
#ifdef DEBUG
		printf("    trying: 0x%08lx\n\r", claim_base);
#endif
		addr = (unsigned long)claim(claim_base, size, 0);
		if ((void *)addr != (void *)-1)
			break;
	}
	if (addr == 0)
		return 0;
	try_map(addr, addr, size);
	claim_base = PAGE_ALIGN(claim_base + size);
	return addr;
}

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0

static void do_gunzip(void *dst, int dstlen, unsigned char *src, int *lenp)
{
	z_stream s;
	int r, i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != Z_DEFLATED || (flags & RESERVED) != 0) {
		printf("bad gzipped data\n\r");
		exit();
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp) {
		printf("gunzip: ran out of data in header\n\r");
		exit();
	}

	if (zlib_inflate_workspacesize() > sizeof(scratch)) {
		printf("zlib needs more mem\n\r");
		exit();
	}
	memset(&s, 0, sizeof(s));
	s.workspace = scratch;
	r = zlib_inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("inflateInit2 returned %d\n\r", r);
		exit();
	}
	s.next_in = src + i;
	s.avail_in = *lenp - i;
	s.next_out = dst;
	s.avail_out = dstlen;
	r = zlib_inflate(&s, Z_FULL_FLUSH);
	if (r != Z_OK && r != Z_STREAM_END) {
		printf("inflate returned %d msg: %s\n\r", r, s.msg);
		exit();
	}
	*lenp = s.next_out - (unsigned char *) dst;
	zlib_inflateEnd(&s);
}

static void gunzip(unsigned long dest, int destlen,
		   unsigned long src, int srclen, const char *what)
{
	int len;
	printf("uncompressing %s (0x%08lx:0x%08lx <- 0x%08lx:0x%08lx)...",
	       what, dest, destlen, src, srclen);
	len = srclen;
	do_gunzip((void *)dest, destlen, (unsigned char *)src, &len);
	printf("done 0x%08lx bytes\n\r", len);
}

static void abort(const char *s)
{
	printf("%s\n\r", s);
	exit();
}

void start(unsigned long a1, unsigned long a2, void *promptr)
{
	void *bootcpu_phandle;
	kernel_entry_t kernel_entry;
	int cputype, elftype;

	prom = (int (*)(void *)) promptr;
	chosen_handle = finddevice("/chosen");
	if (chosen_handle == (void *) -1)
		exit();
	if (getprop(chosen_handle, "stdout", &stdout, sizeof(stdout)) != 4)
		exit();
	stderr = stdout;
	if (getprop(chosen_handle, "stdin", &stdin, sizeof(stdin)) != 4)
		abort("no stdin");

	printf("\n\rzImage starting: loaded at 0x%x (0x%lx/0x%lx/0x%p)\n\r", (unsigned)_start,a1,a2,promptr);

	if (getprop(chosen_handle, "mmu", &mmu, sizeof(mmu)) != 4)
		abort("no mmu");

	if (getprop(chosen_handle, "cpu", &bootcpu, sizeof(bootcpu)) == 4) {
		bootcpu_phandle = instance_to_package(bootcpu);
		if (bootcpu_phandle == (void *)-1)
			exit();

		if (getprop(bootcpu_phandle, "64-bit", NULL, 0) != -1)
			cputype = 64;
		else
			cputype = 32;
	} else
		cputype = 0;

	vmlinuz.addr = (unsigned long)_vmlinuz_start;
	vmlinuz.size = (unsigned long)(_vmlinuz_end - _vmlinuz_start);

	/* Eventually gunzip the ELF header of the kernel */
	if (*(unsigned short *)vmlinuz.addr == 0x1f8b)
		gunzip((unsigned long)elfheader, sizeof(elfheader),
				vmlinuz.addr, vmlinuz.size, "ELF header");
	else
		memcpy(elfheader, _vmlinuz_start, sizeof(elfheader));

	elftype = check_elf64(elfheader);
	if (!elftype)
		elftype = check_elf32(elfheader);
	if (!elftype)
		abort("not a powerpc ELF file\n\r");
	
	if (cputype && cputype != elftype) {
		printf("booting a %d-bit kernel on a %d-bit cpu.\n\r",
				elftype, cputype);
		abort("");
	}

	/*
	 * Additional memory for the device-tree copy, must be mapped
	 * the kernel doesnt claim and map this area itself
	 */
	vmlinux.memsize += 3 * 1024 * 1024;
	printf("Allocating 0x%lx bytes for kernel ...\n\r", vmlinux.memsize);
	vmlinux.addr = try_claim(vmlinux.memsize);
	if (vmlinux.addr == 0)
		abort("Can't allocate memory for kernel image !\n\r");


	/*
	 * Now we try to claim memory for the initrd (and copy it there)
	 */
	initrd.size = (unsigned long)(_initrd_end - _initrd_start);
	initrd.memsize = initrd.size;
	if ( initrd.size > 0 ) {
		printf("Allocating 0x%lx bytes for initrd ...\n\r", initrd.size);
		initrd.addr = try_claim(initrd.size);
		if (initrd.addr == 0)
			abort("Can't allocate memory for initial ramdisk !\n\r");
		a1 = initrd.addr;
		a2 = initrd.size;
		printf("initial ramdisk moving 0x%lx <- 0x%lx (0x%lx bytes)\n\r",
		       initrd.addr, (unsigned long)_initrd_start, initrd.size);
		memmove((void *)initrd.addr, (void *)_initrd_start, initrd.size);
		printf("initrd head: 0x%lx\n\r", *((unsigned long *)initrd.addr));
	}

	/* Eventually gunzip the kernel */
	if (*(unsigned short *)vmlinuz.addr == 0x1f8b)
		gunzip(vmlinux.addr, vmlinux.memsize, vmlinuz.addr, vmlinuz.size,
		       "kernel");
	else
		memmove((void *)vmlinux.addr, (void *)vmlinuz.addr,
			vmlinuz.size);

	if ( _builtin_cmd_line.prefer && _builtin_cmd_line.prefer != '0' ) {
		int l = strlen (_builtin_cmd_line.string)+1;
		printf("copy built-in cmdline(%d) %s\n\r",l,_builtin_cmd_line.string);
		l = (int)setprop( chosen_handle, "bootargs", _builtin_cmd_line.string, l);
		printf ("setprop bootargs: %d\n\r",l);
	}

	flush_cache((void *)vmlinux.addr, vmlinux.memsize);

	kernel_entry = (kernel_entry_t)(vmlinux.addr + vmlinux.offset);
#ifdef DEBUG
	printf( "kernel:\n\r"
		"        entry addr = 0x%lx\n\r"
		"        a1         = 0x%lx,\n\r"
		"        a2         = 0x%lx,\n\r"
		"        prom       = 0x%lx,\n\r"
		"        bi_recs    = 0x%lx,\n\r",
		(unsigned long)kernel_entry, a1, a2,
		(unsigned long)prom, NULL);
#endif

	kernel_entry( a1, a2, prom, NULL );

	abort("Error: Linux kernel returned to zImage bootloader!\n\r");
}

