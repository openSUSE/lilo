#ifndef _PPC_BOOT_STDLIB_H
#define _PPC_BOOT_STDLIB_H
/* $Id$ */

extern void abort(const char *s);
extern void exit(void);

extern void gunzip(unsigned long dest, int destlen, unsigned long src, int srclen, const char *what);

#endif /* _PPC_BOOT_STDLIB_H */
