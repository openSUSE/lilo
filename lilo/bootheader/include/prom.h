#ifndef _PPC_BOOT_PROM_H_
#define _PPC_BOOT_PROM_H_
/* $Id$ */

typedef unsigned int phandle;
typedef unsigned int ihandle;
typedef unsigned int prom_arg_t;

struct prom_args {
	const char *service;
	int nargs;
	int nret;
	prom_arg_t args[10];
};

typedef void (*prom_entry) (struct prom_args *);
extern prom_entry promptr;

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

int call_prom(const char *service, int nargs, int nret, ...);
phandle of1275_child(phandle node);
void *of1275_claim(unsigned int virt, unsigned int size, unsigned int align);
void of1275_close(phandle node);
void of1275_enter(void);
void of1275_exit(void);
phandle of1275_finddevice(const char *name);
int of1275_getprop(phandle node, const char *name, void *buf, int buflen);
phandle of1275_instance_to_package(ihandle node);
int of1275_instance_to_path(ihandle node, void *buf, int buflen);
int of1275_map(unsigned int phys, unsigned int virt, unsigned int size);
ihandle of1275_open(const char *path);
int of1275_package_to_path(phandle node, void *buf, int buflen);
phandle of1275_parent(phandle node);
phandle of1275_peer(phandle node);
void of1275_prominit(prom_entry entry);
int of1275_read(phandle node, void *buf, int buflen);
void of1275_release(unsigned int virt, unsigned int size);
int of1275_seek(ihandle node, unsigned int a, unsigned int b);
int of1275_setprop(phandle node, const char *name, void *buf, int buflen);
int of1275_write(phandle node, void *buf, int buflen);

#endif				/* _PPC_BOOT_PROM_H_ */
