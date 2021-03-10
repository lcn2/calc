/*
 * qmod - modular arithmetic routines for normal numbers and REDC numbers
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
 * Under source code control:	1991/05/22 23:15:07
 * File existed as early as:	1991
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "qmath.h"
#include "config.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * Structure used for caching REDC information.
 */
typedef struct	{
	NUMBER	*rnum;		/* modulus being cached */
	REDC	*redc;		/* REDC information for modulus */
	long	age;		/* age counter for reallocation */
} REDC_CACHE;


STATIC long redc_age;			/* current age counter */
STATIC REDC_CACHE redc_cache[MAXREDC];	/* cached REDC info */


S_FUNC REDC *qfindredc(NUMBER *q);


/*
 * qmod(q1, q2, rnd) returns zero if q1 is a multiple of q2; it
 * q1 if q2 is zero.  For other q1 and q2, it returns one of
 * the two remainders with absolute value less than abs(q2)
 * when q1 is divided by q2;  which remainder is returned is
 * determined by rnd and the signs and relative sizes of q1 and q2.
 */
NUMBER *
qmod(NUMBER *q1, NUMBER *q2, long rnd)
{
	ZVALUE tmp, tmp1, tmp2;
	NUMBER *q;

	if (qiszero(q2)) return qlink(q1);
	if (qiszero(q1)) return qlink(&_qzero_);
	if (qisint(q1) && qisint(q2)) {		/* easy case */
		zmod(q1->num, q2->num, &tmp, rnd);
		if (ziszero(tmp)) {
			zfree(tmp);
			return qlink(&_qzero_);
		}
		if(zisone(tmp)) {
			zfree(tmp);
			return qlink(&_qone_);
		}
		q = qalloc();
		q->num = tmp;
		return q;
	}
	zmul(q1->num, q2->den, &tmp1);
	zmul(q2->num, q1->den, &tmp2);
	zmod(tmp1, tmp2, &tmp, rnd);
	zfree(tmp1);
	zfree(tmp2);
	if (ziszero(tmp)) {
		zfree(tmp);
		return qlink(&_qzero_);
	}
	zmul(q1->den, q2->den, &tmp1);
	q = qalloc();
	zreduce(tmp, tmp1, &q->num, &q->den);
	zfree(tmp1);
	zfree(tmp);
	return q;
}


/*
 * Given two numbers q1, q2, qquomod(q1, q2, quo, mod, rnd) assigns
 * quo(q1, q2, rnd) to quo and mod(q1, q2, rnd) to mod. The quotient
 * quo is always an integer, q1 = q2 * quo + mod, and if q2 is non-zero,
 * abs(mod) < abs(q2). If q2 is zero, quo is assigned the value zero and
 * mod = q1.
 * Which of the two possible quotient-remainder pairs is assigned
 * to quo and mod is determined by the fifth argument rnd.
 * If rnd is zero, the remainder has the sign of q2
 * and the quotient is rounded towards zero.
 *
 * The function returns TRUE or FALSE according as the remainder is
 * nonzero or zero
 *
 * Given:
 *	q1		number to be divided
 *	q2		divisor
 *	quo		quotient
 *	mod		remainder
 *      rnd		rounding-type specifier
 */
