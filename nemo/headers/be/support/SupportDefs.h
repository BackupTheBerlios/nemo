//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		SupportDefs.h
//	Author:			Erik Jaesler (erik@cgsoftware.com)
//	Description:	Common type definitions.
//------------------------------------------------------------------------------

#ifndef _SUPPORT_DEFS_H
#define _SUPPORT_DEFS_H

/* this !must! be located before the include of sys/types.h */
#ifndef _SYS_TYPES_H
typedef unsigned long			ulong;
typedef unsigned int			uint;
typedef unsigned short			ushort;
#endif  // _SYS_TYPES_H


// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <sys/types.h>
#include <Errors.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

// Shorthand type formats ------------------------------------------------------

typedef	signed char				int8;
typedef unsigned char			uint8;
typedef volatile signed char   	vint8;
typedef volatile unsigned char	vuint8;

typedef	short					int16;
typedef unsigned short			uint16;
typedef volatile short			vint16;
typedef volatile unsigned short	vuint16;

typedef	long					int32;
typedef unsigned long			uint32;
typedef volatile long			vint32;
typedef volatile unsigned long	vuint32;

typedef	long long					int64;
typedef unsigned long long			uint64;
typedef volatile long long			vint64;
typedef volatile unsigned long long	vuint64;

typedef volatile long			vlong;
typedef volatile int			vint;
typedef volatile short			vshort;
typedef volatile char			vchar;

typedef volatile unsigned long	vulong;
typedef volatile unsigned int	vuint;
typedef volatile unsigned short	vushort;
typedef volatile unsigned char	vuchar;

typedef unsigned char			uchar;
typedef unsigned short          unichar;


// Descriptive formats ---------------------------------------------------------
typedef int32					status_t;
typedef int64					bigtime_t;
typedef uint32					type_code;
typedef uint32					perform_code;


// Empty string ("") -----------------------------------------------------------
#ifdef __cplusplus
extern _IMPEXP_BE const char *B_EMPTY_STRING;
#endif


// min and max comparisons -----------------------------------------------------
//	min() and max() won't work in C++
#define min_c(a,b) ((a)>(b)?(b):(a))
#define max_c(a,b) ((a)>(b)?(a):(b))

#ifndef __cplusplus
#ifndef min
#define min(a,b) ((a)>(b)?(b):(a))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#endif 


// Grandfathering --------------------------------------------------------------

#ifndef __cplusplus
typedef unsigned char			bool;
#define false	0
#define true	1
#endif 

#ifndef NULL
#define NULL 	(0)
#endif


//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//----- Atomic functions; old value is returned --------------------------------
extern _IMPEXP_ROOT int32	atomic_set(vint32 *value, int32 newValue);
extern _IMPEXP_ROOT int32	atomic_test_and_set(vint32 *value, int32 newValue, int32 testAgainst);
// TODO: re-enable (or delete) this function
//extern _IMPEXP_ROOT int32	atomic_add(vint32 *value, int32 addValue);
extern _IMPEXP_ROOT int32	atomic_and(vint32 *value, int32 andValue);
extern _IMPEXP_ROOT int32	atomic_or(vint32 *value, int32 orValue);	
extern _IMPEXP_ROOT int32	atomic_get(vint32 *value);

extern _IMPEXP_ROOT int64	atomic_set64(vint64 *value, int64 newValue);
extern _IMPEXP_ROOT int64	atomic_test_and_set64(vint64 *value, int64 newValue, int64 testAgainst);
extern _IMPEXP_ROOT int64	atomic_add64(vint64 *value, int64 addValue);
extern _IMPEXP_ROOT int64	atomic_and64(vint64 *value, int64 andValue);
extern _IMPEXP_ROOT int64	atomic_or64(vint64 *value, int64 orValue);	
extern _IMPEXP_ROOT int64	atomic_get64(vint64 *value);

// TODO: remove this block & use linux counterparts
// BEGIN: atomic functions hack
#if defined(__i386__)
struct __atomic_fool_gcc_struct { int a[100]; };
#define __atomic_fool_gcc(x) (*(volatile struct __atomic_fool_gcc_struct *)x)
#endif

static inline int32 atomic_add(vint32 *value, int32 addvalue)
{
    register int32 nOldVal;

#if defined(__i386__)

    register int nTmp = 0;
    __asm__ volatile(
			"  movl %3,%%eax;"	/* Start with the old-value in EAX */
			"0:;"
			"  movl %%eax,%%edx;"	/* Put the old-value where cmpxchgl wants it */
			"  addl %4,%%edx;"
			"  lock; cmpxchgl %%edx,%3;" /* if (*value==eax) {*value=edx;} else {eax=*value;goto 0;} */
			"  jnz 0b;"
			: "=&a" (nOldVal), "=m"(__atomic_fool_gcc(value)), "=&d"(nTmp)
			: "m" (__atomic_fool_gcc(value)), "r" (addvalue)
			:  "cc" );

#elif defined(__ppc__)

	__asm__ volatile(
			"0:	lwarx	%0,0,%2\n\
				add		%0,%1,%0\n\
				stwcx.	%0,0,%2\n\
				bne-	0b\n\
				isync"
				: "=&r" (nOldVal)
				: "r" (addvalue), "r" (&value)
				: "cc", "memory");

#endif
	return nOldVal;
}
// END: atomic functions hack
// Other stuff -----------------------------------------------------------------
extern _IMPEXP_ROOT void *	get_stack_frame(void);

#ifdef __cplusplus
}
#endif

//-------- Obsolete or discouraged API -----------------------------------------

// use 'true' and 'false'
#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif


//------------------------------------------------------------------------------

#endif	// _SUPPORT_DEFS_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

