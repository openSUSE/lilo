#ifndef __PPC32_TYPES_H__
#define __PPC32_TYPES_H__

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef struct {
	__u32 u[4];
} __attribute((aligned(16))) __vector128;

#define BITS_PER_LONG 32

typedef __vector128 vector128;

typedef unsigned long	size_t;
typedef long		ssize_t;

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;
typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
typedef		__s64		int64_t;

#endif /* __PPC32_TYPES_H__ */
