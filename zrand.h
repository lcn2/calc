/*
 * Copyright (c) 1996 by Landon Curt Noll.  All Rights Reserved.
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
 * Prior to calc 2.9.3t9, these routines existed as a calc library called
 * cryrand.cal.  They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
 *
 * Comments, suggestions, bug fixes and questions about these routines
 * are welcome.  Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *			Landon Curt Noll
 *
 *			chongo@toad.com
 *			...!{pyramid,sun,uunet}!hoptoad!chongo
 *
 * chongo was here	/\../\
 */

/*
 * random number generator - see random.c for details
 */

#if !defined(ZRAND_H)
#define	ZRAND_H


#include "value.h"
#include "have_const.h"


/*
 * BITSTR - string of bits within an array of HALFs
 *
 * This typedef records a location of a bit in an array of HALFs.
 * Bit 0 in a HALF is assumed to be the least significant bit in that HALF.
 *
 * The most significant bit is found at (loc,bit).  Bits of lesser
 * significance may be found in previous bits and HALFs.
 */
typedef struct {
	HALF *loc;	/* half address of most significant bit */
	int bit;	/* bit position within half of most significant bit */
	int len;	/* length of string in bits */
} BITSTR;


/*
 * a55 generator defines
 *
 * NOTE: SBITS must be a power of two to make the (&= (SBITS-1))
 *	 in slotcp() to work.
 */
#define SBITS (64)	 /* size of additive or shuffle entry in bits */
#define SBYTES (SBITS/8) /* size of additive or shuffle entry in bytes */
#define SHALFS (SBYTES/sizeof(HALF))	/* size in HALFs */

/*
 * seed defines
 */
#define SEEDXORBITS 64		/* low bits of a55 seed devoted to xor */

/*
 * shuffle table defines
 */
#define SHUFPOW 8		/* power of 2 size of the shuffle table */
#define SHUFCNT (1 << SHUFPOW)	/* size of shuffle table */
#define SHUFLEN (SLEN*SHUFCNT)	/* length of shuffle table in FULLs */
#define SHUFMASK (SHUFLEN-1)	/* mask for shuffle table entry selection */

/*
 * additive 55 constants
 */
#define A55 55		/* slots in an additive 55 table */
#define INIT_J 23	/* initial first walking table index */
#define INIT_K 54	/* initial second walking table index */

/*
 * additive 55 table defines
 *
 * SLEN		- length in FULLs of an additive 55 slot
 *
 * SVAL(x,y)	- form a 64 bit hex slot entry in the additive 55 table
 *		  x: up to 8 hex digits without the leading 0x (upper half)
 *		  y: up to 8 hex digits without the leading 0x (lower half)
 *
 *	NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call!
 *
 * SHVAL(a,b,c,d) - form an 64 bit array of HALFs
 *		  a: up to 4 hex digits without the leading 0x (upper half)
 *		  b: up to 4 hex digits without the leading 0x (2nd half)
 *		  c: up to 4 hex digits without the leading 0x (3rd half)
 *		  d: up to 4 hex digits without the leading 0x (lower half)
 *
 *	NOTE: Due to a SunOS cc bug, don't put spaces in the SHVAL call!
 *
 * HVAL(x,y) - form an array of HALFs given 8 hex digits
 *		  x: up to 4 hex digits without the leading 0x (upper half)
 *		  y: up to 4 hex digits without the leading 0x (lower half)
 *
 *	NOTE: Due to a SunOS cc bug, don't put spaces in the HVAL call!
 *
 * SLOAD(s,i,z)	- load table slot i from additive 55 state s with zvalue z
 *		  s: type RAND
 *		  i: type int, s.slot[i] slot index
 *		  z: type ZVALUE, what to load into s.slot[i]
 *
 * SADD(s,k,j)	- slot[k] += slot[j]
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
 * SBUFFER(s,t) - load a55 buffer with t
 *		  s: type RAND
 *		  t: type int, s.shuf[t] entry index, replace buffer with it
 *
 * SSHUF(s,t,k) - save slot[k] into shuffle entry t
 *		  s: type RAND
 *		  t: type int, s.shuf[t] entry index, what gets changed
 *		  k: type int, s.slot[k] slot index, load into  s.shuf[t]
 *
 * SSWAP(s,j,k)	- swap slot[j] with slot[k]
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
 * SCNT		- length of an additive 55 table in FULLs
 */
#if FULL_BITS == SBITS

# define SLEN 1		/* a 64 bit slot can be held in a FULL */
# if defined(__STDC__) && __STDC__ != 0
#  define SVAL(x,y) (FULL)U(0x ## x ## y)
#  define SHVAL(a,b,c,d) (HALF)0x ## c ## d, (HALF)0x ## a ## b
#  define HVAL(x,y) (HALF)(0x ## x ## y)
# else
#  define SVAL(x,y) (FULL)U(0x/**/x/**/y)
#  define SHVAL(a,b,c,d) (HALF)0x/**/c/**/d,(HALF)0x/**/a/**/b
#  define HVAL(x,y) (HALF)(0x/**/x/**/y)
# endif
#define SLOAD(s,i,z) ((s).slot[i] = ztofull(z))
#define SADD(s,k,j) ((s).slot[k] += (s).slot[j])
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
# if defined(__STDC__) && __STDC__ != 0
#  define SVAL(x,y) (FULL)(0x ## y), (FULL)(0x ## x)
#  define SHVAL(a,b,c,d) (HALF)0x ## d, (HALF)0x ## c, \
			 (HALF)0x ## b, (HALF)0x ## a
