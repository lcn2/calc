/*
 * zmod - modulo arithmetic routines
 *
 * Copyright (C) 1999-2007,2021  David I. Bell, Landon Curt Noll
 *				 and Ernest Bowen
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
 * Under source code control:	1991/05/22 23:03:55
 * File existed as early as:	1991
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Routines to do modulo arithmetic both normally and also using the REDC
 * algorithm given by Peter L. Montgomery in Mathematics of Computation,
 * volume 44, number 170 (April, 1985).	 For multiple multiplies using
 * the same large modulus, the REDC algorithm avoids the usual division
 * by the modulus, instead replacing it with two multiplies or else a
 * special algorithm.  When these two multiplies or the special algorithm
 * are faster then the division, then the REDC algorithm is better.
 */


#include "alloc.h"
#include "config.h"
#include "zmath.h"


#include "banned.h"	/* include after system header <> includes */


#define POWBITS 4		/* bits for power chunks (must divide BASEB) */
#define POWNUMS (1<<POWBITS)	/* number of powers needed in table */

S_FUNC void zmod5(ZVALUE *zp);
S_FUNC void zmod6(ZVALUE z1, ZVALUE *res);
S_FUNC void zredcmodinv(ZVALUE z1, ZVALUE *res);

STATIC REDC *powermodredc = NULL;	/* REDC info for raising to power */

BOOL havelastmod = FALSE;
STATIC ZVALUE lastmod[1];
STATIC ZVALUE lastmodinv[1];


/*
 * Square a number and then mod the result with a second number.
 * The number to be squared can be negative or out of modulo range.
 * The result will be in the range 0 to the modulus - 1.
 *
 * given:
 *	z1		number to be squared
 *	z2		number to take mod with
 *	res		result
 */
void
zsquaremod(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE tmp;
	FULL prod;
	FULL digit;

	if (ziszero(z2) || zisneg(z2))
		math_error("Mod of non-positive integer");
		/*NOTREACHED*/
	if (ziszero(z1) || zisunit(z2)) {
		*res = _zero_;
		return;
	}

	/*
	 * If the modulus is a single digit number, then do the result
	 * cheaply.  Check especially for a small power of two.
	 */
	if (zistiny(z2)) {
		digit = z2.v[0];
		if ((digit & -digit) == digit) {	/* NEEDS 2'S COMP */
			prod = (FULL) z1.v[0];
			prod = (prod * prod) & (digit - 1);
		} else {
			z1.sign = 0;
			prod = (FULL) zmodi(z1, (long) digit);
			prod = (prod * prod) % digit;
		}
		itoz((long) prod, res);
		return;
	}

	/*
	 * The modulus is more than one digit.
	 * Actually do the square and divide if necessary.
	 */
	zsquare(z1, &tmp);
	if ((tmp.len < z2.len) ||
		((tmp.len == z2.len) && (tmp.v[tmp.len-1] < z2.v[z2.len-1]))) {
			*res = tmp;
			return;
	}
	zmod(tmp, z2, res, 0);
	zfree(tmp);
}


/*
 * Calculate the number congruent to the given number whose absolute
 * value is minimal.  The number to be reduced can be negative or out of
 * modulo range.  The result will be within the range -int((modulus-1)/2)
 * to int(modulus/2) inclusive.	 For example, for modulus 7, numbers are
 * reduced to the range [-3, 3], and for modulus 8, numbers are reduced to
 * the range [-3, 4].
 *
 * given:
 *	z1		number to find minimum congruence of
 *	z2		number to take mod with
 *	res		result
 */
void
zminmod(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE tmp1, tmp2;
	int sign;
	int cv;

	if (ziszero(z2) || zisneg(z2)) {
		math_error("Mod of non-positive integer");
		/*NOTREACHED*/
	}
	if (ziszero(z1) || zisunit(z2)) {
		*res = _zero_;
		return;
	}
	if (zistwo(z2)) {
		if (zisodd(z1))
			*res = _one_;
		else
			*res = _zero_;
		return;
	}

	/*
	 * Do a quick check to see if the number is very small compared
	 * to the modulus.  If so, then the result is obvious.
	 */
	if (z1.len < z2.len - 1) {
		zcopy(z1, res);
		return;
	}

	/*
	 * Now make sure the input number is within the modulo range.
	 * If not, then reduce it to be within range and make the
	 * quick check again.
	 */
	sign = z1.sign;
	z1.sign = 0;
	cv = zrel(z1, z2);
	if (cv == 0) {
		*res = _zero_;
		return;
	}
	tmp1 = z1;
	if (cv > 0) {
		z1.sign = (BOOL)sign;
		zmod(z1, z2, &tmp1, 0);
		if (tmp1.len < z2.len - 1) {
			*res = tmp1;
			return;
		}
		sign = 0;
	}

	/*
	 * Now calculate the difference of the modulus and the absolute
	 * value of the original number.  Compare the original number with
	 * the difference, and return the one with the smallest absolute
	 * value, with the correct sign.  If the two values are equal, then
	 * return the positive result.
	 */
	zsub(z2, tmp1, &tmp2);
	cv = zrel(tmp1, tmp2);
	if (cv < 0) {
		zfree(tmp2);
		tmp1.sign = (BOOL)sign;
		if (tmp1.v == z1.v)
			zcopy(tmp1, res);
		else
			*res = tmp1;
	} else {
		if (cv)
			tmp2.sign = !sign;
		if (tmp1.v != z1.v)
			zfree(tmp1);
		*res = tmp2;
	}
}


/*
 * Compare two numbers for equality modulo a third number.
 * The two numbers to be compared can be negative or out of modulo range.
 * Returns TRUE if the numbers are not congruent, and FALSE if they are
 * congruent.
 *
 * given:
 *	z1		first number to be compared
 *	z2		second number to be compared
 *	z3		modulus
 */
BOOL
zcmpmod(ZVALUE z1, ZVALUE z2, ZVALUE z3)
{
	ZVALUE tmp1, tmp2, tmp3;
	FULL digit;
	LEN len;
	int cv;

	if (zisneg(z3) || ziszero(z3)) {
		math_error("Non-positive modulus in zcmpmod");
		/*NOTREACHED*/
	}
	if (zistwo(z3))
		return (((z1.v[0] + z2.v[0]) & 0x1) != 0);

	/*
	 * If the two numbers are equal, then their mods are equal.
	 */
	if ((z1.sign == z2.sign) && (z1.len == z2.len) &&
		(z1.v[0] == z2.v[0]) && (zcmp(z1, z2) == 0))
			return FALSE;

	/*
	 * If both numbers are negative, then we can make them positive.
	 */
	if (zisneg(z1) && zisneg(z2)) {
		z1.sign = 0;
		z2.sign = 0;
	}

	/*
	 * For small negative numbers, make them positive before comparing.
	 * In any case, the resulting numbers are in tmp1 and tmp2.
	 */
	tmp1 = z1;
	tmp2 = z2;
	len = z3.len;
	digit = z3.v[len - 1];

	if (zisneg(z1) && ((z1.len < len) ||
		((z1.len == len) && (z1.v[z1.len - 1] < digit))))
			zadd(z1, z3, &tmp1);

	if (zisneg(z2) && ((z2.len < len) ||
		((z2.len == len) && (z2.v[z2.len - 1] < digit))))
			zadd(z2, z3, &tmp2);

	/*
	 * Now compare the two numbers for equality.
	 * If they are equal we are all done.
	 */
	if (zcmp(tmp1, tmp2) == 0) {
		if (tmp1.v != z1.v)
			zfree(tmp1);
		if (tmp2.v != z2.v)
			zfree(tmp2);
		return FALSE;
	}

	/*
	 * They are not identical.  Now if both numbers are positive
	 * and less than the modulus, then they are definitely not equal.
	 */
	if ((tmp1.sign == tmp2.sign) &&
		((tmp1.len < len) || (zrel(tmp1, z3) < 0)) &&
		((tmp2.len < len) || (zrel(tmp2, z3) < 0))) {
		if (tmp1.v != z1.v)
			zfree(tmp1);
		if (tmp2.v != z2.v)
			zfree(tmp2);
		return TRUE;
	}

	/*
	 * Either one of the numbers is negative or is large.
	 * So do the standard thing and subtract the two numbers.
	 * Then they are equal if the result is 0 (mod z3).
	 */
	zsub(tmp1, tmp2, &tmp3);
	if (tmp1.v != z1.v)
		zfree(tmp1);
	if (tmp2.v != z2.v)
		zfree(tmp2);

	/*
	 * Compare the result with the modulus to see if it is equal to
	 * or less than the modulus.  If so, we know the mod result.
	 */
	tmp3.sign = 0;
	cv = zrel(tmp3, z3);
	if (cv == 0) {
		zfree(tmp3);
		return FALSE;
	}
	if (cv < 0) {
		zfree(tmp3);
		return TRUE;
	}

	/*
	 * We are forced to actually do the division.
	 * The numbers are congruent if the result is zero.
	 */
	zmod(tmp3, z3, &tmp1, 0);
	zfree(tmp3);
	if (ziszero(tmp1)) {
		zfree(tmp1);
		return FALSE;
	} else {
		zfree(tmp1);
		return TRUE;
	}
}


