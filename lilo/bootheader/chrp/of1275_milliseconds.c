/* $Id$ */
#include <prom.h>
int of1275_milliseconds(void)
{
	return call_prom("milliseconds", 0, 1);
}
