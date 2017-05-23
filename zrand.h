/*
 * zrand - subtractive 100 shuffle generator
 *
 * Copyright (C) 1999-2007,2014  Landon Curt Noll
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
 * Under source code control:	1995/01/07 09:45:26
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * random number generator - see zrand.c for details
 */


#if !defined(INCLUDE_ZRAND_H)
#define INCLUDE_ZRAND_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "value.h"
# include "have_const.h"
#else
# include <calc/value.h>
# include <calc/have_const.h>
#endif


/*
 * s100 generator defines
 *
 * NOTE: SBITS must be a power of two to make the (&= (SBITS-1))
 *	 in slotcp() to work.
 */
#define SBITS (64)	 /* size of subtractive or shuffle entry in bits */
#define SBYTES (SBITS/8) /* size of subtractive or shuffle entry in bytes */
#define SHALFS (SBYTES/sizeof(HALF))	/* size in HALFs */

/*
 * seed defines
 */
#define SEEDXORBITS 64		/* low bits of s100 seed devoted to xor */

/*
 * shuffle table defines
 */
#define SHUFPOW 8		/* power of 2 size of the shuffle table */
#define SHUFCNT (1 << SHUFPOW)	/* size of shuffle table */
#define SHUFLEN (SLEN*SHUFCNT)	/* length of shuffle table in FULLs */
#define SHUFMASK (SHUFLEN-1)	/* mask for shuffle table entry selection */

/*
 * subtractive 100 constants
 */
#define S100 100	/* slots in an subtractive 100 table */
#define INIT_J 36	/* initial first walking table index */
#define INIT_K 99	/* initial second walking table index */

/*
 * subtractive 100 table defines
 *
 * SLEN		- length in FULLs of an subtractive 100 slot
 *
 * SLOAD(s,i,z) - load table slot i from subtractive 100 state s with zvalue z
 *		  s: type RAND
 *		  i: type int, s.slot[i] slot index
 *		  z: type ZVALUE, what to load into s.slot[i]
 *
 * SSUB(s,k,j)	- slot[k] -= slot[j]
 *		  s: type RAND
 *		  k: type int, s.slot[k] slot index, what to gets changed
 *		  j: type int, s.slot[j] slot index, what to add to s.slot[k]
 *		  (may use local variable tmp)
 *
 * SINDX(s,k)	- select the shuffle table entry from slot[k] (uses top bits)
 *		  s: type RAND
 *		  k: type int, s.slot[k] slot index, selects shuffle entry
 *		  result type int, refers to s.shuf[SINDX(s,k)]
 *
 * SBUFFER(s,t) - load s100 buffer with t
 *		  s: type RAND
 *		  t: type int, s.shuf[t] entry index, replace buffer with it
 *
 * SSHUF(s,t,k) - save slot[k] into shuffle entry t
 *		  s: type RAND
 *		  t: type int, s.shuf[t] entry index, what gets changed
 *		  k: type int, s.slot[k] slot index, load into	s.shuf[t]
 *
 * SSWAP(s,j,k) - swap slot[j] with slot[k]
 *		  s: type RAND
 *		  j: type int, s.slot[j] slot index, goes into s.slot[k]
 *		  k: type int, s.slot[k] slot index, goes into s.slot[j]
 *		  (uses local variable tmp)
 *
 * SMOD64(t,z)	- t = seed z mod 2^64
 *		  t: type FULL*, array of FULLs that get z mod 2^64
 *		  z: type ZVALUE, what gets (mod 2^64) placed into t
 *
 * SOXR(s,i,v)	- xor slot[i] with lower 64 bits of slot value v
 *		  s: type RAND
 *		  i: type int, s.slot[i] slot index, what gets xored
 *		  v: type FULL*, 64 bit value to xor into s.slot[i]
 *
 * SCNT		- length of an subtractive 100 table in FULLs
 */
#if FULL_BITS == SBITS

# define SLEN 1		/* a 64 bit slot can be held in a FULL */
#define SLOAD(s,i,z) ((s).slot[i] = ztofull(z))
#define SSUB(s,k,j) ((s).slot[k] -= (s).slot[j])
#define SINDX(s,k) ((int)((s).slot[k] >> (FULL_BITS - SHUFPOW)))
#define SBUFFER(s,t) {(s).buffer[0] = ((s).shuf[t] & BASE1); \
		      (s).buffer[1] = ((s).shuf[t] >> BASEB); \
		     }
