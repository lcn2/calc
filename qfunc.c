/*
 * qfunc - extended precision rational arithmetic non-primitive functions
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
 * Under source code control:	1990/02/15 01:48:20
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "qmath.h"
#include "config.h"
#include "prime.h"


#include "banned.h"	/* include after system header <> includes */


STATIC NUMBER **B_table;
STATIC long B_num;
STATIC long B_allocnum;
STATIC NUMBER **E_table;
STATIC long E_num;

#define QALLOCNUM 64

/*
 * Set the default epsilon for approximate calculations.
 * This must be greater than zero.
 *
 * given:
 *	q		number to be set as the new epsilon
 */
void
setepsilon(NUMBER *q)
{
	NUMBER *old;

	if (qisneg(q) || qiszero(q)) {
		math_error("Epsilon value must be greater than zero");
		/*NOTREACHED*/
	}
	old = conf->epsilon;
	conf->epsilonprec = qprecision(q);
	conf->epsilon = qlink(q);
	if (old)
		qfree(old);
}


/*
 * Return the inverse of one number modulo another.
 * That is, find x such that:
 *	Ax = 1 (mod B)
 * Returns zero if the numbers are not relatively prime (temporary hack).
 */
NUMBER *
qminv(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;
	ZVALUE z1, z2, tmp;
	int s, t;
	long rnd;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for minv");
		/*NOTREACHED*/
	}
	if (qiszero(q2)) {
		if (qisunit(q1))
			return qlink(q1);
		return qlink(&_qzero_);
	}
	if (qisunit(q2))
		return qlink(&_qzero_);
	rnd = conf->mod;
	s = (rnd & 4) ? 0 : q2->num.sign;
	if (rnd & 1)
		s^= 1;

	tmp = q2->num;
	tmp.sign = 0;
	if (zmodinv(q1->num, tmp, &z1))
		return qlink(&_qzero_);
	zsub(tmp, z1, &z2);
	if (rnd & 16) {
		t = zrel(z1, z2);
		if (t < 0)
			s = 0;
		else if (t > 0)
			s = 1;
	}
	r = qalloc();
	if (s) {
		zfree(z1);
		z2.sign = TRUE;
		r->num = z2;
		return r;
	}
	zfree(z2);
	r->num = z1;
	return r;
}


/*
 * Return the residue modulo an integer (q3) of an integer (q1)
 * raised to a positive integer (q2) power.
 */
NUMBER *
qpowermod(NUMBER *q1, NUMBER *q2, NUMBER *q3)
{
	NUMBER *r;
	ZVALUE z1, z2, tmp;
	int s, t;
	long rnd;

	if (qisfrac(q1) || qisfrac(q2) || qisfrac(q3)) {
		math_error("Non-integers for pmod");
		/*NOTREACHED*/
	}
	if (qisneg(q2)) {
		math_error("Negative power for pmod");
		/*NOTREACHED*/
	}
	if (qiszero(q3))
		return qpowi(q1, q2);
	if (qisunit(q3))
		return qlink(&_qzero_);
	rnd = conf->mod;
	s = (rnd & 4) ? 0 : q3->num.sign;
	if (rnd & 1)
		s^= 1;
	tmp = q3->num;
	tmp.sign = 0;
	zpowermod(q1->num, q2->num, tmp, &z1);
	if (ziszero(z1)) {
		zfree(z1);
		return qlink(&_qzero_);
	}
	zsub(tmp, z1, &z2);
	if (rnd & 16) {
		t = zrel(z1, z2);
		if (t < 0)
			s = 0;
		else if (t > 0)
			s = 1;
	}
	r = qalloc();
	if (s) {
		zfree(z1);
		z2.sign = TRUE;
		r->num = z2;
		return r;
	}
	zfree(z2);
	r->num = z1;
	return r;
}


/*
 * Return the power function of one number with another.
 * The power must be integral.
 *	q3 = qpowi(q1, q2);
 */
NUMBER *
qpowi(NUMBER *q1, NUMBER *q2)
{
	register NUMBER *r;
	BOOL invert, sign;
	ZVALUE num, zden, z2;

	if (qisfrac(q2)) {
		math_error("Raising number to fractional power");
		/*NOTREACHED*/
	}
	num = q1->num;
	zden = q1->den;
	z2 = q2->num;
	sign = (num.sign && zisodd(z2));
	invert = z2.sign;
	num.sign = 0;
	z2.sign = 0;
	/*
	* Check for trivial cases first.
	*/
	if (ziszero(num) && !ziszero(z2)) {	/* zero raised to a power */
		if (invert) {
			math_error("Zero raised to negative power");
			/*NOTREACHED*/
		}
		return qlink(&_qzero_);
	}
	if (zisunit(num) && zisunit(zden)) {	/* 1 or -1 raised to a power */
		r = (sign ? q1 : &_qone_);
		r->links++;
		return r;
	}
	if (ziszero(z2))	/* raising to zeroth power */
		return qlink(&_qone_);
	if (zisunit(z2)) {	/* raising to power 1 or -1 */
		if (invert)
			return qinv(q1);
		return qlink(q1);
	}
	/*
	 * Not a trivial case.	Do the real work.
	 */
	r = qalloc();
	if (!zisunit(num))
		zpowi(num, z2, &r->num);
	if (!zisunit(zden))
		zpowi(zden, z2, &r->den);
	if (invert) {
		z2 = r->num;
		r->num = r->den;
		r->den = z2;
	}
	r->num.sign = sign;
	return r;
}


/*
 * Given the legs of a right triangle, compute its hypotenuse within
 * the specified error.	 This is sqrt(a^2 + b^2).
 */
NUMBER *
qhypot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon)
{
	NUMBER *tmp1, *tmp2, *tmp3;

	if (qiszero(epsilon)) {
		math_error("Zero epsilon value for hypot");
		/*NOTREACHED*/
	}
	if (qiszero(q1))
		return qqabs(q2);
	if (qiszero(q2))
		return qqabs(q1);
	tmp1 = qsquare(q1);
	tmp2 = qsquare(q2);
	tmp3 = qqadd(tmp1, tmp2);
	qfree(tmp1);
	qfree(tmp2);
	tmp1 = qsqrt(tmp3, epsilon, 24L);
	qfree(tmp3);
	return tmp1;
}


