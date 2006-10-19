/*
 *  prom.h - Routines for talking to the Open Firmware PROM
 *
 *  Copyright (C) 2001 Ethan Benson
 *
 *  Copyright (C) 1999 Benjamin Herrenschmidt
 *
 *  Copyright (C) 1999 Marius Vollmer
 *
 *  Copyright (C) 1996 Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef PROM_H
#define PROM_H

#include <stdarg.h>

typedef void *prom_handle;
typedef void *ihandle;
typedef void *phandle;

#define PROM_INVALID_HANDLE	((prom_handle)-1UL)

struct prom_args;
typedef int (*prom_entry) (struct prom_args *);

extern void prom_init(prom_entry pp);

extern prom_entry prom;

/* I/O */

extern prom_handle prom_stdin;
extern prom_handle prom_stdout;
extern int stdout_is_screen;
extern unsigned int of_built_on;
extern int pegasos_amgia_partition_offset;

prom_handle prom_open(const char *spec);
int prom_read(prom_handle file, void *buf, int len);
int prom_write(prom_handle file, void *buf, int len);
int prom_seek(prom_handle file, unsigned long long pos);
int prom_readblocks(prom_handle file, int blockNum, int blockCount, void *buffer);
void prom_close(prom_handle file);
int prom_getblksize(prom_handle file);
int prom_loadmethod(prom_handle device, void *addr);
extern void find_type_devices(prom_handle * nodes, const char *type, int max);

#define K_UP    0x141
#define K_DOWN  0x142
#define K_LEFT  0x144
#define K_RIGHT 0x143

int prom_getchar();
void prom_putchar(char);
int prom_nbgetchar();

#ifdef __GNUC__
void prom_vprintf(const char *fmt, va_list ap) __attribute__ ((format(printf, 1, 0)));
void prom_fprintf(prom_handle dev, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
void prom_printf(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
#else
void prom_vprintf(const char *fmt, va_list ap);
void prom_fprintf(prom_handle dev, const char *fmt, ...);
void prom_printf(const char *fmt, ...);
#endif

void prom_perror(int error, const char *filename);
void prom_readline(const char *prompt, char *line, int len);
void prom_set_color(prom_handle device, int color, int r, int g, int b);

/* memory */

void *prom_claim(void *virt, unsigned int size, unsigned int align);
void prom_release(void *virt, unsigned int size);
void prom_map(void *phys, void *virt, int size);

/* packages and device nodes */

prom_handle prom_finddevice(const char *name);
prom_handle prom_findpackage(const char *path);
int prom_getprop(prom_handle dev, const char *name, void *buf, int len);
int prom_setprop(prom_handle dev, const char *name, void *buf, int len);
enum device_type prom_get_devtype(const char *device);

/* misc */

void prom_exit();
void prom_abort(const char *fmt, ...);
void prom_sleep(int seconds);

int prom_interpret(const char *forth);

int prom_get_chosen(const char *name, void *mem, int len);
int prom_set_chosen(const char *name, void *mem, int len);

extern int prom_getms(void);
extern void prom_pause(void);

extern void *call_prom(const char *service, int nargs, int nret, ...);
extern void *call_prom_return(const char *service, int nargs, int nret, ...);

#endif
