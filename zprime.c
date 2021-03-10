/*
 * zprime - rapid small prime routines
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll
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
 * Under source code control:	1994/05/29 04:34:36
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "zmath.h"
#include "prime.h"
#include "jump.h"
#include "config.h"
#include "zrand.h"
#include "have_const.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * When performing a probabilistic primality test, check to see
 * if the number has a factor <= PTEST_PRECHECK.
 *
 * XXX - what should this value be?  Perhaps this should be a function
 *	 of the size of the text value and the number of tests?
 */
#define PTEST_PRECHECK ((FULL)101)

/*
 * product of primes that fit into a long
 */
STATIC CONST FULL pfact_tbl[MAX_PFACT_VAL+1] = {
    1, 1, 2, 6, 6, 30, 30, 210, 210, 210, 210, 2310, 2310, 30030, 30030,
    30030, 30030, 510510, 510510, 9699690, 9699690, 9699690, 9699690,
    223092870, 223092870, 223092870, 223092870, 223092870, 223092870
#if FULL_BITS == 64
    , U(6469693230), U(6469693230), U(200560490130), U(200560490130),
    U(200560490130), U(200560490130), U(200560490130), U(200560490130),
    U(7420738134810), U(7420738134810), U(7420738134810), U(7420738134810),
    U(304250263527210), U(304250263527210), U(13082761331670030),
    U(13082761331670030), U(13082761331670030), U(13082761331670030),
    U(614889782588491410), U(614889782588491410), U(614889782588491410),
    U(614889782588491410), U(614889782588491410), U(614889782588491410)
#endif
};

/*
 * determine the top 1 bit of a 8 bit value:
 *
 *	topbit[0] == 0 by convention
 *	topbit[x] gives the highest 1 bit of x
 */