/*
 * Given one leg of a right triangle with unit hypotenuse, calculate
 * the other leg within the specified error.  This is sqrt(1 - a^2).
 * If the wantneg flag is nonzero, then negative square root is returned.
 */
NUMBER *
qlegtoleg(NUMBER *q, NUMBER *epsilon, BOOL wantneg)
{
	NUMBER *res, *qtmp1, *qtmp2;
	ZVALUE num;

	if (qiszero(epsilon)) {
		math_error("Zero epsilon value for legtoleg");
		/*NOTREACHED*/
	}
	if (qisunit(q))
		return qlink(&_qzero_);
	if (qiszero(q)) {
		if (wantneg)
			return qlink(&_qnegone_);
		return qlink(&_qone_);
	}
	num = q->num;
	num.sign = 0;
	if (zrel(num, q->den) >= 0) {
		math_error("Leg too large in legtoleg");
		/*NOTREACHED*/
	}
	qtmp1 = qsquare(q);
	qtmp2 = qsub(&_qone_, qtmp1);
	qfree(qtmp1);
	res = qsqrt(qtmp2, epsilon, 24L);
	qfree(qtmp2);
	if (wantneg) {
		qtmp1 = qneg(res);
		qfree(res);
		res = qtmp1;
	}
	return res;
}


/*
 * Compute the square root of a real number.
 * Type of rounding if any depends on rnd.
 * If rnd & 32 is nonzero, result is exact for square numbers;
 * If rnd & 64 is nonzero, the negative square root is returned;
 * If rnd < 32, result is rounded to a multiple of epsilon
 *	up, down, etc. depending on bits 0, 2, 4 of v.
 */

NUMBER *
qsqrt(NUMBER *q1, NUMBER *epsilon, long rnd)
{
	NUMBER *r, etemp;
	ZVALUE tmp1, tmp2, quo, mul, divisor;
	long  s1, s2, up, RR, RS;
	int sign;

	if (qisneg(q1)) {
		math_error("Square root of negative number");
		/*NOTREACHED*/
	}
	if (qiszero(q1))
		return qlink(&_qzero_);
	sign = (rnd & 64) != 0;
	if (qiszero(epsilon)) {
		math_error("Zero epsilon for qsqrt");
		/*NOTREACHED*/
	}

	etemp = *epsilon;
	etemp.num.sign = 0;
	RS = rnd & 25;
	if (!(RS & 8))
		RS ^= epsilon->num.sign;
	if (rnd & 2)
		RS ^= sign ^ epsilon->num.sign;
	if (rnd & 4)
		RS ^= epsilon->num.sign;
	RR = zisunit(q1->den) && qisunit(epsilon);
	if (rnd & 32 || RR) {
		s1 = zsqrt(q1->num, &tmp1, RS);
		if (RR) {
			if (ziszero(tmp1)) {
				zfree(tmp1);
				return qlink(&_qzero_);
			}
			r = qalloc();
			tmp1.sign = sign;
			r->num = tmp1;
			return r;
		}
		if (!s1) {
			s2 = zsqrt(q1->den, &tmp2, 0);
			if (!s2) {
				r = qalloc();
				tmp1.sign = sign;
				r->num = tmp1;
				r->den = tmp2;
				return r;
			}
			zfree(tmp2);
		}
		zfree(tmp1);
	}
	up = 0;
	zsquare(epsilon->den, &tmp1);
	zmul(tmp1, q1->num, &tmp2);
	zfree(tmp1);
	zsquare(epsilon->num, &tmp1);
	zmul(tmp1, q1->den, &divisor);
	zfree(tmp1);
	if (rnd & 16) {
		zshift(tmp2, 2, &tmp1);
		zfree(tmp2);
		s1 = zquo(tmp1, divisor, &quo, 16);
		zfree(tmp1);
		s2 = zsqrt(quo, &tmp1, s1 ? s1 < 0 : 16);
		zshift(tmp1, -1, &mul);
		up = (*tmp1.v & 1) ? s1 + s2 : -1;
		zfree(tmp1);
	} else {
		s1 = zquo(tmp2, divisor, &quo, 0);
		zfree(tmp2);
		s2 = zsqrt(quo, &mul, 0);
		up = (s1 + s2) ? 0 : -1;
	}
	if (up == 0) {
		if (rnd & 8)
			up = (long)((RS ^ *mul.v) & 1);
		else
			up = RS ^ sign;
	}
	if (up > 0) {
		zadd(mul, _one_, &tmp2);
		zfree(mul);
		mul = tmp2;
	}
	zfree(divisor);
	zfree(quo);
	if (ziszero(mul)) {
		zfree(mul);
		return qlink(&_qzero_);
	}
	r = qalloc();
	zreduce(mul, etemp.den, &tmp1, &r->den);
	zfree(mul);
	tmp1.sign = sign;
	zmul(tmp1, etemp.num, &r->num);
	zfree(tmp1);
	return r;
}


/*
 * Calculate the integral part of the square root of a number.
 * Example:  qisqrt(13) = 3.
 */
NUMBER *
qisqrt(NUMBER *q)
{
	NUMBER *r;
	ZVALUE tmp;

	if (qisneg(q)) {
		math_error("Square root of negative number");
		/*NOTREACHED*/
	}
	if (qiszero(q))
		return qlink(&_qzero_);
	r = qalloc();
	if (qisint(q)) {
		(void) zsqrt(q->num, &r->num,0);
		return r;
	}
	zquo(q->num, q->den, &tmp, 0);
	(void) zsqrt(tmp, &r->num,0);
	freeh(tmp.v);
	return r;
}

/*
 * Return whether or not a number is an exact square.
 */
BOOL
qissquare(NUMBER *q)
{
	BOOL flag;

	flag = zissquare(q->num);
	if (qisint(q) || !flag)
		return flag;
	return zissquare(q->den);
}


/*
 * Compute the greatest integer of the Kth root of a number.
 * Example:  qiroot(85, 3) = 4.
 */
