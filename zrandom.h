/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
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
 *	Landon Curt Noll
 *	http://reality.sgi.com/chongo/
 *
 * chongo <was here> /\../\
 */
/*
 * random number generator - see zrandom.c for details
 */


#if !defined(__ZRANDOM_H__)
#define	__ZRANDOM_H__


#include "value.h"
#include "have_const.h"


/*
 * Blum generator state
 *
 * The size of the buffer implies that a turn of the quadratic residue crank
 * will never yield as many at the than the number of bits in a HALF.  At
 * most this implies that a turn can yield no more than 15 bits when BASEB==16
 * or 31 bits when BASEB==32.  Should we deal with a excessively large
 * Blum modulus (>=2^16 bits long for BASEB==16, >=2^32 bits for BASEB==32)
 * the higher order random bits will be tossed.  This is not a loss as
 * regular sub-segments of the sequence are just as random.  It only means
 * that excessively large Blum modulus values waste CPU time.
 */
struct random {
	int seeded;	/* 1 => state has been seeded */
	int bits;	/* number of unused bits in buffer */
	int loglogn;	/* int(log2(log2(n))), bits produced per turn */
	HALF buffer;	/* unused random bits from previous call */
	HALF mask;	/* mask for the log2(log2(n)) lower bits of r */
	ZVALUE n;	/* Blum modulus */
	ZVALUE r;	/* Blum quadratic residue */
};


/*
 * Blum constants
 */
#define BLUM_PREGEN 20	/* number of non-default predefined Blum generators */


/*
 * Blum generator function declarations
 */
extern RANDOM *zsrandom1(CONST ZVALUE seed, BOOL need_ret);
extern RANDOM *zsrandom2(CONST ZVALUE seed, CONST ZVALUE newn);
extern RANDOM *zsrandom4(CONST ZVALUE seed,
			 CONST ZVALUE ip, CONST ZVALUE iq, long trials);
extern RANDOM *zsetrandom(CONST RANDOM *state);
extern void zrandomskip(long count);
extern void zrandom(long count, ZVALUE *res);
extern void zrandom(long count, ZVALUE *res);
extern void zrandomrange(CONST ZVALUE low, CONST ZVALUE beyond, ZVALUE *res);
extern long irandom(long s);
extern RANDOM *randomcopy(CONST RANDOM *random);
extern void randomfree(RANDOM *random);
extern BOOL randomcmp(CONST RANDOM *s1, CONST RANDOM *s2);
extern void randomprint(CONST RANDOM *state, int flags);
extern void random_libcalc_cleanup(void);


#endif /* !__ZRANDOM_H__ */
