/* $Id$ */
#include <prom.h>
int of1275_map(unsigned int phys, unsigned int virt, unsigned int size)
{
	return call_prom("call-method", 6, 1, "map", mmu, 0, size, virt, phys);
}
