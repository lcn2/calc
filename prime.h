/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo was here	/\../\
 */

#include "qmath.h"
#include "have_const.h"


#define MAX_MAP_PRIME ((FULL)65521)	   /* (2^16-15) larest prime in prmap */
#define MAX_MAP_VAL   ((FULL)65535)	   /* (2^16-1)  larest bit in pr_map */
#define MAX_SM_PRIME  ((FULL)0xfffffffb)   /* (2^32-5)  larest 32 bit prime */
#define MAX_SM_VAL    ((FULL)0xffffffff)   /* (2^32-1)  larest 32 bit value */

#define MAP_POPCNT    6541		   /* number of odd primes in pr_map */

#define NXT_MAP_PRIME ((FULL)65537)	   /* (2^16+1)  smallest prime > 2^16 */

#define PIX_32B	      ((FULL)203280221)	   /* pix(2^32-1) - max pix() value */

/*
 * product of primes that fit into a long
 */
#if BASEB == 32
#define MAX_PFACT_VAL 52	/* max x, for which pfact(x) is a long */
#define NXT_PFACT_VAL 14	/* next prime for higher pfact values */
#else
#define MAX_PFACT_VAL 28	/* max x, for which pfact(x) is a long */
#define NXT_PFACT_VAL 8		/* next prime for higher pfact values */
#endif

/*
 * If n is odd and 1 <= n <= MAX_MAP_VAL, then:
 *
 *	pr_map_bit(n) != 0  ==>  n is prime
 *	pr_map_bit(n) == 0  ==>  n is NOT prime
 */
#define pr_map_bit(n) (pr_map[(HALF)(n)>>4] & (1 << (((HALF)(n)>>1)&0x7)))

/*
 * Limits for piXb tables.  Do not test about this value using the
 * given table, even though the table has a higher sentinal value.
 */
#define MAX_PI10B ((1024*256)-1)	/* largest pi10b value to test */
#define MAX_PI18B ((FULL)(0xFFFFFFFF))	/* largest pi18b value to test */

/*
 * Prime related external arrays.
 */
extern CONST unsigned short prime[];
extern CONST unsigned char pr_map[];
extern CONST unsigned short pi10b[];
extern CONST unsigned short pi18b[];
extern NUMBER _nxtprime_;		/* 2^32+15 - smallest prime > 2^32 */
extern CONST ZVALUE _nxt_prime_;	/* 2^32+15 - smallest prime > 2^32 */
extern CONST ZVALUE _jmpmod2_;		/* JMPMOD*2 as a ZVALUE */
