/* $Id$ */
#include <stdlib.h>
#include <stdio.h>

void abort(const char *s)
{
	printf("%s\n\r", s);
	exit();
}
