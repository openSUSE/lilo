/* Prompt handling
   
   Copyright (C) 1996 Maurizio Plaza
   		 1996 Jakub Jelinek
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "types.h"
#include "stdarg.h"
#include "prom.h"
#include "string.h"

/* Imported functions */
extern void prom_puts (prom_handle file, char *s);

#define CMD_LENG	512
char cmd_buffer[CMD_LENG];

void cmdinit()
{
    cmd_buffer[0] = 0;
}

void cmdedit(void (*tabfunc)(char* cmd_buffer), int c)
{
    int x;

    for (x = 0; x < CMD_LENG - 1 && cmd_buffer[x] != 0; x++)
	;
    prom_puts(prom_stdout, cmd_buffer);

    if (c == -1)
	c = prom_getchar();
    while (c != -1 && c != '\n' && c != '\r') {
	if (c == '\t' && tabfunc)
	    (*tabfunc)(cmd_buffer);
	if (c == '\b' || c == 0x7F) {
	    if (x > 0) {
		--x;
		prom_printf("\b \b");
	    }
	} else if (c >= ' ' && x < CMD_LENG - 1) {
	    cmd_buffer[x] = c;
	    prom_putchar(c);
	    ++x;
	}
	c = prom_getchar();
    }

    cmd_buffer[x] = 0;
    return;
}

void cmdfill(const char *d)
{
    strncpy(cmd_buffer, d, CMD_LENG);
    cmd_buffer[CMD_LENG - 1] = 0;
}
