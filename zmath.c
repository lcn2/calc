/*
 * zmath - extended precision integral arithmetic primitives
 *
 * Copyright (C) 1999-2007,2021  David I. Bell and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:28
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "alloc.h"
#include "zmath.h"


#include "banned.h"	/* include after system header <> includes */


HALF _zeroval_[] = { 0 };
HALF _oneval_[] = { 1 };
HALF _twoval_[] = { 2 };
HALF _threeval_[] = { 3 };
HALF _fourval_[] = { 4 };
HALF _fiveval_[] = { 5 };
HALF _sixval_[] = { 6 };
HALF _sevenval_[] = { 7 };
HALF _eightval_[] = { 8 };
HALF _nineval_[] = { 9 };
HALF _tenval_[] = { 10 };
HALF _elevenval_[] = { 11 };
HALF _twelveval_[] = { 12 };
HALF _thirteenval_[] = { 13 };
HALF _fourteenval_[] = { 14 };
HALF _fifteenval_[] = { 15 };
HALF _sixteenval_[] = { 16 };
HALF _seventeenval_[] = { 17 };
HALF _eightteenval_[] = { 18 };
HALF _nineteenval_[] = { 19 };
HALF _twentyval_[] = { 20 };
HALF _sqbaseval_[] = { 0, 1 };
HALF _pow4baseval_[] = { 0, 0, 1 };
HALF _pow8baseval_[] = { 0, 0, 0, 0, 1 };

ZVALUE zconst[] = {
    { _zeroval_, 1, 0}, { _oneval_, 1, 0}, { _twoval_, 1, 0},
    { _threeval_, 1, 0}, { _fourval_, 1, 0}, { _fiveval_, 1, 0},
    { _sixval_, 1, 0}, { _sevenval_, 1, 0}, { _eightval_, 1, 0},
    { _nineval_, 1, 0}, { _tenval_, 1, 0}, { _elevenval_, 1, 0},
    { _twelveval_, 1, 0}, { _thirteenval_, 1, 0}, { _fourteenval_, 1, 0},
    { _fifteenval_, 1, 0}, { _sixteenval_, 1, 0}, { _seventeenval_, 1, 0},
    { _eightteenval_, 1, 0}, { _nineteenval_, 1, 0}, { _twentyval_, 1, 0}
};

ZVALUE _zero_ = { _zeroval_, 1, 0};
ZVALUE _one_ = { _oneval_, 1, 0 };
ZVALUE _two_ = { _twoval_, 1, 0 };
ZVALUE _ten_ = { _tenval_, 1, 0 };
ZVALUE _sqbase_ = { _sqbaseval_, 2, 0 };
ZVALUE _pow4base_ = { _pow4baseval_, 4, 0 };
ZVALUE _pow8base_ = { _pow8baseval_, 4, 0 };
ZVALUE _neg_one_ = { _oneval_, 1, 1 };

/*
 * 2^64 as a ZVALUE
 */
#if BASEB == 32
ZVALUE _b32_ = { _sqbaseval_, 2, 0 };
ZVALUE _b64_ = { _pow4baseval_, 3, 0 };
#elif BASEB == 16
ZVALUE _b32_ = { _pow4baseval_, 3, 0 };
ZVALUE _b64_ = { _pow8baseval_, 5, 0 };
#else
	-=@=- BASEB not 16 or 32 -=@=-
#endif


/*
 * highhalf[i] - masks off the upper i bits of a HALF
 * rhighhalf[i] - masks off the upper BASEB-i bits of a HALF
 * lowhalf[i] - masks off the upper i bits of a HALF
 * rlowhalf[i] - masks off the upper BASEB-i bits of a HALF
 * bitmask[i] - (1 << i) for  0 <= i <= BASEB*2
 */
HALF highhalf[BASEB+1] = {
#if BASEB == 32
	0x00000000,
	0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
	0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
	0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
	0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
	0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
	0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
	0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
	0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF
#elif BASEB == 16
	0x0000,
	0x8000, 0xC000, 0xE000, 0xF000,
	0xF800, 0xFC00, 0xFE00, 0xFF00,
	0xFF80, 0xFFC0, 0xFFE0, 0xFFF0,
	0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF
#else
	-=@=- BASEB not 16 or 32 -=@=-
#endif
};
HALF rhighhalf[BASEB+1] = {
#if BASEB == 32
	0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
	0xFFFFFFF0, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
	0xFFFFFF00, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
	0xFFFFF000, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
	0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
	0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
	0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
	0xF0000000, 0xE0000000, 0xC0000000, 0x80000000,
	0x00000000
#elif BASEB == 16
	0xFFFF, 0xFFFE, 0xFFFC, 0xFFF8,
	0xFFF0, 0xFFE0, 0xFFC0, 0xFF80,
	0xFF00, 0xFE00, 0xFC00, 0xF800,
	0xF000, 0xE000, 0xC000, 0x8000,
	0x0000
#else
	-=@=- BASEB not 16 or 32 -=@=-
#endif
};
HALF lowhalf[BASEB+1] = {
	0x0,
	0x1,  0x3, 0x7, 0xF,
	0x1F,  0x3F, 0x7F, 0xFF,
	0x1FF,	0x3FF, 0x7FF, 0xFFF,
	0x1FFF,	 0x3FFF, 0x7FFF, 0xFFFF
#if BASEB == 32
       ,0x1FFFF,  0x3FFFF, 0x7FFFF, 0xFFFFF,
	0x1FFFFF,  0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
	0x1FFFFFF,  0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
	0x1FFFFFFF,  0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
#endif
};
HALF rlowhalf[BASEB+1] = {
#if BASEB == 32
	0xFFFFFFFF, 0x7FFFFFFF, 0x3FFFFFFF, 0x1FFFFFFF,
	0xFFFFFFF, 0x7FFFFFF, 0x3FFFFFF, 0x1FFFFFF,
	0xFFFFFF, 0x7FFFFF, 0x3FFFFF, 0x1FFFFF,
	0xFFFFF, 0x7FFFF, 0x3FFFF, 0x1FFFF,
#endif
	0xFFFF, 0x7FFF, 0x3FFF, 0x1FFF,
	0xFFF, 0x7FF, 0x3FF, 0x1FF,
	0xFF, 0x7F, 0x3F, 0x1F,
	0xF, 0x7, 0x3, 0x1,
	0x0
};
HALF bitmask[(2*BASEB)+1] = {
#if BASEB == 32
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x00000001
#elif BASEB == 16
	0x0001, 0x0002, 0x0004, 0x0008,
	0x0010, 0x0020, 0x0040, 0x0080,
	0x0100, 0x0200, 0x0400, 0x0800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x0001, 0x0002, 0x0004, 0x0008,
	0x0010, 0x0020, 0x0040, 0x0080,
	0x0100, 0x0200, 0x0400, 0x0800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x0001
#else
	-=@=- BASEB not 16 or 32 -=@=-
#endif
};		/* actual rotation thru 8 cycles */

