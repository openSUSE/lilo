/* $Id */
#include <prom.h>
int of1275_write(phandle node, void *buf, int buflen)
{
	return call_prom("write", 3, 1, node, buf, buflen);
}
