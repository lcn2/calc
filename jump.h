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
 * chongo was here	/\../\
 */
/*
 * jump - trivial prime jump table
 *
 * If x is divisible by a trivial prime (2,3,5,7,11), then:
 *
 *		x + jmpindx[ (x>>1)%JMPMOD ]
 *
 * is the value of the smallest value > x that is not divisible by a
 * trivial prime.  JMPMOD is the product of the odd trivial primes.
 *
 * This table is useful for skipping values that are obviously not prime
 * by skipping values that are a multiple of trivial prime.
 *
 * If x is not divisible by a trivial prime, then:
 *
 *		x + jmp[ -jmpindx[(x>>1)%JMPMOD] ]
 *
 * is the value of the smallest value > x that is not divisible by a
 * trivial prime.
 *
 * If jmpindx[y] > 0 (y = x mod JMPMOD*2), then it refers to an 'x' that
 * is divisible by a trivial prime and jmpindx[y] is the offset to the next
 * value that is not divisible.
 *
 * If jmpindx[y] <= 0, then 'x' is not divisible by a trivial prime and
 * the negative of jmpindx[y] is the index into the jmp[] table.  We use
 * successive values from jmp[] (wrapping around to the beginning when
 * we move off the end of jmp[]) to move to higher and higher values
 * that are not divisible by trivial primes.
 *
 * Instead of testing successive odd values, this system allows us to
 * skip odd values divisible by trivial primes.  This is process on the
 * average reduces the values we need to test by a factor of at least 2.4.
 */

#if !defined(JUMP_H)
#define JUMP_H

#include "have_const.h"

/*
 * trivial prime CONSTants
 */
#define JMPMOD (3*5*7*11)	/* product of odd trivial primes */
#define JMPSIZE (2*4*6*10)	/* ints mod JMPMOD not div by trivial primes */
#define JPRIME (prime+4)	/* pointer to first non-trivial prime */

/* given x, return the index within jmpindx that applies */
#define jmpmod(x) (((x)>>1)%JMPMOD)

/* jmpindx table value */
#define jmpindxval(x) (jmpindx[jmpmod(x)])

/* return the smallest value >= x not divisible by a trivial prime */
#define firstjmp(x,tmp) ((tmp) = jmpindxval(x), ((tmp) > 0) ? ((x)+(tmp)) : (x))

/* given x not divisible by a trivial prime, return jmp[] index */
#define jmpptr(x) (-jmpindxval(x))

/* given a jmp pointer, return current jump increment and bump the pointer */
#define nxtjmp(p) ( *( ((p)<lastjmp) ? ((p)++) : (((p)=jmp),lastjmp) ) )

/* given a jmp pointer, return dec pointer and return previous jump increment */
#define prevjmp(p) ( *( ((p)>jmp) ? (--(p)) : ((p)=lastjmp) ) )

/*
 * external jump tables
 */
extern CONST short jmpindx[];
extern CONST unsigned char jmp[];
extern CONST unsigned char *CONST lastjmp;

#endif /* !JUMP_H */
