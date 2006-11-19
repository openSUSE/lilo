/*
 *  cfg.c - Handling and parsing of yaboot.conf
 *
 *  Copyright (C) 1995 Werner Almesberger
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

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <prom.h>
#include <cmdline.h>

/* Imported functions */
extern int strcasecmp(const char *s1, const char *s2);

typedef enum {
	cft_strg, cft_flag, cft_end
} CONFIG_TYPE;

typedef struct {
	CONFIG_TYPE type;
	char *name;
	void *data;
} CONFIG;

#define MAX_TOKEN 200
#define MAX_VAR_NAME MAX_TOKEN
#define EOF -1

static const char *archimage;
static const char image_64bit[] = "image[64bit]";
static const char image_32bit[] = "image[32bit]";
static int reuse_prev_image;
CONFIG cf_options[] = {
	{cft_strg, "device", NULL},
	{cft_strg, "partition", NULL},
	{cft_strg, "default", NULL},
	{cft_strg, "timeout", NULL},
	{cft_strg, "password", NULL},
	{cft_flag, "restricted", NULL},
	{cft_strg, "message", NULL},
	{cft_strg, "root", NULL},
	{cft_strg, "ramdisk", NULL},
	{cft_flag, "read-only", NULL},
	{cft_flag, "read-write", NULL},
	{cft_strg, "append", NULL},
	{cft_strg, "initrd", NULL},
	{cft_flag, "initrd-prompt", NULL},
	{cft_strg, "initrd-size", NULL},
	{cft_strg, "init-code", NULL},
	{cft_strg, "init-message", NULL},
	{cft_strg, "fgcolor", NULL},
	{cft_strg, "bgcolor", NULL},
	{cft_end, NULL, NULL}
};

CONFIG cf_image[] = {
	{cft_strg, "image", NULL},
	{cft_strg, "label", NULL},
	{cft_strg, "alias", NULL},
	{cft_flag, "single-key", NULL},
	{cft_flag, "restricted", NULL},
	{cft_strg, "device", NULL},
	{cft_strg, "partition", NULL},
	{cft_strg, "root", NULL},
	{cft_strg, "ramdisk", NULL},
	{cft_flag, "read-only", NULL},
	{cft_flag, "read-write", NULL},
	{cft_strg, "append", NULL},
	{cft_strg, "literal", NULL},
	{cft_strg, "initrd", NULL},
	{cft_flag, "initrd-prompt", NULL},
	{cft_strg, "initrd-size", NULL},
	{cft_flag, "novideo", NULL},
	{cft_end, NULL, NULL}
};

static char flag_set;
static char *last_token, *last_item, *last_value;
static int line_num;
static int back;		/* can go back by one char */
static char *currp;
static char *endp;
static CONFIG *curr_table = cf_options;
static jmp_buf env;
static int tab_completion_len;
static char *tab_completion_buf;

static struct IMAGES {
	CONFIG table[sizeof(cf_image) / sizeof(cf_image[0])];
	struct IMAGES *next;
} *images;

static void cfg_error(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	prom_printf("Config file error: ");
	prom_vprintf(msg, ap);
	va_end(ap);
	prom_printf(" near line %d in config\n", line_num);
	longjmp(env, 1);
}

static void cfg_warn(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	prom_printf("Config file warning: ");
	prom_vprintf(msg, ap);
	va_end(ap);
	prom_printf(" near line %d in config\n", line_num);
}

static inline int getc()
{
	if (currp == endp)
		return EOF;
	return *currp++;
}

#define next_raw next
static int next(void)
{
	int ch;

	if (!back)
		return getc();
	ch = back;
	back = 0;
	return ch;
}

static void again(int ch)
{
	back = ch;
}

static char *cfg_get_token(void)
{
	char buf[MAX_TOKEN + 1];
	char *here;
	int ch, escaped;

	if (last_token) {
		here = last_token;
		last_token = NULL;
		return here;
	}
	while (1) {
		while (ch = next(), ch == ' ' || char_is_tab(ch) || char_is_newline(ch))
			if (char_is_newline(ch))
				line_num++;
		if (ch == EOF || ch == (int)NULL)
			return NULL;
		if (ch != '#')
			break;
		while (ch = next_raw(), !char_is_newline(ch))
			if (ch == EOF)
				return NULL;
		line_num++;
	}
	if (ch == '=')
		return strdup("=");
	if (ch == '"') {
		here = buf;
		while (here - buf < MAX_TOKEN) {
			if ((ch = next()) == EOF)
				cfg_error("EOF in quoted string");
			if (ch == '"') {
				*here = 0;
				return strdup(buf);
			}
			if (ch == '\\') {
				ch = next();
				switch (ch) {
				case '"':
				case '\\':
					break;
				case '\n':
				case '\r':
					while ((ch = next()), ch == ' ' || char_is_tab(ch)) ;
					if (!ch)
						continue;
					again(ch);
					ch = ' ';
					break;
				case 'n':
					ch = '\n';
					break;
				default:
					cfg_error("Bad use of \\ in quoted string");
				}
			} else if (char_is_newline(ch))
				cfg_error("newline is not allowed in quoted strings");
			*here++ = ch;
		}
		cfg_error("Quoted string is too long");
		return NULL;	/* not reached */
	}
	here = buf;
	escaped = 0;
	while (here - buf < MAX_TOKEN) {
		if (escaped) {
			if (ch == EOF)
				cfg_error("\\ precedes EOF");
			if (ch == '\n')
				line_num++;
			else
				*here++ = char_is_tab(ch) ? ' ' : ch;
			escaped = 0;
		} else {
			if (ch == ' ' || char_is_tab(ch) || char_is_newline(ch) || ch == '#' || ch == '=' || ch == EOF) {
				again(ch);
				*here = 0;
				return strdup(buf);
			}
			if (!(escaped = (ch == '\\')))
				*here++ = ch;
		}
		ch = next();
	}
	cfg_error("Token is too long");
	return NULL;		/* not reached */
}