#  define HVAL(x,y) (HALF)(0x ## y), (HALF)(0x ## x)
# else
   /* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
#  define SVAL(x,y) (FULL)(0x/**/y), (FULL)(0x/**/x)
   /* NOTE: Due to a SunOS cc bug, don't put spaces in the SHVAL call! */
#  define SHVAL(a,b,c,d) (HALF)0x/**/d, (HALF)0x/**/c, \
			 (HALF)0x/**/b, (HALF)0x/**/a
   /* NOTE: Due to a SunOS cc bug, don't put spaces in the HVAL call! */
#  define HVAL(x,y) (HALF)(0x/**/y), (HALF)(0x/**/x)
# endif
#define SLOAD(s,i,z) {(s).slot[(i)<<1] = ztofull(z); \
		      (s).slot[1+((i)<<1)] = \
		     	(((z).len <= 2) ? (FULL)0 : \
			 (((z).len == 3) ? (FULL)((z).v[2]) : \
			  ((FULL)((z).v[2]) + ((FULL)((z).v[3]) << BASEB)))); \
		     }
#define SADD(s,k,j) {FULL tmp = (s).slot[(k)<<1]; \
		     (s).slot[(k)<<1] += (s).slot[(j)<<1]; \
		     (s).slot[1+((k)<<1)] += ((tmp <= (s).slot[(k)<<1]) ? \
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

   /\../\	FULL_BITS is assumed to be SBITS or 2*SBITS	/\../\   !!!

#endif

#define SCNT (SLEN*A55)	 /* length of additive 55 table in FULLs */


/*
 * a55 generator state
 */
struct rand {
	int seeded;		/* 1 => state has been seeded */
	int bits;		/* buffer bit count */
	FULL buffer[SLEN];	/* unused random bits from last call */
	int j;			/* first walking table index */
	int k;			/* second walking table index */
	FULL slot[SCNT];	/* additive 55 table */
	FULL shuf[SHUFLEN];	/* shuffle table entries */
};


/*
 * Blum generator state
 *
 * The size of the buffer implies that a turn of the quadratic residue crank
 * will never yield more than the number of bits in a FULL.  At worst
 * this implies that a turn can yield no more than 32 bits.  This implies that
 * the lower bound on the largest modulus supported is 2^32 bits long.
 */
struct random {
	int seeded;	/* 1 => state has been seeded */
	int bits;	/* number of unused bits in buffer */
	int loglogn;	/* int(log2(log2(n))), bits produced per turn */
	HALF buffer;	/* unused random bits from previous call */
	HALF mask;	/* mask for the log2(log2(n)) lower bits of r */
	ZVALUE *n;	/* Blum modulus */
	ZVALUE *r;	/* Blum quadratic residue */
};


/*
 * Blum constants
 */
#define BLUM_PREGEN 20	/* number of non-default predefined Blum generators */


/*
 * Blum random config constants
 */
#define BLUM_CFG_MIN BLUM_CFG_NOCHECK
#define BLUM_CFG_NOCHECK	0	/* no checks are performed */
#define BLUM_CFG_1MOD4		1	/* require 1 mod 4 */
#define BLUM_CFG_1MOD4_PTEST0	2	/* require 1 mod 4 and ptest(n,0) */
#define BLUM_CFG_1MOD4_PTEST1	3	/* require 1 mod 4 and ptest(n,1) */
#define BLUM_CFG_1MOD4_PTEST25	4	/* require 1 mod 4 and ptest(n,25) */
#define BLUM_CFG_MAX BLUM_CFG_1MOD4_PTEST25
#define BLUM_CFG_DEFAULT BLUM_CFG_1MOD4_PTEST1	/* default config value */


/*
 * a55 generator function declarations
 */
extern RAND *zsrand(CONST ZVALUE *seed, CONST MATRIX *pmat55);
extern RAND *zsetrand(CONST RAND *state);
extern void zrandskip(long count);
extern void zrand(long count, ZVALUE *res);
extern void zrandrange(CONST ZVALUE low, CONST ZVALUE beyond, ZVALUE *res);
extern long irand(long s);
extern RAND *randcopy(CONST RAND *rand);
extern void randfree(RAND *rand);
extern BOOL randcmp(CONST RAND *s1, CONST RAND *s2);
extern void randprint(CONST RAND *state, int flags);


/*
 * Blum generator function declarations
 */
extern RANDOM *zsrandom(CONST ZVALUE seed, CONST ZVALUE *newn);
extern RANDOM *zsetrandom(CONST RANDOM *state);
extern RANDOM *randomcopy(CONST RANDOM *random);
extern void randomfree(RANDOM *random);
extern BOOL randomcmp(CONST RANDOM *s1, CONST RANDOM *s2);
extern void randomprint(CONST RANDOM *state, int flags);

#endif /* ZRAND_H */
