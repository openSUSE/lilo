#ifndef _PPC_BOOT_PROM_H_
#define _PPC_BOOT_PROM_H_
/* $Id$ */

extern int (*prom) (void *);
extern void *chosen_handle;

extern void *stdin;
extern void *stdout;
extern void *stderr;
extern void *bootcpu;
extern void *mmu;

extern int write(void *handle, void *ptr, int nb);
extern int read(void *handle, void *ptr, int nb);
extern void exit(void);
extern void pause(void);
extern void *finddevice(const char *);
extern void *instance_to_package(const void *ihandle);
extern void *claim(unsigned long virt, unsigned long size, unsigned long align);
extern int map(unsigned int phys, unsigned int virt, unsigned int size);
extern int getprop(void *phandle, const char *name, void *buf, int buflen);
extern int setprop(void *node, const char *name, void *buf, int buflen);
#endif				/* _PPC_BOOT_PROM_H_ */
