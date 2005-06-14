/* $Id */
#include <prom.h>
phandle of1275_finddevice(const char *name)
{
	return call_prom("finddevice", 1, 1, name);
}
