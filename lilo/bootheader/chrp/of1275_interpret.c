/* $Id$ */
#include <prom.h>
int of1275_interpret(const char *forth)
{
	return call_prom("interpret", 1, 1, forth);
}
