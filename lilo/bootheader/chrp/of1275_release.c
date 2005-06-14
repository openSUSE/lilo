/* $Id */
#include <prom.h>
void of1275_release(unsigned int virt, unsigned int size)
{
	call_prom("release", 2, 1, virt, size);
}
