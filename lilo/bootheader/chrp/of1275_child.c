/* $Id$ */
#include <prom.h>
phandle of1275_child(phandle node)
{
	return call_prom("child", 1, 1, node);
}