NUMBER *
qiroot(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;
	ZVALUE tmp;

	if (qisneg(q2) || qiszero(q2) || qisfrac(q2)) {
		math_error("Taking number to bad root value");
		/*NOTREACHED*/
	}
	if (qiszero(q1))
		return qlink(&_qzero_);
	if (qisone(q1) || qisone(q2))
		return qlink(q1);
	if (qistwo(q2))
		return qisqrt(q1);
	r = qalloc();
	if (qisint(q1)) {
		zroot(q1->num, q2->num, &r->num);
		return r;
	}
	zquo(q1->num, q1->den, &tmp, 0);
	zroot(tmp, q2->num, &r->num);
	zfree(tmp);
	return r;
}


/*
 * Return the greatest integer of the base 2 log of a number.
 * This is the number such that	 1 <= x / log2(x) < 2.
 * Examples:  qilog2(8) = 3, qilog2(1.3) = 1, qilog2(1/7) = -3.
 *
 * given:
 *	q		number to take log of
 */
long
qilog2(NUMBER *q)
{
	long n;			/* power of two */
	int c;			/* result of comparison */
	ZVALUE tmp1, tmp2;	/* temporary values */

	if (qiszero(q)) {
		math_error("Zero argument for ilog2");
		/*NOTREACHED*/
	}
	if (qisint(q))
		return zhighbit(q->num);
	tmp1 = q->num;
	tmp1.sign = 0;
	n = zhighbit(tmp1) - zhighbit(q->den);
	if (n == 0)
		c = zrel(tmp1, q->den);
	else if (n > 0) {
		zshift(q->den, n, &tmp2);
		c = zrel(tmp1, tmp2);
		zfree(tmp2);
	} else {
		zshift(tmp1, -n, &tmp2);
		c = zrel(tmp2, q->den);
		zfree(tmp2);
	}
	if (c < 0)
		n--;
	return n;
}


/*
 * Return the greatest integer of the base 10 log of a number.
 * This is the number such that	 1 <= x / log10(x) < 10.
 * Examples:  qilog10(100) = 2, qilog10(12.3) = 1, qilog10(.023) = -2.
 *
 * given:
 *	q		number to take log of
 */
long
qilog10(NUMBER *q)
{
	long n;			/* log value */
	ZVALUE tmp1, tmp2;	/* temporary values */

	if (qiszero(q)) {
		math_error("Zero argument for ilog10");
		/*NOTREACHED*/
	}
	tmp1 = q->num;
	tmp1.sign = 0;
	if (qisint(q))
		return zlog10(tmp1, NULL);
	/*
	 * The number is not an integer.
	 * Compute the result if the number is greater than one.
	 */
	if (zrel(tmp1, q->den) > 0) {
		zquo(tmp1, q->den, &tmp2, 0);
		n = zlog10(tmp2, NULL);
		zfree(tmp2);
		return n;
	}
	/*
	 * Here if the number is less than one.
	 * If the number is the inverse of a power of ten, then the
	 * obvious answer will be off by one.  Subtracting one if the
	 * number is the inverse of an integer will fix it.
	 */
	if (zisunit(tmp1))
		zsub(q->den, _one_, &tmp2);
	else
		zquo(q->den, tmp1, &tmp2, 0);
	n = -zlog10(tmp2, NULL) - 1;
	zfree(tmp2);
	return n;
}

/*
 * Return the integer floor of the logarithm of a number relative to
 * a specified integral base.
 */
NUMBER *
qilog(NUMBER *q, ZVALUE base)
{
	long n;
	ZVALUE tmp1, tmp2;

	if (qiszero(q))
		return NULL;

	if (qisunit(q))
		return qlink(&_qzero_);
	if (qisint(q))
		return itoq(zlog(q->num, base));
	tmp1 = q->num;
	tmp1.sign = 0;
	if (zrel(tmp1, q->den) > 0) {
		zquo(tmp1, q->den, &tmp2, 0);
		n = zlog(tmp2, base);
		zfree(tmp2);
		return itoq(n);
	}
	if (zisunit(tmp1))
		zsub(q->den, _one_, &tmp2);
	else
		zquo(q->den, tmp1, &tmp2, 0);
	n = -zlog(tmp2, base) - 1;
	zfree(tmp2);
	return itoq(n);
}


/*
 * Return the number of digits in the representation to a specified
 * base of the integral part of a number.
 *
 * Examples: qdigits(3456,10) = 4, qdigits(-23.45, 10) = 2.
 *
 * One should remember these special cases:
 *
 *	digits(12.3456) == 2    computes with the integer part only
 *	digits(-1234) == 4      computes with the absolute value only
 *	digits(0) == 1          special case
 *	digits(-0.123) == 1     combination of all of the above
 *
 * given:
 *	q		number to count digits of
 */
long
qdigits(NUMBER *q, ZVALUE base)
{
	long n;			/* number of digits */
	ZVALUE temp;		/* temporary value */

	if (zabsrel(q->num, q->den) < 0)
		return 1;
	if (qisint(q))
		return 1 + zlog(q->num, base);
	zquo(q->num, q->den, &temp, 2);
	n = 1 + zlog(temp, base);
	zfree(temp);
	return n;
}


/*
 * Return the digit at the specified place in the expansion to specified
 * base of a rational number.  The places specified by dpos are numbered from
 * the "units" place just before the "decimal" point, so that negative
 * dpos indicates the (-dpos)th place to the right of the point.
 * Examples: qdigit(1234.5678, 1, 10) = 3, qdigit(1234.5678, -3, 10) = 7.
 * The signs of the number and the base are ignored.
 */
