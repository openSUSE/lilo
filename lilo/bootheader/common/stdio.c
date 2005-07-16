/* $Id$ */

#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <prom.h>
#include <div64.h>

#if 0
int
read(void *buf, int buflen)
{
	if (promptr)
		return of1275_read(stdin, buf, buflen);
	return -1;
}

int
putc(int c)
{
	char ch = c;

	if (c == '\n')
		putc('\r');
	return write(&ch, 1) == 1? c: -1;
}

int
putchar(int c)
{
	return putc(c);
}

int
fputs(char *str, void *f)
{
	int n = strlen(str);

	return write(str, n) == n? 0: -1;
}

static int
readchar(void)
{
	char ch;

	for (;;) {
		switch (read(&ch, 1)) {
		case 1:
			return ch;
		case -1:
			printf("read(stdin) returned -1\r\n");
			return -1;
		}
	}
}

static char line[256];
static char *lineptr;
static int lineleft;

int
getchar(void)
{
	int c;

	if (lineleft == 0) {
		lineptr = line;
		for (;;) {
			c = readchar();
			if (c == -1 || c == 4)
				break;
			if (c == '\r' || c == '\n') {
				*lineptr++ = '\n';
				putchar('\n');
				break;
			}
			switch (c) {
			case 0177:
			case '\b':
				if (lineptr > line) {
					putchar('\b');
					putchar(' ');
					putchar('\b');
					--lineptr;
				}
				break;
			case 'U' & 0x1F:
				while (lineptr > line) {
					putchar('\b');
					putchar(' ');
					putchar('\b');
					--lineptr;
				}
				break;
			default:
				if (lineptr >= &line[sizeof(line) - 1])
					putchar('\a');
				else {
					putchar(c);
					*lineptr++ = c;
				}
			}
		}
		lineleft = lineptr - line;
		lineptr = line;
	}
	if (lineleft == 0)
		return -1;
	--lineleft;
	return *lineptr++;
}

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((*cp == 'x') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	}
	while (isxdigit(*cp) &&
	       (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}

#endif

/* String functions lifted from lib/vsprintf.c and lib/ctype.c */
unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,			/* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,		/* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,			/* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,			/* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,			/* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,			/* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,			/* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,			/* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,	/* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,			/* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,			/* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,			/* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,	/* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,			/* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,			/* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,			/* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */

size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

static int skip_atoi(const char **s)
{
	int i=0;

	while (isdigit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

static char * number(char * str, unsigned long long num, int base, int size, int precision, int type)
{
	char c,sign,tmp[66];
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if ((signed long long)num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else
		while (num != 0) 
			tmp[i++] = digits[do_div(num, base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long long num;
	int i, base;
	char * str;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
	                        /* 'z' support added 23/7/1999 S.H.    */
				/* 'z' changed to 'Z' --davidm 1/25/99 */

	
	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt =='Z') {
			qualifier = *fmt;
			++fmt;
			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				++fmt;
			}
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			continue;


		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else if (qualifier == 'Z') {
				size_t * ip = va_arg(args, size_t *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

		/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'L')
			num = va_arg(args, long long);
		else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str-buf;
}

int sprintf(char * buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	return i;
}

static char sprint_buf[1024];

int
printf(const char *fmt, ...)
{
	va_list args;
	int n, i;

	va_start(args, fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);
	for (i = 0; sprint_buf[i] && i < sizeof(sprint_buf); i++) {
		if ('\n' == sprint_buf[i]) {
			memmove(&sprint_buf[i] + 1, &sprint_buf[i], n - i + 1);
			n++;
			i++;
			sprint_buf[i] = '\r';
		}
	}
	write(sprint_buf, n);
	return n;
}