/*
 * Given the address of a positive integer whose word count does not
 * exceed twice that of the modulus stored at lastmod, to evaluate and store
 * at that address the value of the integer modulo the modulus.
 */
S_FUNC void
zmod5(ZVALUE *zp)
{
	LEN len, modlen, j;
	ZVALUE tmp1, tmp2;
	ZVALUE z1, z2, z3;
	HALF *a, *b;
	FULL f;
	HALF u;

	int subcount = 0;

	if (zrel(*zp, *lastmod) < 0)
		return;
	modlen = lastmod->len;
	len = zp->len;
	z1.v = zp->v + modlen - 1;
	z1.len = len - modlen + 1;
	z1.sign = z2.sign = z3.sign = 0;
	if (z1.len > modlen + 1) {
		math_error("Bad call to zmod5!!!");
		/*NOTREACHED*/
	}
	z2.v = lastmodinv->v + modlen + 1 - z1.len;
	z2.len = lastmodinv->len - modlen - 1 + z1.len;
	zmul(z1, z2, &tmp1);
	z3.v = tmp1.v + z1.len;
	z3.len = tmp1.len - z1.len;
	if (z3.len > 0) {
		zmul(z3, *lastmod, &tmp2);
		j = modlen;
		a = zp->v;
		b = tmp2.v;
		u = 0;
		len = modlen;
		while (j-- > 0) {
			f = (FULL) *a - (FULL) *b++ - (FULL) u;
			*a++ = (HALF) f;
			u = - (HALF) (f >> BASEB);
		}
		if (z1.len > 1) {
			len++;
			if (tmp2.len > modlen)
				f = (FULL) *a - (FULL) *b - (FULL) u;
			else
				f = (FULL) *a - (FULL) u;
			*a++ = (HALF) f;
		}
		while (len > 0 && *--a == 0)
			len--;
		zp->len = len;
		zfree(tmp2);
	}
	zfree(tmp1);
	while (len > 0 && zrel(*zp, *lastmod) >= 0) {
		subcount++;
		if (subcount > 2) {
			math_error("Too many subtractions in zmod5");
			/*NOTREACHED*/
		}
		j = modlen;
		a = zp->v;
		b = lastmod->v;
		u = 0;
		while (j-- > 0) {
			f = (FULL) *a - (FULL) *b++ - (FULL) u;
			*a++ = (HALF) f;
			u = - (HALF) (f >> BASEB);
		}
		if (len > modlen) {
			f = (FULL) *a - (FULL) u;
			*a++ = (HALF) f;
		}
		while (len > 0 && *--a == 0)
			len--;
		zp->len = len;
	}
	if (len == 0)
		zp->len = 1;
}

S_FUNC void
zmod6(ZVALUE z1, ZVALUE *res)
{
	LEN len, modlen, len0;
	int sign;
	ZVALUE zp0, ztmp;

	if (ziszero(z1) || zisone(*lastmod)) {
		*res = _zero_;
		return;
	}
	sign = z1.sign;
	z1.sign = 0;
	zcopy(z1, &ztmp);
	modlen = lastmod->len;
	zp0.sign = 0;
	while (zrel(ztmp, *lastmod) >= 0) {
		len = ztmp.len;
		zp0.len = len;
		len0 = 0;
		if (len > 2 * modlen) {
			zp0.len = 2 * modlen;
			len0 = len - 2 * modlen;
		}
		zp0.v = ztmp.v + len - zp0.len;
		zmod5(&zp0);
		len = len0 + zp0.len;
		while (len > 0 && ztmp.v[len - 1] == 0)
			len--;
		if (len == 0) {
			zfree(ztmp);
			*res = _zero_;
			return;
		}
		ztmp.len = len;
	}
	if (sign)
		zsub(*lastmod, ztmp, res);
	else
		zcopy(ztmp, res);
	zfree(ztmp);
}



/*
 * Compute the result of raising one number to a power modulo another number.
 * That is, this computes:  a^b (modulo c).
 * This calculates the result by examining the power POWBITS bits at a time,
 * using a small table of POWNUMS low powers to calculate powers for those bits,
 * and repeated squaring and multiplying by the partial powers to generate
 * the complete power.	If the power being raised to is high enough, then
 * this uses the REDC algorithm to avoid doing many divisions.	When using
 * REDC, multiple calls to this routine using the same modulus will be
 * slightly faster.
 */