NUMBER *
qdigit(NUMBER *q, ZVALUE dpos, ZVALUE base)
{
	ZVALUE N, D;
	ZVALUE K;
	long k;
	ZVALUE A, B, C;		/* temporary integers */
	NUMBER *res;

	/*
	 * In the first stage, q is expressed as base^k * N/D where
	 *	gcd(D, base) = 1
  	 * K is k as a ZVALUE
	 */
	base.sign = 0;
	if (ziszero(base) || zisunit(base))
			return NULL;
	if (qiszero(q) || (qisint(q) && zisneg(dpos)) ||
			(zge31b(dpos) && !zisneg(dpos)))
		return qlink(&_qzero_);
	k = zfacrem(q->num, base, &N);
	if (k == 0) {
		k = zgcdrem(q->den, base, &D);
		if (k > 0) {
			zequo(q->den, D, &A);
			itoz(k, &K);
			zpowi(base, K, &B);
			zfree(K);
			zequo(B, A, &C);
			zfree(A);
			zfree(B);
			zmul(C, q->num, &N);
			zfree(C);
			k = -k;
		}
		else
			N = q->num;
	}
	if (k >= 0)
		D = q->den;

	itoz(k, &K);
	if (zrel(dpos, K) >= 0) {
		zsub(dpos, K, &A);
		zfree(K);
		zpowi(base, A, &B);
		zfree(A);
		zmul(D, B, &A);
		zfree(B);
		zquo(N, A, &B, 0);
	} else {
		if (zisunit(D)) {
			if (k != 0)
				zfree(N);
			zfree(K);
			if (k < 0)
				zfree(D);
			return qlink(&_qzero_);
		}
		zsub(K, dpos, &A);
		zfree(K);
		zpowermod(base, A, D, &C);
		zfree(A);
		zmod(N, D, &A, 0);
		zmul(C, A, &B);
		zfree(A);
		zfree(C);
		zmod(B, D, &A, 0);
		zfree(B);
		zmodinv(D, base, &B);
		zsub(base, B, &C);
		zfree(B);
		zmul(C, A, &B);
		zfree(C);
	}
	zfree(A);
	if (k != 0)
		zfree(N);
	if (k < 0)
		zfree(D);
	zmod(B, base, &A, 0);
	zfree(B);
	if (ziszero(A)) {
		zfree(A);
		return qlink(&_qzero_);
	}
	if (zisone(A)) {
		zfree(A);
		return qlink(&_qone_);
	}
	res = qalloc();
	res->num = A;
	return res;
}


/*
 * Return whether or not a bit is set at the specified bit position in a
 * number.  The lowest bit of the integral part of a number is the zeroth
 * bit position.  Negative bit positions indicate bits to the right of the
 * binary decimal point.  Examples: qdigit(17.1, 0) = 1, qdigit(17.1, -1) = 0.
 */
BOOL
qisset(NUMBER *q, long n)
{
	NUMBER *qtmp1, *qtmp2;
	ZVALUE ztmp;
	BOOL res;

	/*
	 * Zero number or negative bit position place of integer is trivial.
	 */
	if (qiszero(q) || (qisint(q) && (n < 0)))
		return FALSE;
	/*
	 * For non-negative bit positions, answer is easy.
	 */
	if (n >= 0) {
		if (qisint(q))
			return zisset(q->num, n);
		zquo(q->num, q->den, &ztmp, 2);
		res = zisset(ztmp, n);
		zfree(ztmp);
		return res;
	}
	/*
	 * Fractional value and want negative bit position, must work harder.
	 */
	qtmp1 = qscale(q, -n);
	qtmp2 = qint(qtmp1);
	qfree(qtmp1);
	res = ((qtmp2->num.v[0] & 0x01) != 0);
	qfree(qtmp2);
	return res;
}


/*
 * Compute the factorial of an integer.
 *	q2 = qfact(q1);
 */
NUMBER *
qfact(NUMBER *q)
{
	register NUMBER *r;

	if (qisfrac(q)) {
		math_error("Non-integral factorial");
		/*NOTREACHED*/
	}
	if (qiszero(q) || zisone(q->num))
		return qlink(&_qone_);
	r = qalloc();
	zfact(q->num, &r->num);
	return r;
}


/*
 * Compute the product of the primes less than or equal to a number.
 *	q2 = qpfact(q1);
 */
NUMBER *
qpfact(NUMBER *q)
{
	NUMBER *r;

	if (qisfrac(q)) {
		math_error("Non-integral factorial");
		/*NOTREACHED*/
	}
	r = qalloc();
	zpfact(q->num, &r->num);
	return r;
}


/*
 * Compute the lcm of all the numbers less than or equal to a number.
 *	q2 = qlcmfact(q1);
 */
NUMBER *
qlcmfact(NUMBER *q)
{
	NUMBER *r;

	if (qisfrac(q)) {
		math_error("Non-integral lcmfact");
		/*NOTREACHED*/
	}
	r = qalloc();
	zlcmfact(q->num, &r->num);
	return r;
}


/*
 * Compute the permutation function  q1 * (q1-1) * ... * (q1-q2+1).
 */
NUMBER *
qperm(NUMBER *q1, NUMBER *q2)
{
	NUMBER *r;
	NUMBER *qtmp1, *qtmp2;
	long i;

	if (qisfrac(q2)) {
		math_error("Non-integral second arg for permutation");
		/*NOTREACHED*/
	}
	if (qiszero(q2))
		return qlink(&_qone_);
	if (qisone(q2))
		return qlink(q1);
	if (qisint(q1) && !qisneg(q1)) {
		if (qrel(q2, q1) > 0)
			return qlink(&_qzero_);
		if (qispos(q2)) {
			r = qalloc();
			zperm(q1->num, q2->num, &r->num);
			return r;
		}
	}
	if (zge31b(q2->num)) {
		math_error("Too large arg2 for permutation");
		/*NOTREACHED*/
	}
	i = qtoi(q2);
	if (i > 0) {
		q1 = qlink(q1);
		r = qlink(q1);
		while (--i > 0) {
			qtmp1 = qdec(q1);
			qtmp2 = qmul(r, qtmp1);
			qfree(q1);
			q1 = qtmp1;
			qfree(r);
			r = qtmp2;
		}
		qfree(q1);
		return r;
	}
	i = -i;
	qtmp1 = qinc(q1);
	r = qinv(qtmp1);
	while (--i > 0) {
		qtmp2 = qinc(qtmp1);
		qfree(qtmp1);
		qtmp1 = qqdiv(r, qtmp2);
		qfree(r);
		r = qtmp1;
		qtmp1 = qtmp2;
	}
	qfree(qtmp1);
	return r;
}


/*
 * Compute the combinatorial function  q(q - 1) ...(q - n + 1)/n!
 * n is to be a nonnegative integer
 */
