#ifndef _PPC_BOOT_PROM_H_
#define _PPC_BOOT_PROM_H_
/* $Id$ */

typedef unsigned int phandle;
typedef unsigned int ihandle;

extern int (*prom) (void *);
extern phandle chosen_handle;

extern phandle stdin;
extern phandle stdout;
extern phandle stderr;
extern phandle bootcpu;
extern ihandle mmu;

extern int write(ihandle handle, void *ptr, int nb);
extern int read(ihandle handle, void *ptr, int nb);
extern void exit(void);
extern void pause(void);
extern ihandle finddevice(const char *);
extern phandle instance_to_package(const ihandle node);
extern void *claim(unsigned long virt, unsigned long size, unsigned long align);
extern int map(unsigned int phys, unsigned int virt, unsigned int size);
extern int getprop(phandle node, const char *name, void *buf, int buflen);
extern int setprop(phandle node, const char *name, void *buf, int buflen);
#endif				/* _PPC_BOOT_PROM_H_ */
