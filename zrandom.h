/*
 * zrandom - Blum-Blum-Shub pseudo-random generator
 *
 * Copyright (C) 1999-2007,2014,2026  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1997/02/15 04:01:56
 * File existed as early as:    1997
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_ZRANDOM_H)
#  define INCLUDE_ZRANDOM_H

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
    int seeded;  /* 1 => state has been seeded */
    int bits;    /* number of unused bits in buffer */
    int loglogn; /* int(log2(log2(n))), bits produced per turn */
    HALF buffer; /* unused random bits from previous call */
    HALF mask;   /* mask for the log2(log2(n)) lower bits of r */
    ZVALUE n;    /* Blum modulus */
    ZVALUE r;    /* Blum quadratic residue */
};
typedef struct random RANDOM;

/*
 * Blum constants
 */
#  define BLUM_PREGEN 20 /* number of non-default predefined Blum generators */

/*
 * Blum generator function declarations
 */
extern RANDOM *zsrandom1(const ZVALUE seed, bool need_ret);
extern RANDOM *zsrandom2(const ZVALUE seed, const ZVALUE newn);
extern RANDOM *zsrandom4(const ZVALUE seed, const ZVALUE ip, const ZVALUE iq, long trials);
extern RANDOM *zsetrandom(const RANDOM *state);
extern void zrandomskip(long count);
extern void zrandom(long count, ZVALUE *res);
extern void zrandom(long count, ZVALUE *res);
extern void zrandomrange(const ZVALUE low, const ZVALUE beyond, ZVALUE *res);
extern long irandom(long s);
extern RANDOM *randomcopy(const RANDOM *random);
extern void randomfree(RANDOM *random);
extern bool randomcmp(const RANDOM *s1, const RANDOM *s2);
extern void randomprint(const RANDOM *state, int flags);
extern void random_libcalc_cleanup(void);

#endif
