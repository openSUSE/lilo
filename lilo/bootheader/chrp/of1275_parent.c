/* $Id */
#include <prom.h>
phandle of1275_parent(phandle node)
{
	return call_prom("parent", 1, 1, node);
}
