/*  $Id$ */
/*
 * Copyright (C) Paul Mackerras 1997.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <prom.h>

int (*prom)(void *);

phandle chosen_handle;
phandle stdin;
phandle stdout;
phandle stderr;
phandle bootcpu;
ihandle mmu;


int
write(ihandle node, void *ptr, int nb)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		ihandle node;
		void *addr;
		int len;
		int actual;
	} args;

	args.service = "write";
	args.nargs = 3;
	args.nret = 1;
	args.node = node;
	args.addr = ptr;
	args.len = nb;
	args.actual = -1;
	(*prom)(&args);
	return args.actual;
}

int
read(ihandle node, void *ptr, int nb)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		ihandle node;
		void *addr;
		int len;
		int actual;
	} args;

	args.service = "read";
	args.nargs = 3;
	args.nret = 1;
	args.node = node;
	args.addr = ptr;
	args.len = nb;
	args.actual = -1;
	(*prom)(&args);
	return args.actual;
}

void
exit(void)
{
	struct prom_args {
		char *service;
	} args;

	for (;;) {
		args.service = "exit";
		(*prom)(&args);
	}
}

void
pause(void)
{
	struct prom_args {
		char *service;
	} args;

	args.service = "enter";
	(*prom)(&args);
}

phandle
finddevice(const char *name)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		const char *devspec;
		phandle phandle;
	} args;

	args.service = "finddevice";
	args.nargs = 1;
	args.nret = 1;
	args.devspec = name;
	args.phandle = (phandle) -1;
	(*prom)(&args);
	return args.phandle;
}

phandle
instance_to_package(const ihandle node)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		ihandle node;
		phandle phandle;
	} args;

	args.service = "instance-to-package";
	args.nargs = 1;
	args.nret = 1;
	args.node = node;
	args.phandle = (phandle)-1;
	(*prom) (&args);
	return args.phandle;
}

void *
claim(unsigned long virt, unsigned long size, unsigned long align)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		unsigned int virt;
		unsigned int size;
		unsigned int align;
		void *ret;
	} args;

	args.service = "claim";
	args.nargs = 3;
	args.nret = 1;
	args.virt = virt;
	args.size = size;
	args.align = align;
	(*prom)(&args);
	return args.ret;
}

int
map(unsigned int phys, unsigned int virt, unsigned int size)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		char *method;
		ihandle mmu_ihandle;
		int misc;
		unsigned int size;
		unsigned int virt;
		unsigned int phys;
		int ret0;
	} args;

	args.service = "call-method";
	args.nargs = 6;
	args.nret = 1;
	args.method = "map";
	args.mmu_ihandle = mmu;
	args.misc = 0;
	args.phys = phys;
	args.virt = virt;
	args.size = size;
	(*prom) (&args);

	return (int)args.ret0;
}

int
getprop(phandle node, const char *name, void *buf, int buflen)
{
	struct prom_args {
		char *service;
		int nargs;
		int nret;
		phandle node;
		const char *name;
		void *buf;
		int buflen;
		int size;
	} args;

	args.service = "getprop";
	args.nargs = 4;
	args.nret = 1;
	args.node = node;
	args.name = name;
	args.buf = buf;
	args.buflen = buflen;
	args.size = -1;
	(*prom)(&args);
	return args.size;
}

int
setprop(phandle node, const char *name, void *buf, int buflen)
{
    struct prom_args {
	char *service;
	int nargs;
	int nret;
	phandle node;
	const char *name;
	void *buf;
	int buflen;
	int size;
    } args;

    args.service = "setprop";
    args.nargs = 4;
    args.nret = 1;
    args.node = node;
    args.name = name;
    args.buf = buf;
    args.buflen = buflen;
    args.size = -1;
    (*prom)(&args);
    return args.size;
}
