/*
 * prime - quickly determine if a small number is prime
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1994/06/04 03:26:15
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_PRIME_H)
#define INCLUDE_PRIME_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "qmath.h"
# include "have_const.h"
#else
# include <calc/math.h>
# include <calc/have_const.h>
#endif


#define MAX_MAP_PRIME ((FULL)65521)	  /* (2^16-15) largest prime in prmap */
#define MAX_MAP_VAL   ((FULL)65535)	   /* (2^16-1)	largest bit in pr_map */
#define MAX_SM_PRIME  ((FULL)0xfffffffb)   /* (2^32-5)	largest 32 bit prime */
#define MAX_SM_VAL    ((FULL)0xffffffff)   /* (2^32-1)	largest 32 bit value */

#define MAP_POPCNT    6541		   /* number of odd primes in pr_map */

#define NXT_MAP_PRIME ((FULL)65537)	   /* (2^16+1)	smallest prime > 2^16 */

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
 *	pr_map_bit(n) != 0  ==>	 n is prime
 *	pr_map_bit(n) == 0  ==>	 n is NOT prime
 */
#define pr_map_bit(n) (pr_map[(HALF)(n)>>4] & (1 << (((HALF)(n)>>1)&0x7)))

/*
 * Limits for piXb tables.  Do not test about this value using the
 * given table, even though the table has a higher sentinel value.
 */
#define MAX_PI10B ((1024*256)-1)	/* largest pi10b value to test */
#define MAX_PI18B ((FULL)(0xFFFFFFFF))	/* largest pi18b value to test */

/*
 * Prime related external arrays.
 */
EXTERN CONST unsigned short prime[];
EXTERN CONST unsigned char pr_map[];
EXTERN CONST unsigned short pi10b[];
EXTERN CONST unsigned short pi18b[];
EXTERN NUMBER _nxtprime_;		/* 2^32+15 - smallest prime > 2^32 */
EXTERN CONST ZVALUE _nxt_prime_;	/* 2^32+15 - smallest prime > 2^32 */
EXTERN CONST ZVALUE _jmpmod2_;		/* JMPMOD*2 as a ZVALUE */


#endif /* !INCLUDE_PRIME_H */
