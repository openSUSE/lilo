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
#include <stdlib.h>
#include <prom.h>
#include <cmdline.h>

extern void flush_cache(void *, unsigned long);
extern int identify_cpu(void);

/* Value picked to match that used by yaboot */
#define PROG_START	0x01400000
#define RAM_END		(128<<20)	// Fixme: use OF */

extern char _coff_start[];
extern char _start[];
extern char __bss_start[];
extern char _end[];
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

static unsigned char elfheader[256];

typedef void (*kernel_entry_t) (unsigned long, unsigned long, prom_entry, void *);

void mdelay(int ms)
{
	ms = of1275_milliseconds() + ms;
	while(of1275_milliseconds() < ms);
}

int read(void *buf, int buflen)
{
	return of1275_read(stdin, buf, buflen);
}

int write(void *buf, int buflen)
{
	return of1275_write(stdout, buf, buflen);
}

static int check_elf32(void *p)
{
	Elf32_Ehdr *elf32 = p;
	Elf32_Phdr *elf32ph;
	unsigned int i;

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

	for (i = 0; i < (unsigned int)elf32->e_phnum; i++, elf32ph++)
		if (elf32ph->p_type == PT_LOAD && elf32ph->p_offset != 0)
			break;

	vmlinux.memsize = (unsigned long)elf32ph->p_memsz;
	vmlinux.offset = (unsigned long)elf32ph->p_offset;

#ifdef DEBUG
	printf("PPC32 ELF file:\n");
	printf("e_ehsize    0x%08x\n", elf32->e_ehsize);
	printf("e_phentsize 0x%08x\n", elf32->e_phentsize);
	printf("e_phnum     0x%08x\n", elf32->e_phnum);
	printf("e_shentsize 0x%08x\n", elf32->e_shentsize);
	printf("e_shnum     0x%08x\n", elf32->e_shnum);
	printf("e_shstrndx  0x%08x\n", elf32->e_shstrndx);

	printf("p_type   0x%08x\n", elf32ph->p_type);
	printf("p_flags  0x%08x\n", elf32ph->p_flags);
	printf("p_offset 0x%08x\n", elf32ph->p_offset);
	printf("p_vaddr  0x%08x\n", elf32ph->p_vaddr);
	printf("p_paddr  0x%08x\n", elf32ph->p_paddr);
	printf("p_filesz 0x%08x\n", elf32ph->p_filesz);
	printf("p_memsz  0x%08x\n", elf32ph->p_memsz);
	printf("p_align  0x%08x\n", elf32ph->p_align);
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
	printf("PPC64 ELF file, ph %d\n", i);
	printf("p_type   0x%08x\n", elf64ph->p_type);
	printf("p_flags  0x%08x\n", elf64ph->p_flags);
	printf("p_offset 0x%016llx\n", elf64ph->p_offset);
	printf("p_vaddr  0x%016llx\n", elf64ph->p_vaddr);
	printf("p_paddr  0x%016llx\n", elf64ph->p_paddr);
	printf("p_filesz 0x%016llx\n", elf64ph->p_filesz);
	printf("p_memsz  0x%016llx\n", elf64ph->p_memsz);
	printf("p_align  0x%016llx\n", elf64ph->p_align);
	printf("... skipping 0x%08lx bytes of ELF header\n",
	       (unsigned long)elf64ph->p_offset);
#endif

	return 64;
}

static unsigned long claim_base /* = PROG_START */ ;

static unsigned long try_claim(unsigned long size)
{
	unsigned long addr = 0;

	for (; claim_base < RAM_END; claim_base += 0x100000) {
#ifdef DEBUG
		printf("    trying: 0x%08lx\n", claim_base);
#endif
		addr = (unsigned long)of1275_claim(claim_base, size, 0);
		if ((void *)addr != (void *)-1)
			break;
	}
	if (addr == 0)
		return 0;
	claim_base = PAGE_ALIGN(claim_base + size);
	return addr;
}

