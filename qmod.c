/*
 * qmod - modular arithmetic routines for normal numbers and REDC numbers
 *
 * Copyright (C) 1999  David I. Bell and Ernest Bowen
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.2 $
 * @(#) $Id: qmod.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/qmod.c,v $
 *
 * Under source code control:	1991/05/22 23:15:07
 * File existed as early as:	1991
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "qmath.h"
#include "config.h"


/*
 * Structure used for caching REDC information.
 */
typedef struct	{
	NUMBER	*rnum;		/* modulus being cached */
	REDC	*redc;		/* REDC information for modulus */
	long	age;		/* age counter for reallocation */
} REDC_CACHE;


static long redc_age;			/* current age counter */
static REDC_CACHE redc_cache[MAXREDC];	/* cached REDC info */


static REDC *qfindredc(NUMBER *q);


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
 * Given two numbers q1, q2, qquomod(q1, q2, retqdiv, retqmod)
 * calculates an integral quotient and numerical remainder such that
 * q1 = q2 * quotient + remainder.  The remainder is zero if
 * q1 is a multiple of q2; the quotient is zero if q2 is zero.
 * In other cases, the remainder always has absolute value less than
 * abs(q2).  Which of the two possible quotient-remainder pairs is returned
 * is determined by the conf->quomod configuration parameter.
 * If the quomod parameter is zero, the remainder has the sign of q2
 * and the qotient is rounded towards zero.
 * The results are returned indirectly through pointers.
 * The function returns FALSE or
 * TRUE according as the remainder is or is not zero.  For
 * example, if conf->quomod = 0,
 *	qquomod(11, 4, &x, &y) sets x to 2, y to 3, and returns TRUE.
 *	qquomod(-7, -3, &x, &y) sets x to 2, y to -1, and returns TRUE.
 *
 * given:
 *	q1		numbers to do quotient with
 *	q2		numbers to do quotient with
 *	retqdiv		returned quotient
 *	retqmod		returned modulo
 */
BOOL
qquomod(NUMBER *q1, NUMBER *q2, NUMBER **retqdiv, NUMBER **retqmod)
{
	NUMBER *qq, *qm;
	ZVALUE tmp1, tmp2, tmp3, tmp4;

	if (qiszero(q2)) {			/* zero modulus case */
		qq = qlink(&_qzero_);
		qm = qlink(q1);
	} else if (qisint(q1) && qisint(q2)) {	/* integer case */
		zdiv(q1->num, q2->num, &tmp1, &tmp2, conf->quomod);
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
		zdiv(tmp1, tmp2, &tmp3, &tmp4, conf->quomod);
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
	*retqdiv = qq;
	*retqmod = qm;
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
static REDC *
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
			qprintnum(rcp->rnum, 0);
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
