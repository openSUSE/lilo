#ifndef __DIV64_H__
#define __DIV64_H__
/*  $Id$ */
/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * The semantics of do_div() are:
 *
 * unsigned int do_div(uint64_t *n, unsigned int base)
 * {
 * 	unsigned int remainder = *n % base;
 * 	*n = *n / base;
 * 	return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
 */

extern unsigned int __div64_32(unsigned long long *dividend, unsigned int divisor);

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
# define do_div(n,base) ({				\
	unsigned int __base = (base);			\
	unsigned int __rem;					\
	(void)(((typeof((n)) *)0) == ((unsigned long long *)0));	\
	if (((n) >> 32) == 0) {			\
		__rem = (unsigned int)(n) % __base;		\
		(n) = (unsigned int)(n) / __base;		\
	} else 						\
		__rem = __div64_32(&(n), __base);	\
	__rem;						\
 })

#endif /* __DIV64_H__ */