void
zpowermod(ZVALUE z1, ZVALUE z2, ZVALUE z3, ZVALUE *res)
{
	HALF *hp;		/* pointer to current word of the power */
	REDC *rp;		/* REDC information to be used */
	ZVALUE *pp;		/* pointer to low power table */
	ZVALUE ans, temp;	/* calculation values */
	ZVALUE modpow;		/* current small power */
	ZVALUE lowpowers[POWNUMS];	/* low powers */
	ZVALUE ztmp;
	int curshift;		/* shift value for word of power */
	HALF curhalf;		/* current word of power */
	unsigned int curpow;	/* current low power */
	unsigned int curbit;	/* current bit of low power */
	int i;

	if (zisneg(z3) || ziszero(z3)) {
		math_error("Non-positive modulus in zpowermod");
		/*NOTREACHED*/
	}
	if (zisneg(z2)) {
		math_error("Negative power in zpowermod");
		/*NOTREACHED*/
	}


	/*
	 * Check easy cases first.
	 */
	if ((ziszero(z1) && !ziszero(z2)) || zisunit(z3)) {
		/* 0^(non_zero) or x^y mod 1 always produces zero */
		*res = _zero_;
		return;
	}
	if (ziszero(z2)) {			/* x^0 == 1 */
		*res = _one_;
		return;
	}
	if (zistwo(z3)) {			/* mod 2 */
		if (zisodd(z1))
			*res = _one_;
		else
			*res = _zero_;
		return;
	}
	if (zisunit(z1) && (!z1.sign || ziseven(z2))) {
		/* 1^x or (-1)^(2x) */
		*res = _one_;
		return;
	}

	/*
	 * Normalize the number being raised to be non-negative and to lie
	 * within the modulo range.  Then check for zero or one specially.
	 */
	ztmp.len = 0;
	if (zisneg(z1) || zrel(z1, z3) >= 0) {
		zmod(z1, z3, &ztmp, 0);
		z1 = ztmp;
	}
	if (ziszero(z1)) {
		if (ztmp.len)
			zfree(ztmp);
		*res = _zero_;
		return;
	}
	if (zisone(z1)) {
		if (ztmp.len)
			zfree(ztmp);
		*res = _one_;
		return;
	}

	/*
	 * If modulus is large enough use zmod5
	 */
	if (z3.len >= conf->pow2) {
		if (havelastmod && zcmp(z3, *lastmod)) {
			zfree(*lastmod);
			zfree(*lastmodinv);
			havelastmod = FALSE;
		}
		if (!havelastmod) {
			zcopy(z3, lastmod);
			zbitvalue(2 * z3.len * BASEB, &temp);
			zquo(temp, z3, lastmodinv, 0);
			zfree(temp);
			havelastmod = TRUE;
		}

		/* zzz */
		for (pp = &lowpowers[2]; pp <= &lowpowers[POWNUMS-1]; pp++) {
			pp->len = 0;
			pp->v = NULL;
		}
		lowpowers[0] = _one_;
		lowpowers[1] = z1;
		ans = _one_;

		hp = &z2.v[z2.len - 1];
		curhalf = *hp;
		curshift = BASEB - POWBITS;
		while (curshift && ((curhalf >> curshift) == 0))
			curshift -= POWBITS;

		/*
		 * Calculate the result by examining the power POWBITS bits at
		 * a time, and use the table of low powers at each iteration.
		 */
		for (;;) {
			curpow = (curhalf >> curshift) & (POWNUMS - 1);
			pp = &lowpowers[curpow];

			/*
			 * If the small power is not yet saved in the table,
			 * then calculate it and remember it in the table for
			 * future use.
			 */
			if (pp->v == NULL) {
				if (curpow & 0x1)
					zcopy(z1, &modpow);
				else
					modpow = _one_;

				for (curbit = 0x2;
				     curbit <= curpow;
				     curbit *= 2) {
					pp = &lowpowers[curbit];
					if (pp->v == NULL) {
						zsquare(lowpowers[curbit/2],
							&temp);
						zmod5(&temp);
						zcopy(temp, pp);
						zfree(temp);
					}
					if (curbit & curpow) {
						zmul(*pp, modpow, &temp);
						zfree(modpow);
						zmod5(&temp);
						zcopy(temp, &modpow);
						zfree(temp);
					}
				}
				pp = &lowpowers[curpow];
				if (pp->v != NULL) {
					zfree(*pp);
				}
				*pp = modpow;
			}

			/*
			 * If the power is nonzero, then accumulate the small
			 * power into the result.
			 */
			if (curpow) {
				zmul(ans, *pp, &temp);
				zfree(ans);
				zmod5(&temp);
				zcopy(temp, &ans);
				zfree(temp);
			}

			/*
			 * Select the next POWBITS bits of the power, if
			 * there is any more to generate.
			 */
			curshift -= POWBITS;
			if (curshift < 0) {
				if (hp == z2.v)
					break;
				curhalf = *--hp;
				curshift = BASEB - POWBITS;
			}

			/*
			 * Square the result POWBITS times to make room for
			 * the next chunk of bits.
			 */
			for (i = 0; i < POWBITS; i++) {
				zsquare(ans, &temp);
				zfree(ans);
				zmod5(&temp);
				zcopy(temp, &ans);
				zfree(temp);
			}
		}

		for (pp = &lowpowers[2]; pp <= &lowpowers[POWNUMS-1]; pp++) {
			if (pp->v != NULL)
				freeh(pp->v);
		}
		*res = ans;
		if (ztmp.len)
			zfree(ztmp);
		return;
	}

	/*
	 * If the modulus is odd and small enough then use
	 * the REDC algorithm.	The size where this is done is configurable.
	 */
	if (z3.len < conf->redc2 && zisodd(z3)) {
		if (powermodredc && zcmp(powermodredc->mod, z3)) {
			zredcfree(powermodredc);
			powermodredc = NULL;
		}
		if (powermodredc == NULL)
			powermodredc = zredcalloc(z3);
		rp = powermodredc;
		zredcencode(rp, z1, &temp);
		zredcpower(rp, temp, z2, &z1);
		zfree(temp);
		zredcdecode(rp, z1, res);
		zfree(z1);
		return;
	}

	/*
	 * Modulus or power is small enough to perform the power raising
	 * directly.  Initialize the table of powers.
	 */
	for (pp = &lowpowers[2]; pp <= &lowpowers[POWNUMS-1]; pp++) {
		pp->len = 0;
		pp->v = NULL;
	}
	lowpowers[0] = _one_;
	lowpowers[1] = z1;
	ans = _one_;

	hp = &z2.v[z2.len - 1];
	curhalf = *hp;
	curshift = BASEB - POWBITS;
	while (curshift && ((curhalf >> curshift) == 0))
		curshift -= POWBITS;

	/*
	 * Calculate the result by examining the power POWBITS bits at a time,
	 * and use the table of low powers at each iteration.
	 */
	for (;;) {
		curpow = (curhalf >> curshift) & (POWNUMS - 1);
		pp = &lowpowers[curpow];

		/*
		 * If the small power is not yet saved in the table, then
		 * calculate it and remember it in the table for future use.
		 */
		if (pp->v == NULL) {
			if (curpow & 0x1)
				zcopy(z1, &modpow);
			else
				modpow = _one_;

			for (curbit = 0x2; curbit <= curpow; curbit *= 2) {
				pp = &lowpowers[curbit];
				if (pp->v == NULL) {
					zsquare(lowpowers[curbit/2], &temp);
					zmod(temp, z3, pp, 0);
					zfree(temp);
				}
				if (curbit & curpow) {
					zmul(*pp, modpow, &temp);
					zfree(modpow);
					zmod(temp, z3, &modpow, 0);
					zfree(temp);
				}
			}
			pp = &lowpowers[curpow];
			if (pp->v != NULL) {
				zfree(*pp);
			}
			*pp = modpow;
		}

		/*
		 * If the power is nonzero, then accumulate the small power
		 * into the result.
		 */
		if (curpow) {
			zmul(ans, *pp, &temp);
			zfree(ans);
			zmod(temp, z3, &ans, 0);
			zfree(temp);
		}

		/*
		 * Select the next POWBITS bits of the power, if there is
		 * any more to generate.
		 */
		curshift -= POWBITS;
		if (curshift < 0) {
			if (hp-- == z2.v)
				break;
			curhalf = *hp;
			curshift = BASEB - POWBITS;
		}

		/*
		 * Square the result POWBITS times to make room for the next
		 * chunk of bits.
		 */
		for (i = 0; i < POWBITS; i++) {
			zsquare(ans, &temp);
			zfree(ans);
			zmod(temp, z3, &ans, 0);
			zfree(temp);
		}
	}

	for (pp = &lowpowers[2]; pp <= &lowpowers[POWNUMS-1]; pp++) {
		if (pp->v != NULL)
			freeh(pp->v);
	}
	*res = ans;
	if (ztmp.len)
		zfree(ztmp);
}

/*
 * Given a positive odd N-word integer z, evaluate minv(-z, BASEB^N)
 */
S_FUNC void
zredcmodinv(ZVALUE z, ZVALUE *res)
{
	ZVALUE tmp;
	HALF *a0, *a, *b;
	HALF bit, h, inv, v;
	FULL f;
	LEN N, i, j, len;

	N = z.len;
	tmp.sign = 0;
	tmp.len = N;
	tmp.v = alloc(N);
	zclearval(tmp);
	*tmp.v = 1;
	h = 1 + *z.v;
	bit = 1;
	inv = 1;
	while (h) {
		bit <<= 1;
		if (bit & h) {
			inv |= bit;
			h += bit * *z.v;
		}
	}

	j = N;
	a0 = tmp.v;
	while (j-- > 0) {
		v = inv * *a0;
		i = j;
		a = a0;
		b = z.v;
		f = (FULL) v * (FULL) *b++ + (FULL) *a++;
		*a0 = v;
		while (i-- > 0) {
			f = (FULL) v * (FULL) *b++  + (FULL) *a + (f >> BASEB);
			*a++ = (HALF) f;
		}
		while (j > 0 && *++a0 == 0)
			j--;
	}
	a = tmp.v + N;
	len = N;
	while (*--a == 0)
		len--;
	tmp.len = len;
	zcopy(tmp, res);
	zfree(tmp);
}