#define SSHUF(s,t,k) ((s).shuf[t] = (s).slot[k])
#define SSWAP(s,j,k) {FULL tmp = (s).slot[j]; \
		      (s).slot[j] = (s).slot[k]; \
		      (s).slot[k] = tmp; \
		     }
#define SMOD64(t,z) ((t)[0] = ztofull(z))
#define SXOR(s,i,v) ((s).slot[i] ^= (v)[0])

#elif 2*FULL_BITS == SBITS

# define SLEN 2			/* a 64 bit slot needs 2 FULLs */
#define SLOAD(s,i,z) {(s).slot[(i)<<1] = ztofull(z); \
		      (s).slot[1+((i)<<1)] = \
			(((z).len <= 2) ? (FULL)0 : \
			 (((z).len == 3) ? (FULL)((z).v[2]) : \
			  ((FULL)((z).v[2]) + ((FULL)((z).v[3]) << BASEB)))); \
		     }
#define SSUB(s,k,j) {FULL tmp = (s).slot[(k)<<1]; \
		     (s).slot[(k)<<1] -= (s).slot[(j)<<1]; \
		     (s).slot[1+((k)<<1)] -= ((tmp <= (s).slot[(k)<<1]) ? \
					       (s).slot[1+((j)<<1)] : \
					       (s).slot[1+((j)<<1)] + 1); \
		    }
#define SINDX(s,k) ((int)((s).slot[1+((k)<<1)] >> (FULL_BITS - SHUFPOW)))
#define SBUFFER(s,t) {(s).buffer[0] = ((s).shuf[(t)<<1] & BASE1); \
		      (s).buffer[1] = ((s).shuf[(t)<<1] >> BASEB); \
		      (s).buffer[2] = ((s).shuf[1+((t)<<1)] & BASE1); \
		      (s).buffer[3] = ((s).shuf[1+((t)<<1)] >> BASEB); \
		     }
#define SSHUF(s,t,k) {(s).shuf[(t)<<1] = (s).slot[(k)<<1]; \
		      (s).shuf[1+((t)<<1)] = (s).slot[1+((k)<<1)]; \
		     }
#define SSWAP(s,j,k) {FULL tmp = (s).slot[(j)<<1]; \
		      (s).slot[(j)<<1] = (s).slot[(k)<<1]; \
		      (s).slot[(k)<<1] = tmp; \
		      tmp = (s).slot[1+((j)<<1)]; \
		      (s).slot[1+((j)<<1)] = (s).slot[1+((k)<<1)]; \
		      (s).slot[1+((k)<<1)] = tmp; \
		     }
#define SMOD64(t,z) {(t)[0] = ztofull(z); \
		     (t)[1] = (((z).len <= 2) ? (FULL)0 : \
			 (((z).len == 3) ? (FULL)((z).v[2]) : \
			  ((FULL)((z).v[2]) + ((FULL)((z).v[3]) << BASEB)))); \
		    }
#define SXOR(s,i,v) {(s).slot[(i)<<1] ^= (v)[0]; \
		     (s).slot[1+((i)<<1)] ^= (v)[1]; \
		    }

#else

   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!

#endif

#define SCNT (SLEN*S100)	/* length of subtractive 100 table in FULLs */

#define RAND_CONSEQ_USE (100)		  /* use this many before skipping */
#define RAND_SKIP (1009-RAND_CONSEQ_USE)  /* skip this many after use */


/*
 * s100 generator state
 */
struct rand {
	int seeded;		/* 1 => state has been seeded */
	int bits;		/* buffer bit count */
	FULL buffer[SLEN];	/* unused random bits from last call */
	int j;			/* first walking table index */
	int k;			/* second walking table index */
	int need_to_skip;	/* 1 => skip the next 909 values */
	FULL slot[SCNT];	/* subtractive 100 table */
	FULL shuf[SHUFLEN];	/* shuffle table entries */
};


/*
 * s100 generator function declarations
 */
E_FUNC RAND *zsrand(CONST ZVALUE *seed, CONST MATRIX *pmat100);
E_FUNC RAND *zsetrand(CONST RAND *state);
E_FUNC void zrandskip(long count);
E_FUNC void zrand(long count, ZVALUE *res);
E_FUNC void zrandrange(CONST ZVALUE low, CONST ZVALUE beyond, ZVALUE *res);
E_FUNC long irand(long s);
E_FUNC RAND *randcopy(CONST RAND *rand);
E_FUNC void randfree(RAND *rand);
E_FUNC BOOL randcmp(CONST RAND *s1, CONST RAND *s2);
E_FUNC void randprint(CONST RAND *state, int flags);


#endif /* !INCLUDE_ZRAND_H */
