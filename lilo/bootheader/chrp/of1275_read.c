/* $Id$ */
#include <prom.h>
int of1275_read(phandle node, void *buf, int buflen)
{
	return call_prom("read", 3, 1, node, buf, buflen);
}
