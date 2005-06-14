/* $Id */
#include <prom.h>
void *of1275_claim(unsigned int virt, unsigned int size, unsigned int align)
{
	return (void *)call_prom("claim", 3, 1, virt, size, align);
}