NUMBER *
qcomb(NUMBER *q, NUMBER *n)
{
	NUMBER *r;
	NUMBER *q1, *q2;
	long i, j;
	ZVALUE z;

	if (!qisint(n) || qisneg(n)) {
		math_error("Bad second arg in call to qcomb!");
		/*NOTREACHED*/
	}
	if (qisint(q)) {
		switch (zcomb(q->num, n->num, &z)) {
		case 0:
			return qlink(&_qzero_);
		case 1:
			return qlink(&_qone_);
		case -1:
			return qlink(&_qnegone_);
		case 2:
			return qlink(q);
		case -2:
			return NULL;
		default:
			r = qalloc();
			r->num = z;
			return r;
		}
	}
	if (zge31b(n->num))
		return NULL;
	i = ztoi(n->num);
	q = qlink(q);
	r = qlink(q);
	j = 1;
	while (--i > 0) {
		q1 = qdec(q);
		qfree(q);
		q = q1;
		q2 = qmul(r, q);
		qfree(r);
		r = qdivi(q2, ++j);
		qfree(q2);
	}
	qfree(q);
	return r;
}


/*
 * Compute the Bernoulli number with index n
 * For even positive n, newly calculated values for all even indices up
 * to n are stored in table at B_table.
 */
NUMBER *
qbern(ZVALUE z)
{
	long n, i, k, m, nn, dd;
	NUMBER **p;
	NUMBER *s, *s1, *c, *c1, *t;
	size_t sz;

	if (zisone(z))
		return qlink(&_qneghalf_);

	if (zisodd(z) || z.sign)
		return qlink(&_qzero_);

	if (zge31b(z))
		return NULL;

	n = ztoi(z);

	if (n == 0)

		return qlink(&_qone_);

	m = (n >> 1) - 1;

	if (m < B_num)
		return qlink(B_table[m]);

	if (m >= B_allocnum) {
		k = (m/QALLOCNUM + 1) * QALLOCNUM;
		sz = k * sizeof(NUMBER *);
		if (sz < (size_t) k)
			return NULL;
		if (B_allocnum == 0)
			p = (NUMBER **) malloc(sz);
		else
			p = (NUMBER **) realloc(B_table, sz);
		if (p == NULL)
			return NULL;
		B_allocnum = k;
		B_table = p;
	}
	for (k = B_num; k <= m; k++) {
		nn = 2 * k + 3;
		dd = 1;
		c1 = itoq(nn);
		c = qinv(c1);
		qfree(c1);
		s = qsub(&_qonehalf_, c);
		i = k;
		for (i = 0; i < k; i++) {
			c1 = qmuli(c, nn--);
			qfree(c);
			c = qdivi(c1, dd++);
			qfree(c1);
			c1 = qmuli(c, nn--);
			qfree(c);
			c = qdivi(c1, dd++);
			qfree(c1);
			t = qmul(c, B_table[i]);
			s1 = qsub(s, t);
			qfree(t);
			qfree(s);
			s = s1;
		}
		B_table[k] = s;
		qfree(c);
	}
	B_num = k;
	return qlink(B_table[m]);
}


void
qfreebern(void)
{
	long k;

	if (B_num > 0) {
		for (k = 0; k < B_num; k++)
			qfree(B_table[k]);
		free(B_table);
	}
	B_table = NULL;
	B_allocnum = 0;
	B_num = 0;
}


/*
 * Compute the Euler number with index n = z
 * For even positive n, newly calculated values with all even indices up
 * to n are stored in E_table for later quick access.
 */
NUMBER *
qeuler(ZVALUE z)
{
	long i, k, m, n, nn, dd;
	NUMBER **p;
	NUMBER *s, *s1, *c, *c1, *t;
	size_t sz;


	if (ziszero(z))
		return qlink(&_qone_);
	if (zisodd(z) || zisneg(z))
		return qlink(&_qzero_);
	if (zge31b(z))
		return NULL;
	n = ztoi(z);
	m = (n >> 1) - 1;
	if (m < E_num)
		return qlink(E_table[m]);
	sz = (m + 1) * sizeof(NUMBER *);
	if (sz < (size_t) m + 1)
		return NULL;
	if (E_num)
		p = (NUMBER **) realloc(E_table, sz);
	else
		p = (NUMBER **) malloc(sz);
	if (p == NULL)
		return NULL;
	E_table = p;
	for (k = E_num; k <= m; k++) {
		nn = 2 * k + 2;
		dd = 1;
		c = qlink(&_qone_);
		s = qlink(&_qnegone_);
		i = k;
		for (i = 0; i < k; i++) {
			c1 = qmuli(c, nn--);
			qfree(c);
			c = qdivi(c1, dd++);
			qfree(c1);
			c1 = qmuli(c, nn--);
			qfree(c);
			c = qdivi(c1, dd++);
			qfree(c1);
			t = qmul(c, E_table[i]);
			s1 = qsub(s, t);
			qfree(t);
			qfree(s);
			s = s1;
		}
		E_table[k] = s;
		qfree(c);
	}
	E_num = k;
	return qlink(E_table[m]);
}


void
qfreeeuler(void)
{
	long k;

	if (E_num > 0) {
		for (k = 0; k < E_num; k++)
			qfree(E_table[k]);
		free(E_table);
	}
	E_table = NULL;
	E_num = 0;
}


/*
 * Catalan numbers: catalan(n) = comb(2*n, n)/(n+1)
 * To be called only with integer q
 */
NUMBER *
qcatalan(NUMBER *q)
{
	NUMBER *A, *B;
	NUMBER *res;

	if (qisneg(q))
		return qlink(&_qzero_);
	A = qscale(q, 1);
	B = qcomb(A, q);
	if (B == NULL)
		return NULL;
	qfree(A);
	A = qinc(q);
	res = qqdiv(B, A);
	qfree(A);
	qfree(B);
	return res;
}

/*
 * Compute the Jacobi function (a / b).
 * -1 => a is not quadratic residue mod b
 * 1 => b is composite, or a is quad residue of b
 * 0 => b is even or b < 0
 */