/*
 * Initialize the REDC algorithm for a particular modulus,
 * returning a pointer to a structure that is used for other
 * REDC calls.	An error is generated if the structure cannot
 * be allocated.  The modulus must be odd and positive.
 *
 * given:
 *	z1		modulus to initialize for
 */
REDC *
zredcalloc(ZVALUE z1)
{
	REDC *rp;		/* REDC information */
	ZVALUE tmp;
	long bit;

	if (ziseven(z1) || zisneg(z1)) {
		math_error("REDC requires positive odd modulus");
		/*NOTREACHED*/
	}

	rp = (REDC *) malloc(sizeof(REDC));
	if (rp == NULL) {
		math_error("Cannot allocate REDC structure");
		/*NOTREACHED*/
	}

	/*
	 * Round up the binary modulus to the next power of two
	 * which is at a word boundary.	 Then the shift and modulo
	 * operations mod the binary modulus can be done very cheaply.
	 * Calculate the REDC format for the number 1 for future use.
	 */
	zcopy(z1, &rp->mod);
	zredcmodinv(z1, &rp->inv);
	bit = zhighbit(z1) + 1;
	if (bit % BASEB)
		bit += (BASEB - (bit % BASEB));
	zbitvalue(bit, &tmp);
	zmod(tmp, rp->mod, &rp->one, 0);
	zfree(tmp);
	rp->len = (LEN)(bit / BASEB);
	return rp;
}


/*
 * Free any numbers associated with the specified REDC structure,
 * and then the REDC structure itself.
 *
 * given:
 *	rp		REDC information to be cleared
 */
void
zredcfree(REDC *rp)
{
	zfree(rp->mod);
	zfree(rp->inv);
	zfree(rp->one);
	free(rp);
}


/*
 * Convert a normal number into the specified REDC format.
 * The number to be converted can be negative or out of modulo range.
 * The resulting number can be used for multiplying, adding, subtracting,
 * or comparing with any other such converted numbers, as if the numbers
 * were being calculated modulo the number which initialized the REDC
 * information.	 When the final value is not converted, the result is the
 * same as if the usual operations were done with the original numbers.
 *
 * given:
 *	rp		REDC information
 *	z1		number to be converted
 *	res		returned converted number
 */
void
zredcencode(REDC *rp, ZVALUE z1, ZVALUE *res)
{
	ZVALUE tmp1;

	/*
	 * Confirm or initialize lastmod information when modulus is a
	 * big number.
	 */

	if (rp->len >= conf->pow2) {
		if (havelastmod && zcmp(rp->mod, *lastmod)) {
			zfree(*lastmod);
			zfree(*lastmodinv);
			havelastmod = FALSE;
		}
		if (!havelastmod) {
			zcopy(rp->mod, lastmod);
			zbitvalue(2 * rp->len * BASEB, &tmp1);
			zquo(tmp1, rp->mod, lastmodinv, 0);
			zfree(tmp1);
			havelastmod = TRUE;
		}
	}
	/*
	 * Handle the cases 0, 1, -1, and 2 specially since these are
	 * easy to calculate.  Zero transforms to zero, and the others
	 * can be obtained from the precomputed REDC format for 1 since
	 * addition and subtraction act normally for REDC format numbers.
	 */
	if (ziszero(z1)) {
		*res = _zero_;
		return;
	}
	if (zisone(z1)) {
		zcopy(rp->one, res);
		return;
	}
	if (zisunit(z1)) {
		zsub(rp->mod, rp->one, res);
		return;
	}
	if (zistwo(z1)) {
		zadd(rp->one, rp->one, &tmp1);
		if (zrel(tmp1, rp->mod) < 0) {
			*res = tmp1;
			return;
		}
		zsub(tmp1, rp->mod, res);
		zfree(tmp1);
		return;
	}

	/*
	 * Not a trivial number to convert, so do the full transformation.
	 */
	zshift(z1, rp->len * BASEB, &tmp1);
	if (rp->len < conf->pow2)
		zmod(tmp1, rp->mod, res, 0);
	else
		zmod6(tmp1, res);
	zfree(tmp1);
}


/*
 * The REDC algorithm used to convert numbers out of REDC format and also
 * used after multiplication of two REDC numbers.  Using this routine
 * avoids any divides, replacing the divide by two multiplications.
 * If the numbers are very large, then these two multiplies will be
 * quicker than the divide, since dividing is harder than multiplying.
 *
 * given:
 *	rp		REDC information
 *	z1		number to be transformed
 *	res		returned transformed number
 */
void
zredcdecode(REDC *rp, ZVALUE z1, ZVALUE *res)
{
	ZVALUE tmp1, tmp2;
	ZVALUE ztmp;
	ZVALUE ztop;
	ZVALUE zp1;
	FULL muln;
	HALF *h1;
	HALF *h3;
	HALF *hd = NULL;
	HALF Ninv;
	LEN modlen;
	LEN len;
	FULL f;
	int sign;
	int i, j;

	/*
	 * Check first for the special values for 0 and 1 that are easy.
	 */
	if (ziszero(z1)) {
		*res = _zero_;
		return;
	}
	if ((z1.len == rp->one.len) && (z1.v[0] == rp->one.v[0]) &&
		(zcmp(z1, rp->one) == 0)) {
			*res = _one_;
			return;
	}
	ztop.len = 0;
	ztmp.len = 0;
	modlen = rp->len;
	sign = z1.sign;
	z1.sign = 0;
	if (z1.len > modlen) {
		ztop.v = z1.v + modlen;
		ztop.len = z1.len - modlen;
		ztop.sign = 0;
		if (zrel(ztop, rp->mod) >= 0) {
			zmod(ztop, rp->mod, &ztmp, 0);
			ztop = ztmp;
		}
		len = modlen;
		h1 = z1.v + len;
		while (len > 0 && *--h1 == 0)
			len--;
		if (len == 0) {
			if (ztmp.len)
				*res = ztmp;
			else
				zcopy(ztop, res);
			return;
		}
		z1.len = len;

	}
	if (rp->mod.len < conf->pow2) {
		Ninv = rp->inv.v[0];
		res->sign = 0;
		res->len = modlen;
		res->v = alloc(modlen);
		zclearval(*res);
		h1 = z1.v;
		for (i = 0; i < modlen; i++) {
			h3 = rp->mod.v;
			hd = res->v;
			f = (FULL) *hd++;
			if (i < z1.len)
				f += (FULL) *h1++;
			muln = (HALF) ((f & BASE1) * Ninv);
			f = ((muln * (FULL) *h3++) + f) >> BASEB;
			j = modlen;
			while (--j > 0) {
				f += (muln * (FULL) *h3++) + (FULL) *hd;
				hd[-1] = (HALF) f;
				f >>= BASEB;
				hd++;
			}
			hd[-1] = (HALF) f;
		}
		len = modlen;
		while (*--hd == 0 && len > 1)
			len--;
		if (len == 0)
			len = 1;
		res->len = len;
	} else {
		/* Here 0 < z1 < 2^bitnum */

		/*
		 * First calculate the following:
		 *	tmp2 = ((z1 * inv) % 2^bitnum.
		 * The mod operations can be done with no work since the bit
		 * number was selected as a multiple of the word size.	Just
		 * reduce the sizes of the numbers as required.
		 */
		zmul(z1, rp->inv, &tmp2);
		if (tmp2.len > modlen) {
			h1 = tmp2.v + modlen;
			len = modlen;
			while (len > 0 && *--h1 == 0)
				len--;
			tmp2.len = len;
		}

		/*
		 * Next calculate the following:
		 *	res = (z1 + tmp2 * modulus) / 2^bitnum
		 * Since 0 < z1 < 2^bitnum and the division is always exact,
		 * the quotient can be evaluated by rounding up
		 * (tmp2 * modulus)/2^bitnum.  This can be achieved by defining
		 * zp1 by an appropriate shift and then adding one.
		 */
		zmul(tmp2, rp->mod, &tmp1);
		zfree(tmp2);
		if (tmp1.len > modlen) {
			zp1.v = tmp1.v + modlen;
			zp1.len = tmp1.len - modlen;
			zp1.sign = 0;
			zadd(zp1, _one_, res);
		} else {
			*res = _one_;
		}
		zfree(tmp1);
	}
	if (ztop.len) {
		zadd(*res, ztop, &tmp1);
		zfree(*res);
		if (ztmp.len)
			zfree(ztmp);
		*res = tmp1;
	}

	/*
	 * Finally do a final modulo by a simple subtraction if necessary.
	 * This is all that is needed because the previous calculation is
	 * guaranteed to always be less than twice the modulus.
	 */

	if (zrel(*res, rp->mod) >= 0) {
		zsub(*res, rp->mod, &tmp1);
		zfree(*res);
		*res = tmp1;
	}
	if (sign && !ziszero(*res)) {
		zsub(rp->mod, *res, &tmp1);
		zfree(*res);
		*res = tmp1;
	}
	return;
}


