#ifndef _PPC_BOOT_BYTESWAB_H_
#define _PPC_BOOT_BYTESWAB_H_
/* $Id$ */

# define __inline__	__inline__	__attribute__((always_inline))
#define __attribute_const__	__attribute__((__const__))

#define ___swab16(x) \
({ \
	unsigned short __x = (x); \
	((unsigned short)( \
		(((unsigned short)(__x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(__x) & (unsigned short)0xff00U) >> 8) )); \
})

#define ___swab32(x) \
({ \
	unsigned int __x = (x); \
	((unsigned int)( \
		(((unsigned int)(__x) & (unsigned int)0x000000ffUL) << 24) | \
		(((unsigned int)(__x) & (unsigned int)0x0000ff00UL) <<  8) | \
		(((unsigned int)(__x) & (unsigned int)0x00ff0000UL) >>  8) | \
		(((unsigned int)(__x) & (unsigned int)0xff000000UL) >> 24) )); \
})

#  define __swab16(x) \
(__builtin_constant_p((unsigned short)(x)) ? \
 ___swab16((x)) : \
 __fswab16((x)))
#  define __swab32(x) \
(__builtin_constant_p((unsigned int)(x)) ? \
 ___swab32((x)) : \
 __fswab32((x)))


static __inline__ __attribute_const__ unsigned short ___arch__swab16(unsigned short value)
{
	unsigned short result;

	__asm__("rlwimi %0,%2,8,16,23" : "=&r" (result) : "0" (value >> 8), "r" (value));
	return result;
}

static __inline__ __attribute_const__ unsigned int ___arch__swab32(unsigned int value)
{
	unsigned int result;

	__asm__("rlwimi %0,%2,24,16,23" : "=&r" (result) : "0" (value>>24), "r" (value));
	__asm__("rlwimi %0,%2,8,8,15"   : "=&r" (result) : "0" (result),    "r" (value));
	__asm__("rlwimi %0,%2,24,0,7"   : "=&r" (result) : "0" (result),    "r" (value));

	return result;
}
#define __arch__swab32(x) ___arch__swab32(x)
#define __arch__swab16(x) ___arch__swab16(x)

static __inline__ __attribute_const__ unsigned short __fswab16(unsigned short x)
{
	return __arch__swab16(x);
}

static __inline__ __attribute_const__ unsigned int __fswab32(unsigned int x)
{
	return __arch__swab32(x);
}

#define __cpu_to_le32(x) ((unsigned int)__swab32((x)))
#define __le32_to_cpu(x) __swab32((unsigned int)(unsigned int)(x))
#define __cpu_to_le16(x) ((unsigned short)__swab16((x)))
#define __le16_to_cpu(x) __swab16((unsigned short)(unsigned short)(x))

#define cpu_to_le32 __cpu_to_le32
#define le32_to_cpu __le32_to_cpu
#define cpu_to_le16 __cpu_to_le16
#define le16_to_cpu __le16_to_cpu

#endif				/* _PPC_BOOT_BYTESWAB_H_ */
