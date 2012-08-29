/**
 * @file  nge_endian_internal.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/23 14:00:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_ENDIAN_INTERNAL_H
#define _NGE_ENDIAN_INTERNAL_H

#include "nge_common.h"

#define OP_LITTLEENDIAN '<'
#define OP_BIGENDIAN    '>'
#define OP_NATIVE       '='

inline static int doendian(int c)
{
	int x=1;
	int e=*(char*)&x;
	if (c==OP_LITTLEENDIAN) return !e;
	if (c==OP_BIGENDIAN) return e;
	if (c==OP_NATIVE) return 0;
	return 0;
}

inline static void doswap(int swap, void *p, size_t n)
{
	if (swap)
	{
		char *a=(char*)p;
		int i,j;
		for (i=0, j=n-1, n=n/2; n--; i++, j--)
		{
			char t=a[i]; a[i]=a[j]; a[j]=t;
		}
	}
}

#define DECL_ENDIAN() uint8_t __swap__ = 0
#define SET_ENDIAN(endian) __swap__ = doendian(endian)
#define DO_ENDIAN(type, ptr)						\
	doswap(__swap__, (void*)(ptr), sizeof(type))

#define PUSH_STRUCT(type, ptr, mem)				\
	memcpy(mem, (ptr), sizeof(type));			\
	mem += sizeof(type)
#define POP_STRUCT(type, ptr, mem)				\
	memcpy((ptr), mem, sizeof(type));			\
	mem += sizeof(type)

#define PUSH_UINT8(val, mem)					\
	mem[0] = (uint8_t)val;						\
	mem += 1
#define POP_UINT8(val, mem)						\
	val = mem[0];								\
	mem += 1

#define PUSH_UINT8S(ptr, mem, size)				\
	memcpy(&(mem[0]), ptr, size);				\
	mem += size
#define POP_UINT8S(val, mem, size)				\
	memcpy(ptr, &(mem[0]), size);				\
	mem += size

#define PUSH_FLOAT(val, mem)					\
	memcpy(&(mem[0]), &(val), sizeof(float));	\
	mem += sizeof(float)
#define POP_FLOAT(val, mem)						\
	memcpy(&(val), &(mem[0]), sizeof(float));	\
	mem += sizeof(float)

#define PUSH_FLOATS(ptr, mem, size)					\
	memcpy(&(mem[0]), ptr, size * sizeof(float));	\
	mem += size * sizeof(float)
#define POP_FLOATS(ptr, mem, size)					\
	memcpy(ptr, &(mem[0]), size * sizeof(float));	\
	mem += size * sizeof(float)

#define PUSH_TYPE(type, val, mem)				\
	memcpy(&(mem[0]), &(val), sizeof(type));	\
	DO_ENDIAN(type, &(mem[0]));					\
	mem += sizeof(type)
#define POP_TYPE(type, val, mem)				\
	memcpy(&(val), &(mem[0]), sizeof(type));	\
	DO_ENDIAN(type, &(var));					\
	mem += sizeof(type)

#define SKIP_SIZE(mem, size)					\
	mem += size

#endif /* _NGE_ENDIAN_INTERNAL_H */