/*
 * Multiply two numbers in REDC format together producing a result also
 * in REDC format.  If the result is converted back to a normal number,
 * then the result is the same as the modulo'd multiplication of the
 * original numbers before they were converted to REDC format.	This
 * calculation is done in one of two ways, depending on the size of the
 * modulus.  For large numbers, the REDC definition is used directly
 * which involves three multiplies overall.  For small numbers, a
 * complicated routine is used which does the indicated multiplication
 * and the REDC algorithm at the same time to produce the result.
 *
 * given:
 *	rp		REDC information
 *	z1		first REDC number to be multiplied
 *	z2		second REDC number to be multiplied
 *	res		resulting REDC number
 */
void
zredcmul(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	FULL mulb;
	FULL muln;
	HALF *h1;
	HALF *h2;
	HALF *h3;
	HALF *hd;
	HALF Ninv;
	HALF topdigit = 0;
	LEN modlen;
	LEN len;
	LEN len2;
	SIUNION sival1;
	SIUNION sival2;
	SIUNION carry;
	ZVALUE tmp;
	ZVALUE z1tmp, z2tmp;
	int sign;

	sign = z1.sign ^ z2.sign;
	z1.sign = 0;
	z2.sign = 0;
	z1tmp.len = 0;
	if (zrel(z1, rp->mod) >= 0) {
		zmod(z1, rp->mod, &z1tmp, 0);
		z1 = z1tmp;
	}
	z2tmp.len = 0;
	if (zrel(z2, rp->mod) >= 0) {
		zmod(z2, rp->mod, &z2tmp, 0);
		z2 = z2tmp;
	}


	/*
	 * Check for special values which we easily know the answer.
	 */
	if (ziszero(z1) || ziszero(z2)) {
		*res = _zero_;
		if (z1tmp.len)
			zfree(z1tmp);
		if (z2tmp.len)
			zfree(z2tmp);
		return;
	}

	if ((z1.len == rp->one.len) && (z1.v[0] == rp->one.v[0]) &&
		(zcmp(z1, rp->one) == 0)) {
			if (sign)
				zsub(rp->mod, z2, res);
			else
				zcopy(z2, res);
			if (z1tmp.len)
				zfree(z1tmp);
			if (z2tmp.len)
				zfree(z2tmp);
			return;
	}

	if ((z2.len == rp->one.len) && (z2.v[0] == rp->one.v[0]) &&
		(zcmp(z2, rp->one) == 0)) {
			if (sign)
				zsub(rp->mod, z1, res);
			else
				zcopy(z1, res);
			if (z1tmp.len)
				zfree(z1tmp);
			if (z2tmp.len)
				zfree(z2tmp);
			return;
	}

	/*
	 * If the size of the modulus is large, then just do the multiply,
	 * followed by the two multiplies contained in the REDC routine.
	 * This will be quicker than directly doing the REDC calculation
	 * because of the O(N^1.585) speed of the multiplies.  The size
	 * of the number which this is done is configurable.
	 */
	if (rp->mod.len >= conf->redc2) {
		zmul(z1, z2, &tmp);
		zredcdecode(rp, tmp, res);
		zfree(tmp);
		if (sign && !ziszero(*res)) {
			zsub(rp->mod, *res, &tmp);
			zfree(*res);
			*res = tmp;
		}
		if (z1tmp.len)
			zfree(z1tmp);
		if (z2tmp.len)
			zfree(z2tmp);
		return;
	}

	/*
	 * The number is small enough to calculate by doing the O(N^2) REDC
	 * algorithm directly.	This algorithm performs the multiplication and
	 * the reduction at the same time.  Notice the obscure facts that
	 * only the lowest word of the inverse value is used, and that
	 * there is no shifting of the partial products as there is in a
	 * normal multiply.
	 */
	modlen = rp->mod.len;
	Ninv = rp->inv.v[0];

	/*
	 * Allocate the result and clear it.
	 * The size of the result will be equal to or smaller than
	 * the modulus size.
	 */
	res->sign = 0;
	res->len = modlen;
	res->v = alloc(modlen);

	hd = res->v;
	len = modlen;
	zclearval(*res);

	/*
	 * Do this outermost loop over all the digits of z1.
	 */
	h1 = z1.v;
	len = z1.len;
	while (len--) {
		/*
		 * Start off with the next digit of z1, the first
		 * digit of z2, and the first digit of the modulus.
		 */
		mulb = (FULL) *h1++;
		h2 = z2.v;
		h3 = rp->mod.v;
		hd = res->v;
		sival1.ivalue = mulb * ((FULL) *h2++) + ((FULL) *hd++);
		muln = ((HALF) (sival1.silow * Ninv));
		sival2.ivalue = muln * ((FULL) *h3++) + ((FULL) sival1.silow);
		carry.ivalue = ((FULL) sival1.sihigh) + ((FULL) sival2.sihigh);

		/*
		 * Do this innermost loop for each digit of z2, except
		 * for the first digit which was just done above.
		 */
		len2 = z2.len;
		while (--len2 > 0) {
			sival1.ivalue = mulb * ((FULL) *h2++)
				+ ((FULL) *hd) + ((FULL) carry.silow);
			sival2.ivalue = muln * ((FULL) *h3++)
				+ ((FULL) sival1.silow);
			carry.ivalue = ((FULL) sival1.sihigh)
				+ ((FULL) sival2.sihigh)
				+ ((FULL) carry.sihigh);

			hd[-1] = sival2.silow;
			hd++;
		}

		/*
		 * Now continue the loop as necessary so the total number
		 * of iterations is equal to the size of the modulus.
		 * This acts as if the innermost loop was repeated for
		 * high digits of z2 that are zero.
		 */
		len2 = modlen - z2.len;
		while (len2--) {
			sival2.ivalue = muln * ((FULL) *h3++)
				+ ((FULL) *hd)
				+ ((FULL) carry.silow);
			carry.ivalue = ((FULL) sival2.sihigh)
				+ ((FULL) carry.sihigh);

			hd[-1] = sival2.silow;
			hd++;
		}

		carry.ivalue += topdigit;
		hd[-1] = carry.silow;
		topdigit = carry.sihigh;
	}

	/*
	 * Now continue the loop as necessary so the total number
	 * of iterations is equal to the size of the modulus.
	 * This acts as if the outermost loop was repeated for high
	 * digits of z1 that are zero.
	 */
	len = modlen - z1.len;
	while (len--) {
		/*
		 * Start off with the first digit of the modulus.
		 */
		h3 = rp->mod.v;
		hd = res->v;
		muln = ((HALF) (*hd * Ninv));
		sival2.ivalue = muln * ((FULL) *h3++) + (FULL) *hd++;
		carry.ivalue = ((FULL) sival2.sihigh);

		/*
		 * Do this innermost loop for each digit of the modulus,
		 * except for the first digit which was just done above.
		 */
		len2 = modlen;
		while (--len2 > 0) {
			sival2.ivalue = muln * ((FULL) *h3++)
				+ ((FULL) *hd) + ((FULL) carry.silow);
			carry.ivalue = ((FULL) sival2.sihigh)
				+ ((FULL) carry.sihigh);

			hd[-1] = sival2.silow;
			hd++;
		}
		carry.ivalue += topdigit;
		hd[-1] = carry.silow;
		topdigit = carry.sihigh;
	}

	/*
	 * Determine the true size of the result, taking the top digit of
	 * the current result into account.  The top digit is not stored in
	 * the number because it is temporary and would become zero anyway
	 * after the final subtraction is done.
	 */
	if (topdigit == 0) {
		len = modlen;
		while (*--hd == 0 && len > 1) {
			len--;
		}
		res->len = len;

	/*
	 * Compare the result with the modulus.
	 * If it is less than the modulus, then the calculation is complete.
	 */

		if (zrel(*res, rp->mod) < 0) {
			if (z1tmp.len)
				zfree(z1tmp);
			if (z2tmp.len)
				zfree(z2tmp);
			if (sign && !ziszero(*res)) {
				zsub(rp->mod, *res, &tmp);
				zfree(*res);
				*res = tmp;
			}
			return;
		}
	}

	/*
	 * Do a subtraction to reduce the result to a value less than
	 * the modulus.	 The REDC algorithm guarantees that a single subtract
	 * is all that is needed.  Ignore any borrowing from the possible
	 * highest word of the current result because that would affect
	 * only the top digit value that was not stored and would become
	 * zero anyway.
	 */
	carry.ivalue = 0;
	h1 = rp->mod.v;
	hd = res->v;
	len = modlen;
	while (len--) {
		carry.ivalue = BASE1 - ((FULL) *hd) + ((FULL) *h1++)
			+ ((FULL) carry.silow);
		*hd++ = (HALF)(BASE1 - carry.silow);
		carry.silow = carry.sihigh;
	}

	/*
	 * Now finally recompute the size of the result.
	 */
	len = modlen;
	hd = &res->v[len - 1];
	while ((*hd == 0) && (len > 1)) {
		hd--;
		len--;
	}
	res->len = len;
	if (z1tmp.len)
		zfree(z1tmp);
	if (z2tmp.len)
		zfree(z2tmp);
	if (sign && !ziszero(*res)) {
		zsub(rp->mod, *res, &tmp);
		zfree(*res);
		*res = tmp;
	}

}

