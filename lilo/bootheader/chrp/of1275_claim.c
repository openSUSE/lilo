/* $Id$ */
#include <prom.h>
void *of1275_claim(unsigned int virt, unsigned int size, unsigned int align)
{
	int ret;
	unsigned int result;

	if (align || !claim_needs_map)
		return (void *)call_prom("claim", 3, 1, virt, size, align);

	ret = call_prom_ret("call-method", 5, 2, &result, "claim", memory,
			    align, size, virt);
	if (ret != 0 || result == -1)
		return (void *)-1;
	ret = call_prom_ret("call-method", 5, 2, &result, "claim", mmu,
			    align, size, virt);
	/* 0x12 == coherent + read/write */
	ret = call_prom("call-method", 6, 1, "map", mmu,
			0x12, size, virt, virt);
	return (void *)virt;
}