void start(unsigned long a1, unsigned long a2, void *promptr, void *sp)
{
	phandle bootcpu_phandle[1];
	kernel_entry_t kernel_entry;
	int cputype, elftype;

	/* Clear out the BSS as per ANSI C requirements */
	memset(__bss_start, 0, _end - __bss_start);

	of1275_prominit(promptr);

	printf("\nSuSE Linux zImage starting: loaded at 0x%p-0x%p (0x%lx/0x%lx/0x%p; sp: 0x%p)\n",
	       _coff_start, _end, a1, a2, promptr, sp);

	/* Maple firmware returns memory which is still in use for message passing
	 * Thats why claim_base is set to 32MB
	 * OldWorld Pmac can not claim above RAM end, even with MMU enabled
	 * on >601, a BAT max size is 256Mb and the kernel uses 2 of them,
	 * but on 601, a BAT max size is 8Mb. Try to claim memory above 5MB in this case
	 */
	if (claim_needs_map)
		claim_base = 5 * 1024 * 1024;
	else
		claim_base = 32 * 1024 * 1024;
	/* the executable memrange may not be claimed by firmware */
	of1275_claim((unsigned int)_coff_start, (unsigned int)(_end - _coff_start), 0);
	
	bootcpu_phandle[0] = 0;
	find_type_devices(bootcpu_phandle, "cpu", sizeof(bootcpu_phandle)/sizeof(phandle));
	if (!bootcpu_phandle[0])
		abort("must be a dream");

	if (of1275_getprop(bootcpu_phandle[0], "64-bit", NULL, 0) != -1)
		cputype = 64;
	else
		cputype = identify_cpu();

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
		abort("not a powerpc ELF file\n");

	if (cputype && cputype != elftype) {
		printf("booting a %d-bit kernel on a %d-bit cpu.\n",
		       elftype, cputype);
		abort("");
	}

	/*
	 * Additional memory for the device-tree copy, must be mapped
	 * the kernel doesnt claim and map this area itself
	 */
	vmlinux.memsize += 3 * 1024 * 1024;
	vmlinux.addr = try_claim(vmlinux.memsize);
	if (vmlinux.addr == 0)
		abort("Can't allocate memory for kernel image !\n");

	printf("Allocated 0x%08lx bytes for kernel @ 0x%08lx\n",
	       vmlinux.memsize, vmlinux.addr);

	/*
	 * Now we try to claim memory for the initrd (and copy it there)
	 */
	initrd.size = (unsigned long)(_initrd_end - _initrd_start);
	initrd.memsize = initrd.size;
	if (initrd.size > 0) {
		initrd.addr = try_claim(initrd.size);
		if (initrd.addr == 0)
			abort
			    ("Can't allocate memory for initial ramdisk !\n");
		printf("Allocated 0x%08lx bytes for initrd @ 0x%08lx\n",
		       initrd.size, initrd.addr);
		a1 = initrd.addr;
		a2 = initrd.size;
#ifdef DEBUG
		printf
		    ("initial ramdisk moving 0x%08lx <- 0x%08lx (0x%08lx bytes)\n",
		     initrd.addr, (unsigned long)_initrd_start, initrd.size);
		printf("initrd head: 0x%08lx\n",
		       *((unsigned long *)_initrd_start));
#endif
		memmove((void *)initrd.addr, (void *)_initrd_start,
			initrd.size);
	}

	/* Eventually gunzip the kernel */
	if (*(unsigned short *)vmlinuz.addr == 0x1f8b)
		gunzip(vmlinux.addr, vmlinux.memsize, vmlinuz.addr,
		       vmlinuz.size, "kernel");
	else
		memmove((void *)vmlinux.addr, (void *)vmlinuz.addr,
			vmlinuz.size);

	if (_builtin_cmd_line.prefer && _builtin_cmd_line.prefer != '0') {
		int l = get_cmdline(_builtin_cmd_line.string, strlen(_builtin_cmd_line.string), sizeof(_builtin_cmd_line.string));
#ifdef DEBUG
		printf("copy built-in cmdline(%d):\n%s\n", l,
		       _builtin_cmd_line.string);
#endif
		l = of1275_setprop(chosen_handle, "bootargs",
				 _builtin_cmd_line.string, l + 1);
#ifdef DEBUG
		printf("setprop bootargs: %d\n", l);
#endif
	}

	flush_cache((void *)vmlinux.addr, vmlinux.memsize);

	kernel_entry = (kernel_entry_t) (vmlinux.addr + vmlinux.offset);
	printf( "entering kernel at 0x%p(%lx/%lx/%p)\n",
			kernel_entry, a1, a2, promptr);
	kernel_entry(a1, a2, promptr, NULL);

	abort("Error: Linux kernel returned to zImage bootloader!\n");
}
