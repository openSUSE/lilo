/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <cmdline.h>

#undef DEBUG
#define seconds 10

int get_cmdline(char *p, int l, int max)
{
	int c, escape, tmp, len = l;

	printf("edit kernel cmdline within %d seconds and press RETURN:\n%s", seconds, p);
	tmp = 10 * seconds;
	do {
		c = getchar(0);
		if (c)
			break;
		mdelay(100);
	} while (tmp--);

	if (c > 0) {
		max--;
		escape = 0;
		do {
			if (c) {
#ifdef DEBUG
				printf("\n.c %02x len %d.\n%s", c, len, p);
#endif
				if ('\e' == c)
					escape = 1;
				else if ('[' == c && escape == 1)
					escape = 2;
				else if (escape == 2)
					escape = 0;
				else if ('\n' == c || '\r' == c)
					break;
				else if ('\b' == c || '\177' == c) {
					if (len) {
						printf("\b \b");
						p[--len] = '\0';
					}
				}
				/* ^x or ^u */
				else if ('\030' == c || '\025' == c) {
					while (len) {
						printf("\b \b");
						p[--len] = '\0';
					}
				}
				/* ^w */
				else if ('\027' == c) {
					while (len) {
						printf("\b \b");
						tmp = ' ' != p[--len];
						p[len] = '\0';
						if (tmp && ' ' == p[len - 1])
							break;
					}
				} else if (c >= ' ' && len < max) {
					p[len] = c;
					putc(c);
					len++;
				} else
					printf("\b ");
			}
			c = getchar(1);
		} while (1);
	}
	p[len] = '\0';
	printf("\n");
#ifdef DEBUG
	print_keys();
#endif
	return len;
}