NUMBER *
qjacobi(NUMBER *q1, NUMBER *q2)
{
	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integral arguments for jacobi");
		/*NOTREACHED*/
	}
	return itoq((long) zjacobi(q1->num, q2->num));
}


/*
 * Compute the Fibonacci number F(n).
 */
NUMBER *
qfib(NUMBER *q)
{
	register NUMBER *r;

	if (qisfrac(q)) {
		math_error("Non-integral Fibonacci number");
		/*NOTREACHED*/
	}
	r = qalloc();
	zfib(q->num, &r->num);
	return r;
}


/*
 * Truncate a number to the specified number of decimal places.
 */
NUMBER *
qtrunc(NUMBER *q1, NUMBER *q2)
{
	long places;
	NUMBER *r, *e;

	if (qisfrac(q2) || !zistiny(q2->num)) {
		math_error("Bad number of places for qtrunc");
		/*NOTREACHED*/
	}
	places = qtoi(q2);
	e = qtenpow(-places);
	r = qmappr(q1, e, 2);
	qfree(e);
	return r;
}




/*
 * Truncate a number to the specified number of binary places.
 * Specifying zero places makes the result identical to qint.
 */
NUMBER *
qbtrunc(NUMBER *q1, NUMBER *q2)
{
	long places;
	NUMBER *r, *e;

	if (qisfrac(q2) || !zistiny(q2->num)) {
		math_error("Bad number of places for qtrunc");
		/*NOTREACHED*/
	}
	places = qtoi(q2);
	e = qbitvalue(-places);
	r = qmappr(q1, e, 2);
	qfree(e);
	return r;
}


/*
 * Round a number to a specified number of binary places.
 */
NUMBER *
qbround(NUMBER *q, long places, long rnd)
{
	NUMBER *e, *r;

	if (qiszero(q))
		return qlink(&_qzero_);
	if (rnd & 32)
		places -= qilog2(q) + 1;
	e = qbitvalue(-places);
	r = qmappr(q, e, rnd & 31);
	qfree(e);
	return r;
}

/*
 * Round a number to a specified number of decimal digits.
 */
NUMBER *
qround(NUMBER *q, long places, long rnd)
{
	NUMBER *e, *r;

	if (qiszero(q))
		return qlink(&_qzero_);
	if (rnd & 32)
		places -= qilog10(q) + 1;
	e = qtenpow(-places);
	r = qmappr(q, e, rnd & 31);
	qfree(e);
	return r;
}

/*
 * Approximate a number to nearest multiple of a given number.	Whether
 * rounding is down, up, etc. is determined by rnd.
 */
NUMBER *
qmappr(NUMBER *q, NUMBER *e, long rnd)
{
	NUMBER *r;
	ZVALUE tmp1, tmp2, mul;

	if (qiszero(e))
		return qlink(q);
	if (qiszero(q))
		return qlink(&_qzero_);
	zmul(q->num, e->den, &tmp1);
	zmul(q->den, e->num, &tmp2);
	zquo(tmp1, tmp2, &mul, rnd);
	zfree(tmp1);
	zfree(tmp2);
	if (ziszero(mul)) {
		zfree(mul);
		return qlink(&_qzero_);
	}
	r = qalloc();
	zreduce(mul, e->den, &tmp1, &r->den);
	zmul(tmp1, e->num, &r->num);
	zfree(tmp1);
	zfree(mul);
	return r;
}


/*
 * Determine the smallest-denominator rational number in the interval of
 * length abs(epsilon) (< 1) with center or one end at q, or to determine
 * the number nearest above or nearest below q with denominator
 * not exceeding abs(epsilon).
 * Whether the approximation is nearest above or nearest below is
 * determined by rnd and the signs of epsilon and q.
 */

NUMBER *
qcfappr(NUMBER *q, NUMBER *epsilon, long rnd)
{
	NUMBER *res, etemp, *epsilon1;
	ZVALUE num, zden, oldnum, oldden;
	ZVALUE rem, oldrem, quot;
	ZVALUE tmp1, tmp2, tmp3, tmp4;
	ZVALUE denbnd;
	ZVALUE f, g, k;
	BOOL esign;
	int s;
	BOOL bnddencase;
	BOOL useold = FALSE;

	if (qiszero(epsilon) || qisint(q))
		return qlink(q);

	esign = epsilon->num.sign;
	etemp = *epsilon;
	etemp.num.sign = 0;
	bnddencase = (zrel(etemp.num, etemp.den) >= 0);
	if (bnddencase) {
		zquo(etemp.num, etemp.den, &denbnd, 0);
		if (zrel(q->den, denbnd) <= 0) {
			zfree(denbnd);
			return qlink(q);
		}
	} else {
		if (rnd & 16)
			epsilon1 = qscale(epsilon, -1);
		else
			epsilon1 = qlink(epsilon);
		zreduce(q->den, epsilon1->den, &tmp1, &g);
		zmul(epsilon1->num, tmp1, &f);
		f.sign = 0;
		zfree(tmp1);
		qfree(epsilon1);
	}
	if (rnd & 16 && !zistwo(q->den)) {
		s = 0;
	} else {
		s = esign ? -1 : 1;
		if (rnd & 1)
			s = -s;
		if (rnd & 2 && q->num.sign ^ esign)
			s = -s;
		if (rnd & 4 && esign)
			s = -s;
	}
	oldnum = _one_;
	oldden = _zero_;
	zcopy(q->den, &oldrem);
	zdiv(q->num, q->den, &num, &rem, 0);
	zden = _one_;
	for (;;) {
		if (!bnddencase) {
			zmul(f, zden, &tmp1);
			zmul(g, rem, &tmp2);
			if (ziszero(rem) || (s >= 0 && zrel(tmp1,tmp2) >= 0))
				break;
			zfree(tmp1);
			zfree(tmp2);
		}
		zdiv(oldrem, rem, &quot, &tmp1, 0);
		zfree(oldrem);
		oldrem = rem;
		rem = tmp1;
		zmul(quot, zden, &tmp1);
		zadd(tmp1, oldden, &tmp2);
		zfree(tmp1);
		zfree(oldden);
		oldden = zden;
		zden = tmp2;
		zmul(quot, num, &tmp1);
		zadd(tmp1, oldnum, &tmp2);
		zfree(tmp1);
		zfree(oldnum);
		oldnum = num;
		num = tmp2;
		zfree(quot);
		if (bnddencase) {
			if (zrel(zden, denbnd) >= 0)
				break;
		}
		s = -s;
	}
	if (bnddencase) {
		if (s > 0) {
			useold = TRUE;
		} else {
			zsub(zden, denbnd, &tmp1);
			zquo(tmp1, oldden, &k, 1);
			zfree(tmp1);
		}
		zfree(denbnd);
	} else {
		if (s < 0) {
			zfree(tmp1);
			zfree(tmp2);
			zfree(f);
			zfree(g);
			zfree(oldnum);
			zfree(oldden);
			zfree(num);
			zfree(zden);
			zfree(oldrem);
			zfree(rem);
			return qlink(q);
		}
		zsub(tmp1, tmp2, &tmp3);
		zfree(tmp1);
		zfree(tmp2);
		zmul(f, oldden, &tmp1);
		zmul(g, oldrem, &tmp2);
		zfree(f);
		zfree(g);
		zadd(tmp1, tmp2, &tmp4);
		zfree(tmp1);
		zfree(tmp2);
		zquo(tmp3, tmp4, &k, 0);
		zfree(tmp3);
		zfree(tmp4);
	}
	if (!useold && !ziszero(k)) {
		zmul(k, oldnum, &tmp1);
		zsub(num, tmp1, &tmp2);
		zfree(tmp1);
		zfree(num);
		num = tmp2;
		zmul(k, oldden, &tmp1);
		zsub(zden, tmp1, &tmp2);
		zfree(tmp1);
		zfree(zden);
		zden = tmp2;
	}
	if (bnddencase && s == 0) {
		zmul(k, oldrem, &tmp1);
		zadd(rem, tmp1, &tmp2);
		zfree(tmp1);
		zfree(rem);
		rem = tmp2;
		zmul(rem, oldden, &tmp1);
		zmul(zden, oldrem, &tmp2);
		useold = (zrel(tmp1, tmp2) >= 0);
		zfree(tmp1);
		zfree(tmp2);
	}
	if (!bnddencase || s <= 0)
		zfree(k);
	zfree(rem);
	zfree(oldrem);
	res = qalloc();
	if (useold) {
		zfree(num);
		zfree(zden);
		res->num = oldnum;
		res->den = oldden;
		return res;
	}
	zfree(oldnum);
	zfree(oldden);
	res->num = num;
	res->den = zden;
	return res;
}


