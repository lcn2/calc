/*
 * qmath - declarations for extended precision rational arithmetic
 *
 * Copyright (C) 1999-2007,2014  David I. Bell
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
 * Under source code control:	1993/07/30 19:42:47
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_QMATH_H)
#define INCLUDE_QMATH_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "zmath.h"
#else
# include <calc/zmath.h>
#endif


#define INITCONSTCOUNT 9    /* number of initnumbs[] pre-defined constants */

/*
 * Rational arithmetic definitions.
 */
struct number {
	ZVALUE num;		/* numerator (containing sign) */
	ZVALUE den;		/* denominator (always positive) */
	long links;		/* number of links to this value */
	struct number *next;	/* pointer to next number */
};

typedef struct number NUMBER;

EXTERN NUMBER _qlge_;

/*
 * Input. output, allocation, and conversion routines.
 */
E_FUNC NUMBER *qalloc(void);
E_FUNC NUMBER *qcopy(NUMBER *q);
E_FUNC NUMBER *uutoq(FULL i1, FULL i2);
E_FUNC NUMBER *iitoq(long i1, long i2);
E_FUNC NUMBER *str2q(char *str);
E_FUNC NUMBER *itoq(long i);
E_FUNC NUMBER *utoq(FULL i);
E_FUNC NUMBER *stoq(SFULL i);
E_FUNC long qtoi(NUMBER *q);
E_FUNC FULL qtou(NUMBER *q);
E_FUNC SFULL qtos(NUMBER *q);
E_FUNC long qparse(char *str, int flags);
E_FUNC void qfreenum(NUMBER *q);
E_FUNC void qprintnum(NUMBER *q, int mode, LEN outdigits);
E_FUNC void qprintff(NUMBER *q, long width, long precision);
E_FUNC void qprintfe(NUMBER *q, long width, long precision);
E_FUNC void qprintfr(NUMBER *q, long width, BOOL force);
E_FUNC void qprintfd(NUMBER *q, long width);
E_FUNC void qprintfx(NUMBER *q, long width);
E_FUNC void qprintfb(NUMBER *q, long width);
E_FUNC void qprintfo(NUMBER *q, long width);
E_FUNC void qprintf(char *, ...);
E_FUNC void shownumbers(void);
E_FUNC void showredcdata(void);
E_FUNC void freeredcdata(void);
E_FUNC void fitprint(NUMBER *, long);



/*
 * Basic numeric routines.
 */