BOOL
qquomod(NUMBER *q1, NUMBER *q2, NUMBER **quo, NUMBER **mod, long rnd)
{
	NUMBER *qq, *qm;
	ZVALUE tmp1, tmp2, tmp3, tmp4;

	if (qiszero(q2)) {			/* zero divisor case */
		qq = qlink(&_qzero_);
		qm = qlink(q1);
	} else if (qisint(q1) && qisint(q2)) {	/* integer args case */
		zdiv(q1->num, q2->num, &tmp1, &tmp2, rnd);
		if (ziszero(tmp1)) {
			zfree(tmp1);
			zfree(tmp2);
			qq = qlink(&_qzero_);
			qm = qlink(q1);
		} else {
			qq = qalloc();
			qq->num = tmp1;
			if (ziszero(tmp2)) {
				zfree(tmp2);
				qm = qlink(&_qzero_);
			} else {
				qm = qalloc();
				qm->num = tmp2;
			}
		}
	} else {					/* fractional case */
		zmul(q1->num, q2->den, &tmp1);
		zmul(q2->num, q1->den, &tmp2);
		zdiv(tmp1, tmp2, &tmp3, &tmp4, rnd);
		zfree(tmp1);
		zfree(tmp2);
		if (ziszero(tmp3)) {
			zfree(tmp3);
			zfree(tmp4);
			qq = qlink(&_qzero_);
			qm = qlink(q1);
		} else {
			qq = qalloc();
			qq->num = tmp3;
			if (ziszero(tmp4)) {
				zfree(tmp4);
				qm = qlink(&_qzero_);
			} else {
				qm = qalloc();
				zmul(q1->den, q2->den, &tmp1);
				zreduce(tmp4, tmp1, &qm->num, &qm->den);
				zfree(tmp1);
				zfree(tmp4);
			}
		}
	}
	*quo = qq;
	*mod = qm;
	return !qiszero(qm);
}


/*
 * Return whether or not two integers are congruent modulo a third integer.
 * Returns TRUE if the numbers are not congruent, and FALSE if they are.
 */
BOOL
qcmpmod(NUMBER *q1, NUMBER *q2, NUMBER *q3)
{
	if (qisneg(q3) || qiszero(q3))
		math_error("Non-positive modulus");
	if (qisfrac(q1) || qisfrac(q2) || qisfrac(q3))
		math_error("Non-integers for qcmpmod");
	if (q1 == q2)
		return FALSE;
	return zcmpmod(q1->num, q2->num, q3->num);
}


/*
 * Convert an integer into REDC format for use in faster modular arithmetic.
 * The number can be negative or out of modulus range.
 *
 * given:
 *	q1		number to convert into REDC format
 *	q2		modulus
 */
NUMBER *
qredcin(NUMBER *q1, NUMBER *q2)
{
	REDC *rp;		/* REDC information */
	NUMBER *r;		/* result */

	if (qisfrac(q1))
		math_error("Non-integer for qredcin");
	rp = qfindredc(q2);
	r = qalloc();
	zredcencode(rp, q1->num, &r->num);
	if (qiszero(r)) {
		qfree(r);
		return qlink(&_qzero_);
	}
	return r;
}


/*
 * Convert a REDC format number back into a normal integer.
 * The resulting number is in the range 0 to the modulus - 1.
 *
 * given:
 *	q1		number to convert into REDC format
 *	q2		modulus
 */
NUMBER *
qredcout(NUMBER *q1, NUMBER *q2)
{
	REDC *rp;		/* REDC information */
	NUMBER *r;		/* result */

	if (qisfrac(q1))
		math_error("Non-integer argument for rcout");
	rp = qfindredc(q2);
	if (qiszero(q1) || qisunit(q2))
		return qlink(&_qzero_);
	r = qalloc();
	zredcdecode(rp, q1->num, &r->num);
	if (zisunit(r->num)) {
		qfree(r);
		r = qlink(&_qone_);
	}
	return r;
}


/*
 * Multiply two REDC format numbers together producing a REDC format result.
 * This multiplication is done modulo the specified modulus.
 *
 * given:
 *	q1		REDC numbers to be multiplied
 *	q2		REDC numbers to be multiplied
 *	q3		modulus
 */
NUMBER *
qredcmul(NUMBER *q1, NUMBER *q2, NUMBER *q3)
{
	REDC *rp;		/* REDC information */
	NUMBER *r;		/* result */

	if (qisfrac(q1) || qisfrac(q2))
		math_error("Non-integer argument for rcmul");
	rp = qfindredc(q3);
	if (qiszero(q1) || qiszero(q2) || qisunit(q3))
		return qlink(&_qzero_);
	r = qalloc();
	zredcmul(rp, q1->num, q2->num, &r->num);
	return r;
}


/*
 * Square a REDC format number to produce a REDC format result.
 * This squaring is done modulo the specified modulus.
 *
 * given:
 *	q1		REDC numbers to be squared
 *	q2		modulus
 */
