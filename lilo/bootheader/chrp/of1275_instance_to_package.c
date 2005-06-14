/* $Id$ */
#include <prom.h>
phandle of1275_instance_to_package(ihandle node)
{
	return call_prom("instance-to-package", 1, 1, node);
}