BOOL _math_abort_;		/* nonzero to abort calculations */


/*
 * popcnt - popcnt[x] number of 1 bits in 0 <= x < 256
 */
char popcnt[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};



#ifdef ALLOCTEST
STATIC long nalloc = 0;
STATIC long nfree = 0;
#endif


HALF *
alloc(LEN len)
{
	HALF *hp;

	if (_math_abort_) {
		math_error("Calculation aborted");
		/*NOTREACHED*/
	}
	hp = (HALF *) malloc((len+1) * sizeof(HALF));
	if (hp == 0) {
		math_error("Not enough memory");
		/*NOTREACHED*/
	}
#ifdef ALLOCTEST
	++nalloc;
#endif
	return hp;
}


#ifdef ALLOCTEST
void
freeh(HALF *h)
{
	if ((h != _zeroval_) && (h != _oneval_)) {
		free(h);
		++nfree;
	}
}


void
allocStat(void)
{
	fprintf(stderr, "nalloc: %ld nfree: %ld kept: %ld\n",
		nalloc, nfree, nalloc - nfree);
}
#endif


/*
 * Convert a normal integer to a number.
 */
void
itoz(long i, ZVALUE *res)
{
	long diddle, len;

	res->len = 1;
	res->sign = 0;
	diddle = 0;
	if (i == 0) {
		res->v = _zeroval_;
		return;
	}
	if (i < 0) {
		res->sign = 1;
		i = -i;
		if (i < 0) {	/* fix most negative number */
			diddle = 1;
			i--;
		}
	}
	if (i == 1) {
		res->v = _oneval_;
		return;
	}
	len = 1 + (((FULL) i) >= BASE);
	res->len = (LEN)len;
	res->v = alloc((LEN)len);
	res->v[0] = (HALF) (i + diddle);
	if (len == 2)
		res->v[1] = (HALF) (i / BASE);
}


/*
 * Convert a number to a normal integer, as far as possible.
 * If the number is out of range, the largest number is returned.
 */
long
ztoi(ZVALUE z)
{
	long i;

	if (zgtmaxlong(z)) {
		i = MAXLONG;
		return (z.sign ? -i : i);
	}
	i = ztolong(z);
	return (z.sign ? -i : i);
}


/*
 * Convert a normal unsigned integer to a number.
 */
void
utoz(FULL i, ZVALUE *res)
{
	long len;

	res->len = 1;
	res->sign = 0;
	if (i == 0) {
		res->v = _zeroval_;
		return;
	}
	if (i == 1) {
		res->v = _oneval_;
		return;
	}
	len = 1 + (((FULL) i) >= BASE);
	res->len = (LEN)len;
	res->v = alloc((LEN)len);
	res->v[0] = (HALF)i;
	if (len == 2)
		res->v[1] = (HALF) (i / BASE);
}


/*
 * Convert a normal signed integer to a number.
 */
void
stoz(SFULL i, ZVALUE *res)
{
	long diddle, len;

	res->len = 1;
	res->sign = 0;
	diddle = 0;
	if (i == 0) {
		res->v = _zeroval_;
		return;
	}
	if (i < 0) {
		res->sign = 1;
		i = -i;
		if (i < 0) {	/* fix most negative number */
			diddle = 1;
			i--;
		}
	}
	if (i == 1) {
		res->v = _oneval_;
		return;
	}
	len = 1 + (((FULL) i) >= BASE);
	res->len = (LEN)len;
	res->v = alloc((LEN)len);
	res->v[0] = (HALF) (i + diddle);
	if (len == 2)
		res->v[1] = (HALF) (i / BASE);
}


/*
 * Convert a number to a unsigned normal integer, as far as possible.
 * If the number is out of range, the largest number is returned.
 * The absolute value of z is converted.
 */
FULL
ztou(ZVALUE z)
{
	if (z.len > 2) {
		return MAXUFULL;
	}
	return ztofull(z);
}


/*
 * Convert a number to a signed normal integer, as far as possible.
 *
 * If the number is too large to fit into an integer, than the largest
 * positive or largest negative integer is returned depending on the sign.
 */
SFULL
ztos(ZVALUE z)
{
	FULL val;	/* absolute value of the return value */

	/* negative value processing */
	if (z.sign) {
	    if (z.len > 2) {
		return MINSFULL;
	    }
	    val = ztofull(z);
	    if (val > TOPFULL) {
		return MINSFULL;
	    }
	    return (SFULL)(-val);
	}

	/* positive value processing */
	if (z.len > 2) {
	    return (SFULL)MAXFULL;
	}
	val = ztofull(z);
	if (val > MAXFULL) {
	    return (SFULL)MAXFULL;
	}
	return (SFULL)val;
}


/*
 * Make a copy of an integer value
 */
void
zcopy(ZVALUE z, ZVALUE *res)
{
	res->sign = z.sign;
	res->len = z.len;
	if (zisabsleone(z)) {	/* zero or plus or minus one are easy */
		res->v = (z.v[0] ? _oneval_ : _zeroval_);
		return;
	}
	res->v = alloc(z.len);
	zcopyval(z, *res);
}


/*
 * Add together two integers.
 */
