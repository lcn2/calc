/*
 * commath - extended precision complex arithmetic primitive routines
 *
 * Copyright (C) 1999  David I. Bell
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
 * @(#) $Revision: 29.3 $
 * @(#) $Id: commath.c,v 29.3 2002/03/12 09:38:26 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/commath.c,v $
 *
 * Under source code control:	1990/02/15 01:48:10
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "cmath.h"


COMPLEX _czero_ =		{ &_qzero_, &_qzero_, 1 };
COMPLEX _cone_ =		{ &_qone_, &_qzero_, 1 };
COMPLEX _conei_ =		{ &_qzero_, &_qone_, 1 };

static COMPLEX _cnegone_ =	{ &_qnegone_, &_qzero_, 1 };


/*
 * Add two complex numbers.
 */
COMPLEX *
cadd(COMPLEX *c1, COMPLEX *c2)
{
	COMPLEX *r;

	if (ciszero(c1))
		return clink(c2);
	if (ciszero(c2))
		return clink(c1);
	r = comalloc();
	if (!qiszero(c1->real) || !qiszero(c2->real)) {
		qfree(r->real);
		r->real = qqadd(c1->real, c2->real);
	}
	if (!qiszero(c1->imag) || !qiszero(c2->imag)) {
		qfree(r->imag);
		r->imag = qqadd(c1->imag, c2->imag);
	}
	return r;
}


/*
 * Subtract two complex numbers.
 */
COMPLEX *
csub(COMPLEX *c1, COMPLEX *c2)
{
	COMPLEX *r;

	if ((c1->real == c2->real) && (c1->imag == c2->imag))
		return clink(&_czero_);
	if (ciszero(c2))
		return clink(c1);
	r = comalloc();
	if (!qiszero(c1->real) || !qiszero(c2->real)) {
		qfree(r->real);
		r->real = qsub(c1->real, c2->real);
	}
	if (!qiszero(c1->imag) || !qiszero(c2->imag)) {
		qfree(r->imag);
		r->imag = qsub(c1->imag, c2->imag);
	}
	return r;
}


/*
 * Multiply two complex numbers.
 * This saves one multiplication over the obvious algorithm by
 * trading it for several extra additions, as follows.	Let
 *	q1 = (a + b) * (c + d)
 *	q2 = a * c
 *	q3 = b * d
 * Then (a+bi) * (c+di) = (q2 - q3) + (q1 - q2 - q3)i.
 */
COMPLEX *
cmul(COMPLEX *c1, COMPLEX *c2)
{
	COMPLEX *r;
	NUMBER *q1, *q2, *q3, *q4;

	if (ciszero(c1) || ciszero(c2))
		return clink(&_czero_);
	if (cisone(c1))
		return clink(c2);
	if (cisone(c2))
		return clink(c1);
	if (cisreal(c2))
		return cmulq(c1, c2->real);
	if (cisreal(c1))
		return cmulq(c2, c1->real);
	/*
	 * Need to do the full calculation.
	 */
	r = comalloc();
	q2 = qqadd(c1->real, c1->imag);
	q3 = qqadd(c2->real, c2->imag);
	q1 = qmul(q2, q3);
	qfree(q2);
	qfree(q3);
	q2 = qmul(c1->real, c2->real);
	q3 = qmul(c1->imag, c2->imag);
	q4 = qqadd(q2, q3);
	qfree(r->real);
	r->real = qsub(q2, q3);
	qfree(r->imag);
	r->imag = qsub(q1, q4);
	qfree(q1);
	qfree(q2);
	qfree(q3);
	qfree(q4);
	return r;
}


/*
 * Square a complex number.
 */
COMPLEX *
csquare(COMPLEX *c)
{
	COMPLEX *r;
	NUMBER *q1, *q2;

	if (ciszero(c))
		return clink(&_czero_);
	if (cisrunit(c))
		return clink(&_cone_);
	if (cisiunit(c))
		return clink(&_cnegone_);
	r = comalloc();
	if (cisreal(c)) {
		qfree(r->real);
		r->real = qsquare(c->real);
		return r;
	}
	if (cisimag(c)) {
		qfree(r->real);
		q1 = qsquare(c->imag);
		r->real = qneg(q1);
		qfree(q1);
		return r;
	}
	q1 = qsquare(c->real);
	q2 = qsquare(c->imag);
	qfree(r->real);
	r->real = qsub(q1, q2);
	qfree(q1);
	qfree(q2);
	qfree(r->imag);
	q1 = qmul(c->real, c->imag);
	r->imag = qscale(q1, 1L);
	qfree(q1);
	return r;
}


/*
 * Divide two complex numbers.
 */