static void cfg_return_token(char *token)
{
	last_token = token;
}

static int cfg_next(char **item, char **value)
{
	char *this;

	if (last_item) {
		*item = last_item;
		*value = last_value;
		last_item = NULL;
		return 1;
	}
	*value = NULL;
	if (!(*item = cfg_get_token()))
		return 0;
	if (!strcmp(*item, "="))
		cfg_error("Syntax error");
	if (!(this = cfg_get_token()))
		return 1;
	if (strcmp(this, "=")) {
		cfg_return_token(this);
		return 1;
	}
	if (!(*value = cfg_get_token()))
		cfg_error("Value expected at EOF");
	if (!strcmp(*value, "="))
		cfg_error("Syntax error after %s", *item);
	return 1;
}

static int cfg_set(char *item, char *value)
{
	CONFIG *walk;

	if (!(strcasecmp(item, "image") && strcasecmp(item, image_32bit) && strcasecmp(item, image_64bit))) {
		struct IMAGES **p = &images;

		if (!archimage && (!strcasecmp(item, image_32bit) || !strcasecmp(item, image_64bit))) {
			reuse_prev_image = 1;
			return 0;
		}
		if (archimage && strcmp(item, "image") && strcmp(item, archimage)) {
			reuse_prev_image = 1;
			return 0;
		}
		reuse_prev_image = 0;
		while (*p)
			p = &((*p)->next);
		*p = (struct IMAGES *)malloc(sizeof(struct IMAGES));
		if (*p == NULL) {
			prom_printf("malloc error in cfg_set\n");
			return -1;
		}
		(*p)->next = 0;
		curr_table = ((*p)->table);
		memcpy(curr_table, cf_image, sizeof(cf_image));
		if (archimage)
			item = "image";
	} else if (reuse_prev_image)
		return 0;

	for (walk = curr_table; walk->type != cft_end; walk++) {
		if (walk->name && !strcasecmp(walk->name, item)) {
			if (value && walk->type != cft_strg)
				cfg_warn("'%s' doesn't have a value", walk->name);
			else if (!value && walk->type == cft_strg)
				cfg_warn("Value expected for '%s'", walk->name);
			else {
				if (walk->data)
					cfg_warn("Duplicate entry '%s'", walk->name);
				if (walk->type == cft_flag)
					walk->data = &flag_set;
				else if (walk->type == cft_strg) {
					walk->data = value;
					if (strcmp(item, "label") == 0 || strcmp(item, "alias") == 0) {
						int len = strlen(value);
						if (len > tab_completion_len)
							tab_completion_len = len;
					}
				}
			}
			break;
		}
	}
	if (walk->type != cft_end)
		return 1;
	return 0;
}

int cfg_parse(char *buff, int len, int cpu)
{
	char *item, *value;

	switch (cpu) {
	case 32:
		archimage = image_32bit;
		break;
	case 64:
		archimage = image_64bit;
		break;
	}
	currp = buff;
	endp = currp + len;

	if (setjmp(env))
		return 0;
	while (1) {
		if (!cfg_next(&item, &value))
			break;
		if (!cfg_set(item, value)) {
#ifdef DEBUG
			prom_printf("Can't set item %s to value %s\n", item, value);
#endif
		}
		free(item);
	}
	if (tab_completion_len)
		tab_completion_buf = malloc(tab_completion_len + 2);
	return !!tab_completion_buf;
}

static char *cfg_get_strg_i(CONFIG * table, char *item)
{
	CONFIG *walk;

	for (walk = table; walk->type != cft_end; walk++)
		if (walk->name && !strcasecmp(walk->name, item))
			return walk->data;
	return NULL;
}