void
zadd(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE dest;
	HALF *p1, *p2, *pd;
	long len;
	FULL carry;
	SIUNION sival;

	if (z1.sign && !z2.sign) {
		z1.sign = 0;
		zsub(z2, z1, res);
		return;
	}
	if (z2.sign && !z1.sign) {
		z2.sign = 0;
		zsub(z1, z2, res);
		return;
	}
	if (z2.len > z1.len) {
		pd = z1.v; z1.v = z2.v; z2.v = pd;
		len = z1.len; z1.len = z2.len; z2.len = (LEN)len;
	}
	dest.len = z1.len + 1;
	dest.v = alloc(dest.len);
	dest.sign = z1.sign;
	carry = 0;
	pd = dest.v;
	p1 = z1.v;
	p2 = z2.v;
	len = z2.len;
	while (len--) {
		sival.ivalue = ((FULL) *p1++) + ((FULL) *p2++) + carry;
		/* ignore Saber-C warning #112 - get ushort from uint */
		/*	  ok to ignore on name zadd`sival */
		*pd++ = sival.silow;
		carry = sival.sihigh;
	}
	len = z1.len - z2.len;
	while (len--) {
		sival.ivalue = ((FULL) *p1++) + carry;
		*pd++ = sival.silow;
		carry = sival.sihigh;
	}
	*pd = (HALF)carry;
	zquicktrim(dest);
	*res = dest;
}


/*
 * Subtract two integers.
 */
void
zsub(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	register HALF *h1, *h2, *hd;
	long len1, len2;
	FULL carry;
	SIUNION sival;
	ZVALUE dest;

	if (z1.sign != z2.sign) {
		z2.sign = z1.sign;
		zadd(z1, z2, res);
		return;
	}
	len1 = z1.len;
	len2 = z2.len;
	if (len1 == len2) {
		h1 = z1.v + len1;
		h2 = z2.v + len2;
		while ((len1 > 0) && ((FULL)*--h1 == (FULL)*--h2)) {
			len1--;
		}
		if (len1 == 0) {
			*res = _zero_;
			return;
		}
		len2 = len1;
		carry = ((FULL)*h1 < (FULL)*h2);
	} else {
		carry = (len1 < len2);
	}
	dest.sign = z1.sign;
	h1 = z1.v;
	h2 = z2.v;
	if (carry) {
		carry = len1;
		len1 = len2;
		len2 = (long)carry;
		h1 = z2.v;
		h2 = z1.v;
		dest.sign = !dest.sign;
	}
	hd = alloc((LEN)len1);
	dest.v = hd;
	dest.len = (LEN)len1;
	len1 -= len2;
	carry = 0;
	while (--len2 >= 0) {
		/* ignore Saber-C warning #112 - get ushort from uint */
		/*	  ok to ignore on name zsub`sival */
		sival.ivalue = (BASE1 - ((FULL) *h1++)) + *h2++ + carry;
		*hd++ = (HALF)(BASE1 - sival.silow);
		carry = sival.sihigh;
	}
	while (--len1 >= 0) {
		sival.ivalue = (BASE1 - ((FULL) *h1++)) + carry;
		*hd++ = (HALF)(BASE1 - sival.silow);
		carry = sival.sihigh;
	}
	if (hd[-1] == 0)
		ztrim(&dest);
	*res = dest;
}


/*
 * Multiply an integer by a small number.
 */
void
zmuli(ZVALUE z, long n, ZVALUE *res)
{
	register HALF *h1, *sd;
	FULL low;
	FULL high;
	FULL carry;
	long len;
	SIUNION sival;
	ZVALUE dest;

	if ((n == 0) || ziszero(z)) {
		*res = _zero_;
		return;
	}
	if (n < 0) {
		n = -n;
		z.sign = !z.sign;
	}
	if (n == 1) {
		zcopy(z, res);
		return;
	}
#if LONG_BITS > BASEB
	low = ((FULL) n) & BASE1;
	high = ((FULL) n) >> BASEB;
#else
	low = (FULL)n;
	high = 0;
#endif
	dest.len = z.len + 2;
	dest.v = alloc(dest.len);
	dest.sign = z.sign;
	/*
	 * Multiply by the low digit.
	 */
	h1 = z.v;
	sd = dest.v;
	len = z.len;
	carry = 0;
	while (len--) {
		/* ignore Saber-C warning #112 - get ushort from uint */
		/*	  ok to ignore on name zmuli`sival */
		sival.ivalue = ((FULL) *h1++) * low + carry;
		*sd++ = sival.silow;
		carry = sival.sihigh;
	}
	*sd = (HALF)carry;
	/*
	 * If there was only one digit, then we are all done except
	 * for trimming the number if there was no last carry.
	 */
	if (high == 0) {
		dest.len--;
		if (carry == 0)
			dest.len--;
		*res = dest;
		return;
	}
	/*
	 * Need to multiply by the high digit and add it into the
	 * previous value.  Clear the final word of rubbish first.
	 */
	*(++sd) = 0;
	h1 = z.v;
	sd = dest.v + 1;
	len = z.len;
	carry = 0;
	while (len--) {
		sival.ivalue = ((FULL) *h1++) * high + ((FULL) *sd) + carry;
		*sd++ = sival.silow;
		carry = sival.sihigh;
	}
	*sd = (HALF)carry;
	zquicktrim(dest);
	*res = dest;
}


/*
 * Divide two numbers by their greatest common divisor.
 * This is useful for reducing the numerator and denominator of
 * a fraction to its lowest terms.
 */
void
zreduce(ZVALUE z1, ZVALUE z2, ZVALUE *z1res, ZVALUE *z2res)
{
	ZVALUE tmp;

	if (zisabsleone(z1) || zisabsleone(z2))
		tmp = _one_;
	else
		zgcd(z1, z2, &tmp);
	if (zisunit(tmp)) {
		zcopy(z1, z1res);
		zcopy(z2, z2res);
	} else {
		zequo(z1, tmp, z1res);
		zequo(z2, tmp, z2res);
	}
	zfree(tmp);
}



/*
 * Compute the quotient and remainder for division of an integer by an
 * integer, rounding criteria determined by rnd.  Returns the sign of
 * the remainder.
 */
