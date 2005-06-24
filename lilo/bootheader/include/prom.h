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

extern phandle chosen_handle;

extern phandle stdin;
extern phandle stdout;
extern phandle stderr;
extern ihandle mmu;

extern int call_prom(const char *service, int nargs, int nret, ...);
extern phandle of1275_child(phandle node);
extern void *of1275_claim(unsigned int virt, unsigned int size, unsigned int align);
extern void of1275_close(phandle node);
extern void of1275_enter(void);
extern void of1275_exit(void);
extern phandle of1275_finddevice(const char *name);
extern int of1275_getprop(phandle node, const char *name, void *buf, int buflen);
extern phandle of1275_instance_to_package(ihandle node);
extern int of1275_instance_to_path(ihandle node, void *buf, int buflen);
extern int of1275_interpret(const char *forth);
extern int of1275_map(unsigned int phys, unsigned int virt, unsigned int size);
extern ihandle of1275_open(const char *path);
extern int of1275_package_to_path(phandle node, void *buf, int buflen);
extern phandle of1275_parent(phandle node);
extern phandle of1275_peer(phandle node);
extern void of1275_prominit(prom_entry entry);
extern int of1275_read(phandle node, void *buf, int buflen);
extern void of1275_release(unsigned int virt, unsigned int size);
extern int of1275_seek(ihandle node, unsigned int a, unsigned int b);
extern int of1275_setprop(phandle node, const char *name, void *buf, int buflen);
extern int of1275_write(phandle node, void *buf, int buflen);

extern void find_type_devices(phandle *, const char *type, int);
extern void show_block_devices(void);
#endif				/* _PPC_BOOT_PROM_H_ */