/*
 * Square a number in REDC format producing a result also in REDC format.
 *
 * given:
 *	rp		REDC information
 *	z1		REDC number to be squared
 *	res		resulting REDC number
 */
void
zredcsquare(REDC *rp, ZVALUE z1, ZVALUE *res)
{
	FULL mulb;
	FULL muln;
	HALF *h1;
	HALF *h2;
	HALF *h3;
	HALF *hd = NULL;
	HALF Ninv;
	HALF topdigit = 0;
	LEN modlen;
	LEN len;
	SIUNION sival1;
	SIUNION sival2;
	SIUNION sival3;
	SIUNION carry;
	ZVALUE tmp, ztmp;
	FULL f;
	int i, j;

	ztmp.len = 0;
	z1.sign = 0;
	if (zrel(z1, rp->mod) >= 0) {
		zmod(z1, rp->mod, &ztmp, 0);
		z1 = ztmp;
	}
	if (ziszero(z1)) {
		*res = _zero_;
		if (ztmp.len)
			zfree(ztmp);
		return;
	}
	if ((z1.len == rp->one.len) && (z1.v[0] == rp->one.v[0]) &&
		(zcmp(z1, rp->one) == 0)) {
			zcopy(z1, res);
			if (ztmp.len)
				zfree(ztmp);
			return;
	}


	/*
	 * If the modulus is small enough, then call the multiply
	 * routine to produce the result.  Otherwise call the O(N^1.585)
	 * routines to get the answer.
	 */
	if (rp->mod.len >= conf->redc2
			|| 3 * z1.len < 2 * rp->mod.len) {
		zsquare(z1, &tmp);
		zredcdecode(rp, tmp, res);
		zfree(tmp);
		if (ztmp.len)
			zfree(ztmp);
		return;
	}
	modlen = rp->mod.len;
	Ninv = rp->inv.v[0];

	res->sign = 0;
	res->len = modlen;
	res->v = alloc(modlen);

	zclearval(*res);

	h1 = z1.v;

	for (i = 0; i < z1.len; i++) {
		mulb = (FULL) *h1++;
		h2 = h1;
		h3 = rp->mod.v;
		hd = res->v;
		if (i == 0) {
			sival1.ivalue = mulb * mulb;
			muln = (HALF) (sival1.silow * Ninv);
			sival2.ivalue = muln * ((FULL) *h3++)
				+ (FULL) sival1.silow;
			carry.ivalue = (FULL) sival1.sihigh
				+ (FULL) sival2.sihigh;
			hd++;
		} else {
			muln = (HALF) (*hd * Ninv);
			f = (muln * ((FULL) *h3++) + (FULL) *hd++) >> BASEB;
			j = i;
			while (--j > 0) {
				f += muln * ((FULL) *h3++) + *hd;
				hd[-1] = (HALF) f;
				f >>= BASEB;
				hd++;
			}
			carry.ivalue = f;
			sival1.ivalue = mulb * mulb + (FULL) carry.silow;
			sival2.ivalue = muln * ((FULL) *h3++)
				+ (FULL) *hd
				+ (FULL) sival1.silow;
			carry.ivalue = (FULL) sival1.sihigh
				+ (FULL) sival2.sihigh
				+ (FULL) carry.sihigh;
			hd[-1] = sival2.silow;
			hd++;
		}
		j = z1.len - i;
		while (--j > 0) {
			sival1.ivalue = mulb * ((FULL) *h2++);
			sival2.ivalue = ((FULL) sival1.silow << 1)
				+ muln * ((FULL) *h3++);
			sival3.ivalue = (FULL) sival2.silow
				+ (FULL) *hd
				+ (FULL) carry.silow;
			carry.ivalue = ((FULL) sival1.sihigh << 1)
				+ (FULL) sival2.sihigh
				+ (FULL) sival3.sihigh
				+ (FULL) carry.sihigh;
			hd[-1] = sival3.silow;
			hd++;
		}
		j = modlen - z1.len;
		while (j-- > 0) {
			sival1.ivalue = muln * ((FULL) *h3++)
				+ (FULL) *hd
				+ (FULL) carry.silow;
			carry.ivalue = (FULL) sival1.sihigh
				+ (FULL) carry.sihigh;
			hd[-1] = sival1.silow;
			hd++;
		}
		carry.ivalue += (FULL) topdigit;
		hd[-1] = carry.silow;
		topdigit = carry.sihigh;
	}
	i = modlen - z1.len;
	while (i-- > 0) {
		h3 = rp->mod.v;
		hd = res->v;
		muln = (HALF) (*hd * Ninv);
		sival1.ivalue = muln * ((FULL) *h3++) + (FULL) *hd++;
		carry.ivalue = (FULL) sival1.sihigh;
		j = modlen;
		while (--j > 0) {
			sival1.ivalue = muln * ((FULL) *h3++)
				+ (FULL) *hd
				+ (FULL) carry.silow;
			carry.ivalue = (FULL) sival1.sihigh
				+ (FULL) carry.sihigh;
			hd[-1] = sival1.silow;
			hd++;
		}
		carry.ivalue += (FULL) topdigit;
		hd[-1] = carry.silow;
		topdigit = carry.sihigh;
	}
	if (topdigit == 0) {
		len = modlen;
		while (*--hd == 0 && len > 1) {
			len--;
		}
		res->len = len;
		if  (zrel(*res, rp->mod) < 0) {
			if (ztmp.len)
				zfree(ztmp);
			return;
		}
	}

	carry.ivalue = 0;
	h1 = rp->mod.v;
	hd = res->v;
	len = modlen;
	while (len--) {
		carry.ivalue = BASE1 - ((FULL) *hd) + ((FULL) *h1++)
			+ ((FULL) carry.silow);
		*hd++ = (HALF)(BASE1 - carry.silow);
		carry.silow = carry.sihigh;
	}

	len = modlen;
	hd = &res->v[len - 1];
	while ((*hd == 0) && (len > 1)) {
		hd--;
		len--;
	}
	res->len = len;
	if (ztmp.len)
		zfree(ztmp);
}


