/*
 * a little tool to modify the cmdline on a zImage
 * Olaf Hering <olh@suse.de>  Copyright (C) 2003
 */

/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MY_VERSION 1

#define cmdline_start_string   "cmd_line_start"
#define cmdline_end_string     "cmd_line_end"
#if 0
expected memory layout:
#define CMD_LINE_SIZE 512
    struct _builtin_cmd_line {
	unsigned char prefer;
	unsigned char cmdling_start_flag[sizeof (cmdline_start_string) - 1];	/* without trailing zero */
	unsigned char string[CMD_LINE_SIZE];
	unsigned char cmdline_end_flag[sizeof (cmdline_end_string)];	/* with trailing zero */
};
#endif

static const char cmdstart[] = cmdline_start_string;
static const char cmdend[] = cmdline_end_string;

#define bootbuffer 4096
struct mybuffer {
	FILE *f;
	char b[bootbuffer + bootbuffer], b1[bootbuffer], b2[bootbuffer];
	char *bp[2];
	char *p;
	int ff;			/* flipflop */
	int cls_off, cle_off;
	char *prefer;
	char *string;
	size_t string_len;
	int opt_activate;
	int opt_clear;
	int opt_set;
	char *opt_filename;
	char *opt_new_string;
};

static void
my_version(void)
{
	printf("version: %d\n", MY_VERSION);
	printf("(C) SuSE Linux AG, Nuernberg, Germany, 2003\n");
	return;
}

static void
my_rtfm(const char *app)
{
	printf("modify the built-in cmdline of a CHRP boot image\n");
	printf("%s filename\n", app);
	printf("work with zImage named 'filename'\n");
	printf(" [-h] display this help\n");
	printf(" [-v] display version\n");
	printf(" [-a 0|1] disable/enable built-in cmdline\n");
	printf("          overrides whatever is passed from OpenFirmware\n");
	printf(" [-s STRING] store STRING in the boot image\n");
	printf(" [-c] clear previous content before update\n");
	printf(" no option will show the current settings in 'filename'\n");
	return;
}

static char *
my_search(char *buffer, const char *string, size_t buf_size)
{
	char *p;
	size_t i, l, li;
	p = buffer;
	l = strlen(string);
	for (i = 0; i < (buf_size - l); ++i) {
		for (li = 0; li < l; ++li) {
			if (p[li] != string[li])
				break;
			if (li == l - 1)
				return p;
		}
		p++;
	}

	return NULL;
}

static void
my_update(struct mybuffer *b)
{
	if (b->opt_clear)
		memset(b->string, 0, b->string_len);
	if (b->opt_set)
		snprintf(b->string, b->string_len, "%s", b->opt_new_string);
	rewind(b->f);
	fseek(b->f, b->cls_off, SEEK_SET);
	fwrite(b->b, b->string_len + sizeof (cmdstart), 1, b->f);
	fflush(b->f);

	return;
}

static int
my_main(struct mybuffer *b)
{
	int i;
	b->bp[0] = b->b1;
	b->bp[1] = b->b2;

	b->f =
	    fopen(b->opt_filename,
		  (b->opt_set || b->opt_activate) ? "r+" : "r");
	if (!b->f) {
		perror(b->opt_filename);
		goto out;
	}

	if (!fread(b->bp[b->ff], bootbuffer, 1, b->f)) {
		fprintf(stderr, "%s too short?\n", b->opt_filename);
		goto out;
	}
	memcpy(b->b + (b->ff ? bootbuffer : 0), b->bp[b->ff], bootbuffer);
	b->ff = !b->ff;
	while (!b->p && fread(b->bp[b->ff], bootbuffer, 1, b->f)) {
		if (b->ff) {
			memcpy(b->b, b->bp[0], bootbuffer);
			memcpy(b->b + bootbuffer, b->bp[1], bootbuffer);
		} else {
			memcpy(b->b, b->bp[1], bootbuffer);
			memcpy(b->b + bootbuffer, b->bp[0], bootbuffer);
		}
		b->p = my_search(b->b, cmdstart,
				 sizeof (b->b) - sizeof (cmdstart));
		b->ff = !b->ff;
	}
	if (!b->p)
		goto start_not_found;
	i = ftell(b->f) - sizeof (b->b) + (b->p - b->b);
	b->cls_off = --i;
	rewind(b->f);
	fseek(b->f, b->cls_off, SEEK_SET);
	fread(b->b, sizeof (b->b), 1, b->f);
	b->p = my_search(b->b, cmdend, sizeof (b->b) - sizeof (cmdend));
	if (!b->p)
		goto end_not_found;
	i = ftell(b->f) - sizeof (b->b) + (b->p - b->b);
	b->cle_off = --i;
	b->prefer = b->b;
	b->string = b->b + sizeof (cmdstart);
	b->string_len = b->cle_off - b->cls_off - sizeof (cmdstart) + 1;
	if (b->opt_activate) {
		if (b->opt_activate == 1)
			*b->prefer = '1';
		else
			*b->prefer = '0';

	} else {
		if (!*b->prefer || *b->prefer == '0')
			*b->prefer = '0';
		else
			*b->prefer = '1';
	}
	if ((b->opt_set || b->opt_activate))
		my_update(b);
	else {
		printf("cmd_line size:%d\n", b->string_len);
		printf("cmd_line: %s\n", b->string);
		printf("active: %c\n", *b->prefer);
	}
	return 0;

      start_not_found:
      end_not_found:
	fprintf(stderr, "%s not found, not a Linux CHRP zImage?\n",
		b->cls_off ? cmdend : cmdstart);
      out:
	return 1;
}

int
main(int argc, char **argv)
{
	struct mybuffer *b;

	if (argc < 2) {
		my_rtfm(argv[0]);
		exit(1);
	}
	b = malloc(sizeof (struct mybuffer));
	if (!b) {
		fprintf(stderr, "buf: no mem\n");
		exit(1);
	}
	memset(b, 0, sizeof (struct mybuffer));

	while (1) {
		int i;
		i = getopt(argc, argv, "a:hcvs:");
		if (i == -1)
			break;
		switch (i) {
		case 'a':
			if (*optarg == '0')
				b->opt_activate = -1;
			else
				b->opt_activate = 1;
			break;
		case 'c':
			b->opt_clear = 1;
			break;

		case 'h':
			my_rtfm(argv[0]);
			exit(0);
		case 's':
			b->opt_new_string = strdup(optarg);
			if (!b->opt_new_string) {
				fprintf(stderr, "set: no mem\n");
				exit(1);
			}
			b->opt_set = 1;
			break;
		case 'v':
			my_version();
			exit(0);
		default:
			printf("unknown option\n");
			my_rtfm(argv[0]);
			exit(1);
		}
	}
	b->opt_filename = strdup(argv[argc - 1]);
	if (!b->opt_filename) {
		fprintf(stderr, "no mem\n");
		exit(1);
	}

	return  my_main(b);
}
