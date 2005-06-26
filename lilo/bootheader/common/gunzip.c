/* $Id$ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib/zlib.h>

static char scratch[46912];	/* scratch space for gunzip, from zlib_inflate_workspacesize() */

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0

static void do_gunzip(void *dst, int dstlen, unsigned char *src, int *lenp)
{
	z_stream s;
	int r, i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != Z_DEFLATED || (flags & RESERVED) != 0)
		abort("bad gzipped data");
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp)
		abort("gunzip: ran out of data in header");

	if (zlib_inflate_workspacesize() > sizeof(scratch)) 
		abort("zlib needs more mem");
	memset(&s, 0, sizeof(s));
	s.workspace = scratch;
	r = zlib_inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("inflateInit2 returned %d\n\r", r);
		exit();
	}
	s.next_in = src + i;
	s.avail_in = *lenp - i;
	s.next_out = dst;
	s.avail_out = dstlen;
	r = zlib_inflate(&s, Z_FULL_FLUSH);
	if (r != Z_OK && r != Z_STREAM_END) {
		printf("inflate returned %d msg: %s\n\r", r, s.msg);
		exit();
	}
	*lenp = s.next_out - (unsigned char *) dst;
	zlib_inflateEnd(&s);
}

void gunzip(unsigned long dest, int destlen,
		   unsigned long src, int srclen, const char *what)
{
	int len;
	printf("uncompressing %s ", what);
#ifdef DEBUG
	printf("(0x%08lx:0x%08lx <- 0x%08lx:0x%08lx)...",
	       dest, destlen, src, srclen);
#endif
	len = srclen;
	do_gunzip((void *)dest, destlen, (unsigned char *)src, &len);
	printf("done. (0x%08lx bytes)\n\r", len);
}

