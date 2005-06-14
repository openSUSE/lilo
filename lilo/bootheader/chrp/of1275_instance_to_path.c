/* $Id$ */
#include <prom.h>
int of1275_instance_to_path(ihandle node, void *buf, int buflen)
{
	return call_prom("instance-to-path", 3, 1, node, buf, buflen);
}
