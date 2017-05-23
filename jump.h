/*
 * jump - trivial prime jump table
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
 * Under source code control:	1994/06/29 04:03:55
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
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
 * skip odd values divisible by trivial primes.	 This is process on the
 * average reduces the values we need to test by a factor of at least 2.4.
 */


#if !defined(INCLUDE_JUMP_H)
#define INCLUDE_JUMP_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "have_const.h"
# include "decl.h"
#else
# include <calc/have_const.h>
# include <calc/decl.h>
#endif


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
EXTERN CONST short jmpindx[];
EXTERN CONST unsigned char jmp[];
EXTERN CONST unsigned char *CONST lastjmp;

#endif /* !INCLUDE_JUMP_H */