E_FUNC NUMBER *qaddi(NUMBER *q, long i);
E_FUNC NUMBER *qmuli(NUMBER *q, long i);
E_FUNC NUMBER *qdivi(NUMBER *q, long i);
E_FUNC NUMBER *qqadd(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qsub(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qmul(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qqdiv(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qquo(NUMBER *q1, NUMBER *q2, long rnd);
E_FUNC NUMBER *qmod(NUMBER *q1, NUMBER *q2, long rnd);
E_FUNC NUMBER *qmin(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qmax(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qand(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qor(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qxor(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qandnot(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qcomp(NUMBER *q);
E_FUNC NUMBER *qpowermod(NUMBER *q1, NUMBER *q2, NUMBER *q3);
E_FUNC NUMBER *qpowi(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qsquare(NUMBER *q);
E_FUNC NUMBER *qneg(NUMBER *q);
E_FUNC NUMBER *qsign(NUMBER *q);
E_FUNC NUMBER *qint(NUMBER *q);
E_FUNC NUMBER *qfrac(NUMBER *q);
E_FUNC NUMBER *qnum(NUMBER *q);
E_FUNC NUMBER *qden(NUMBER *q);
E_FUNC NUMBER *qinv(NUMBER *q);
E_FUNC NUMBER *qqabs(NUMBER *q);
E_FUNC NUMBER *qinc(NUMBER *q);
E_FUNC NUMBER *qdec(NUMBER *q);
E_FUNC NUMBER *qshift(NUMBER *q, long n);
E_FUNC NUMBER *qtrunc(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qround(NUMBER *q, long places, long rnd);
E_FUNC NUMBER *qbtrunc(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qbround(NUMBER *q, long places, long rnd);
E_FUNC NUMBER *qscale(NUMBER *q, long i);
E_FUNC BOOL qdivides(NUMBER *q1, NUMBER *q2);
E_FUNC BOOL qcmp(NUMBER *q1, NUMBER *q2);
E_FUNC BOOL qcmpi(NUMBER *q, long i);
E_FUNC FLAG qrel(NUMBER *q1, NUMBER *q2);
E_FUNC FLAG qreli(NUMBER *q, long i);
E_FUNC BOOL qisset(NUMBER *q, long i);


/*
 * More complicated numeric functions.
 */
E_FUNC NUMBER *qcomb(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qgcd(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qlcm(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qfact(NUMBER *q);
E_FUNC NUMBER *qpfact(NUMBER *q);
E_FUNC NUMBER *qminv(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qfacrem(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qperm(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qgcdrem(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qlowfactor(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qfib(NUMBER *q);
E_FUNC NUMBER *qcfappr(NUMBER *q, NUMBER *epsilon, long R);
E_FUNC NUMBER *qcfsim(NUMBER *q, long R);
E_FUNC NUMBER *qisqrt(NUMBER *q);
E_FUNC NUMBER *qjacobi(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qiroot(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qmappr(NUMBER *q, NUMBER *e, long R);
E_FUNC NUMBER *qlcmfact(NUMBER *q);
E_FUNC NUMBER *qredcin(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qredcout(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qredcmul(NUMBER *q1, NUMBER *q2, NUMBER *q3);
E_FUNC NUMBER *qredcsquare(NUMBER *q1, NUMBER *q2);
E_FUNC NUMBER *qredcpower(NUMBER *q1, NUMBER *q2, NUMBER *q3);
E_FUNC BOOL qprimetest(NUMBER *q1, NUMBER *q2, NUMBER *q3);
E_FUNC BOOL qissquare(NUMBER *q);
E_FUNC long qilog2(NUMBER *q);
E_FUNC long qilog10(NUMBER *q);
E_FUNC NUMBER *qilog(NUMBER *q, ZVALUE base);
E_FUNC BOOL qcmpmod(NUMBER *q1, NUMBER *q2, NUMBER *q3);
E_FUNC BOOL qquomod(NUMBER *q1, NUMBER *q2, NUMBER **quo, NUMBER **mod,
		    long rnd);
E_FUNC FLAG qnear(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC NUMBER *qdigit(NUMBER *q, ZVALUE dpos, ZVALUE base);
E_FUNC long qprecision(NUMBER *q);
E_FUNC long qplaces(NUMBER *q, ZVALUE base);
E_FUNC long qdecplaces(NUMBER *q);
E_FUNC long qdigits(NUMBER *q, ZVALUE base);
E_FUNC void setepsilon(NUMBER *q);
E_FUNC NUMBER *qbitvalue(long i);
E_FUNC NUMBER *qtenpow(long i);


/*
 * Transcendental functions.  These all take an epsilon argument to
 * specify the required accuracy of the calculation.
 */
E_FUNC void qsincos(NUMBER *q, long bitnum, NUMBER **vs, NUMBER **vc);
E_FUNC NUMBER *qsqrt(NUMBER *q, NUMBER *epsilon, long R);
E_FUNC NUMBER *qpower(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC NUMBER *qroot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC NUMBER *qcos(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qsin(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qexp(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qln(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qlog(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qtan(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qsec(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qcot(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qcsc(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacos(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qasin(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qatan(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qasec(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacsc(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacot(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qatan2(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC NUMBER *qhypot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC NUMBER *qcosh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qsinh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qtanh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qcoth(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qsech(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qcsch(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacosh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qasinh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qatanh(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qasech(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacsch(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qacoth(NUMBER *q, NUMBER *epsilon);
E_FUNC NUMBER *qlegtoleg(NUMBER *q, NUMBER *epsilon, BOOL wantneg);
E_FUNC NUMBER *qpi(NUMBER *epsilon);
E_FUNC NUMBER *qcatalan(NUMBER *);
E_FUNC NUMBER *qbern(ZVALUE z);
E_FUNC void qfreebern(void);
E_FUNC NUMBER *qeuler(ZVALUE z);
E_FUNC void qfreeeuler(void);


/*
 * pseudo-seed generator
 */
E_FUNC NUMBER *pseudo_seed(void);


/*
 * external swap functions
 */
E_FUNC NUMBER *swap_b8_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);
E_FUNC NUMBER *swap_b16_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);
E_FUNC NUMBER *swap_HALF_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);


/*
 * macro expansions to speed this thing up
 */
#define qiszero(q)	(ziszero((q)->num))
#define qisneg(q)	(zisneg((q)->num))
#define qispos(q)	(zispos((q)->num))
#define qisint(q)	(zisunit((q)->den))
#define qisfrac(q)	(!zisunit((q)->den))
#define qisunit(q)	(zisunit((q)->num) && zisunit((q)->den))
#define qisone(q)	(zisone((q)->num) && zisunit((q)->den))
#define qisnegone(q)	(zisnegone((q)->num) && zisunit((q)->den))
#define qistwo(q)	(zistwo((q)->num) && zisunit((q)->den))
#define qiseven(q)	(zisunit((q)->den) && ziseven((q)->num))
#define qisodd(q)	(zisunit((q)->den) && zisodd((q)->num))
#define qistwopower(q)	(zisunit((q)->den) && zistwopower((q)->num))
#define qistiny(q)	(zistiny((q)->num))

#define qhighbit(q)	(zhighbit((q)->num))
#define qlowbit(q)	(zlowbit((q)->num))
#define qdivcount(q1, q2)	(zdivcount((q1)->num, (q2)->num))
/* operation on #q may be undefined, so replace with an inline-function */
/* was: #define qlink(q)	((q)->links++, (q)) */
static inline NUMBER* qlink(NUMBER* q) { if(q) { (q)->links++; } return q; }

#define qfree(q)	{if (--((q)->links) <= 0) qfreenum(q);}


/*
 * Flags for qparse calls
 */
#define QPF_SLASH	0x1	/* allow slash for fractional number */
#define QPF_IMAG	0x2	/* allow trailing 'i' for imaginary number */


/*
 * constants used often by the arithmetic routines
 */
EXTERN NUMBER _qzero_, _qone_, _qnegone_, _qonehalf_, _qneghalf_, _qonesqbase_;
EXTERN NUMBER _qtwo_, _qthree_, _qfour_, _qten_;
EXTERN NUMBER * initnumbs[];


#endif /* !INCLUDE_QMATH_H */
