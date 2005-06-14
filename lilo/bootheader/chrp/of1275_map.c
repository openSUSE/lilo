/* $Id */
#include <prom.h>
int of1275_map(unsigned int phys, unsigned int virt, unsigned int size)
{
	return call_prom("map", 3, 1, phys, virt, size);
}
