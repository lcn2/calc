/*
 * qmath - declarations for extended precision rational arithmetic
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
 * @(#) $Revision: 29.5 $
 * @(#) $Id: qmath.h,v 29.5 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/qmath.h,v $
 *
 * Under source code control:	1993/07/30 19:42:47
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__QMATH_H__)
#define __QMATH_H__


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

extern NUMBER _qlge_;

/*
 * Input. output, allocation, and conversion routines.
 */
extern NUMBER *qalloc(void);
extern NUMBER *qcopy(NUMBER *q);
extern NUMBER *uutoq(FULL i1, FULL i2);
extern NUMBER *iitoq(long i1, long i2);
extern NUMBER *str2q(char *str);
extern NUMBER *itoq(long i);
extern NUMBER *utoq(FULL i);
extern long qtoi(NUMBER *q);
extern FULL qtou(NUMBER *q);
extern long qparse(char *str, int flags);
extern void qfreenum(NUMBER *q);
extern void qprintnum(NUMBER *q, int mode);
extern void qprintff(NUMBER *q, long width, long precision);
extern void qprintfe(NUMBER *q, long width, long precision);
extern void qprintfr(NUMBER *q, long width, BOOL force);
extern void qprintfd(NUMBER *q, long width);
extern void qprintfx(NUMBER *q, long width);
extern void qprintfb(NUMBER *q, long width);
extern void qprintfo(NUMBER *q, long width);
extern void qprintf(char *, ...);
extern void shownumbers(void);
extern void showredcdata(void);
extern void freeredcdata(void);
extern void fitprint(NUMBER *, long);



/*
 * Basic numeric routines.
 */
