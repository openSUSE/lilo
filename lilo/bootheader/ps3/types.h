#ifndef __TYPES_H
#define __TYPES_H

#define min(x,y) ({ \
		        typeof(x) _x = (x);     \
		        typeof(y) _y = (y);     \
		        (void) (&_x == &_y);    \
		        _x < _y ? _x : _y; })

#define max(x,y) ({ \
		        typeof(x) _x = (x);     \
		        typeof(y) _y = (y);     \
		        (void) (&_x == &_y);    \
		        _x > _y ? _x : _y; })

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define BITS_PER_LONG 32


#endif