char *cfg_get_strg(char *image, char *item)
{
	struct IMAGES *p;
	char *label, *alias;
	char *ret;

	if (!image)
		return cfg_get_strg_i(cf_options, item);
	for (p = images; p; p = p->next) {
		label = cfg_get_strg_i(p->table, "label");
		if (!label) {
			label = cfg_get_strg_i(p->table, "image");
			alias = strrchr(label, '/');
			if (alias)
				label = alias + 1;
		}
		alias = cfg_get_strg_i(p->table, "alias");
		if (!strcmp(label, image) || (alias && !strcmp(alias, image))) {
			ret = cfg_get_strg_i(p->table, item);
			if (!ret)
				ret = cfg_get_strg_i(cf_options, item);
			return ret;
		}
	}
	return NULL;
}

int cfg_get_flag(char *image, char *item)
{
	return !!cfg_get_strg(image, item);
}

static int printl_count;
static void printlabel(char *label, int defflag)
{
	int len = strlen(label);

	if (!printl_count) {
		printl_count++;
		prom_printf("\n");
	}
	prom_printf("%s %s", defflag ? "*" : " ", label);
	if (printl_count < 4)
		while (len++ < 25)
			prom_putchar(' ');
	printl_count++;
	if (printl_count == 4) {
		prom_printf("\n");
		printl_count = 1;
	}
}

int cfg_print_images(char *buf, int len, int remaining)
{
	struct IMAGES *p;
	char *label, *alias, *image, *last_match;
	char *def;
	int label_match_count, curlen, following_char, print_matching_labels, added;

	if (len > tab_completion_len)
		return 0;

	printl_count = print_matching_labels = 0;
	following_char = -1;
	curlen = len;
	if (len)
		memcpy(tab_completion_buf, buf, len);
	tab_completion_buf[len] = '\0';
	def = cfg_get_strg_i(cf_options, "default");

	do {
		label_match_count = 0;
		last_match = NULL;
		for (p = images; p; p = p->next) {
			label = cfg_get_strg_i(p->table, "label");
			alias = cfg_get_strg_i(p->table, "alias");
			image = NULL;
			if (!label && !alias) {
				if (len)
					continue;
				image = cfg_get_strg_i(p->table, "image");
				if (!image)
					continue;
				alias = strrchr(image, '/');
				if (alias) {
					image = alias + 1;
					alias = NULL;
				}
			}
			if (len) {
				if (label && strlen(label) < remaining && memcmp(label, tab_completion_buf, curlen) == 0) {
					if (print_matching_labels) {
						print_matching_labels++;
						printlabel(label, strcmp(def, label) == 0);
					} else {
						if (following_char >= 0) {
							if (label[curlen] == following_char) {
								label_match_count++;
								if (!last_match)
									last_match = label;
							} else {
								print_matching_labels = 1;
								label_match_count = 0;
								break;
							}
						} else {
							if (!last_match)
								last_match = label;
							label_match_count++;
						}
					}
				}
				if (alias && strlen(alias) < remaining && memcmp(alias, tab_completion_buf, curlen) == 0) {
					if (print_matching_labels) {
						print_matching_labels++;
						printlabel(alias, strcmp(def, alias) == 0);
					} else {
						if (following_char >= 0) {
							if (alias[curlen] == following_char) {
								label_match_count++;
								if (!last_match)
									last_match = alias;
							} else {
								print_matching_labels = 1;
								label_match_count = 0;
								break;
							}
						} else {
							if (!last_match)
								last_match = alias;
							label_match_count++;
						}
					}
				}
			} else {
				if (label)
					printlabel(label, strcmp(def, label) == 0);
				if (alias)
					printlabel(alias, strcmp(def, alias) == 0);
			}
		}
		if (label_match_count) {
			if (label_match_count == 1) {
				sprintf(tab_completion_buf, "%s ", last_match);
				memcpy(buf, tab_completion_buf, strlen(tab_completion_buf) + 1);
				prom_printf("%s", buf + curlen);
				label_match_count = 0;
			} else {
				if (following_char >= 0) {
					tab_completion_buf[curlen] = following_char;
					curlen++;
					tab_completion_buf[curlen] = '\0';
					prom_printf("%s", tab_completion_buf + curlen - 1);
				}
				following_char = last_match[curlen];
			}
		} else
			following_char = -1;
	}
	while (label_match_count || print_matching_labels == 1);

	added = strlen(tab_completion_buf) - len;
	if (printl_count == 2 || printl_count == 3 || (!len && !printl_count))
		prom_printf("\n");
	if (len)
		memcpy(buf, tab_completion_buf, strlen(tab_completion_buf) + 1);
	return added;
}

char *cfg_get_default(void)
{
	char *label;
	char *ret = cfg_get_strg_i(cf_options, "default");

	if (ret)
		return ret;
	if (!images)
		return NULL;
	ret = cfg_get_strg_i(images->table, "label");
	if (!ret) {
		ret = cfg_get_strg_i(images->table, "image");
		label = strrchr(ret, '/');
		if (label)
			ret = label + 1;
	}
	return ret;
}

/* 
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 8
 * End:
 */