extern NUMBER *qaddi(NUMBER *q, long i);
extern NUMBER *qmuli(NUMBER *q, long i);
extern NUMBER *qdivi(NUMBER *q, long i);
extern NUMBER *qqadd(NUMBER *q1, NUMBER *q2);
extern NUMBER *qsub(NUMBER *q1, NUMBER *q2);
extern NUMBER *qmul(NUMBER *q1, NUMBER *q2);
extern NUMBER *qqdiv(NUMBER *q1, NUMBER *q2);
extern NUMBER *qquo(NUMBER *q1, NUMBER *q2, long rnd);
extern NUMBER *qmod(NUMBER *q1, NUMBER *q2, long rnd);
extern NUMBER *qmin(NUMBER *q1, NUMBER *q2);
extern NUMBER *qmax(NUMBER *q1, NUMBER *q2);
extern NUMBER *qand(NUMBER *q1, NUMBER *q2);
extern NUMBER *qor(NUMBER *q1, NUMBER *q2);
extern NUMBER *qxor(NUMBER *q1, NUMBER *q2);
extern NUMBER *qandnot(NUMBER *q1, NUMBER *q2);
extern NUMBER *qcomp(NUMBER *q);
extern NUMBER *qpowermod(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern NUMBER *qpowi(NUMBER *q1, NUMBER *q2);
extern NUMBER *qsquare(NUMBER *q);
extern NUMBER *qneg(NUMBER *q);
extern NUMBER *qsign(NUMBER *q);
extern NUMBER *qint(NUMBER *q);
extern NUMBER *qfrac(NUMBER *q);
extern NUMBER *qnum(NUMBER *q);
extern NUMBER *qden(NUMBER *q);
extern NUMBER *qinv(NUMBER *q);
extern NUMBER *qqabs(NUMBER *q);
extern NUMBER *qinc(NUMBER *q);
extern NUMBER *qdec(NUMBER *q);
extern NUMBER *qshift(NUMBER *q, long n);
extern NUMBER *qtrunc(NUMBER *q1, NUMBER *q2);
extern NUMBER *qround(NUMBER *q, long places, long rnd);
extern NUMBER *qbtrunc(NUMBER *q1, NUMBER *q2);
extern NUMBER *qbround(NUMBER *q, long places, long rnd);
extern NUMBER *qscale(NUMBER *q, long i);
extern BOOL qdivides(NUMBER *q1, NUMBER *q2);
extern BOOL qcmp(NUMBER *q1, NUMBER *q2);
extern BOOL qcmpi(NUMBER *q, long i);
extern FLAG qrel(NUMBER *q1, NUMBER *q2);
extern FLAG qreli(NUMBER *q, long i);
extern BOOL qisset(NUMBER *q, long i);


/*
 * More complicated numeric functions.
 */
extern NUMBER *qcomb(NUMBER *q1, NUMBER *q2);
extern NUMBER *qgcd(NUMBER *q1, NUMBER *q2);
extern NUMBER *qlcm(NUMBER *q1, NUMBER *q2);
extern NUMBER *qfact(NUMBER *q);
extern NUMBER *qpfact(NUMBER *q);
extern NUMBER *qminv(NUMBER *q1, NUMBER *q2);
extern NUMBER *qfacrem(NUMBER *q1, NUMBER *q2);
extern NUMBER *qperm(NUMBER *q1, NUMBER *q2);
extern NUMBER *qgcdrem(NUMBER *q1, NUMBER *q2);
extern NUMBER *qlowfactor(NUMBER *q1, NUMBER *q2);
extern NUMBER *qfib(NUMBER *q);
extern NUMBER *qcfappr(NUMBER *q, NUMBER *epsilon, long R);
extern NUMBER *qcfsim(NUMBER *q, long R);
extern NUMBER *qisqrt(NUMBER *q);
extern NUMBER *qjacobi(NUMBER *q1, NUMBER *q2);
extern NUMBER *qiroot(NUMBER *q1, NUMBER *q2);
extern NUMBER *qmappr(NUMBER *q, NUMBER *e, long R);
extern NUMBER *qlcmfact(NUMBER *q);
extern NUMBER *qredcin(NUMBER *q1, NUMBER *q2);
extern NUMBER *qredcout(NUMBER *q1, NUMBER *q2);
extern NUMBER *qredcmul(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern NUMBER *qredcsquare(NUMBER *q1, NUMBER *q2);
extern NUMBER *qredcpower(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern BOOL qprimetest(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern BOOL qissquare(NUMBER *q);
extern long qilog2(NUMBER *q);
extern long qilog10(NUMBER *q);
extern NUMBER *qilog(NUMBER *q, ZVALUE base);
extern BOOL qcmpmod(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern BOOL qquomod(NUMBER *q1, NUMBER *q2, NUMBER **retdiv, NUMBER **retmod);
extern FLAG qnear(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qdigit(NUMBER *q, ZVALUE dpos, ZVALUE base);
extern long qprecision(NUMBER *q);
extern long qplaces(NUMBER *q, ZVALUE base);
extern long qdecplaces(NUMBER *q);
extern long qdigits(NUMBER *q, ZVALUE base);
extern void setepsilon(NUMBER *q);
extern NUMBER *qbitvalue(long i);
extern NUMBER *qtenpow(long i);


/*
 * Transcendental functions.  These all take an epsilon argument to
 * specify the required accuracy of the calculation.
 */
extern void qsincos(NUMBER *q, long bitnum, NUMBER **vs, NUMBER **vc);
extern NUMBER *qsqrt(NUMBER *q, NUMBER *epsilon, long R);
extern NUMBER *qpower(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qroot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qcos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qsin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qexp(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qln(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qtan(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qsec(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcot(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcsc(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qasin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qatan(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qasec(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacsc(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacot(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qatan2(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qhypot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qcosh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qsinh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qtanh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcoth(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qsech(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcsch(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacosh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qasinh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qatanh(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qasech(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacsch(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacoth(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qlegtoleg(NUMBER *q, NUMBER *epsilon, BOOL wantneg);
extern NUMBER *qpi(NUMBER *epsilon);
extern NUMBER *qcatalan(NUMBER *);
extern NUMBER *qbern(ZVALUE z);
extern void qfreebern(void);
extern NUMBER *qeuler(ZVALUE z);
extern void qfreeeuler(void);


/*
 * pseudo-seed generator
 */
extern NUMBER *pseudo_seed(void);


/*
 * external swap functions
 */
extern NUMBER *swap_b8_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);
extern NUMBER *swap_b16_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);
extern NUMBER *swap_HALF_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all);


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

#define qhighbit(q)	(zhighbit((q)->num))
#define qlowbit(q)	(zlowbit((q)->num))
#define qdivcount(q1, q2)	(zdivcount((q1)->num, (q2)->num))
#define qlink(q)	((q)->links++, (q))

#define qfree(q)	{if (--((q)->links) <= 0) qfreenum(q);}


/*
 * Flags for qparse calls
 */
#define QPF_SLASH	0x1	/* allow slash for fractional number */
#define QPF_IMAG	0x2	/* allow trailing 'i' for imaginary number */


/*
 * constants used often by the arithmetic routines
 */
extern NUMBER _qzero_, _qone_, _qnegone_, _qonehalf_, _qneghalf_, _qonesqbase_;
extern NUMBER _qtwo_, _qthree_, _qfour_;
extern NUMBER * initnumbs[];


#endif /* !__QMATH_H__ */
