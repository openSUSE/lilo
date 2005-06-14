/* $Id */
#include <prom.h>
void of1275_enter(void)
{
	call_prom("enter", 0, 0);
}
