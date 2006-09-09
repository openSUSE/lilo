#ifndef _PPC_BOOT_STDIO_H_
#define _PPC_BOOT_STDIO_H_
/* $Id$ */

#include <stdarg.h>

#ifdef __GNUC__
extern int printf(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
extern int sprintf(char *buf, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
extern int vsprintf(char *buf, const char *fmt, va_list args) __attribute__ ((format(printf, 2, 0)));
#else
extern int printf(const char *fmt, ...);
extern int sprintf(char *buf, const char *fmt, ...);
extern int vsprintf(char *buf, const char *fmt, va_list args);
#endif

extern int putc(int c);
extern int putchar(int c);
extern int getchar(int block);

extern int fputs(char *str, void *f);

extern int write(void *buf, int buflen);
extern int read(void *buf, int buflen);

#endif				/* _PPC_BOOT_STDIO_H_ */