COMPLEX *
cdiv(COMPLEX *c1, COMPLEX *c2)
{
	COMPLEX *r;
	NUMBER *q1, *q2, *q3, *den;

	if (ciszero(c2)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if ((c1->real == c2->real) && (c1->imag == c2->imag))
		return clink(&_cone_);
	r = comalloc();
	if (cisreal(c1) && cisreal(c2)) {
		qfree(r->real);
		r->real = qqdiv(c1->real, c2->real);
		return r;
	}
	if (cisimag(c1) && cisimag(c2)) {
		qfree(r->real);
		r->real = qqdiv(c1->imag, c2->imag);
		return r;
	}
	if (cisimag(c1) && cisreal(c2)) {
		qfree(r->imag);
		r->imag = qqdiv(c1->imag, c2->real);
		return r;
	}
	if (cisreal(c1) && cisimag(c2)) {
		qfree(r->imag);
		q1 = qqdiv(c1->real, c2->imag);
		r->imag = qneg(q1);
		qfree(q1);
		return r;
	}
	if (cisreal(c2)) {
		qfree(r->real);
		qfree(r->imag);
		r->real = qqdiv(c1->real, c2->real);
		r->imag = qqdiv(c1->imag, c2->real);
		return r;
	}
	q1 = qsquare(c2->real);
	q2 = qsquare(c2->imag);
	den = qqadd(q1, q2);
	qfree(q1);
	qfree(q2);
	q1 = qmul(c1->real, c2->real);
	q2 = qmul(c1->imag, c2->imag);
	q3 = qqadd(q1, q2);
	qfree(q1);
	qfree(q2);
	qfree(r->real);
	r->real = qqdiv(q3, den);
	qfree(q3);
	q1 = qmul(c1->real, c2->imag);
	q2 = qmul(c1->imag, c2->real);
	q3 = qsub(q2, q1);
	qfree(q1);
	qfree(q2);
	qfree(r->imag);
	r->imag = qqdiv(q3, den);
	qfree(q3);
	qfree(den);
	return r;
}


/*
 * Invert a complex number.
 */
COMPLEX *
cinv(COMPLEX *c)
{
	COMPLEX *r;
	NUMBER *q1, *q2, *den;

	if (ciszero(c)) {
		math_error("Inverting zero");
		/*NOTREACHED*/
	}
	r = comalloc();
	if (cisreal(c)) {
		qfree(r->real);
		r->real = qinv(c->real);
		return r;
	}
	if (cisimag(c)) {
		q1 = qinv(c->imag);
		qfree(r->imag);
		r->imag = qneg(q1);
		qfree(q1);
		return r;
	}
	q1 = qsquare(c->real);
	q2 = qsquare(c->imag);
	den = qqadd(q1, q2);
	qfree(q1);
	qfree(q2);
	qfree(r->real);
	r->real = qqdiv(c->real, den);
	q1 = qqdiv(c->imag, den);
	qfree(r->imag);
	r->imag = qneg(q1);
	qfree(q1);
	qfree(den);
	return r;
}


/*
 * Negate a complex number.
 */
COMPLEX *
cneg(COMPLEX *c)
{
	COMPLEX *r;

	if (ciszero(c))
		return clink(&_czero_);
	r = comalloc();
	if (!qiszero(c->real)) {
		qfree(r->real);
		r->real = qneg(c->real);
	}
	if (!qiszero(c->imag)) {
		qfree(r->imag);
		r->imag = qneg(c->imag);
	}
	return r;
}


/*
 * Take the integer part of a complex number.
 * This means take the integer part of both components.
 */
COMPLEX *
cint(COMPLEX *c)
{
	COMPLEX *r;

	if (cisint(c))
		return clink(c);
	r = comalloc();
	qfree(r->real);
	r->real = qint(c->real);
	qfree(r->imag);
	r->imag = qint(c->imag);
	return r;
}


/*
 * Take the fractional part of a complex number.
 * This means take the fractional part of both components.
 */
COMPLEX *
cfrac(COMPLEX *c)
{
	COMPLEX *r;

	if (cisint(c))
		return clink(&_czero_);
	r = comalloc();
	qfree(r->real);
	r->real = qfrac(c->real);
	qfree(r->imag);
	r->imag = qfrac(c->imag);
	return r;
}


/*
 * Take the conjugate of a complex number.
 * This negates the complex part.
 */
COMPLEX *
cconj(COMPLEX *c)
{
	COMPLEX *r;

	if (cisreal(c))
		return clink(c);
	r = comalloc();
	if (!qiszero(c->real)) {
		qfree(r->real);
		r->real = qlink(c->real);
	}
	qfree(r->imag);
	r->imag = qneg(c->imag);
	return r;
}


/*
 * Return the real part of a complex number.
 */
COMPLEX *
c_real(COMPLEX *c)
{
	COMPLEX *r;

	if (cisreal(c))
		return clink(c);
	r = comalloc();
	if (!qiszero(c->real)) {
		qfree(r->real);
		r->real = qlink(c->real);
	}
	return r;
}


/*
 * Return the imaginary part of a complex number as a real.
 */
COMPLEX *
c_imag(COMPLEX *c)
{
	COMPLEX *r;

	if (cisreal(c))
		return clink(&_czero_);
	r = comalloc();
	qfree(r->real);
	r->real = qlink(c->imag);
	return r;
}


/*
 * Add a real number to a complex number.
 */
COMPLEX *
caddq(COMPLEX *c, NUMBER *q)
{
	COMPLEX *r;

	if (qiszero(q))
		return clink(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qqadd(c->real, q);
	r->imag = qlink(c->imag);
	return r;
}


/*
 * Subtract a real number from a complex number.
 */
COMPLEX *
csubq(COMPLEX *c, NUMBER *q)
{
	COMPLEX *r;

	if (qiszero(q))
		return clink(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qsub(c->real, q);
	r->imag = qlink(c->imag);
	return r;
}


/*
 * Shift the components of a complex number left by the specified
 * number of bits.  Negative values shift to the right.
 */
COMPLEX *
cshift(COMPLEX *c, long n)
{
	COMPLEX *r;

	if (ciszero(c) || (n == 0))
		return clink(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qshift(c->real, n);
	r->imag = qshift(c->imag, n);
	return r;
}


/*
 * Scale a complex number by a power of two.
 */
COMPLEX *
cscale(COMPLEX *c, long n)
{
	COMPLEX *r;

	if (ciszero(c) || (n == 0))
		return clink(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qscale(c->real, n);
	r->imag = qscale(c->imag, n);
	return r;
}


/*
 * Multiply a complex number by a real number.
 */
COMPLEX *
cmulq(COMPLEX *c, NUMBER *q)
{
	COMPLEX *r;

	if (qiszero(q))
		return clink(&_czero_);
	if (qisone(q))
		return clink(c);
	if (qisnegone(q))
		return cneg(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qmul(c->real, q);
	r->imag = qmul(c->imag, q);
	return r;
}


/*
 * Divide a complex number by a real number.
 */
COMPLEX *
cdivq(COMPLEX *c, NUMBER *q)
{
	COMPLEX *r;

	if (qiszero(q)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	if (qisone(q))
		return clink(c);
	if (qisnegone(q))
		return cneg(c);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qqdiv(c->real, q);
	r->imag = qqdiv(c->imag, q);
	return r;
}




/*
 * Construct a complex number given the real and imaginary components.
 */
COMPLEX *
qqtoc(NUMBER *q1, NUMBER *q2)
{
	COMPLEX *r;

	if (qiszero(q1) && qiszero(q2))
		return clink(&_czero_);
	r = comalloc();
	qfree(r->real);
	qfree(r->imag);
	r->real = qlink(q1);
	r->imag = qlink(q2);
	return r;
}


/*
 * Compare two complex numbers for equality, returning FALSE if they are equal,
 * and TRUE if they differ.
 */
BOOL
ccmp(COMPLEX *c1, COMPLEX *c2)
{
	BOOL i;

	i = qcmp(c1->real, c2->real);
	if (!i)
		i = qcmp(c1->imag, c2->imag);
	return i;
}


/*
 * Compare two complex numbers and return a complex number with real and
 * imaginary parts -1, 0 or 1 indicating relative values of the real and
 * imaginary parts of the two numbers.
 */
COMPLEX *
crel(COMPLEX *c1, COMPLEX *c2)
{
	COMPLEX *c;

	c = comalloc();
	qfree(c->real);
	qfree(c->imag);
	c->real = itoq((long) qrel(c1->real, c2->real));
	c->imag = itoq((long) qrel(c1->imag, c2->imag));

	return c;
}


/*
 * Allocate a new complex number.
 */
COMPLEX *
comalloc(void)
{
	COMPLEX *r;

	r = (COMPLEX *) malloc(sizeof(COMPLEX));
	if (r == NULL) {
		math_error("Cannot allocate complex number");
		/*NOTREACHED*/
	}
	r->links = 1;
	r->real = qlink(&_qzero_);
	r->imag = qlink(&_qzero_);
	return r;
}


/*
 * Free a complex number.
 */
void
comfree(COMPLEX *c)
{
	if (--(c->links) > 0)
		return;
	qfree(c->real);
	qfree(c->imag);
	free(c);
}