/*
 * Compute the result of raising a REDC format number to a power.
 * The result is within the range 0 to the modulus - 1.
 * This calculates the result by examining the power POWBITS bits at a time,
 * using a small table of POWNUMS low powers to calculate powers for those bits,
 * and repeated squaring and multiplying by the partial powers to generate
 * the complete power.
 *
 * given:
 *	rp		REDC information
 *	z1		REDC number to be raised
 *	z2		normal number to raise number to
 *	res		result
 */
void
zredcpower(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	HALF *hp;		/* pointer to current word of the power */
	ZVALUE *pp;		/* pointer to low power table */
	ZVALUE ans, temp;	/* calculation values */
	ZVALUE ztmp;
	ZVALUE modpow;		/* current small power */
	ZVALUE lowpowers[POWNUMS];	/* low powers */
	int curshift;		/* shift value for word of power */
	HALF curhalf;		/* current word of power */
	unsigned int curpow;	/* current low power */
	unsigned int curbit;	/* current bit of low power */
	int sign;
	int i;

	if (zisneg(z2)) {
		math_error("Negative power in zredcpower");
		/*NOTREACHED*/
	}

	if (zisunit(rp->mod)) {
		*res = _zero_;
		return;
	}

	sign = zisodd(z2) ? z1.sign : 0;
	z1.sign = 0;
	ztmp.len = 0;
	if (zrel(z1, rp->mod) >= 0) {
		zmod(z1, rp->mod, &ztmp, 0);
		z1 = ztmp;
	}
	/*
	 * Check for zero or the REDC format for one.
	 */
	if (ziszero(z1)) {
		if (ziszero(z2))
			*res = _one_;
		else
			*res = _zero_;
		if (ztmp.len)
			zfree(ztmp);
		return;
	}
	if (zcmp(z1, rp->one) == 0) {
		if (sign)
			zsub(rp->mod, rp->one, res);
		else
			zcopy(rp->one, res);
		if (ztmp.len)
			zfree(ztmp);
		return;
	}

	/*
	 * See if the number being raised is the REDC format for -1.
	 * If so, then the answer is the REDC format for one or minus one.
	 * To do this check, calculate the REDC format for -1.
	 */
	if (((HALF)(z1.v[0] + rp->one.v[0])) == rp->mod.v[0]) {
		zsub(rp->mod, rp->one, &temp);
		if (zcmp(z1, temp) == 0) {
			if (zisodd(z2) ^ sign) {
				*res = temp;
				if (ztmp.len)
					zfree(ztmp);
				return;
			}
			zfree(temp);
			zcopy(rp->one, res);
			if (ztmp.len)
				zfree(ztmp);
			return;
		}
		zfree(temp);
	}

	for (pp = &lowpowers[2]; pp < &lowpowers[POWNUMS]; pp++)
		pp->len = 0;
	zcopy(rp->one, &lowpowers[0]);
	zcopy(z1, &lowpowers[1]);
	zcopy(rp->one, &ans);

	hp = &z2.v[z2.len - 1];
	curhalf = *hp;
	curshift = BASEB - POWBITS;
	while (curshift && ((curhalf >> curshift) == 0))
		curshift -= POWBITS;

	/*
	 * Calculate the result by examining the power POWBITS bits at a time,
	 * and use the table of low powers at each iteration.
	 */
	for (;;) {
		curpow = (curhalf >> curshift) & (POWNUMS - 1);
		pp = &lowpowers[curpow];

		/*
		 * If the small power is not yet saved in the table, then
		 * calculate it and remember it in the table for future use.
		 */
		if (pp->len == 0) {
			if (curpow & 0x1)
				zcopy(z1, &modpow);
			else
				zcopy(rp->one, &modpow);

			for (curbit = 0x2; curbit <= curpow; curbit *= 2) {
				pp = &lowpowers[curbit];
				if (pp->len == 0)
					zredcsquare(rp, lowpowers[curbit/2],
						pp);
				if (curbit & curpow) {
					zredcmul(rp, *pp, modpow, &temp);
					zfree(modpow);
					modpow = temp;
				}
			}
			pp = &lowpowers[curpow];
			if (pp->len > 0) {
				zfree(*pp);
			}
			*pp = modpow;
		}

		/*
		 * If the power is nonzero, then accumulate the small power
		 * into the result.
		 */
		if (curpow) {
			zredcmul(rp, ans, *pp, &temp);
			zfree(ans);
			ans = temp;
		}

		/*
		 * Select the next POWBITS bits of the power, if there is
		 * any more to generate.
		 */
		curshift -= POWBITS;
		if (curshift < 0) {
			if (hp-- == z2.v)
				break;
			curhalf = *hp;
			curshift = BASEB - POWBITS;
		}

		/*
		 * Square the result POWBITS times to make room for the next
		 * chunk of bits.
		 */
		for (i = 0; i < POWBITS; i++) {
			zredcsquare(rp, ans, &temp);
			zfree(ans);
			ans = temp;
		}
	}

	for (pp = lowpowers; pp < &lowpowers[POWNUMS]; pp++) {
		if (pp->len)
			freeh(pp->v);
	}
	if (sign && !ziszero(ans)) {
		zsub(rp->mod, ans, res);
		zfree(ans);
	} else {
		*res = ans;
	}
	if (ztmp.len)
		zfree(ztmp);
}