/*
 * Calculate the nearest-above, or nearest-below, or nearest, number
 * with denominator less than the given number, the choice between
 * possibilities being determined by the parameter rnd.
 */
NUMBER *
qcfsim(NUMBER *q, long rnd)
{
	NUMBER *res;
	ZVALUE tmp1, tmp2, den1, den2;
	int s;

	if (qiszero(q) && rnd & 26)
		return qlink(&_qzero_);
	if (rnd & 24) {
		s = q->num.sign;
	} else {
		s = rnd & 1;
		if (rnd & 2)
			s ^= q->num.sign;
	}
	if (qisint(q)) {
		if ((rnd & 8) && !(rnd & 16))
			return qlink(&_qzero_);
		if (s)
			return qinc(q);
		return qdec(q);
	}
	if (zistwo(q->den)) {
		if (rnd & 16)
			s ^= 1;
		if (s)
			zadd(q->num, _one_, &tmp1);
		else
			zsub(q->num, _one_, &tmp1);
		res = qalloc();
		zshift(tmp1, -1, &res->num);
		zfree(tmp1);
		return res;
	}
	s = s ? 1 : -1;
	if (rnd & 24)
		s = 0;
	res = qalloc();
	zmodinv(q->num, q->den, &den1);
	if (s >= 0) {
		zsub(q->den, den1, &den2);
		if (s > 0 || ((zrel(den1, den2) < 0) ^ (!(rnd & 16)))) {
			zfree(den1);
			res->den = den2;
			zmul(den2, q->num, &tmp1);
			zadd(tmp1, _one_, &tmp2);
			zfree(tmp1);
			zequo(tmp2, q->den, &res->num);
			zfree(tmp2);
			return res;
		}
		zfree(den2);
	}
	res->den = den1;
	zmul(den1, q->num, &tmp1);
	zsub(tmp1, _one_, &tmp2);
	zfree(tmp1);
	zequo(tmp2, q->den, &res->num);
	zfree(tmp2);
	return res;
}



/*
 * Return an indication on whether or not two fractions are approximately
 * equal within the specified epsilon. Returns negative if the absolute value
 * of the difference between the two numbers is less than epsilon, zero if
 * the difference is equal to epsilon, and positive if the difference is
 * greater than epsilon.
 */
FLAG
qnear(NUMBER *q1, NUMBER *q2, NUMBER *epsilon)
{
	int res;
	NUMBER qtemp, etemp, *qq;

	etemp = *epsilon;
	etemp.num.sign = 0;
	if (q1 == q2) {
		if (qiszero(epsilon))
			return 0;
		return -1;
	}
	if (qiszero(epsilon))
		return qcmp(q1, q2);
	if (qiszero(q2)) {
		qtemp = *q1;
		qtemp.num.sign = 0;
		return qrel(&qtemp, &etemp);
	}
	if (qiszero(q1)) {
		qtemp = *q2;
		qtemp.num.sign = 0;
		return qrel(&qtemp, &etemp);
	}
	qq = qsub(q1, q2);
	qtemp = *qq;
	qtemp.num.sign = 0;
	res = qrel(&qtemp, &etemp);
	qfree(qq);
	return res;
}


/*
 * Compute the gcd (greatest common divisor) of two numbers.
 *	q3 = qgcd(q1, q2);
 */
