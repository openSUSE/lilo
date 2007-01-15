/*
 *  cmdline.c - Prompt handling
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
 *
 *  Adapted from SILO
 *
 *  Copyright (C) 1996 Maurizio Plaza
 *                1996 Jakub Jelinek
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

#include <types.h>
#include <stdarg.h>
#include <prom.h>
#include <string.h>
#include <cfg.h>
#include <cmdline.h>
#include <stdlib.h>

#define CMD_LENG	512
static char *cmdbuff;
static char *passwdbuff;
extern int useconf;

char *passwdinit(void)
{
	if (!passwdbuff)
		passwdbuff = malloc(CMD_LENG);
	if (passwdbuff)
		passwdbuff[0] = 0;
	return passwdbuff;
}

char *cmdlineinit(void)
{
	if (!cmdbuff)
		cmdbuff = malloc(CMD_LENG);
	if (cmdbuff)
		cmdbuff[0] = 0;
	return cmdbuff;
}

static int tabfunc(char *buf, const int len, void (*func) (const char *p))
{
	int label_len, print_label, c;
	int ret_len, pb;
	char *p;
	ret_len = 0;
	pb = 1;
	if (len > 0) {
		print_label = 0;
		p = buf;
		for (c = 0; c < len; c++) {
			if (buf[c] == ' ') {
				if (print_label) {
					print_label = 0;
					break;
				}
				p++;
				continue;
			}
			print_label = 1;
		}
		if (print_label) {
			label_len = len - (int)(p - buf);
			ret_len = cfg_print_images(p, label_len, CMD_LENG - len - 1);
			if (ret_len == 0)
				prom_printf("\n");
			if (ret_len && buf[len + ret_len - 1] == ' ')
				pb = 0;
		} else
			prom_printf("\n");
	} else
		cfg_print_images(NULL, 0, 0);
	if (pb)
		(*func) (buf);
	return ret_len + len;
}

static char *buffer_edit(char *buf, void (*func) (const char *p))
{
	int len, c;
	len = strlen(buf);
	if (func)
		prom_printf(buf);
	else {
		for (c = 0; c < len; c++)
			prom_printf("*");
	}

	for (;;) {
		c = prom_getchar();
		if (c == -1)
			break;
		if (char_is_newline(c))
			break;
		if (char_is_tab(c) && func)
			len = tabfunc(buf, len, func);
		else if (char_is_backspace(c)) {
			if (len > 0) {
				--len;
				buf[len] = 0;
				prom_printf("\b \b");
			}
		} else {
			c = char_to_ascii(c);
			if (c && len < CMD_LENG - 2) {
				buf[len] = c;
				buf[len + 1] = 0;
				if (func)
					prom_printf(buf + len);
				else
					prom_printf("*");
				len++;
			}
		}
	}

	buf[len] = 0;
	return buf;
}

char *passwordedit(char *buf)
{
	return buffer_edit(buf, NULL);
}

char *cmdlineedit(char *buf, void (*func) (const char *p))
{
	return buffer_edit(buf, func);
}
