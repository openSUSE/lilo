/* $Id$ */
#include <prom.h>
int of1275_seek(ihandle node, unsigned int a, unsigned int b)
{
	return call_prom("seek", 3, 1, node, a, b);
}