/*
 * zhnrmod - compute z mod h*2^n+r
 *
 * We compute v mod h*2^n+r, where h>0, n>0, abs(r) <= 1, as follows:
 *
 *	Let v = b*2^n + a, where 0 <= a < 2^n
 *
 *	Now v mod h*2^n+r == b*2^n + a mod h*2^n+r,
 *	and thus v mod h*2^n+r == b*2^n mod h*2^n+r + a mod h*2^n+r.
 *
 *	Because 0 <= a < 2^n < h*2^n+r, a mod h*2^n+r == a.
 *	Thus v mod h*2^n+r == b*2^n mod h*2^n+r + a.
 *
 *	It can be shown that b*2^n mod h*2^n == 2^n * (b mod h).
 *
 *	Thus for r == 0, v mod h*2^n+r == (2^n)*(b mod h) + a.
 *
 *	It can be shown that v mod 2^n-1 == a+b mod 2^n-1.
 *
 *	Thus for r == -1, v mod h*2^n+r == (2^n)*(b mod h) + a + int(b/h).
 *
 *	It can be shown that v mod 2^n+1 == a-b mod 2^n+1.
 *
 *	Thus for r == +1, v mod h*2^n+r == (2^n)*(b mod h) + a - int(b/h).
 *
 *	Therefore, v mod h*2^n+r == (2^n)*(b mod h) + a - r*int(b/h).
 *
 * The above proof leads to the following calc resource file which computes
 * the value z mod h*2^n+r:
 *
 *    define hnrmod(v,h,n,r)
 *    {
 *	local a,b,modulus,tquo,tmod,lbit,ret;
 *
 *	if (!isint(h) || h < 1) {
 *	    quit "h must be an integer be > 0";
 *	}
 *	if (!isint(n) || n < 1) {
 *	    quit "n must be an integer be > 0";
 *	}
 *	if (r != 1 && r != 0 && r != -1) {
 *	    quit "r must be -1, 0 or 1";
 *	}
 *
 *	lbit = lowbit(h);
 *	if (lbit > 0) {
 *	    n += lbit;
 *	    h >>= lbit;
 *	}
 *
 *	modulus = h<<n+r;
 *	if (modulus <= 2^31-1) {
 *	    return v % modulus;
 *	}
 *	ret = v;
 *
 *	do {
 *	    if (highbit(ret) < n) {
 *		break;
 *	    }
 *	    b = ret>>n;
 *	    a = ret - (b<<n);
 *
 *	    switch (r) {
 *	    case -1:
 *		if (h == 1) {
 *		    ret = a + b;
 *		} else {
 *		    quomod(b, h, tquo, tmod);
 *		    ret = tmod<<n + a + tquo;
 *		}
 *		break;
 *	    case 0:
 *		if (h == 1) {
 *		    ret = a;
 *		} else {
 *		    ret = (b%h)<<n + a;
 *		}
 *		break;
 *	    case 1:
 *		if (h == 1) {
 *		    ret = ((a > b) ? a-b : modulus+a-b);
 *		} else {
 *		    quomod(b, h, tquo, tmod);
 *		    tmod = tmod<<n + a;
 *		    ret = ((tmod >= tquo) ? tmod-tquo : modulus+tmod-tquo);
 *		}
 *		break;
 *	    }
 *	} while (ret > modulus);
 *	ret = ((ret < 0) ? ret+modlus : ((ret == modulus) ? 0 : ret));
 *
 *	return ret;
 *    }
 *
 * This function implements the above calc resource file.
 *
 * given:
 *	v		take mod of this value, v >= 0
 *	zh		h from modulus h*2^n+r, h > 0
 *	zn		n from modulus h*2^n+r, n > 0
 *	zr		r from modulus h*2^n+r, abs(r) <= 1
 *	res		v mod h*2^n+r
 */
void
zhnrmod(ZVALUE v, ZVALUE zh, ZVALUE zn, ZVALUE zr, ZVALUE *res)
{
	ZVALUE a;		/* lower n bits of v */
	ZVALUE b;		/* bits above the lower n bits of v */
	ZVALUE h;		/* working zh value */
	ZVALUE modulus;		/* h^2^n + r */
	ZVALUE tquo;		/* b // h */
	ZVALUE tmod;		/* b % h or (b%h)<<n + a */
	ZVALUE t;		/* temp ZVALUE */
	ZVALUE t2;		/* temp ZVALUE */
	ZVALUE ret;		/* return value, what *res is set to */
	long n;			/* integer value of zn */
	long r;			/* integer value of zr */
	long hbit;		/* highbit(res) */
	long lbit;		/* lowbit(h) */
	int zrelval;		/* return value of zrel() */
	int hisone;		/* 1 => h == 1, 0 => h != 1 */

	/*
	 * firewall
	 */
	if (zisneg(zh) || ziszero(zh)) {
		math_error("h must be > 0");
		/*NOTREACHED*/
	}
	if (zisneg(zn) || ziszero(zn)) {
		math_error("n must be > 0");
		/*NOTREACHED*/
	}
	if (zge31b(zn)) {
		math_error("n must be < 2^31");
		/*NOTREACHED*/
	}
	if (!zisabsleone(zr)) {
		math_error("r must be -1, 0 or 1");
		/*NOTREACHED*/
	}


	/*
	 * setup for loop
	 */
	n = ztolong(zn);
	r = ztolong(zr);
	if (zisneg(zr)) {
		r = -r;
	}
	/* lbit = lowbit(h); */
	lbit = zlowbit(zh);
	/* if (lbit > 0) { n += lbit; h >>= lbit; } */
	if (lbit > 0) {
		n += lbit;
		zshift(zh, -lbit, &h);
	} else {
		h = zh;
	}
	/* modulus = h<<n+r; */
	zshift(h, n, &t);
	switch (r) {
	case 1:
		zadd(t, _one_, &modulus);
		zfree(t);
		break;
	case 0:
		modulus = t;
		break;
	case -1:
		zsub(t, _one_, &modulus);
		zfree(t);
		break;
	}
	/* if (modulus <= MAXLONG) { return v % modulus; } */
	if (!zgtmaxlong(modulus)) {
		itoz(zmodi(v, ztolong(modulus)), res);
		zfree(modulus);
		if (lbit > 0) {
			zfree(h);
		}
		return;
	}
	/* ret = v; */
	zcopy(v, &ret);

	/*
	 * shift-add modulus loop
	 */
	hisone = zisone(h);
	do {

		/*
		 * split ret into to chunks, the lower n bits
		 * and everything above the lower n bits
		 */
		/* if (highbit(ret) < n) { break; } */
		hbit = (long)zhighbit(ret);
		if (hbit < n) {
			zrelval = (zcmp(ret, modulus) ? -1 : 0);
			break;
		}
		/* b = ret>>n; */
		zshift(ret, -n, &b);
		b.sign = ret.sign;
		/* a = ret - (b<<n); */
		a.sign = ret.sign;
		a.len = (n+BASEB-1)/BASEB;
		a.v = alloc(a.len);
		memcpy(a.v, ret.v, a.len*sizeof(HALF));
		if (n % BASEB) {
			a.v[a.len - 1] &= lowhalf[n % BASEB];
		}
		ztrim(&a);

		/*
		 * switch depending on r == -1, 0 or 1
		 */
		switch (r) {
		case -1:	/* v mod h*2^h-1 */
			/* if (h == 1) ... */
			if (hisone) {
				/* ret = a + b; */
				zfree(ret);
				zadd(a, b, &ret);

			/* ... else ... */
			} else {
				/* quomod(b, h, tquo, tmod); */
				(void) zdiv(b, h, &tquo, &tmod, 0);
				/* ret = tmod<<n + a + tquo; */
				zshift(tmod, n, &t);
				zfree(tmod);
				zadd(a, tquo, &t2);
				zfree(tquo);
				zfree(ret);
				zadd(t, t2, &ret);
				zfree(t);
				zfree(t2);
			}
			break;

		case 0:		/* v mod h*2^h-1 */
			/* if (h == 1) ... */
			if (hisone) {
				/* ret = a; */
				zfree(ret);
				zcopy(a, &ret);

			/* ... else ... */
			} else {
				/* ret = (b%h)<<n + a; */
				(void) zmod(b, h, &tmod, 0);
				zshift(tmod, n, &t);
				zfree(tmod);
				zfree(ret);
				zadd(t, a, &ret);
				zfree(t);
			}
			break;

		case 1:		/* v mod h*2^h-1 */
			/* if (h == 1) ... */
			if (hisone) {
				/* ret = a-b; */
				zfree(ret);
				zsub(a, b, &ret);

			/* ... else ... */
			} else {
				/* quomod(b, h, tquo, tmod); */
				(void) zdiv(b, h, &tquo, &tmod, 0);
				/* tmod = tmod<<n + a; */
				zshift(tmod, n, &t);
				zfree(tmod);
				zadd(t, a, &tmod);
				zfree(t);
				/* ret = tmod-tquo; */
				zfree(ret);
				zsub(tmod, tquo, &ret);
				zfree(tquo);
				zfree(tmod);
			}
			break;
		}
		zfree(a);
		zfree(b);

	/* ... while (abs(ret) > modulus); */
	} while ((zrelval = zabsrel(ret, modulus)) > 0);
	/* ret = ((ret < 0) ? ret+modlus : ((ret == modulus) ? 0 : ret)); */
	if (ret.sign) {
		zadd(ret, modulus, &t);
		zfree(ret);
		ret = t;
	} else if (zrelval == 0) {
		zfree(ret);
		ret = _zero_;
	}
	zfree(modulus);
	if (lbit > 0) {
		zfree(h);
	}

	/*
	 * return ret
	 */
	*res = ret;
	return;
}
