/* $Id */
#include <prom.h>
int of1275_package_to_path(phandle node, void *buf, int buflen)
{
	return call_prom("package-to-path", 3, 1, node, buf, buflen);
}