long
zdiv(ZVALUE z1, ZVALUE z2, ZVALUE *quo, ZVALUE *rem, long rnd)
{
	register HALF *a, *b;
	HALF s, u;
	HALF *A, *B, *a1, *b0;
	FULL f, g, h, x;
	BOOL adjust, onebit;
	LEN m, n, len, i, p, j1, j2, k;
	long t, val;

	if (ziszero(z2)) {
		math_error("Division by zero in zdiv");
		/*NOTREACHED*/
	}
	m = z1.len;
	n = z2.len;
	B = z2.v;
	s = 0;
	if (m < n) {
		A = alloc(n + 1);
		memcpy(A, z1.v, m * sizeof(HALF));
		for (i = m; i <= n; i++)
			A[i] = 0;
		a1 = A + n;
		len = 1;
		goto done;
	}
	A = alloc(m + 2);
	memcpy(A, z1.v, m * sizeof(HALF));
	A[m] = 0;
	A[m + 1] = 0;
	len = m - n + 1;	/* quotient length will be len or len +/- 1 */
	a1 = A + n;		/* start of digits for quotient */
	b0 = B;
	p = n;
	while (!*b0) {		/* b0: working start for divisor */
		++b0;
		--p;
	}
	if (p == 1) {
		u = *b0;
		if (u == 1) {
			for (; m >= n; m--)
				A[m] = A[m - 1];
			A[m] = 0;
			goto done;
		}
		f = 0;
		a = A + m;
		i = len;
		while (i--) {
			f = f << BASEB | *--a;
			a[1] = (HALF)(f / u);
			f = f % u;
		}
		*a = (HALF)f;
		m = n;
		goto done;
	}
	f = B[n - 1];
	k = 1;
	while (f >>= 1)
		k++;		/* k: number of bits in top divisor digit */
	j1 = BASEB - k;
	j2 = BASEB + j1;
	h = j1 ? ((FULL) B[n - 1] << j1 | B[n - 2] >> k) : B[n-1];
	onebit = (BOOL)((B[n - 2] >> (k - 1)) & 1);
	m++;
	while (m > n) {
		m--;
		f = (FULL) A[m] << j2 | (FULL) A[m - 1] << j1;
		if (j1) f |= A[m - 2] >> k;
		if (s) f = ~f;
		x = f / h;
		if (x) {
			if (onebit && x > TOPHALF + f % h)
				x--;
			a = A + m - p;
			b = b0;
			u = 0;
			i = p;
			if (s) {
				while (i--) {
					f = (FULL) *a + u + x * *b++;
					*a++ = (HALF) f;
					u = (HALF) (f >> BASEB);
				}
				s = *a + u;
				A[m] = (HALF) (~x + !s);
			} else {
				while (i--) {
					f = (FULL) *a - u - x * *b++;
					*a++ = (HALF) f;
					u = -(HALF)(f >> BASEB);
				}
				s = *a - u;
				A[m] = (HALF)(x + s);
			}
		}
		else
			A[m] = s;
	}
done:	while (m > 0 && A[m - 1] == 0)
		m--;
	if (m == 0 && s == 0) {
		*rem = _zero_;
		val = 0;
		if (a1[len - 1] == 0)
			len--;
		if (len == 0) {
			*quo = _zero_;
		} else {
			quo->len = len;
			quo->v = alloc(len);
			memcpy(quo->v, a1, len * sizeof(HALF));
			quo->sign = z1.sign ^ z2.sign;
		}
		freeh(A);
		return val;
	}
	if (rnd & 8)
		adjust = (((*a1 ^ rnd) & 1) ? TRUE : FALSE);
	else
		adjust = (((rnd & 1) ^ z1.sign ^ z2.sign) ? TRUE : FALSE);
	if (rnd & 2)
		adjust ^= z1.sign ^ z2.sign;
	if (rnd & 4)
		adjust ^= z2.sign;
	if (rnd & 16) {			/* round-off case */
		a = A + n;
		b = B + n;
		i = n + 1;
		f = g = 0;
		t = -1;
		if (s) {
			while (--i > 0) {
				g = (FULL) *--a + (*--b >> 1 | f);
				f = *b & 1 ? TOPHALF : 0;
				if (g != BASE1)
					break;
			}
			if (g == BASE && f == 0) {
				while ((--i > 0) && ((*--a | *--b) == 0));
				t = (i > 0);
			} else if (g >= BASE) {
				t = 1;
			}
		} else {
			while (--i > 0) {
				g = (FULL) *--a - (*--b >> 1 | f);
				f = *b & 1 ? TOPHALF : 0;
				if (g != 0)
					break;
			}
			if (g > 0 && g < BASE)
				t = 1;
			else if (g == 0 && f == 0)
				t = 0;
		}
		if (t)
			adjust = (t > 0);
	}
	if (adjust) {
		i = len;
		a = a1;
		while (i > 0 && *a == BASE1) {
			i--;
			*a++ = 0;
		}
		(*a)++;
		if (i == 0)
			len++;
	}
	if (s && adjust) {
		i = 0;
		while (A[i] == 0)
			i++;
		A[i] = -A[i];
		while (++i < n)
			A[i] = ~A[i];
		m = n;
		while (A[m - 1] == 0)
			m--;
	}
	if (!s && adjust) {
		a = A;
		b = B;
		i = n;
		u = 0;
		while (i--) {
			f = (FULL) *b++ - *a - (FULL) u;
			*a++ = (HALF) f;
			u = -(HALF)(f >> BASEB);
		}
		m = n;
		while (A[m - 1] == 0)
			m--;
	}
	if (s && !adjust) {
		a = A;
		b = B;
		i = n;
		f = 0;
		while (i--) {
			f = (FULL) *b++ + *a + (f >> BASEB);
			*a++ = (HALF) f;
		}
		m = n;
		while (A[m-1] == 0)
			m--;
	}
	rem->len = m;
	rem->v = alloc(m);
	memcpy(rem->v, A, m * sizeof(HALF));
	rem->sign = z1.sign ^ adjust;
	val = rem->sign ? -1 : 1;
	if (a1[len - 1] == 0)
		len--;
	if (len == 0) {
		*quo = _zero_;
	} else {
		quo->len = len;
		quo->v = alloc(len);
		memcpy(quo->v, a1, len * sizeof(HALF));
		quo->sign = z1.sign ^ z2.sign;
	}
	freeh(A);
	return val;
}


/*
 * Compute and store at a specified location the integer quotient
 * of two integers, the type of rounding being determined by rnd.
 * Returns the sign of z1/z2 - calculated quotient.
 */
