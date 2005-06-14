/* $Id$ */
#include <prom.h>
void of1275_exit(void)
{
	call_prom("exit", 0, 0);
}
