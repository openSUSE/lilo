/* $Id */
#include <prom.h>
int of1275_setprop(phandle node, const char *name, void *buf, int buflen)
{
	return call_prom("setprop", 4, 1, node, name, buf, buflen);
}
