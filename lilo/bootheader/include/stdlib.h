#ifndef _PPC_BOOT_STDLIB_H
#define _PPC_BOOT_STDLIB_H
/* $Id$ */

extern void abort(const char *s);
extern void exit(void);

extern void mdelay(int ms);

extern int gunzip(unsigned long dest, int destlen, unsigned long src, int srclen, const char *what);

/* for global string constants */
extern unsigned long add_reloc_offset(unsigned long);
#define PTRRELOC(x)  ((typeof(x))add_reloc_offset((unsigned long)(x)))

#endif /* _PPC_BOOT_STDLIB_H */