long
zquo(ZVALUE z1, ZVALUE z2, ZVALUE *res, long rnd)
{
	ZVALUE tmp;
	long val;

	val = zdiv(z1, z2, res, &tmp, rnd);
	if (z2.sign)
		val = -val;
	zfree(tmp);
	return val;
}


/*
 * Compute and store at a specified location the remainder for
 * division of an integer by an integer, the type of rounding
 * used being determined by rnd.  Returns the sign of the remainder.
 */
long
zmod(ZVALUE z1, ZVALUE z2, ZVALUE *res, long rnd)
{
	ZVALUE tmp;
	long val;

	val = zdiv(z1, z2, &tmp, res, rnd);
	zfree(tmp);
	return val;
}


/*
 * Computes the quotient z1/z2 on the assumption that this is exact.
 * There is no thorough check on the exactness of the division
 * so this should not be called unless z1/z2 is an integer.
 */
void
zequo(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	LEN i, j, k, len, m, n, o, p;
	HALF *a, *a0, *A, *b, *B, u, v, w, x;
	FULL f, g;

	if (ziszero(z1)) {
		*res = _zero_;
		return;
	}
	if (ziszero(z2)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (zisone(z2)) {
		zcopy(z1, res);
		return;
	}
	if (zhighbit(z1) < zhighbit(z2)) {
		math_error("Bad call to zequo");
		/*NOTREACHED*/
	}
	B = z2.v;
	o = 0;
	while (!*B) {
		++B;
		++o;
	}
	m = z1.len - o;
	n = z2.len - o;
	len = m - n + 1;		/* Maximum length of quotient */
	v = *B;
	A = alloc(len+1);
	memcpy(A, z1.v + o, len * sizeof(HALF));
	A[len] = 0;
	if (n == 1) {
		if (v > 1) {
			a = A + len;
			i = len;
			f = 0;
			while (i--) {
				f = f << BASEB | *--a;
				*a = (HALF)(f / v);
				f %= v;
			}
		}
	} else {
		k = 0;
		while (!(v & 1)) {
			k++;
			v >>= 1;
		}
		j = BASEB - k;
		if (n > 1 && k > 0)
			v |= B[1] << j;
		u = v - 1;
		w = x = 1;
		while (u) {	/* To find w = inverse of v modulo BASE */
			do {
				v <<= 1;
				x <<= 1;
			}
			while (!(u & x));
			u += v;
			w |= x;
		}
		a0 = A;
		p = len;
		while (p > 1) {
			if (!*a0) {
				while (!*++a0 && p > 1)
					p--;
				--a0;
			}
			if (p == 1)
				break;
			x = k ? w * (*a0 >> k | a0[1] << j) : w * *a0;
			g = x;
			if (x) {
				a = a0;
				b = B;
				u = 0;
				i = n;
				if (i > p)
					i = p;
				while (i--) {
					f = (FULL) *a - g * *b++ - (FULL) u;
					*a++ = (HALF)f;
					u = -(HALF)(f >> BASEB);
				}
				if (u && p > n) {
					i = p - n;
					while (u && i--) {
						f = (FULL) *a - u;
						*a++ = (HALF) f;
						u = -(HALF)(f >> BASEB);
					}
				}
			}
			*a0++ = x;
			p--;
		}
		if (k == 0) {
			*a0 = w * *a0;
		} else {
			u = (HALF)(w * *a0) >> k;
			x = (HALF)(((FULL) z1.v[z1.len - 1] << BASEB
				| z1.v[z1.len - 2])
				/((FULL) B[n-1] << BASEB | B[n-2]));
			if ((x ^ u) & 1) x--;
			*a0 = x;
		}
	}
	if (!A[len - 1]) len--;
	res->v = A;
	res->len = len;
	res->sign = z1.sign != z2.sign;
}



/*
 * Return the quotient and remainder of an integer divided by a small
 * number.  A nonzero remainder is only meaningful when both numbers
 * are positive.
 */
long
zdivi(ZVALUE z, long n, ZVALUE *res)
{
	HALF *h1, *sd;
	FULL val;
	HALF divval[2];
	ZVALUE div;
	ZVALUE dest;
	LEN len;

	if (n == 0) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (ziszero(z)) {
		*res = _zero_;
		return 0;
	}
	if (n < 0) {
		n = -n;
		z.sign = !z.sign;
	}
	if (n == 1) {
		zcopy(z, res);
		return 0;
	}
	/*
	 * If the division is by a large number, then call the normal
	 * divide routine.
	 */
	if (n & ~BASE1) {
		div.sign = 0;
		div.v = divval;
		divval[0] = (HALF) n;
#if LONG_BITS > BASEB
		divval[1] = (HALF)(((FULL) n) >> BASEB);
		div.len = 2;
#else
		div.len = 1;
#endif
		zdiv(z, div, res, &dest, 0);
		n = ztolong(dest);
		zfree(dest);
		return n;
	}
	/*
	 * Division is by a small number, so we can be quick about it.
	 */
	len = z.len;
	dest.sign = z.sign;
	dest.len = len;
	dest.v = alloc(len);
	h1 = z.v + len;
	sd = dest.v + len;
	val = 0;
	while (len--) {
		val = ((val << BASEB) + ((FULL) *--h1));
		*--sd = (HALF)(val / n);
		val %= n;
	}
	zquicktrim(dest);
	*res = dest;
	return (long)val;
}



/*
 * Calculate the mod of an integer by a small number.
 * This is only defined for positive moduli.
 */
long
zmodi(ZVALUE z, long n)
{
	register HALF *h1;
	FULL val;
	HALF divval[2];
	ZVALUE div;
	ZVALUE temp;
	long len;

	if (n == 0) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (n < 0) {
		math_error("Non-positive modulus");
		/*NOTREACHED*/
	}
	if (ziszero(z) || (n == 1))
		return 0;
	if (zisone(z))
		return 1;
	/*
	 * If the modulus is by a large number, then call the normal
	 * modulo routine.
	 */
	if (n & ~BASE1) {
		div.sign = 0;
		div.v = divval;
		divval[0] = (HALF) n;
#if LONG_BITS > BASEB
		divval[1] = (HALF)(((FULL) n) >> BASEB);
		div.len = 2;
#else
		div.len = 1;
#endif
		zmod(z, div, &temp, 0);
		n = ztolong(temp);
		zfree(temp);
		return n;
	}
	/*
	 * The modulus is by a small number, so we can do this quickly.
	 */
	len = z.len;
	h1 = z.v + len;
	val = 0;
	while (len-- > 0)
		val = ((val << BASEB) + ((FULL) *--h1)) % n;
	if (val && z.sign)
		val = n - val;
	return (long)val;
}


/*
 * Return whether or not one number exactly divides another one.
 * Returns TRUE if division occurs with no remainder.
 * z1 is the number to be divided by z2.
 */

BOOL
zdivides(ZVALUE z1, ZVALUE z2)
{
	LEN i, j, k, m, n;
	HALF u, v, w, x;
	HALF *a, *a0, *A, *b, *B, *c, *d;
	FULL f;
	BOOL ans;

	if (zisunit(z2) || ziszero(z1)) return TRUE;
	if (ziszero(z2)) return FALSE;

	m = z1.len;
	n = z2.len;
	if (m < n) return FALSE;

	c = z1.v;
	d = z2.v;

	while (!*d) {
		if (*c++) return FALSE;
		d++;
		m--;
		n--;
	}

	j = 0;
	u = *c;
	v = *d;
	while (!(v & 1)) {	/* Counting and checking zero bits */
		if (u & 1) return FALSE;
		u >>= 1;
		v >>= 1;
		j++;
	}

	if (n == 1 && v == 1) return TRUE;
	if (!*c) {			/* Removing any further zeros */
		while(!*++c)
			m--;
		c--;
	}

	if (m < n) return FALSE;

	if (j) {
		B = alloc((LEN)n);	/* Array for shifted z2 */
		d += n;
		b = B + n;
		i = n;
		f = 0;
		while(i--) {
			f = f << BASEB | *--d;
			*--b = (HALF)(f >> j);
		}
		if (!B[n - 1]) n--;
	}
	else B = d;
	u = *B;
	v = x = 1;
	w = 0;
	while (x) {			/* Finding minv(*B, BASE) */
		if (v & x) {
			v -= x * u;
			w |= x;
		}
		x <<= 1;
	}

	A = alloc((LEN)(m + 2));	/* Main working array */
	memcpy(A, c, m * sizeof(HALF));
	A[m + 1] = 1;
	A[m] = 0;

	k = m - n + 1;			/* Length of presumed quotient */

	a0 = A;

	while (k--) {
		if (*a0) {
			x = w * *a0;
			a = a0;
			b = B;
			i = n;
			u = 0;
			while (i--) {
				f = (FULL) *a - (FULL) x * *b++ - u;
				*a++ = (HALF)f;
				u = -(HALF)(f >> BASEB);
			}
			f = (FULL) *a - u;
			*a++ = (HALF)f;
			u = -(HALF)(f >> BASEB);
			if (u) {
				while (*a == 0) *a++ = BASE1;
				(*a)--;
			}
		}
		a0++;
	}
	ans = FALSE;
	if (A[m + 1]) {
		a = A + m;
		while (--n && !*--a);
		if (!n) ans = TRUE;
	}
	freeh(A);
	if (j) freeh(B);
	return ans;
}


/*
 * Compute the bitwise OR of two integers
 */
void
zor(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	register HALF *sp, *dp;
	long len;
	ZVALUE bz, lz, dest;

	if (z1.len >= z2.len) {
		bz = z1;
		lz = z2;
	} else {
		bz = z2;
		lz = z1;
	}
	dest.len = bz.len;
	dest.v = alloc(dest.len);
	dest.sign = 0;
	zcopyval(bz, dest);
	len = lz.len;
	sp = lz.v;
	dp = dest.v;
	while (len--)
		*dp++ |= *sp++;
	*res = dest;
}


/*
 * Compute the bitwise AND of two integers
 */
void
zand(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	HALF *h1, *h2, *hd;
	LEN len;
	ZVALUE dest;

	len = ((z1.len <= z2.len) ? z1.len : z2.len);
	h1 = &z1.v[len-1];
	h2 = &z2.v[len-1];
	while ((len > 1) && ((*h1 & *h2) == 0)) {
		h1--;
		h2--;
		len--;
	}
	dest.len = len;
	dest.v = alloc(len);
	dest.sign = 0;
	h1 = z1.v;
	h2 = z2.v;
	hd = dest.v;
	while (len--)
		*hd++ = (*h1++ & *h2++);
	*res = dest;
}


/*
 * Compute the bitwise XOR of two integers.
 */
void
zxor(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	HALF *dp, *h1, *h2;
	LEN len, j, k;
	ZVALUE dest;

	h1 = z1.v;
	h2 = z2.v;
	len = z1.len;
	j = z2.len;
	if (z1.len < z2.len) {
		len = z2.len;
		j = z1.len;
		h1 = z2.v;
		h2 = z1.v;
	} else if (z1.len == z2.len) {
		while (len > 1 && z1.v[len-1] == z2.v[len-1])
			len--;
		j = len;
	}
	k = len - j;
	dest.len = len;
	dest.v = alloc(len);
	dest.sign = 0;
	dp = dest.v;
	while (j-- > 0)
		*dp++ = *h1++ ^ *h2++;
	while (k-- > 0)
		*dp++ = *h1++;
	*res = dest;
}


/*
 * Compute the bitwise AND NOT of two integers.
 */
void
zandnot(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	HALF *dp, *h1, *h2;
	LEN len, j, k;
	ZVALUE dest;

	len = z1.len;
	if (z2.len >= len) {
		while (len > 1 && (z1.v[len-1] & ~z2.v[len-1]) == 0)
			len--;
		j = len;
		k = 0;
	} else {
		j = z2.len;
		k = len - z2.len;
	}
	dest.len = len;
	dest.v = alloc(len);
	dest.sign = 0;
	dp = dest.v;
	h1 = z1.v;
	h2 = z2.v;
	while (j-- > 0)
		*dp++ = *h1++ & ~*h2++;
	while (k-- > 0)
		*dp++ = *h1++;
	*res = dest;
}


/*
 * Shift a number left (or right) by the specified number of bits.
 * Positive shift means to the left.  When shifting right, rightmost
 * bits are lost.  The sign of the number is preserved.
 */
void
zshift(ZVALUE z, long n, ZVALUE *res)
{
	ZVALUE ans;
	LEN hc;		/* number of halfwords shift is by */

	if (ziszero(z)) {
		*res = _zero_;
		return;
	}
	if (n == 0) {
		zcopy(z, res);
		return;
	}
	/*
	 * If shift value is negative, then shift right.
	 * Check for large shifts, and handle word-sized shifts quickly.
	 */
	if (n < 0) {
		n = -n;
		if ((n < 0) || (n >= (z.len * BASEB))) {
			*res = _zero_;
			return;
		}
		hc = (LEN)(n / BASEB);
		n %= BASEB;
		z.v += hc;
		z.len -= hc;
		ans.len = z.len;
		ans.v = alloc(ans.len);
		ans.sign = z.sign;
		zcopyval(z, ans);
		if (n > 0) {
			zshiftr(ans, n);
			ztrim(&ans);
		}
		if (ziszero(ans)) {
			zfree(ans);
			ans = _zero_;
		}
		*res = ans;
		return;
	}
	/*
	 * Shift value is positive, so shift leftwards.
	 * Check specially for a shift of the value 1, since this is common.
	 * Also handle word-sized shifts quickly.
	 */
	if (zisunit(z)) {
		zbitvalue(n, res);
		res->sign = z.sign;
		return;
	}
	hc = (LEN)(n / BASEB);
	n %= BASEB;
	ans.len = z.len + hc + 1;
	ans.v = alloc(ans.len);
	ans.sign = z.sign;
	if (hc > 0)
		memset((char *) ans.v, 0, hc * sizeof(HALF));
	memcpy((char *) (ans.v + hc),
	    (char *) z.v, z.len * sizeof(HALF));
	ans.v[ans.len - 1] = 0;
	if (n > 0) {
		ans.v += hc;
		ans.len -= hc;
		zshiftl(ans, n);
		ans.v -= hc;
		ans.len += hc;
	}
	ztrim(&ans);
	*res = ans;
}


/*
 * Return the position of the lowest bit which is set in the binary
 * representation of a number (counting from zero).  This is the highest
 * power of two which evenly divides the number.
 */
long
zlowbit(ZVALUE z)
{
	register HALF *zp;
	long n;
	HALF dataval;
	HALF *bitval;

	n = 0;
	for (zp = z.v; *zp == 0; zp++)
		if (++n >= z.len)
			return 0;
	dataval = *zp;
	bitval = bitmask;
	/* ignore Saber-C warning #530 about empty while statement */
	/*	  ok to ignore in proc zlowbit */
	while ((*(bitval++) & dataval) == 0) {
	}
	return (n*BASEB)+(bitval-bitmask-1);
}


/*
 * Return the position of the highest bit which is set in the binary
 * representation of a number (counting from zero).  This is the highest power
 * of two which is less than or equal to the number (which is assumed nonzero).
 */
LEN
zhighbit(ZVALUE z)
{
	HALF dataval;
	HALF *bitval;

	dataval = z.v[z.len-1];
	if (dataval == 0) {
		return 0;
	}
	bitval = bitmask+BASEB;
	if (dataval) {
		/* ignore Saber-C warning #530 about empty while statement */
		/*	  ok to ignore in proc zhighbit */
		while ((*(--bitval) & dataval) == 0) {
		}
	}
	return (z.len*BASEB)+(LEN)(bitval-bitmask-BASEB);
}


/*
 * Return whether or not the specified bit number is set in a number.
 * Rightmost bit of a number is bit 0.
 */
BOOL
zisset(ZVALUE z, long n)
{
	if ((n < 0) || ((n / BASEB) >= z.len))
		return FALSE;
	return ((z.v[n / BASEB] & (((HALF) 1) << (n % BASEB))) != 0);
}


/*
 * Check whether or not a number has exactly one bit set, and
 * thus is an exact power of two.  Returns TRUE if so.
 */
BOOL
zisonebit(ZVALUE z)
{
	register HALF *hp;
	register LEN len;

	if (ziszero(z) || zisneg(z))
		return FALSE;
	hp = z.v;
	len = z.len;
	while (len > 4) {
		len -= 4;
		if (*hp++ || *hp++ || *hp++ || *hp++)
			return FALSE;
	}
	while (--len > 0) {
		if (*hp++)
			return FALSE;
	}
	return ((*hp & -*hp) == *hp);		/* NEEDS 2'S COMPLEMENT */
}


/*
 * Check whether or not a number has all of its bits set below some
 * bit position, and thus is one less than an exact power of two.
 * Returns TRUE if so.
 */
BOOL
zisallbits(ZVALUE z)
{
	register HALF *hp;
	register LEN len;
	HALF digit;

	if (ziszero(z) || zisneg(z))
		return FALSE;
	hp = z.v;
	len = z.len;
	while (len > 4) {
		len -= 4;
		if ((*hp++ != BASE1) || (*hp++ != BASE1) ||
			(*hp++ != BASE1) || (*hp++ != BASE1))
				return FALSE;
	}
	while (--len > 0) {
		if (*hp++ != BASE1)
			return FALSE;
	}
	digit = (HALF)(*hp + 1);
	return ((digit & -digit) == digit);	/* NEEDS 2'S COMPLEMENT */
}


/*
 * Return the number whose binary representation contains only one bit which
 * is in the specified position (counting from zero).  This is equivalent
 * to raising two to the given power.
 */
void
zbitvalue(long n, ZVALUE *res)
{
	ZVALUE z;

	if (n < 0) n = 0;
	z.sign = 0;
	z.len = (LEN)((n / BASEB) + 1);
	z.v = alloc(z.len);
	zclearval(z);
	z.v[z.len-1] = (((HALF) 1) << (n % BASEB));
	*res = z;
}


/*
 * Compare a number against zero.
 * Returns the sgn function of the number (-1, 0, or 1).
 */
FLAG
ztest(ZVALUE z)
{
	register int sign;
	register HALF *h;
	register long len;

	sign = 1;
	if (z.sign)
		sign = -sign;
	h = z.v;
	len = z.len;
	while (len--)
		if (*h++)
			return sign;
	return 0;
}


/*
 * Return the sign of z1 - z2, i.e. 1 if the first integer is greater,
 * 0 if they are equal, -1 otherwise.
 */
FLAG
zrel(ZVALUE z1, ZVALUE z2)
{
	HALF *h1, *h2;
	LEN len;
	int sign;

	sign = 1;
	if (z1.sign < z2.sign)
		return 1;
	if (z2.sign < z1.sign)
		return -1;
	if (z2.sign)
		sign = -1;
	if (z1.len != z2.len)
		return (z1.len > z2.len) ? sign : -sign;
	len = z1.len;
	h1 = z1.v + len;
	h2 = z2.v + len;

	while (len > 0) {
		if (*--h1 != *--h2)
			break;
		len--;
	}
	if (len > 0)
		return (*h1 > *h2) ? sign : -sign;
	return 0;
}


/*
 * Return the sign of abs(z1) - abs(z2), i.e. 1 if the first integer
 * has greater absolute value, 0 is they have equal absolute value,
 * -1 otherwise.
 */
FLAG
zabsrel(ZVALUE z1, ZVALUE z2)
{
	HALF *h1, *h2;
	LEN len;

	if (z1.len != z2.len)
		return (z1.len > z2.len) ? 1 : -1;

	len = z1.len;
	h1 = z1.v + len;
	h2 = z2.v + len;
	while (len > 0) {
		if (*--h1 != *--h2)
			break;
		len--;
	}
	if (len > 0)
		return (*h1 > *h2) ? 1 : -1;
	return 0;
}


/*
 * Compare two numbers to see if they are equal or not.
 * Returns TRUE if they differ.
 */
BOOL
zcmp(ZVALUE z1, ZVALUE z2)
{
	register HALF *h1, *h2;
	register long len;

	if ((z1.sign != z2.sign) || (z1.len != z2.len) || (*z1.v != *z2.v))
		return TRUE;
	len = z1.len;
	h1 = z1.v;
	h2 = z2.v;
	while (--len > 0) {
		if (*++h1 != *++h2)
			return TRUE;
	}
	return FALSE;
}


/*
 * Utility to calculate the gcd of two FULL integers.
 */
FULL
uugcd(FULL f1, FULL f2)
{
	FULL temp;

	while (f1) {
		temp = f2 % f1;
		f2 = f1;
		f1 = temp;
	}
	return (FULL) f2;
}


/*
 * Utility to calculate the gcd of two small integers.
 */
long
iigcd(long i1, long i2)
{
	FULL f1, f2, temp;

	f1 = (FULL) ((i1 >= 0) ? i1 : -i1);
	f2 = (FULL) ((i2 >= 0) ? i2 : -i2);
	while (f1) {
		temp = f2 % f1;
		f2 = f1;
		f1 = temp;
	}
	return (long) f2;
}


void
ztrim(ZVALUE *z)
{
	HALF *h;
	LEN len;

	h = z->v + z->len - 1;
	len = z->len;
	while (*h == 0 && len > 1) {
		--h;
		--len;
	}
	z->len = len;
}


/*
 * Utility routine to shift right.
 *
 * NOTE: The ZVALUE length is not adjusted instead, the value is
 *	 zero padded from the left.  One may need to call ztrim()
 *	 or use zshift() instead.
 */
void
zshiftr(ZVALUE z, long n)
{
	register HALF *h, *lim;
	FULL mask, maskt;
	long len;

	if (n >= BASEB) {
		len = n / BASEB;
		h = z.v;
		lim = z.v + z.len - len;
		while (h < lim) {
			h[0] = h[len];
			++h;
		}
		n -= BASEB * len;
		lim = z.v + z.len;
		while (h < lim)
			*h++ = 0;
	}
	if (n) {
		len = z.len;
		h = z.v + len;
		mask = 0;
		while (len--) {
			maskt = (((FULL) *--h) << (BASEB - n)) & BASE1;
			*h = ((*h >> n) | (HALF)mask);
			mask = maskt;
		}
	}
}


/*
 * Utility routine to shift left.
 *
 * NOTE: The ZVALUE length is not adjusted.  The bits in the upper
 *	 HALF are simply tossed.  You may want to use zshift() instead.
 */
void
zshiftl(ZVALUE z, long n)
{
	register HALF *h;
	FULL mask, i;
	long len;

	if (n >= BASEB) {
		len = n / BASEB;
		h = z.v + z.len - 1;
		while (!*h)
			--h;
		while (h >= z.v) {
			h[len] = h[0];
			--h;
		}
		n -= BASEB * len;
		while (len)
			h[len--] = 0;
	}
	if (n > 0) {
		len = z.len;
		h = z.v;
		mask = 0;
		while (len--) {
			i = (((FULL) *h) << n) | mask;
			if (i > BASE1) {
				mask = i >> BASEB;
				i &= BASE1;
			} else {
				mask = 0;
			}
			*h = (HALF) i;
			++h;
		}
	}
}


/*
 * popcnt - count the number of 0 or 1 bits in an integer
 *
 * We ignore all 0 bits above the highest bit.
 */
long
zpopcnt(ZVALUE z, int bitval)
{
	long cnt = 0;	/* number of times found */
	HALF h;		/* HALF to count */
	int i;

	/*
	 * count 1's
	 */
	if (bitval) {

		/*
		 * count each HALF
		 */
		for (i=0; i < z.len; ++i) {
			/* count each octet */
			for (h = z.v[i]; h; h >>= 8) {
				cnt += (long)popcnt[h & 0xff];
			}
		}

	/*
	 * count 0's
	 */
	} else {

		/*
		 * count each HALF up until the last
		 */
		for (i=0; i < z.len-1; ++i) {

			/* count each octet */
			cnt += BASEB;
			for (h = z.v[i]; h; h >>= 8) {
				cnt -= (long)popcnt[h & 0xff];
			}
		}

		/*
		 * count the last octet up until the highest 1 bit
		 */
		for (h = z.v[z.len-1]; h; h>>=1) {
			/* count each 0 bit */
			if ((h & 0x1) == 0) {
				++cnt;
			}
		}
	}

	/*
	 * return count
	 */
	return cnt;
}
