/* $Id$ */
#include <stdlib.h>
#include <prom.h>

void exit(void)
{
	if (promptr)
		of1275_exit();
	while (1) ;
}