STATIC CONST unsigned char topbit[256] = {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/*
 * integer square roots of powers of 2
 *
 * isqrt_pow2[x] == (int)(sqrt(2 to the x power))	      (for 0 <= x < 64)
 *
 * We have enough table entries for a FULL that is 64 bits long.
 */
STATIC CONST FULL isqrt_pow2[64] = {
    1, 1, 2, 2, 4, 5, 8, 11,					/*  0 ..  7 */
    16, 22, 32, 45, 64, 90, 128, 181,				/*  8 .. 15 */
    256, 362, 512, 724, 1024, 1448, 2048, 2896,			/* 16 .. 23 */
    4096, 5792, 8192, 11585, 16384, 23170, 32768, 46340,	/* 24 .. 31 */
    65536, 92681, 131072, 185363,				/* 32 .. 35 */
    262144, 370727, 524288, 741455,				/* 36 .. 39 */
    1048576, 1482910, 2097152, 2965820,				/* 40 .. 43 */
    4194304, 5931641, 8388608, 11863283,			/* 44 .. 47 */
    16777216, 23726566, 33554432, 47453132,			/* 48 .. 51 */
    67108864, 94906265, 134217728, 189812531,			/* 52 .. 55 */
    268435456, 379625062, 536870912, 759250124,			/* 56 .. 59 */
    1073741824, 1518500249, 0x80000000, 0xb504f333		/* 60 .. 63 */
};

/*
 * static functions
 */
S_FUNC FULL fsqrt(FULL v);		/* quick square root of v */
S_FUNC long pix(FULL x);		/* pi of x */
S_FUNC FULL small_factor(ZVALUE n, FULL limit);	  /* factor or 0 */


/*
 * Determine if a value is a small (32 bit) prime
 *
 * Returns:
 *	1	z is a prime <= MAX_SM_VAL
 *	0	z is not a prime <= MAX_SM_VAL
 *	-1	z > MAX_SM_VAL
 */
FLAG
zisprime(ZVALUE z)
{
	FULL n;				/* number to test */
	FULL isqr;			/* factor limit */
	CONST unsigned short *tp;	/* pointer to a prime factor */

	z.sign = 0;
	if (zisleone(z)) {
		return 0;
	}

	/* even numbers > 2 are not prime */
	if (ziseven(z)) {
		/*
		 * "2 is the greatest odd prime because it is the least even!"
		 *					- Dr. Dan Jurca	 1978
		 */
		return zisabstwo(z);
	}

	/* ignore non-small values */
	if (zge32b(z)) {
		return -1;
	}

	/* we now know that we are dealing with a value 0 <= n < 2^32 */
	n = ztofull(z);

	/* lookup small cases in pr_map */
	if (n <= MAX_MAP_VAL) {
		return (pr_map_bit(n) ? 1 : 0);
	}

	/* ignore Saber-C warning #530 about empty for statement */
	/*	  ok to ignore in proc zisprime */
	/* a number >=2^16 and < 2^32 */
	for (isqr=fsqrt(n), tp=prime; (*tp <= isqr) && (n % *tp); ++tp) {
	}
	return ((*tp <= isqr && *tp != 1) ? 0 : 1);
}


/*
 * Determine the next small (32 bit) prime > a 32 bit value.
 *
 * given:
 *	z		search point
 *
 * Returns:
 *	0	next prime is 2^32+15
 *	1	abs(z) >= 2^32
 *	smallest prime > abs(z) otherwise
 */
FULL
znprime(ZVALUE z)
{
	FULL n;			/* search point */

	z.sign = 0;

	/* ignore large values */
	if (zge32b(z)) {
		return (FULL)1;
	}

	/* deal a search point of 0 or 1 */
	if (zisabsleone(z)) {
		return (FULL)2;
	}

	/* deal with returning a value that is beyond our reach */
	n = ztofull(z);
	if (n >= MAX_SM_PRIME) {
		return (FULL)0;
	}

	/* return the next prime */
	return next_prime(n);
}


/*
 * Compute the next prime beyond a small (32 bit) value.
 *
 * This function assumes that 2 <= n < 2^32-5.
 *
 * given:
 *	n		search point
 */
FULL
next_prime(FULL n)
{
	CONST unsigned short *tp;	/* pointer to a prime factor */
	CONST unsigned char *j;		/* current jump increment */
	int tmp;

	/* find our search point */
	n = ((n & 0x1) ? n+2 : n+1);

	/* if we can just search the bit map, then search it */
	if (n <= MAX_MAP_PRIME) {

		/* search until we find a 1 bit */
		while (pr_map_bit(n) == 0) {
			n += (FULL)2;
		}

	/* too large for our table, find the next prime the hard way */
	} else {
		FULL isqr;		/* factor limit */

		/*
		 * Our search for a prime may cause us to increment n over
		 * a perfect square, but never two perfect squares.  The largest
		 * prime gap <= 2614941711251 is 651.  Shanks conjectures that
		 * the largest gap below P is about ln(P)^2.
		 *
		 * The value fsqrt(n)^2 will always be the perfect square
		 * that is <= n.  Given the smallness of prime gaps we will
		 * deal with, we know that n could carry us across the next
		 * perfect square (fsqrt(n)+1)^2 but not the following
		 * perfect square (fsqrt(n)+2)^2.
		 *
		 * Now the factor search limit for values < (fsqrt(n)+2)^2
		 * is the same limit for (fsqrt(n)+1)^2; namely fsqrt(n)+1.
		 * Therefore setting our limit at fsqrt(n)+1 and never
		 * bothering with it after that is safe.
		 */
		isqr = fsqrt(n)+1;

		/*
		 * If our factor limit is even, then we can reduce it to
		 * the next lowest odd value.  We already tested if n
		 * was even and all of our remaining potential factors
		 * are odd.
		 */
		if ((isqr & 0x1) == 0) {
			--isqr;
		}

		/*
		 * Skip to next value not divisible by a trivial prime.
		 */
		n = firstjmp(n, tmp);
		j = jmp + jmpptr(n);

		/*
		 * Look for tiny prime factors of increasing n until we
		 * find a prime.
		 */
		do {
			/* ignore Saber-C warning #530 - empty for statement */
			/*	  ok to ignore in proc next_prime */
			/* XXX - speed up test for large n by using gcds */
			/* find a factor, or give up if not found */
			for (tp=JPRIME; (*tp <= isqr) && (n % *tp); ++tp) {
			}
		} while (*tp <= isqr && *tp != 1 && (n += nxtjmp(j)));
	}

	/* return the prime that we found */
	return n;
}


/*
 * Determine the previous small (32 bit) prime < a 32 bit value
 *
 * given:
 *	z		search point
 *
 * Returns:
 *	1	abs(z) >= 2^32
 *	0	abs(z) <= 2
 *	greatest prime < abs(z) otherwise
 */
FULL
zpprime(ZVALUE z)
{
	CONST unsigned short *tp;	/* pointer to a prime factor */
	FULL isqr;			/* isqrt(z) */
	FULL n;				/* search point */
	CONST unsigned char *j;		/* current jump increment */
	int tmp;

	z.sign	= 0;

	/* ignore large values */
	if (zge32b(z)) {
		return (FULL)1;
	}

	/* deal with special case small values */
	n = ztofull(z);
	switch ((int)n) {
	case 0:
	case 1:
	case 2:
		/* ignore values <= 2 */
		return (FULL)0;
	case 3:
		/* 3 returns the only even prime */
		return (FULL)2;
	}

	/* deal with values above the bit map */
	if (n > NXT_MAP_PRIME) {

		/* find our search point */
		n = ((n & 0x1) ? n-2 : n-1);

		/* our factor limit - see next_prime for why this works */
		isqr = fsqrt(n)+1;
		if ((isqr & 0x1) == 0) {
			--isqr;
		}

		/*
		 * Skip to previous value not divisible by a trivial prime.
		 */
		tmp = jmpindxval(n);
		if (tmp >= 0) {

			/* find next value not divisible by a trivial prime */
			n += tmp;

			/* find the previous jump index */
			j = jmp + jmpptr(n);

			/* jump back */
			n -= prevjmp(j);

		/* already not divisible by a trivial prime */
		} else {
			/* find the current jump index */
			j = jmp + jmpptr(n);
		}

		/* factor values until we find a prime */
		do {
			/* ignore Saber-C warning #530 - empty for statement */
			/*	  ok to ignore in proc zpprime */
			/* XXX - speed up test for large n by using gcds */
			/* find a factor, or give up if not found */
			for (tp=prime; (*tp <= isqr) && (n % *tp); ++tp) {
			}
		} while (*tp <= isqr && *tp != 1 && (n -= prevjmp(j)));

	/* deal with values within the bit map */
	} else if (n <= MAX_MAP_PRIME) {

		/* find our search point */
		n = ((n & 0x1) ? n-2 : n-1);

		/* search until we find a 1 bit */
		while (pr_map_bit(n) == 0) {
			n -= (FULL)2;
		}

	/* deal with values that could cross into the bit map */
	} else {
		/* MAX_MAP_PRIME < n <= NXT_MAP_PRIME returns MAX_MAP_PRIME */
		return MAX_MAP_PRIME;
	}

	/* return what we found */
	return n;
}


/*
 * Compute the number of primes <= a ZVALUE that can fit into a FULL
 *
 * given:
 *	z		compute primes <= z
 *
 * Returns:
 *	-1	error
 *	>=0	number of primes <= x
 */
long
zpix(ZVALUE z)
{
	/* pi(<0) is always 0 */
	if (zisneg(z)) {
		return (long)0;
	}

	/* firewall */
	if (zge32b(z)) {
		return (long)-1;
	}
	return pix(ztofull(z));
}


/*
 * Compute the number of primes <= a ZVALUE
 *
 * given:
 *	x		value of z
 *
 * Returns:
 *	-1	error
 *	>=0	number of primes <= x
 */
S_FUNC long
pix(FULL x)
{
	long count;			/* pi(x) */
	FULL top;			/* top of the range to test */
	CONST unsigned short *tp;	/* pointer to a tiny prime */
	FULL i;

	/* compute pi(x) using the 2^8 step table */
	if (x <= MAX_PI10B) {

		/* x within the prime table, so use it */
		if (x < MAX_MAP_PRIME) {
			/* firewall - pix(x) ==0 for x < 2 */
			if (x < 2) {
				count = 0;

			} else {
				/* determine how and where we will count */
				if (x < 1024) {
					count = 1;
					tp = prime;
				} else {
					count = pi10b[x>>10];
					tp = prime+count-1;
				}
				/* count primes in the table */
				while (*tp++ <= x) {
					++count;
				}
			}

		/* x is larger than the prime table, so count the hard way */
		} else {

			/* case: count down from pi18b entry to x */
			if (x & 0x200) {
				top = (x | 0x3ff);
				count = pi10b[(top+1)>>10];
				for (i=next_prime(x); i <= top;
				     i=next_prime(i)) {
					--count;
				}

			/* case: count up from pi10b entry to x */
			} else {
				count = pi10b[x>>10];
				for (i=next_prime(x&(~0x3ff));
				     i <= x; i = next_prime(i)) {
					++count;
				}
			}
		}

	/* compute pi(x) using the 2^18 interval table */
	} else {

		/* compute sum of intervals up to our interval */
		for (count=0, i=0; i < (x>>18); ++i) {
			count += pi18b[i];
		}

		/* case: count down from pi18b entry to x */
		if (x & 0x20000) {
			top = (x | 0x3ffff);
			count += pi18b[i];
			if (top > MAX_SM_PRIME) {
				if (x < MAX_SM_PRIME) {
					for (i=next_prime(x); i < MAX_SM_PRIME;
					     i=next_prime(i)) {
						--count;
					}
					--count;
				}
			} else {
				for (i=next_prime(x); i<=top; i=next_prime(i)) {
					--count;
				}
			}

		/* case: count up from pi18b entry to x */
		} else {
			for (i=next_prime(x&(~0x3ffff));
			     i <= x; i = next_prime(i)) {
				++count;
			}
		}
	}
	return count;
}


/*
 * Compute the smallest prime factor < limit
 *
 * given:
 *	n		number to factor
 *	zlimit		ending search point
 *	res		factor, if found, or NULL
 *
 * Returns:
 *	-1	error, limit >= 2^32
 *	0	no factor found, res is not changed
 *	1	factor found, res (if non-NULL) is smallest prime factor
 *
 * NOTE: This routine will not return a factor == the test value
 *	 except when the test value is 1 or -1.
 */
FLAG
zfactor(ZVALUE n, ZVALUE zlimit, ZVALUE *res)
{
	FULL f;			/* factor found, or 0 */

	/*
	 * determine the limit
	 */
	if (zge32b(zlimit)) {
		/* limit is too large to be reasonable */
		return -1;
	}
	n.sign = 0;		/* ignore sign of n */

	/*
	 * find the smallest factor <= limit, if possible
	 */
	f = small_factor(n, ztofull(zlimit));

	/*
	 * report the results
	 */
	if (f > 0) {
		/* return factor if requested */
		if (res) {
			utoz(f, res);
		}
		/* report a factor was found */
		return 1;
	}
	/* no factor was found */
	return 0;
}


/*
 * Find a smallest prime factor <= some small (32 bit) limit of a value
 *
 * given:
 *	z		number to factor
 *	limit		largest factor we will test
 *
 * Returns:
 *	0	no prime <= the limit was found
 *	!= 0	the smallest prime factor
 */
S_FUNC FULL
small_factor(ZVALUE z, FULL limit)
{
	FULL top;			/* current max factor level */
	CONST unsigned short *tp;	/* pointer to a tiny prime */
	FULL factlim;			/* highest factor to test */
	CONST unsigned short *p;	/* test factor */
	FULL factor;			/* test factor */
	HALF tlim;			/* limit on prime table use */
	HALF divval[2];			/* divisor value */
	ZVALUE div;			/* test factor/divisor */
	ZVALUE tmp;
	CONST unsigned char *j;

	/*
	 * catch impossible ranges
	 */
	if (limit < 2) {
		/* range is too small */
		return 0;
	}

	/*
	 * perform the even test
	 */
	if (ziseven(z)) {
		if (zistwo(z)) {
			/* z is 2, so don't return 2 as a factor */
			return 0;
		}
		return 2;

	/*
	 * value is odd
	 */
	} else if (limit == 2) {
		/* limit is 2, value is odd, no factors will ever be found */
		return 0;
	}

	/*
	 * force the factor limit to be odd
	 */
	if ((limit & 0x1) == 0) {
		--limit;
	}

	/*
	 * case: number to factor fits into a FULL
	 */
	if (!zgtmaxufull(z)) {
		FULL val = ztofull(z);	/* find the smallest factor of val */
		FULL isqr;		/* sqrt of val */

		/*
		 * special case: val is a prime <= MAX_MAP_PRIME
		 */
		if (val <= MAX_MAP_PRIME && pr_map_bit(val)) {
			/* z is prime, so no factors will be found */
			return 0;
		}

		/*
		 * we need not search above the sqrt of val
		 */
		isqr = fsqrt(val);
		if (limit > isqr) {
			/* limit is largest odd value <= sqrt of val */
			limit = ((isqr & 0x1) ? isqr : isqr-1);
		}

		/*
		 * search for a small prime factor
		 */
		top = ((limit < MAX_MAP_VAL) ? limit : MAX_MAP_VAL);
		for (tp = prime; *tp <= top && *tp != 1; ++tp) {
			if (val%(*tp) == 0) {
				return ((FULL)*tp);
			}
		}

#if FULL_BITS == 64
		/*
		 * Our search will carry us beyond the prime table.  We will
		 * continue to values until we reach our limit or until a
		 * factor is found.
		 *
		 * It is faster to simply test odd values and ignore non-prime
		 * factors because the work needed to find the next prime is
		 * more than the work one saves in not factor with non-prime
		 * values.
		 *
		 * We can improve on this method by skipping odd values that
		 * are a multiple of 3, 5, 7 and 11.  We use a table of
		 * bytes that indicate the offsets between odd values that
		 * are not a multiple of 3,4,5,7 & 11.
		 */
		/* XXX - speed up test for large z by using gcds */
		j = jmp + jmpptr(NXT_MAP_PRIME);
		for (top=NXT_MAP_PRIME; top <= limit; top += nxtjmp(j)) {
			if ((val % top) == 0) {
				return top;
			}
		}
#endif /* FULL_BITS == 64 */

		/* no prime factors found */
		return 0;
	}

	/*
	 * Find a factor of a value that is too large to fit into a FULL.
	 *
	 * determine if/what our sqrt factor limit will be
	 */
	if (zge64b(z)) {
		/* we have no factor limit, avoid highest factor */
		factlim = MAX_SM_PRIME-1;
	} else if (zge32b(z)) {
		/* determine if limit is too small to matter */
		if (limit < BASE) {
			factlim = limit;
		} else {
			/* find the isqrt(z) */
			if (!zsqrt(z, &tmp, 0)) {
				/* sqrt is exact */
				factlim = ztofull(tmp);
			} else {
				/* sqrt is inexact */
				factlim = ztofull(tmp)+1;
			}
			zfree(tmp);

			/* avoid highest factor */
			if (factlim >= MAX_SM_PRIME) {
				factlim = MAX_SM_PRIME-1;
			}
		}
	} else {
		/* determine our factor limit */
		factlim = fsqrt(ztofull(z));
		if (factlim >= MAX_SM_PRIME) {
			factlim = MAX_SM_PRIME-1;
		}
	}
	if (factlim > limit) {
		factlim = limit;
	}

	/*
	 * walk the prime table looking for factors
	 *
	 * XXX - consider using gcd of products of primes to speed this
	 *	 section up
	 */
	tlim = (HALF)((factlim >= MAX_MAP_PRIME) ? MAX_MAP_PRIME-1 : factlim);
	div.sign = 0;
	div.v = divval;
	div.len = 1;
	for (p=prime; (HALF)*p <= tlim; ++p) {

		/* setup factor */
		div.v[0] = (HALF)(*p);

		if (zdivides(z, div))
			return (FULL)(*p);
	}
	if ((FULL)*p > factlim) {
		/* no factor found */
		return (FULL)0;
	}

	/*
	 * test the highest factor possible
	 */
	div.v[0] = MAX_MAP_PRIME;

	if (zdivides(z, div))
		return (FULL)MAX_MAP_PRIME;

	/*
	 * generate higher test factors as needed
	 *
	 * XXX - consider using gcd of products of primes to speed this
	 *	 section up
	 */
#if BASEB == 16
	div.len = 2;
#endif
	factor = NXT_MAP_PRIME;
	j = jmp + jmpptr(factor);
	for(; factor <= factlim; factor += nxtjmp(j)) {

		/* setup factor */
#if BASEB == 32
		div.v[0] = (HALF)factor;
#else
		div.v[0] = (HALF)(factor & BASE1);
		div.v[1] = (HALF)(factor >> BASEB);
#endif

		if (zdivides(z, div))
			return (FULL)(factor);
	}
	if (factor >= factlim) {
		/* no factor found */
		return (FULL)0;
	}

	/*
	 * test the highest factor possible
	 */
#if BASEB == 32
	div.v[0] = MAX_SM_PRIME;
#else
	div.v[0] = (MAX_SM_PRIME & BASE1);
	div.v[1] = (MAX_SM_PRIME >> BASEB);
#endif
	if (zdivides(z, div))
		return (FULL)MAX_SM_PRIME;

	/*
	 * no factor found
	 */
	return (FULL)0;
}


/*
 * Compute the product of the primes up to the specified number.
 */
void
zpfact(ZVALUE z, ZVALUE *dest)
{
	long n;				/* limiting number to multiply by */
	long p;				/* current prime */
	CONST unsigned short *tp;	/* pointer to a tiny prime */
	CONST unsigned char *j;		/* current jump increment */
	ZVALUE res, temp;

	/* firewall */
	if (zisneg(z)) {
		math_error("Negative argument for factorial");
		/*NOTREACHED*/
	}
	if (zge24b(z)) {
		math_error("Very large factorial");
		/*NOTREACHED*/
	}
	n = ztolong(z);

	/*
	 * Deal with table lookup pfact values
	 */
	if (n <= MAX_PFACT_VAL) {
		utoz(pfact_tbl[n], dest);
		return;
	}

	/*
	 * Multiply by the primes in the static table
	 */
	utoz(pfact_tbl[MAX_PFACT_VAL], &res);
	for (tp=(&prime[NXT_PFACT_VAL]); *tp != 1 && (long)(*tp) <= n; ++tp) {
		zmuli(res, *tp, &temp);
		zfree(res);
		res = temp;
	}

	/*
	 * if needed, multiply by primes beyond the static table
	 */
	j = jmp + jmpptr(NXT_MAP_PRIME);
	for (p = NXT_MAP_PRIME; p <= n; p += nxtjmp(j)) {
		FULL isqr;	/* isqrt(p) */

		/* our factor limit - see next_prime for why this works */
		isqr = fsqrt(p)+1;
		if ((isqr & 0x1) == 0) {
			--isqr;
		}

		/* ignore Saber-C warning #530 about empty for statement */
		/*	  ok to ignore in proc zpfact */
		/* find the next prime */
		for (tp=prime; (*tp <= isqr) && (p % (long)(*tp)); ++tp) {
		}
		if (*tp <= isqr && *tp != 1) {
			continue;
		}

		/* multiply by the next prime */
		zmuli(res, p, &temp);
		zfree(res);
		res = temp;
	}
	*dest = res;
}


/*
 * Perform a probabilistic primality test (algorithm P in Knuth vol2, 4.5.4).
 * Returns FALSE if definitely not prime, or TRUE if probably prime.
 * Count determines how many times to check for primality.
 * The chance of a non-prime passing this test is less than (1/4)^count.
 * For example, a count of 100 fails for only 1 in 10^60 numbers.
 *
 * It is interesting to note that ptest(a,1,x) (for any x >= 0) of this
 * test will always return TRUE for a prime, and rarely return TRUE for
 * a non-prime.	 The 1/4 is appears in practice to be a poor upper
 * bound.  Even so the only result that is EXACT and TRUE is when
 * this test returns FALSE for a non-prime.  When ptest returns TRUE,
 * one cannot determine if the value in question is prime, or the value
 * is one of those rare non-primes that produces a false positive.
 *
 * The absolute value of count determines how many times to check
 * for primality.  If count < 0, then the trivial factor check is
 * omitted.
 * skip = 0 uses random bases
 * skip = 1 uses prime bases 2, 3, 5, ...
 * skip > 1 or < 0 uses bases skip, skip + 1, ...
 */
BOOL
zprimetest(ZVALUE z, long count, ZVALUE skip)
{
	long limit = 0;		/* test odd values from skip up to limit */
	ZVALUE zbase;		/* base as a ZVALUE */
	long i, ij, ik;
	ZVALUE zm1, z1, z2, z3;
	int type;		/* random, prime or consecutive integers */
	CONST unsigned short *pr;	/* pointer to small prime */

	/*
	 * firewall - ignore sign of z, values 0 and 1 are not prime
	 */
	z.sign = 0;
	if (zisleone(z)) {
		return 0;
	}

	/*
	 * firewall - All even values, except 2, are not prime
	 */
	if (ziseven(z))
		return zistwo(z);

	if (z.len == 1 && *z.v == 3)
		return 1;		/* 3 is prime */

	/*
	 * we know that z is an odd value > 1
	 */

	/*
	 * Perform trivial checks if count is not negative
	 */
	if (count >= 0) {

		/*
		 * If the number is a small (32 bit) value, do a direct test
		 */
		if (!zge32b(z)) {
			return zisprime(z);
		}

		/*
		 * See if the number has a tiny factor.
		 */
		if (small_factor(z, PTEST_PRECHECK) != 0) {
			/* a tiny factor was found */
			return FALSE;
		}

		/*
		 * If our count is zero, do nothing more
		 */
		if (count == 0) {
			/* no test was done, so no test failed! */
			return TRUE;
		}

	} else {
		/* use the absolute value of count */
		count = -count;
	}
	if (z.len < conf->redc2) {
		return zredcprimetest(z, count, skip);
	}

	if (ziszero(skip)) {
		type = 0;
		zbase = _zero_;
	} else if (zisone(skip)) {
		type = 1;
		itoz(2, &zbase);
		limit = 1 << 16;
		if (!zge16b(z))
			limit = ztolong(z);
	} else {
		type = 2;
		if (zrel(skip, z) >= 0 || zisneg(skip))
			zmod(skip, z, &zbase, 0);
		else
			zcopy(skip, &zbase);
	}
	/*
	 * Loop over various bases, testing each one.
	 */
	zsub(z, _one_, &zm1);
	ik = zlowbit(zm1);
	zshift(zm1, -ik, &z1);
	pr = prime;
	for (i = 0; i < count; i++) {
		switch (type) {
		case 0:
			zfree(zbase);
			zrandrange(_two_, zm1, &zbase);
			break;
		case 1:
			if (i == 0)
				break;
			zfree(zbase);
			if (*pr == 1 || (long)*pr >= limit) {
				zfree(z1);
				zfree(zm1);
				return TRUE;
			}
			itoz((long) *pr++, &zbase);
			break;
		default:
			if (i == 0)
				break;
			zadd(zbase, _one_, &z3);
			zfree(zbase);
			zbase = z3;
		}

		ij = 0;
		zpowermod(zbase, z1, z, &z3);
		for (;;) {
			if (zisone(z3)) {
				if (ij) {
				/* number is definitely not prime */
					zfree(z3);
					zfree(zm1);
					zfree(z1);
					zfree(zbase);
					return FALSE;
				}
				break;
			}
			if (!zcmp(z3, zm1))
				break;
			if (++ij >= ik) {
				/* number is definitely not prime */
				zfree(z3);
				zfree(zm1);
				zfree(z1);
				zfree(zbase);
				return FALSE;
			}
			zsquare(z3, &z2);
			zfree(z3);
			zmod(z2, z, &z3, 0);
			zfree(z2);
		}
		zfree(z3);
	}
	zfree(zm1);
	zfree(z1);
	zfree(zbase);

	/* number might be prime */
	return TRUE;
}


/*
 * Called by zprimetest when simple cases have been eliminated
 * and z.len < conf->redc2.  Here count > 0, z is odd and > 3.
 */
BOOL
zredcprimetest(ZVALUE z, long count, ZVALUE skip)
{
	long limit = 0;		/* test odd values from skip up to limit */
	ZVALUE zbase;		/* base as a ZVALUE */
	REDC *rp;
	long i, ij, ik;
	ZVALUE zm1, z1, z2, z3;
	ZVALUE zredcm1;
	int type;		/* random, prime or consecutive integers */
	CONST unsigned short *pr;	/* pointer to small prime */


	rp = zredcalloc(z);
	zsub(z, rp->one, &zredcm1);
	if (ziszero(skip)) {
		zbase = _zero_;
		type = 0;
	} else if (zisone(skip)) {
		itoz(2, &zbase);
		type = 1;
		limit = 1 << 16;
		if (!zge16b(z))
			limit = ztolong(z);
	} else {
		zredcencode(rp, skip, &zbase);
		type = 2;
	}
	/*
	 * Loop over various "random" numbers, testing each one.
	 */
	zsub(z, _one_, &zm1);
	ik = zlowbit(zm1);
	zshift(zm1, -ik, &z1);
	pr = prime;

	for (i = 0; i < count; i++) {
		switch (type) {
		case 0:
			do {
				zfree(zbase);
				zrandrange(_one_, z, &zbase);
			}
			while (!zcmp(zbase, rp->one) ||
					!zcmp(zbase, zredcm1));
			break;
		case 1:
			if (i == 0) {
				break;
			}
			zfree(zbase);
			if (*pr == 1 || (long)*pr >= limit) {
				zfree(z1);
				zfree(zm1);
				if (z.len < conf->redc2) {
					zredcfree(rp);
					zfree(zredcm1);
				}
				return TRUE;
			}
			itoz((long) *pr++, &z3);
			zredcencode(rp, z3, &zbase);
			zfree(z3);
			break;
		default:
			if (i == 0)
				break;
			zadd(zbase, rp->one, &z3);
			zfree(zbase);
			zbase = z3;
			if (zrel(zbase, z) >= 0) {
				zsub(zbase, z, &z3);
				zfree(zbase);
				zbase = z3;
			}
		}

		ij = 0;
		zredcpower(rp, zbase, z1, &z3);
		for (;;) {
			if (!zcmp(z3, rp->one)) {
				if (ij) {
				/* number is definitely not prime */
					zfree(z3);
					zfree(zm1);
					zfree(z1);
					zfree(zbase);
					zredcfree(rp);
					zfree(zredcm1);
					return FALSE;
				}
				break;
			}
			if (!zcmp(z3, zredcm1))
				break;
			if (++ij >= ik) {
				/* number is definitely not prime */
				zfree(z3);
				zfree(zm1);
				zfree(z1);
				zfree(zbase);
				zredcfree(rp);
				zfree(zredcm1);
				return FALSE;
			}
			zredcsquare(rp, z3, &z2);
			zfree(z3);
			z3 = z2;
		}
		zfree(z3);
	}
	zfree(zbase);
	zredcfree(rp);
	zfree(zredcm1);
	zfree(zm1);
	zfree(z1);

	/* number might be prime */
	return TRUE;
}


/*
 * znextcand - find the next integer that passes ptest().
 * The signs of z and mod are ignored.	Result is the least integer
 * greater than abs(z) congruent to res modulo abs(mod), or if there
 * is no such integer, zero.
 *
 * given:
 *	z		search point > 2
 *	count		ptests to perform per candidate
 *	skip		ptests to skip
 *	res		return congruent to res modulo abs(mod)
 *	mod		congruent to res modulo abs(mod)
 *	cand		candidate found
 */
BOOL
znextcand(ZVALUE z, long count, ZVALUE skip, ZVALUE res, ZVALUE mod,
	  ZVALUE *cand)
{
	ZVALUE tmp1;
	ZVALUE tmp2;

	z.sign	= 0;
	mod.sign = 0;
	if (ziszero(mod)) {
		if (zrel(res, z) > 0 && zprimetest(res, count, skip)) {
			zcopy(res, cand);
			return TRUE;
		}
		return FALSE;
	}
	if (ziszero(z) && zisone(mod)) {
		zcopy(_two_, cand);
		return TRUE;
	}
	zsub(res, z, &tmp1);
	if (zmod(tmp1, mod, &tmp2, 0))
		zadd(z, tmp2, cand);
	else
		zadd(z, mod, cand);

	/*
	 * Now *cand is least integer greater than abs(z) and congruent
	 * to res modulo mod.
	 */
	zfree(tmp1);
	zfree(tmp2);
	if (zprimetest(*cand, count, skip))
		return TRUE;
	zgcd(*cand, mod, &tmp1);
	if (!zisone(tmp1)) {
		zfree(tmp1);
		zfree(*cand);
		return FALSE;
	}
	zfree(tmp1);
	if (ziseven(*cand)) {
		zadd(*cand, mod, &tmp1);
		zfree(*cand);
		*cand = tmp1;
		if (zprimetest(*cand, count, skip))
			return TRUE;
	}
	/*
	 * *cand is now least odd integer > abs(z) and congruent to
	 * res modulo mod.
	 */
	if (zisodd(mod))
		zshift(mod, 1, &tmp1);
	else
		zcopy(mod, &tmp1);
	do {
		zadd(*cand, tmp1, &tmp2);
		zfree(*cand);
		*cand = tmp2;
	} while (!zprimetest(*cand, count, skip));
	zfree(tmp1);
	return TRUE;
}


/*
 * zprevcand - find the nearest previous integer that passes ptest().
 * The signs of z and mod are ignored.	Result is greatest positive integer
 * less than abs(z) congruent to res modulo abs(mod), or if there
 * is no such integer, zero.
 *
 * given:
 *	z		search point > 2
 *	count		ptests to perform per candidate
 *	skip		ptests to skip
 *	res		return congruent to res modulo abs(mod)
 *	mod		congruent to res modulo abs(mod)
 *	cand		candidate found
 */
BOOL
zprevcand(ZVALUE z, long count, ZVALUE skip, ZVALUE res, ZVALUE mod,
	  ZVALUE *cand)
{
	ZVALUE tmp1;
	ZVALUE tmp2;

	z.sign	= 0;
	mod.sign = 0;
	if (ziszero(mod)) {
		if (zispos(res)&&zrel(res, z)<0 && zprimetest(res,count,skip)) {
			zcopy(res, cand);
			return TRUE;
		}
		return FALSE;
	}
	zsub(z, res, &tmp1);
	if (zmod(tmp1, mod, &tmp2, 0))
		zsub(z, tmp2, cand);
	else
		zsub(z, mod, cand);
	/*
	 * *cand is now the greatest integer < z that is congruent to res
	 * modulo mod.
	 */
	zfree(tmp1);
	zfree(tmp2);
	if (zisneg(*cand)) {
		zfree(*cand);
		return FALSE;
	}
	if (zprimetest(*cand, count, skip))
		return TRUE;
	zgcd(*cand, mod, &tmp1);
	if (!zisone(tmp1)) {
		zfree(tmp1);
		zmod(*cand, mod, &tmp1, 0);
		zfree(*cand);
		if (zprimetest(tmp1, count, skip)) {
			*cand = tmp1;
			return TRUE;
		}
		if (ziszero(tmp1)) {
			zfree(tmp1);
			if (zprimetest(mod, count, skip)) {
				zcopy(mod, cand);
				return TRUE;
			}
			return FALSE;
		}
		zfree(tmp1);
		return FALSE;
	}
	zfree(tmp1);
	if (ziseven(*cand)) {
		zsub(*cand, mod, &tmp1);
		zfree(*cand);
		if (zisneg(tmp1)) {
			zfree(tmp1);
			return FALSE;
		}
		*cand = tmp1;
		if (zprimetest(*cand, count, skip))
			return TRUE;
	}
	/*
	 * *cand is now the greatest odd integer < z that is congruent to
	 * res modulo mod.
	 */
	if (zisodd(mod))
		zshift(mod, 1, &tmp1);
	else
		zcopy(mod, &tmp1);

	do {
		zsub(*cand, tmp1, &tmp2);
		zfree(*cand);
		*cand = tmp2;
	} while (!zprimetest(*cand, count, skip) && !zisneg(*cand));
	zfree(tmp1);
	if (zisneg(*cand)) {
			zadd(*cand, mod, &tmp1);
			zfree(*cand);
			*cand = tmp1;
			if (zistwo(*cand))
				return TRUE;
			zfree(*cand);
			return FALSE;
	}
	return TRUE;
}


/*
 * Find the lowest prime factor of a number if one can be found.
 * Search is conducted for the first count primes.
 *
 * Returns:
 *	1	no factor found or z < 3
 *	>1	factor found
 */
FULL
zlowfactor(ZVALUE z, long count)
{
	FULL factlim;			/* highest factor to test */
	CONST unsigned short *p;	/* test factor */
	FULL factor;			/* test factor */
	HALF tlim;			/* limit on prime table use */
	HALF divval[2];			/* divisor value */
	ZVALUE div;			/* test factor/divisor */
	ZVALUE tmp;

	z.sign = 0;

	/*
	 * firewall
	 */
	if (count <= 0 || zisleone(z) || zistwo(z)) {
		/* number is < 3 or count is <= 0 */
		return (FULL)1;
	}

	/*
	 * test for the first factor
	 */
	if (ziseven(z)) {
		return (FULL)2;
	}
	if (count <= 1) {
		/* count was 1, tested the one and only factor */
		return (FULL)1;
	}

	/*
	 * determine if/what our sqrt factor limit will be
	 */
	if (zge64b(z)) {
		/* we have no factor limit, avoid highest factor */
		factlim = MAX_SM_PRIME-1;
	} else if (zge32b(z)) {
		/* find the isqrt(z) */
		if (!zsqrt(z, &tmp, 0)) {
			/* sqrt is exact */
			factlim = ztofull(tmp);
		} else {
			/* sqrt is inexact */
			factlim = ztofull(tmp)+1;
		}
		zfree(tmp);

		/* avoid highest factor */
		if (factlim >= MAX_SM_PRIME) {
			factlim = MAX_SM_PRIME-1;
		}
	} else {
		/* determine our factor limit */
		factlim = fsqrt(ztofull(z));
	}
	if (factlim >= MAX_SM_PRIME) {
		factlim = MAX_SM_PRIME-1;
	}

	/*
	 * walk the prime table looking for factors
	 */
	tlim = (HALF)((factlim >= MAX_MAP_PRIME) ? MAX_MAP_PRIME-1 : factlim);
	div.sign = 0;
	div.v = divval;
	div.len = 1;
	for (p=prime, --count; count > 0 && (HALF)*p <= tlim; ++p, --count) {

		/* setup factor */
		div.v[0] = (HALF)(*p);

		if (zdivides(z, div))
			return (FULL)(*p);
	}
	if (count <= 0 || (FULL)*p > factlim) {
		/* no factor found */
		return (FULL)1;
	}

	/*
	 * test the highest factor possible
	 */
	div.v[0] = MAX_MAP_PRIME;
	if (zdivides(z, div))
		return (FULL)MAX_MAP_PRIME;

	/*
	 * generate higher test factors as needed
	 */
#if BASEB == 16
	div.len = 2;
#endif
	for(factor = NXT_MAP_PRIME;
	    count > 0 && factor <= factlim;
	    factor = next_prime(factor), --count) {

		/* setup factor */
#if BASEB == 32
		div.v[0] = (HALF)factor;
#else
		div.v[0] = (HALF)(factor & BASE1);
		div.v[1] = (HALF)(factor >> BASEB);
#endif

		if (zdivides(z, div))
			return (FULL)(factor);
	}
	if (count <= 0 || factor >= factlim) {
		/* no factor found */
		return (FULL)1;
	}

	/*
	 * test the highest factor possible
	 */
#if BASEB == 32
	div.v[0] = MAX_SM_PRIME;
#else
	div.v[0] = (MAX_SM_PRIME & BASE1);
	div.v[1] = (MAX_SM_PRIME >> BASEB);
#endif
	if (zdivides(z, div))
		return (FULL)MAX_SM_PRIME;

	/*
	 * no factor found
	 */
	return (FULL)1;
}


/*
 * Compute the least common multiple of all the numbers up to the
 * specified number.
 */
void
zlcmfact(ZVALUE z, ZVALUE *dest)
{
	long n;				/* limiting number to multiply by */
	long p;				/* current prime */
	long pp = 0;			/* power of prime */
	long i;				/* test value */
	CONST unsigned short *pr;	/* pointer to a small prime */
	ZVALUE res, temp;

	if (zisneg(z) || ziszero(z)) {
		math_error("Non-positive argument for lcmfact");
		/*NOTREACHED*/
	}
	if (zge24b(z)) {
		math_error("Very large lcmfact");
		/*NOTREACHED*/
	}
	n = ztolong(z);
	/*
	 * Multiply by powers of the necessary odd primes in order.
	 * The power for each prime is the highest one which is not
	 * more than the specified number.
	 */
	res = _one_;
	for (pr=prime; (long)(*pr) <= n && *pr > 1; ++pr) {
		i = p = *pr;
		while (i <= n) {
			pp = i;
			i *= p;
		}
		zmuli(res, pp, &temp);
		zfree(res);
		res = temp;
	}
	for (p = NXT_MAP_PRIME; p <= n; p = (long)next_prime(p)) {
		i = p;
		while (i <= n) {
			pp = i;
			i *= p;
		}
		zmuli(res, pp, &temp);
		zfree(res);
		res = temp;
	}
	/*
	 * Finish by scaling by the necessary power of two.
	 */
	zshift(res, zhighbit(z), dest);
	zfree(res);
}


/*
 * fsqrt - fast square root of a FULL value
 *
 * We will determine the square root of a given value.
 * Starting with the integer square root of the largest power of
 * two <= the value, we will perform 3 Newton iterations to
 * arrive at our guess.
 *
 * We have verified that fsqrt(x) == (FULL)sqrt((double)x), or
 * fsqrt(x)-1 == (FULL)sqrt((double)x) for all 0 <= x < 2^32.
 *
 * given:
 *	x		compute the integer square root of x
 */
S_FUNC FULL
fsqrt(FULL x)
{
	FULL y;		/* (FULL)temporary value */
	int i;

	/* firewall - deal with 0 */
	if (x == 0) {
	    return 0;
	}

	/* ignore Saber-C warning #530 about empty for statement */
	/*	  ok to ignore in proc fsqrt */
	/* determine our initial guess */
	for (i=0, y=x; y >= (FULL)256; i+=8, y>>=8) {
	}
	y = isqrt_pow2[i + topbit[y]];

	/* perform 3 Newton interactions */
	y = (y+x/y)>>1;
	y = (y+x/y)>>1;
	y = (y+x/y)>>1;
#if FULL_BITS == 64
	y = (y+x/y)>>1;
#endif

	/* return the result */
	return y;
}