NUMBER *
qredcsquare(NUMBER *q1, NUMBER *q2)
{
	REDC *rp;		/* REDC information */
	NUMBER *r;		/* result */

	if (qisfrac(q1))
		math_error("Non-integer argument for rcsq");
	rp = qfindredc(q2);
	if (qiszero(q1) || qisunit(q2))
		return qlink(&_qzero_);
	r = qalloc();
	zredcsquare(rp, q1->num, &r->num);
	return r;
}


/*
 * Raise a REDC format number to the indicated power producing a REDC
 * format result.  This is done modulo the specified modulus.  The
 * power to be raised to is a normal number.
 *
 * given:
 *	q1		REDC number to be raised
 *	q2		power to be raised to
 *	q3		modulus
 */
NUMBER *
qredcpower(NUMBER *q1, NUMBER *q2, NUMBER *q3)
{
	REDC *rp;		/* REDC information */
	NUMBER *r;		/* result */

	if (qisfrac(q1) || qisfrac(q2) || qisfrac(q2))
		math_error("Non-integer argument for rcpow");
	if (qisneg(q2))
		math_error("Negative exponent argument for rcpow");
	rp = qfindredc(q3);
	r = qalloc();
	zredcpower(rp, q1->num, q2->num, &r->num);
	return r;
}


/*
 * Search for and return the REDC information for the specified number.
 * The information is cached into a local table so that future calls
 * for this information will be quick.	If the table fills up, then
 * the oldest cached entry is reused.
 *
 * given:
 *	q		modulus to find REDC information of
 */
S_FUNC REDC *
qfindredc(NUMBER *q)
{
	register REDC_CACHE *rcp;
	REDC_CACHE *bestrcp;

	/*
	 * First try for an exact pointer match in the table.
	 */
	for (rcp = redc_cache; rcp <= &redc_cache[MAXREDC-1]; rcp++) {
		if (q == rcp->rnum) {
			rcp->age = ++redc_age;
			return rcp->redc;
		}
	}

	/*
	 * Search the table again looking for a value which matches.
	 */
	for (rcp = redc_cache; rcp <= &redc_cache[MAXREDC-1]; rcp++) {
		if (rcp->age && (qcmp(q, rcp->rnum) == 0)) {
			rcp->age = ++redc_age;
			return rcp->redc;
		}
	}

	/*
	 * Must invalidate an existing entry in the table.
	 * Find the oldest (or first unused) entry.
	 * But first make sure the modulus will be reasonable.
	 */
	if (qisfrac(q) || qisneg(q)) {
		math_error("REDC modulus must be positive odd integer");
		/*NOTREACHED*/
	}

	bestrcp = NULL;
	for (rcp = redc_cache; rcp <= &redc_cache[MAXREDC-1]; rcp++) {
		if ((bestrcp == NULL) || (rcp->age < bestrcp->age))
			bestrcp = rcp;
	}

	/*
	 * Found the best entry.
	 * Free the old information for the entry if necessary,
	 * then initialize it.
	 */
	rcp = bestrcp;
	if (rcp->age) {
		rcp->age = 0;
		qfree(rcp->rnum);
		zredcfree(rcp->redc);
	}

	rcp->redc = zredcalloc(q->num);
	rcp->rnum = qlink(q);
	rcp->age = ++redc_age;
	return rcp->redc;
}

void
showredcdata(void)
{
	REDC_CACHE *rcp;
	long i;

	for (i = 0, rcp = redc_cache; i < MAXREDC; i++, rcp++) {
		if (rcp->age > 0) {
			printf("%-8ld%-8ld", i, rcp->age);
			qprintnum(rcp->rnum, 0, conf->outdigits);
			printf("\n");
		}
	}
}

void
freeredcdata(void)
{
	REDC_CACHE *rcp;
	long i;

	for (i = 0, rcp = redc_cache; i < MAXREDC; i++, rcp++) {
		if (rcp->age > 0) {
			rcp->age = 0;
			qfree(rcp->rnum);
			zredcfree(rcp->redc);
		}
	}
}
