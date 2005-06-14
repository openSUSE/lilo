/* $Id */
#include <prom.h>
void of1275_close(phandle node)
{
	call_prom("close", 1, 1, node);
}