NUMBER *
qgcd(NUMBER *q1, NUMBER *q2)
{
	ZVALUE z;
	NUMBER *q;

	if (q1 == q2)
		return qqabs(q1);
	if (qisfrac(q1) || qisfrac(q2)) {
		q = qalloc();
		zgcd(q1->num, q2->num, &q->num);
		zlcm(q1->den, q2->den, &q->den);
		return q;
	}
	if (qiszero(q1))
		return qqabs(q2);
	if (qiszero(q2))
		return qqabs(q1);
	if (qisunit(q1) || qisunit(q2))
		return qlink(&_qone_);
	zgcd(q1->num, q2->num, &z);
	if (zisunit(z)) {
		zfree(z);
		return qlink(&_qone_);
	}
	q = qalloc();
	q->num = z;
	return q;
}


/*
 * Compute the lcm (least common multiple) of two numbers.
 *	q3 = qlcm(q1, q2);
 */
NUMBER *
qlcm(NUMBER *q1, NUMBER *q2)
{
	NUMBER *q;

	if (qiszero(q1) || qiszero(q2))
		return qlink(&_qzero_);
	if (q1 == q2)
		return qqabs(q1);
	if (qisunit(q1))
		return qqabs(q2);
	if (qisunit(q2))
		return qqabs(q1);
	q = qalloc();
	zlcm(q1->num, q2->num, &q->num);
	if (qisfrac(q1) || qisfrac(q2))
		zgcd(q1->den, q2->den, &q->den);
	return q;
}


/*
 * Remove all occurrences of the specified factor from a number.
 * Returned number is always positive or zero.
 */
NUMBER *
qfacrem(NUMBER *q1, NUMBER *q2)
{
	long count;
	ZVALUE tmp;
	NUMBER *r;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for factor removal");
		/*NOTREACHED*/
	}
	if (qiszero(q2))
		return qqabs(q1);
	if (qiszero(q1))
		return qlink(&_qzero_);
	count = zfacrem(q1->num, q2->num, &tmp);
	if (zisunit(tmp)) {
		zfree(tmp);
		return qlink(&_qone_);
	}
	if (count == 0 && !qisneg(q1)) {
		zfree(tmp);
		return qlink(q1);
	}
	r = qalloc();
	r->num = tmp;
	return r;
}


/*
 * Divide one number by the gcd of it with another number repeatedly until
 * the number is relatively prime.
 */
NUMBER *
qgcdrem(NUMBER *q1, NUMBER *q2)
{
	ZVALUE tmp;
	NUMBER *r;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for gcdrem");
		/*NOTREACHED*/
	}
	if (qiszero(q2))
		return qlink(&_qone_);
	if (qiszero(q1))
		return qlink(&_qzero_);
	if (zgcdrem(q1->num, q2->num, &tmp) == 0)
		return qqabs(q1);
	if (zisunit(tmp)) {
		zfree(tmp);
		return qlink(&_qone_);
	}
	if (zcmp(q1->num, tmp) == 0) {
		zfree(tmp);
		return qlink(q1);
	}
	r = qalloc();
	r->num = tmp;
	return r;
}


/*
 * Return the lowest prime factor of a number.
 * Search is conducted for the specified number of primes.
 * Returns one if no factor was found.
 */
NUMBER *
qlowfactor(NUMBER *q1, NUMBER *q2)
{
	unsigned long count;

	if (qisfrac(q1) || qisfrac(q2)) {
		math_error("Non-integers for lowfactor");
		/*NOTREACHED*/
	}
	count = ztoi(q2->num);
	if (count > PIX_32B) {
		math_error("lowfactor count is too large");
		/*NOTREACHED*/
	}
	return utoq(zlowfactor(q1->num, count));
}

/*
 * Return the number of places after the decimal point needed to exactly
 * represent the specified number as a real number.  Integers return zero,
 * and non-terminating decimals return minus one.  Examples:
 * qdecplaces(1.23) = 2, qdecplaces(3) = 0, qdecplaces(1/7) = -1.
 */
long
qdecplaces(NUMBER *q)
{
	long twopow, fivepow;
	HALF fiveval[2];
	ZVALUE five;
	ZVALUE tmp;

	if (qisint(q))	/* no decimal places if number is integer */
		return 0;
	/*
	 * The number of decimal places of a fraction in lowest terms is finite
	 * if an only if the denominator is of the form 2^A * 5^B, and then the
	 * number of decimal places is equal to MAX(A, B).
	 */
	five.sign = 0;
	five.len = 1;
	five.v = fiveval;
	fiveval[0] = 5;
	fivepow = zfacrem(q->den, five, &tmp);
	if (!zisonebit(tmp)) {
		zfree(tmp);
		return -1;
	}
	twopow = zlowbit(tmp);
	zfree(tmp);
	if (twopow < fivepow)
		twopow = fivepow;
	return twopow;
}


/*
 * Return, if possible, the minimum number of places after the decimal
 * point needed to exactly represent q with the specified base.
 * Integers return 0 and numbers with non-terminating expansions -1.
 * Returns -2 if base inadmissible
 */
long
qplaces(NUMBER *q, ZVALUE base)
{
	long count;
	ZVALUE tmp;

	if (base.len == 1 && base.v[0] == 10)
		return qdecplaces(q);
	if (ziszero(base) || zisunit(base))
			return -2;
	if (qisint(q))
		return 0;
	if (zisonebit(base)) {
		if (!zisonebit(q->den))
			return -1;
		return 1 + (zlowbit(q->den) - 1)/zlowbit(base);
	}
	count = zgcdrem(q->den, base, &tmp);
	if (count == 0)
		return -1;
	if (!zisunit(tmp))
		count = -1;
	zfree(tmp);
	return count;
}


/*
 * Perform a probabilistic primality test (algorithm P in Knuth).
 * Returns FALSE if definitely not prime, or TRUE if probably prime.
 *
 * The absolute value of the 2nd arg determines how many times
 * to check for primality.  If 2nd arg < 0, then the trivial
 * check is omitted.  The 3rd arg determines how tests to
 * initially skip.
 */
BOOL
qprimetest(NUMBER *q1, NUMBER *q2, NUMBER *q3)
{
	if (qisfrac(q1) || qisfrac(q2) || qisfrac(q3)) {
		math_error("Bad arguments for ptest");
		/*NOTREACHED*/
	}
	if (zge24b(q2->num)) {
		math_error("ptest count >= 2^24");
		/*NOTREACHED*/
	}
	return zprimetest(q1->num, ztoi(q2->num), q3->num);
}
