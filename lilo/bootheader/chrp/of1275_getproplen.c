/* $Id$ */
#include <prom.h>
int of1275_getproplen(phandle node, const char *name)
{
	return call_prom("getproplen", 2, 1, node, name);
}

