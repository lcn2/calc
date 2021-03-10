/*
 * qmath - extended precision rational arithmetic primitive routines
 *
 * Copyright (C) 1999-2007,2014,2021  David I. Bell and Ernest Bowen
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
 * Under source code control:	1990/02/15 01:48:21
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "qmath.h"
#include "config.h"


#include "banned.h"	/* include after system header <> includes */


NUMBER _qzero_ =	{ { _zeroval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qone_ =		{ { _oneval_, 1, 0  }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qtwo_ =		{ { _twoval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qthree_ =	{ { _threeval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qfour_ =	{ { _fourval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qten_ =		{ { _tenval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qnegone_ =	{ { _oneval_, 1, 1 }, { _oneval_, 1, 0 }, 1, NULL };
NUMBER _qonehalf_ =	{ { _oneval_, 1, 0 }, { _twoval_, 1, 0 }, 1, NULL };
NUMBER _qneghalf_ = 	{ { _oneval_, 1, 1 }, { _twoval_, 1, 0 }, 1, NULL };
NUMBER _qonesqbase_ =	{ { _oneval_, 1, 0 }, { _sqbaseval_, 2, 0 }, 1, NULL };

NUMBER * initnumbs[INITCONSTCOUNT] = {&_qzero_, &_qone_, &_qtwo_, &_qthree_,
	&_qfour_, &_qten_, &_qnegone_, &_qonehalf_, &_qneghalf_};


/*
 * Create another copy of a number.
 *	q2 = qcopy(q1);
 */
NUMBER *
qcopy(NUMBER *q)
{
	register NUMBER *r;

	r = qalloc();
	r->num.sign = q->num.sign;
	if (!zisunit(q->num)) {
		r->num.len = q->num.len;
		r->num.v = alloc(r->num.len);
		zcopyval(q->num, r->num);
	}
	if (!zisunit(q->den)) {
		r->den.len = q->den.len;
		r->den.v = alloc(r->den.len);
		zcopyval(q->den, r->den);
	}
	return r;
}


/*
 * Convert a number to a normal integer.
 *	i = qtoi(q);
 */
long
qtoi(NUMBER *q)
{
	long i;
	ZVALUE res;

	if (qisint(q))
		return ztoi(q->num);
	zquo(q->num, q->den, &res, 0);
	i = ztoi(res);
	zfree(res);
	return i;
}


/*
 * Convert a normal integer into a number.
 *	q = itoq(i);
 */
NUMBER *
itoq(long i)
{
	register NUMBER *q;

	if ((i >= -1) && (i <= 10)) {
		switch ((int) i) {
		case 0: q = &_qzero_; break;
		case 1: q = &_qone_; break;
		case 2: q = &_qtwo_; break;
		case 10: q = &_qten_; break;
		case -1: q = &_qnegone_; break;
		default: q = NULL;
		}
		if (q)
			return qlink(q);
	}
	q = qalloc();
	itoz(i, &q->num);
	return q;
}


/*
 * Convert a number to a normal unsigned integer.
 *	u = qtou(q);
 */
FULL
qtou(NUMBER *q)
{
	FULL i;
	ZVALUE res;

	if (qisint(q))
		return ztou(q->num);
	zquo(q->num, q->den, &res, 0);
	i = ztou(res);
	zfree(res);
	return i;
}


/*
 * Convert a number to a normal signed integer.
 *	s = qtos(q);
 */
SFULL
qtos(NUMBER *q)
{
	SFULL i;
	ZVALUE res;

	if (qisint(q))
		return ztos(q->num);
	zquo(q->num, q->den, &res, 0);
	i = ztos(res);
	zfree(res);
	return i;
}


/*
 * Convert a normal unsigned integer into a number.
 *	q = utoq(i);
 */
NUMBER *
utoq(FULL i)
{
	register NUMBER *q;

	if (i <= 10) {
		switch ((int) i) {
		case 0: q = &_qzero_; break;
		case 1: q = &_qone_; break;
		case 2: q = &_qtwo_; break;
		case 10: q = &_qten_; break;
		default: q = NULL;
		}
		if (q)
			return qlink(q);
	}
	q = qalloc();
	utoz(i, &q->num);
	return q;
}


/*
 * Convert a normal signed integer into a number.
 *	q = stoq(s);
 */
NUMBER *
stoq(SFULL i)
{
	register NUMBER *q;

	if (i <= 10) {
		switch ((int) i) {
		case 0: q = &_qzero_; break;
		case 1: q = &_qone_; break;
		case 2: q = &_qtwo_; break;
		case 10: q = &_qten_; break;
		default: q = NULL;
		}
		if (q)
			return qlink(q);
	}
	q = qalloc();
	stoz(i, &q->num);
	return q;
}


/*
 * Create a number from the given FULL numerator and denominator.
 *	q = uutoq(inum, iden);
 */
NUMBER *
uutoq(FULL inum, FULL iden)
{
	register NUMBER *q;
	FULL d;
	BOOL sign;

	if (iden == 0) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (inum == 0)
		return qlink(&_qzero_);
	sign = 0;
	d = uugcd(inum, iden);
	inum /= d;
	iden /= d;
	if (iden == 1)
		return utoq(inum);
	q = qalloc();
	if (inum != 1)
		utoz(inum, &q->num);
	utoz(iden, &q->den);
	q->num.sign = sign;
	return q;
}


/*
 * Create a number from the given integral numerator and denominator.
 *	q = iitoq(inum, iden);
 */
NUMBER *
iitoq(long inum, long iden)
{
	register NUMBER *q;
	long d;
	BOOL sign;

	if (iden == 0) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (inum == 0)
		return qlink(&_qzero_);
	sign = 0;
	if (inum < 0) {
		sign = 1;
		inum = -inum;
	}
	if (iden < 0) {
		sign = 1 - sign;
		iden = -iden;
	}
	d = iigcd(inum, iden);
	inum /= d;
	iden /= d;
	if (iden == 1)
		return itoq(sign ? -inum : inum);
	q = qalloc();
	if (inum != 1)
		itoz(inum, &q->num);
	itoz(iden, &q->den);
	q->num.sign = sign;
	return q;
}


/*
 * Add two numbers to each other.
 *	q3 = qqadd(q1, q2);
 */
NUMBER *
qqadd(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;
	ZVALUE t1, t2, temp, d1, d2, vpd1, upd1;

	if (qiszero(q1))
		return qlink(q2);
	if (qiszero(q2))
		return qlink(q1);
	r = qalloc();
	/*
	 * If either number is an integer, then the result is easy.
	 */
	if (qisint(q1) && qisint(q2)) {
		zadd(q1->num, q2->num, &r->num);
		return r;
	}
	if (qisint(q2)) {
		zmul(q1->den, q2->num, &temp);
		zadd(q1->num, temp, &r->num);
		zfree(temp);
		zcopy(q1->den, &r->den);
		return r;
	}
	if (qisint(q1)) {
		zmul(q2->den, q1->num, &temp);
		zadd(q2->num, temp, &r->num);
		zfree(temp);
		zcopy(q2->den, &r->den);
		return r;
	}
	/*
	 * Both arguments are true fractions, so we need more work.
	 * If the denominators are relatively prime, then the answer is the
	 * straightforward cross product result with no need for reduction.
	 */
	zgcd(q1->den, q2->den, &d1);
	if (zisunit(d1)) {
		zfree(d1);
		zmul(q1->num, q2->den, &t1);
		zmul(q1->den, q2->num, &t2);
		zadd(t1, t2, &r->num);
		zfree(t1);
		zfree(t2);
		zmul(q1->den, q2->den, &r->den);
		return r;
	}
	/*
	 * The calculation is now more complicated.
	 * See Knuth Vol 2 for details.
	 */
	zquo(q2->den, d1, &vpd1, 0);
	zquo(q1->den, d1, &upd1, 0);
	zmul(q1->num, vpd1, &t1);
	zmul(q2->num, upd1, &t2);
	zadd(t1, t2, &temp);
	zfree(t1);
	zfree(t2);
	zfree(vpd1);
	zgcd(temp, d1, &d2);
	zfree(d1);
	if (zisunit(d2)) {
		zfree(d2);
		r->num = temp;
		zmul(upd1, q2->den, &r->den);
		zfree(upd1);
		return r;
	}
	zquo(temp, d2, &r->num, 0);
	zfree(temp);
	zquo(q2->den, d2, &temp, 0);
	zfree(d2);
	zmul(temp, upd1, &r->den);
	zfree(temp);
	zfree(upd1);
	return r;
}


/*
 * Subtract one number from another.
 *	q3 = qsub(q1, q2);
 */
NUMBER *
qsub(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;

	if (q1 == q2)
		return qlink(&_qzero_);
	if (qiszero(q2))
		return qlink(q1);
	if (qisint(q1) && qisint(q2)) {
		r = qalloc();
		zsub(q1->num, q2->num, &r->num);
		return r;
	}
	q2 = qneg(q2);
	if (qiszero(q1))
		return q2;
	r = qqadd(q1, q2);
	qfree(q2);
	return r;
}


/*
 * Increment a number by one.
 */
NUMBER *
qinc(NUMBER *q)
{
	NUMBER *r;

	r = qalloc();
	if (qisint(q)) {
		zadd(q->num, _one_, &r->num);
		return r;
	}
	zadd(q->num, q->den, &r->num);
	zcopy(q->den, &r->den);
	return r;
}


/*
 * Decrement a number by one.
 */
NUMBER *
qdec(NUMBER *q)
{
	NUMBER *r;

	r = qalloc();
	if (qisint(q)) {
		zsub(q->num, _one_, &r->num);
		return r;
	}
	zsub(q->num, q->den, &r->num);
	zcopy(q->den, &r->den);
	return r;
}


/*
 * Add a normal small integer value to an arbitrary number.
 */
NUMBER *
qaddi(NUMBER *q1, long n)
{
	NUMBER addnum;		/* temporary number */
	HALF addval[2];		/* value of small number */
	BOOL neg;		/* TRUE if number is neg */
#if LONG_BITS > BASEB
	FULL nf;
#endif

	if (n == 0)
		return qlink(q1);
	if (n == 1)
		return qinc(q1);
	if (n == -1)
		return qdec(q1);
	if (qiszero(q1))
		return itoq(n);
	addnum.num.sign = 0;
	addnum.num.v = addval;
	addnum.den = _one_;
	neg = (n < 0);
	if (neg)
		n = -n;
	addval[0] = (HALF) n;
#if LONG_BITS > BASEB
	nf = (((FULL) n) >> BASEB);
	if (nf) {
		addval[1] = (HALF) nf;
		addnum.num.len = 2;
	}
#else
	addnum.num.len = 1;
#endif
	if (neg)
		return qsub(q1, &addnum);
	else
		return qqadd(q1, &addnum);
}


/*
 * Multiply two numbers.
 *	q3 = qmul(q1, q2);
 */
NUMBER *
qmul(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;			/* returned value */
	ZVALUE n1, n2, d1, d2;		/* numerators and denominators */
	ZVALUE tmp;

	if (qiszero(q1) || qiszero(q2))
		return qlink(&_qzero_);
	if (qisone(q1))
		return qlink(q2);
	if (qisone(q2))
		return qlink(q1);
	if (qisint(q1) && qisint(q2)) { /* easy results if integers */
		r = qalloc();
		zmul(q1->num, q2->num, &r->num);
		return r;
	}
	n1 = q1->num;
	n2 = q2->num;
	d1 = q1->den;
	d2 = q2->den;
	if (ziszero(d1) || ziszero(d2)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (ziszero(n1) || ziszero(n2))
		return qlink(&_qzero_);
	if (!zisunit(n1) && !zisunit(d2)) {	/* possibly reduce */
		zgcd(n1, d2, &tmp);
		if (!zisunit(tmp)) {
			zequo(q1->num, tmp, &n1);
			zequo(q2->den, tmp, &d2);
		}
		zfree(tmp);
	}
	if (!zisunit(n2) && !zisunit(d1)) {	/* again possibly reduce */
		zgcd(n2, d1, &tmp);
		if (!zisunit(tmp)) {
			zequo(q2->num, tmp, &n2);
			zequo(q1->den, tmp, &d1);
		}
		zfree(tmp);
	}
	r = qalloc();
	zmul(n1, n2, &r->num);
	zmul(d1, d2, &r->den);
	if (q1->num.v != n1.v)
		zfree(n1);
	if (q1->den.v != d1.v)
		zfree(d1);
	if (q2->num.v != n2.v)
		zfree(n2);
	if (q2->den.v != d2.v)
		zfree(d2);
	return r;
}


/*
 * Multiply a number by a small integer.
 *	q2 = qmuli(q1, n);
 */
NUMBER *
qmuli(NUMBER *q, long n)
{
	NUMBER *r;
	long d;			/* gcd of multiplier and denominator */
	int sign;

	if ((n == 0) || qiszero(q))
		return qlink(&_qzero_);
	if (n == 1)
		return qlink(q);
	r = qalloc();
	if (qisint(q)) {
		zmuli(q->num, n, &r->num);
		return r;
	}
	sign = 1;
	if (n < 0) {
		n = -n;
		sign = -1;
	}
	d = zmodi(q->den, n);
	d = iigcd(d, n);
	zmuli(q->num, (n * sign) / d, &r->num);
	(void) zdivi(q->den, d, &r->den);
	return r;
}


/*
 * Divide two numbers (as fractions).
 *	q3 = qqdiv(q1, q2);
 */
NUMBER *
qqdiv(NUMBER *q1, NUMBER *q2)
{
	NUMBER temp;

	if (qiszero(q2)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if ((q1 == q2) || !qcmp(q1, q2))
		return qlink(&_qone_);
	if (qisone(q1))
		return qinv(q2);
	temp.num = q2->den;
	temp.den = q2->num;
	temp.num.sign = temp.den.sign;
	temp.den.sign = 0;
	temp.links = 1;
	return qmul(q1, &temp);
}


/*
 * Divide a number by a small integer.
 *	q2 = qdivi(q1, n);
 */
NUMBER *
qdivi(NUMBER *q, long n)
{
	NUMBER *r;
	long d;			/* gcd of divisor and numerator */
	int sign;

	if (n == 0) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if ((n == 1) || qiszero(q))
		return qlink(q);
	sign = 1;
	if (n < 0) {
		n = -n;
		sign = -1;
	}
	r = qalloc();
	d = zmodi(q->num, n);
	d = iigcd(d, n);
	(void) zdivi(q->num, d * sign, &r->num);
	zmuli(q->den, n / d, &r->den);
	return r;
}


/*
 * Return the integer quotient of a pair of numbers
 * If q1/q2 is an integer qquo(q1, q2) returns this integer
 * If q2 is zero, zero is returned
 * In other cases whether rounding is down, up, towards zero, etc.
 * is determined by rnd.
 */
NUMBER *
qquo(NUMBER *q1, NUMBER *q2, long rnd)
{
	ZVALUE tmp, tmp1, tmp2;
	NUMBER *q;

	if (qiszero(q1) || qiszero(q2))
		return qlink(&_qzero_);
	if (qisint(q1) && qisint(q2)) {
		zquo(q1->num, q2->num, &tmp, rnd);
	} else {
		zmul(q1->num, q2->den, &tmp1);
		zmul(q2->num, q1->den, &tmp2);
		zquo(tmp1, tmp2, &tmp, rnd);
		zfree(tmp1);
		zfree(tmp2);
	}
	if (ziszero(tmp)) {
		zfree(tmp);
		return qlink(&_qzero_);
	}
	q = qalloc();
	q->num = tmp;
	return q;
}


/*
 * Return the absolute value of a number.
 *	q2 = qqabs(q1);
 */
NUMBER *
qqabs(NUMBER *q)
{
	register NUMBER *r;

	if (q->num.sign == 0)
		return qlink(q);
	r = qalloc();
	if (!zisunit(q->num))
		zcopy(q->num, &r->num);
	if (!zisunit(q->den))
		zcopy(q->den, &r->den);
	r->num.sign = 0;
	return r;
}


/*
 * Negate a number.
 *	q2 = qneg(q1);
 */
NUMBER *
qneg(NUMBER *q)
{
	register NUMBER *r;

	if (qiszero(q))
		return qlink(&_qzero_);
	r = qalloc();
	if (!zisunit(q->num))
		zcopy(q->num, &r->num);
	if (!zisunit(q->den))
		zcopy(q->den, &r->den);
	r->num.sign = !q->num.sign;
	return r;
}


/*
 * Return the sign of a number (-1, 0 or 1)
 */
NUMBER *
qsign(NUMBER *q)
{
	if (qiszero(q))
		return qlink(&_qzero_);
	if (qisneg(q))
		return qlink(&_qnegone_);
	return qlink(&_qone_);
}


/*
 * Invert a number.
 *	q2 = qinv(q1);
 */
NUMBER *
qinv(NUMBER *q)
{
	register NUMBER *r;

	if (qisunit(q)) {
		r = (qisneg(q) ? &_qnegone_ : &_qone_);
		return qlink(r);
	}
	if (qiszero(q)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	r = qalloc();
	if (!zisunit(q->num))
		zcopy(q->num, &r->den);
	if (!zisunit(q->den))
		zcopy(q->den, &r->num);
	r->num.sign = q->num.sign;
	r->den.sign = 0;
	return r;
}


/*
 * Return just the numerator of a number.
 *	q2 = qnum(q1);
 */
NUMBER *
qnum(NUMBER *q)
{
	register NUMBER *r;

	if (qisint(q))
		return qlink(q);
	if (zisunit(q->num)) {
		r = (qisneg(q) ? &_qnegone_ : &_qone_);
		return qlink(r);
	}
	r = qalloc();
	zcopy(q->num, &r->num);
	return r;
}


/*
 * Return just the denominator of a number.
 *	q2 = qden(q1);
 */
NUMBER *
qden(NUMBER *q)
{
	register NUMBER *r;

	if (qisint(q))
		return qlink(&_qone_);
	r = qalloc();
	zcopy(q->den, &r->num);
	return r;
}


/*
 * Return the fractional part of a number.
 *	q2 = qfrac(q1);
 */
NUMBER *
qfrac(NUMBER *q)
{
	register NUMBER *r;

	if (qisint(q))
		return qlink(&_qzero_);
	if ((q->num.len < q->den.len) || ((q->num.len == q->den.len) &&
		(q->num.v[q->num.len - 1] < q->den.v[q->num.len - 1])))
			return qlink(q);
	r = qalloc();
	zmod(q->num, q->den, &r->num, 2);
	zcopy(q->den, &r->den);
	return r;
}


/*
 * Return the integral part of a number.
 *	q2 = qint(q1);
 */
NUMBER *
qint(NUMBER *q)
{
	register NUMBER *r;

	if (qisint(q))
		return qlink(q);
	if ((q->num.len < q->den.len) || ((q->num.len == q->den.len) &&
		(q->num.v[q->num.len - 1] < q->den.v[q->num.len - 1])))
			return qlink(&_qzero_);
	r = qalloc();
	zquo(q->num, q->den, &r->num, 2);
	return r;
}


/*
 * Compute the square of a number.
 */
NUMBER *
qsquare(NUMBER *q)
{
	ZVALUE num, zden;

	if (qiszero(q))
		return qlink(&_qzero_);
	if (qisunit(q))
		return qlink(&_qone_);
	num = q->num;
	zden = q->den;
	q = qalloc();
	if (!zisunit(num))
		zsquare(num, &q->num);
	if (!zisunit(zden))
		zsquare(zden, &q->den);
	return q;
}


/*
 * Shift an integer by a given number of bits. This multiplies the number
 * by the appropriate power of two.  Positive numbers shift left, negative
 * ones shift right.  Low bits are truncated when shifting right.
 */
NUMBER *
qshift(NUMBER *q, long n)
{
	register NUMBER *r;

	if (qisfrac(q)) {
		math_error("Shift of non-integer");
		/*NOTREACHED*/
	}
	if (qiszero(q) || (n == 0))
		return qlink(q);
	if (n <= -(q->num.len * BASEB))
		return qlink(&_qzero_);
	r = qalloc();
	zshift(q->num, n, &r->num);
	return r;
}


/*
 * Scale a number by a power of two, as in:
 *	ans = q * 2^n.
 * This is similar to shifting, except that fractions work.
 */
NUMBER *
qscale(NUMBER *q, long pow)
{
	long numshift, denshift, tmp;
	NUMBER *r;

	if (qiszero(q) || (pow == 0))
		return qlink(q);
	numshift = zisodd(q->num) ? 0 : zlowbit(q->num);
	denshift = zisodd(q->den) ? 0 : zlowbit(q->den);
	if (pow > 0) {
		tmp = pow;
		if (tmp > denshift)
		tmp = denshift;
		denshift = -tmp;
		numshift = (pow - tmp);
	} else {
		pow = -pow;
		tmp = pow;
		if (tmp > numshift)
			tmp = numshift;
		numshift = -tmp;
		denshift = (pow - tmp);
	}
	r = qalloc();
	if (numshift)
		zshift(q->num, numshift, &r->num);
	else
		zcopy(q->num, &r->num);
	if (denshift)
		zshift(q->den, denshift, &r->den);
	else
		zcopy(q->den, &r->den);
	return r;
}


/*
 * Return the minimum of two numbers.
 */
NUMBER *
qmin(NUMBER *q1, NUMBER *q2)
{
	if (q1 == q2)
		return qlink(q1);
	if (qrel(q1, q2) > 0)
		q1 = q2;
	return qlink(q1);
}


/*
 * Return the maximum of two numbers.
 */
NUMBER *
qmax(NUMBER *q1, NUMBER *q2)
{
	if (q1 == q2)
		return qlink(q1);
	if (qrel(q1, q2) < 0)
		q1 = q2;
	return qlink(q1);
}


/*
 * Perform the bitwise OR of two integers.
 */
NUMBER *
qor(NUMBER *q1, NUMBER *q2)
{
	register NUMBER *r;
	NUMBER *q1tmp, *q2tmp, *q;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for bitwise or");
		/*NOTREACHED*/
	}
	if (qcmp(q1,q2) == 0 || qiszero(q2))
		return qlink(q1);
	if (qiszero(q1))
		return qlink(q2);
	if (qisneg(q1)) {
		q1tmp = qcomp(q1);
		if (qisneg(q2)) {
			q2tmp = qcomp(q2);
			q = qand(q1tmp,q2tmp);
			r = qcomp(q);
			qfree(q1tmp);
			qfree(q2tmp);
			qfree(q);
			return r;
		}
		q = qandnot(q1tmp, q2);
		qfree(q1tmp);
		r = qcomp(q);
		qfree(q);
		return r;
	}
	if (qisneg(q2)) {
		q2tmp = qcomp(q2);
		q = qandnot(q2tmp, q1);
		qfree(q2tmp);
		r = qcomp(q);
		qfree(q);
		return r;
	}
	r = qalloc();
	zor(q1->num, q2->num, &r->num);
	return r;
}


/*
 * Perform the bitwise AND of two integers.
 */
NUMBER *
qand(NUMBER *q1, NUMBER *q2)
{
	register NUMBER *r;
	NUMBER *q1tmp, *q2tmp, *q;
	ZVALUE res;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for bitwise and");
		/*NOTREACHED*/
	}
	if (qcmp(q1, q2) == 0)
		return qlink(q1);
	if (qiszero(q1) || qiszero(q2))
		return qlink(&_qzero_);
	if (qisneg(q1)) {
		q1tmp = qcomp(q1);
		if (qisneg(q2)) {
			q2tmp = qcomp(q2);
			q = qor(q1tmp, q2tmp);
			qfree(q1tmp);
			qfree(q2tmp);
			r = qcomp(q);
			qfree(q);
			return r;
		}
		r = qandnot(q2, q1tmp);
		qfree(q1tmp);
		return r;
	}
	if (qisneg(q2)) {
		q2tmp = qcomp(q2);
		r = qandnot(q1, q2tmp);
		qfree(q2tmp);
		return r;
	}
	zand(q1->num, q2->num, &res);
	if (ziszero(res)) {
		zfree(res);
		return qlink(&_qzero_);
	}
	r = qalloc();
	r->num = res;
	return r;
}


/*
 * Perform the bitwise XOR of two integers.
 */
NUMBER *
qxor(NUMBER *q1, NUMBER *q2)
{
	register NUMBER *r;
	NUMBER *q1tmp, *q2tmp, *q;
	ZVALUE res;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for bitwise xor");
		/*NOTREACHED*/
	}
	if (qcmp(q1,q2) == 0)
		return qlink(&_qzero_);
	if (qiszero(q1))
		return qlink(q2);
	if (qiszero(q2))
		return qlink(q1);
	if (qisneg(q1)) {
		q1tmp = qcomp(q1);
		if (qisneg(q2)) {
			q2tmp = qcomp(q2);
			r = qxor(q1tmp, q2tmp);
			qfree(q1tmp);
			qfree(q2tmp);
			return r;
		}
		q = qxor(q1tmp, q2);
		qfree(q1tmp);
		r = qcomp(q);
		qfree(q);
		return r;
	}
	if (qisneg(q2)) {
		q2tmp = qcomp(q2);
		q = qxor(q1, q2tmp);
		qfree(q2tmp);
		r = qcomp(q);
		qfree(q);
		return r;
	}
	zxor(q1->num, q2->num, &res);
	if (ziszero(res)) {
		zfree(res);
		return qlink(&_qzero_);
	}
	r = qalloc();
	r->num = res;
	return r;
}


/*
 * Perform the bitwise AND-NOT of two integers.
 */
NUMBER *
qandnot(NUMBER *q1, NUMBER *q2)
{
	register NUMBER *r;
	NUMBER *q1tmp, *q2tmp, *q;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for bitwise xor");
		/*NOTREACHED*/
	}
	if (qcmp(q1,q2) == 0 || qiszero(q1))
		return qlink(&_qzero_);
	if (qiszero(q2))
		return qlink(q1);
	if (qisneg(q1)) {
		q1tmp = qcomp(q1);
		if (qisneg(q2)) {
			q2tmp = qcomp(q2);
			r = qandnot(q2tmp, q1tmp);
			qfree(q1tmp);
			qfree(q2tmp);
			return r;
		}
		q = qor(q1tmp, q2);
		qfree(q1tmp);
		r = qcomp(q);
		qfree(q);
		return r;
	}
	if (qisneg(q2)) {
		q2tmp = qcomp(q2);
		r = qand(q1, q2tmp);
		qfree(q2tmp);
		return r;
	}
	r = qalloc();
	zandnot(q1->num, q2->num, &r->num);
	return r;
}

/*
 * Return the bitwise "complement" of a number.	 This is - q -1 if q is an
 * integer, - q otherwise.
 */
NUMBER *
qcomp(NUMBER *q)
{
	NUMBER *qtmp;
	NUMBER *res;

	if (qiszero(q))
		return qlink(&_qnegone_);
	if (qisnegone(q))
		return qlink(&_qzero_);
	qtmp = qneg(q);
	if (qisfrac(q))
		return qtmp;
	res = qdec(qtmp);
	qfree(qtmp);
	return res;
}


/*
 * Return the number whose binary representation only has the specified
 * bit set (counting from zero).  This thus produces a given power of two.
 */
NUMBER *
qbitvalue(long n)
{
	register NUMBER *r;

	if (n == 0)
		return qlink(&_qone_);
	r = qalloc();
	if (n > 0)
		zbitvalue(n, &r->num);
	else
		zbitvalue(-n, &r->den);
	return r;
}

/*
 * Return 10^n
 */
NUMBER *
qtenpow(long n)
{
	register NUMBER *r;

	if (n == 0)
		return qlink(&_qone_);
	r = qalloc();
	if (n > 0)
		ztenpow(n, &r->num);
	else
		ztenpow(-n, &r->den);
	return r;
}


/*
 * Return the precision of a number (usually for examining an epsilon value).
 * The precision of a number e less than 1 is the positive
 * integer p for which e = 2^-p * f, where 1 <= f < 2.
 * Numbers greater than or equal to one have a precision of zero.
 * For example, the precision of e is 6 if 1/64 <= e < 1/32.
 */
long
qprecision(NUMBER *q)
{
	long r;

	if (qiszero(q) || qisneg(q)) {
		math_error("Non-positive number for precision");
		/*NOTREACHED*/
	}
	r = - qilog2(q);
	return (r < 0 ? 0 : r);
}


/*
 * Determine whether or not one number exactly divides another one.
 * Returns TRUE if the first number is an integer multiple of the second one.
 */
BOOL
qdivides(NUMBER *q1, NUMBER *q2)
{
	if (qiszero(q1))
		return TRUE;
	if (qisint(q1) && qisint(q2)) {
		if (qisunit(q2))
			return TRUE;
		return zdivides(q1->num, q2->num);
	}
	return zdivides(q1->num, q2->num) && zdivides(q2->den, q1->den);
}


/*
 * Compare two numbers and return an integer indicating their relative size.
 *	i = qrel(q1, q2);
 */
FLAG
qrel(NUMBER *q1, NUMBER *q2)
{
	ZVALUE z1, z2;
	long wc1, wc2;
	int sign;
	int z1f = 0, z2f = 0;

	if (q1 == q2)
		return 0;
	sign = q2->num.sign - q1->num.sign;
	if (sign)
		return sign;
	if (qiszero(q2))
		return !qiszero(q1);
	if (qiszero(q1))
		return -1;
	/*
	 * Make a quick comparison by calculating the number of words
	 * resulting as if we multiplied through by the denominators,
	 * and then comparing the word counts.
	 */
	sign = 1;
	if (qisneg(q1))
		sign = -1;
	wc1 = q1->num.len + q2->den.len;
	wc2 = q2->num.len + q1->den.len;
	if (wc1 < wc2 - 1)
		return -sign;
	if (wc2 < wc1 - 1)
		return sign;
	/*
	 * Quick check failed, must actually do the full comparison.
	 */
	if (zisunit(q2->den)) {
		z1 = q1->num;
	} else if (zisone(q1->num)) {
		z1 = q2->den;
	} else {
		z1f = 1;
		zmul(q1->num, q2->den, &z1);
	}
	if (zisunit(q1->den)) {
		z2 = q2->num;
	} else if (zisone(q2->num)) {
		z2 = q1->den;
	} else {
		z2f = 1;
		zmul(q2->num, q1->den, &z2);
	}
	sign = zrel(z1, z2);
	if (z1f)
		zfree(z1);
	if (z2f)
		zfree(z2);
	return sign;
}


/*
 * Compare two numbers to see if they are equal.
 * This differs from qrel in that the numbers are not ordered.
 * Returns TRUE if they differ.
 */
BOOL
qcmp(NUMBER *q1, NUMBER *q2)
{
	if (q1 == q2)
		return FALSE;
	if ((q1->num.sign != q2->num.sign) || (q1->num.len != q2->num.len) ||
		(q1->den.len != q2->den.len) || (*q1->num.v != *q2->num.v) ||
		(*q1->den.v != *q2->den.v))
			return TRUE;
	if (zcmp(q1->num, q2->num))
		return TRUE;
	if (qisint(q1))
		return FALSE;
	return zcmp(q1->den, q2->den);
}


/*
 * Compare a number against a normal small integer.
 * Returns 1, 0, or -1, according to whether the first number is greater,
 * equal, or less than the second number.
 *	res = qreli(q, n);
 */
FLAG
qreli(NUMBER *q, long n)
{
	ZVALUE z1, z2;
	FLAG res;

	if (qiszero(q))
		return ((n > 0) ? -1 : (n < 0));

	if (n == 0)
		return (q->num.sign ? -1 : 0);

	if (q->num.sign != (n < 0))
		return ((n < 0) ? 1 : -1);

	itoz(n, &z1);

	if (qisfrac(q)) {
		zmul(q->den, z1, &z2);
		zfree(z1);
		z1 = z2;
	}

	res = zrel(q->num, z1);
	zfree(z1);
	return res;
}


/*
 * Compare a number against a small integer to see if they are equal.
 * Returns TRUE if they differ.
 */
BOOL
qcmpi(NUMBER *q, long n)
{
	long len;
#if LONG_BITS > BASEB
	FULL nf;
#endif

	len = q->num.len;
	if (qisfrac(q) || (q->num.sign != (n < 0)))
		return TRUE;
	if (n < 0)
		n = -n;
	if (((HALF)(n)) != q->num.v[0])
		return TRUE;
#if LONG_BITS > BASEB
	nf = ((FULL) n) >> BASEB;
	return ((nf != 0 || len > 1) && (len != 2 || nf != q->num.v[1]));
#else
	return (len > 1);
#endif
}


/*
 * Number node allocation routines
 */

#define NNALLOC 1000


STATIC NUMBER	*freeNum = NULL;
STATIC NUMBER	**firstNums = NULL;
STATIC long	blockcount = 0;


NUMBER *
qalloc(void)
{
	NUMBER *temp;
	NUMBER ** newfn;

	if (freeNum == NULL) {
		freeNum = (NUMBER *) malloc(sizeof (NUMBER) * NNALLOC);
		if (freeNum == NULL) {
			math_error("Not enough memory");
			/*NOTREACHED*/
		}
		freeNum[NNALLOC - 1].next = NULL;
		freeNum[NNALLOC - 1].links = 0;

		/*
		 * We prevent the temp pointer from walking behind freeNum
		 * by stopping one short of the end and running the loop one
		 * more time.
		 *
		 * We would stop the loop with just temp >= freeNum, but
		 * doing this helps make code checkers such as insure happy.
		 */
		for (temp = freeNum + NNALLOC - 2; temp > freeNum; --temp) {
			temp->next = temp + 1;
			temp->links = 0;
		}
		/* run the loop manually one last time */
		temp->next = temp + 1;
		temp->links = 0;

		blockcount++;
		if (firstNums == NULL) {
		    newfn = (NUMBER **) malloc(blockcount * sizeof(NUMBER *));
		} else {
		    newfn = (NUMBER **)
			    realloc(firstNums, blockcount * sizeof(NUMBER *));
		}
		if (newfn == NULL) {
			math_error("Cannot allocate new number block");
			/*NOTREACHED*/
		}
		firstNums = newfn;
		firstNums[blockcount - 1] = freeNum;
	}
	temp = freeNum;
	freeNum = temp->next;
	temp->links = 1;
	temp->num = _one_;
	temp->den = _one_;
	return temp;
}


void
qfreenum(NUMBER *q)
{
	if (q == NULL) {
		math_error("Calling qfreenum with null argument!!!");
		/*NOTREACHED*/
	}
	if (q->links != 0) {
		math_error("Calling qfreenum with nozero links!!!");
		/*NOTREACHED*/
	}
	zfree(q->num);
	zfree(q->den);
	q->next = freeNum;
	freeNum = q;
}

void
shownumbers(void)
{
	NUMBER *vp;
	long i, j, k;
	long count = 0;

	printf("Index  Links  Digits	       Value\n");
	printf("-----  -----  ------	       -----\n");

	for (i = 0, k = 0; i < INITCONSTCOUNT; i++) {
		count++;
		vp = initnumbs[i];
		printf("%6ld  %4ld  ", k++, vp->links);
		fitprint(vp, 40);
		printf("\n");
	}

	for (i = 0; i < blockcount; i++) {
		vp = firstNums[i];
		for (j = 0; j < NNALLOC; j++, k++, vp++) {
			if (vp->links > 0) {
				count++;
				printf("%6ld  %4ld  ", k, vp->links);
				fitprint(vp, 40);
				printf("\n");
			}
		}
	}
	printf("\nNumber: %ld\n", count);
}
