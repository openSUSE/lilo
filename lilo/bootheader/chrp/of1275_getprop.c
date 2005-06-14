/* $Id$ */
#include <prom.h>
int of1275_getprop(phandle node, const char *name, void *buf, int buflen)
{
	return call_prom("getprop", 4, 1, node, name, buf, buflen);
}
