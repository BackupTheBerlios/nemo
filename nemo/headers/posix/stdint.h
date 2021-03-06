#ifndef _STDINT_H_
#define _STDINT_H_
/* 
** Distributed under the terms of the OpenBeOS License.
*/

typedef signed char int8_t;
#define INT8_MIN 	(-128)
#define INT8_MAX 	(127)
typedef unsigned char uint8_t;
#define UINT8_MIN	(0U)
#define UINT8_MAX	(255U)

typedef signed short int16_t;
#define INT16_MIN 	(-32768)
#define INT16_MAX 	(32767)
typedef unsigned short uint16_t;
#define UINT16_MIN	(0U)
#define UINT16_MAX	(65535U)

typedef signed long int32_t;
#define INT32_MIN 	(-2147483648L)
#define INT32_MAX 	(2147483647L)
typedef unsigned long uint32_t;
#define UINT32_MIN	(0UL)
#define UINT32_MAX	(4294967295UL)

typedef signed long long int64_t;
#define INT64_MIN	(-9223372036854775808LL)
#define INT64_MAX	(9223372036854775807LL)
typedef unsigned long long uint64_t;
#define UINT64_MIN	(0ULL)
#define UINT64_MAX	(18446744073709551615ULL)

typedef signed long long intmax_t;
#define INTMAX_MIN	INT64_MIN
#define INTMAX_MAX	INT64_MAX
typedef unsigned long long uintmax_t;
#define UINTMAX_MIN	UINT64_MIN
#define UINTMAX_MAX	UINT64_MAX

// least types

typedef int8_t int_least8_t;
#define INT_LEAST8_MIN  	INT8_MIN
#define INT_LEAST8_MAX  	INT8_MAX
typedef uint8_t uint_least8_t;
#define UINT_LEAST8_MIN 	UINT8_MIN
#define UINT_LEAST8_MAX 	UINT8_MAX

typedef int16_t int_least16_t;
#define INT_LEAST16_MIN 	INT16_MIN
#define INT_LEAST16_MAX 	INT16_MAX
typedef uint16_t uint_least16_t;
#define UINT_LEAST16_MIN	UINT16_MIN
#define UINT_LEAST16_MAX	UINT16_MAX

typedef int32_t int_least32_t;
#define INT_LEAST32_MIN 	INT32_MIN
#define INT_LEAST32_MAX 	INT32_MAX
typedef uint32_t uint_least32_t;
#define UINT_LEAST32_MIN	UINT32_MIN
#define UINT_LEAST32_MAX	UINT32_MAX

typedef int64_t int_least64_t;
#define INT_LEAST64_MIN 	INT64_MIN
#define INT_LEAST64_MAX 	INT64_MAX
typedef uint64_t uint_least64_t;
#define UINT_LEAST64_MIN	UINT64_MIN
#define UINT_LEAST64_MAX	UINT64_MAX


// fast types assume a 32-bit processor

typedef int32_t int_fast8_t;
#define INT_FAST8_MIN  	INT8_MIN
#define INT_FAST8_MAX  	INT8_MAX
typedef uint32_t uint_fast8_t;
#define UINT_FAST8_MIN 	UINT8_MIN
#define UINT_FAST8_MAX 	UINT8_MAX

typedef int32_t int_fast16_t;
#define INT_FAST16_MIN 	INT16_MIN
#define INT_FAST16_MAX 	INT16_MAX
typedef uint32_t uint_fast16_t;
#define UINT_FAST16_MIN	UINT16_MIN
#define UINT_FAST16_MAX	UINT16_MAX

typedef int32_t int_fast32_t;
#define INT_FAST32_MIN 	INT32_MIN
#define INT_FAST32_MAX 	INT32_MAX
typedef uint32_t uint_fast32_t;
#define UINT_FAST32_MIN	UINT32_MIN
#define UINT_FAST32_MAX	UINT32_MAX

typedef int64_t int_fast64_t;
#define INT_FAST64_MIN 	INT64_MIN
#define INT_FAST64_MAX 	INT64_MAX
typedef uint64_t uint_fast64_t;
#define UINT_FAST64_MIN	UINT64_MIN
#define UINT_FAST64_MAX	UINT64_MAX

/* BSD compatibility */
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

#endif	/* _STDINT_H_ */
