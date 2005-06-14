/* $Id */
#include <prom.h>
phandle of1275_peer(phandle node)
{
	return call_prom("peer", 1, 1, node);
}
