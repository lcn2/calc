/*
 * qmath - declarations for extended precision rational arithmetic
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  David I. Bell and Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1993/07/30 19:42:47
 * File existed as early as:    1993
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_QMATH_H)
#  define INCLUDE_QMATH_H

/*
 * Rational arithmetic definitions.
 */
struct number {
    ZVALUE num;          /* numerator (containing sign) */
    ZVALUE den;          /* denominator (always positive) */
    long links;          /* number of links to this value */
    struct number *next; /* pointer to next number */
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
extern NUMBER *stoq(SFULL i);
extern long qtoi(NUMBER *q);
extern FULL qtou(NUMBER *q);
extern SFULL qtos(NUMBER *q);
extern long qparse(char *str, int flags);
extern void qfreenum(NUMBER *q);
extern void qprintnum(NUMBER *q, int mode, LEN outdigits);
extern void qprintff(NUMBER *q, long width, long precision);
extern void qprintfe(NUMBER *q, long width, long precision);
extern void qprintfr(NUMBER *q, long width, bool force);
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
extern bool qdivides(NUMBER *q1, NUMBER *q2);
extern bool qcmp(NUMBER *q1, NUMBER *q2);
extern bool qcmpi(NUMBER *q, long i);
extern FLAG qrel(NUMBER *q1, NUMBER *q2);
extern FLAG qreli(NUMBER *q, long i);
extern bool qisset(NUMBER *q, long i);

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
extern bool qprimetest(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern bool qissquare(NUMBER *q);
extern long qilog2(NUMBER *q);
extern long qilog10(NUMBER *q);
extern NUMBER *qilog(NUMBER *q, ZVALUE base);
extern bool qcmpmod(NUMBER *q1, NUMBER *q2, NUMBER *q3);
extern bool qquomod(NUMBER *q1, NUMBER *q2, NUMBER **quo, NUMBER **mod, long rnd);
extern FLAG qnear(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern NUMBER *qdigit(NUMBER *q, ZVALUE dpos, ZVALUE base);
extern long qprecision(NUMBER *q);
extern long qplaces(NUMBER *q, ZVALUE base);
extern long qdecplaces(NUMBER *q);
extern long qdigits(NUMBER *q, ZVALUE base);
extern bool check_epsilon(NUMBER *q);
extern void setepsilon(NUMBER *q);
extern NUMBER *qbitvalue(long i);
extern NUMBER *qqbitvalue(NUMBER pos);
extern NUMBER *qtenpow(long i);
extern bool qispowerof2(NUMBER *q, NUMBER **qlog2);

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
extern NUMBER *qlog(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qlog2(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qlogn(NUMBER *q, NUMBER *n, NUMBER *epsilon);
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
extern NUMBER *qlegtoleg(NUMBER *q, NUMBER *epsilon, bool wantneg);
extern NUMBER *qpi(NUMBER *epsilon);
extern NUMBER *qpidiv180(NUMBER *epsilon);
extern NUMBER *qpidiv200(NUMBER *epsilon);
extern NUMBER *qcatalan(NUMBER *);
extern NUMBER *qbern(ZVALUE z);
extern void qfreebern(void);
extern NUMBER *qeuler(ZVALUE z);
extern void qfreeeuler(void);

/*
 * historical trig functions
 */
extern NUMBER *qversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaversin_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcoversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacoversin_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacoversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qvercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qavercos_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qavercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcovercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacovercos_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacovercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qhaversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahaversin_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahaversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qhacoversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahacoversin_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahacoversin(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qhavercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahavercos_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahavercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qhacovercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahacovercos_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qahacovercos(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qexsec(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaexsec_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaexsec(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qexcsc(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaexcsc_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qaexcsc(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcrd(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacrd_or_NULL(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qacrd(NUMBER *q, NUMBER *epsilon);
extern NUMBER *qcas(NUMBER *q, NUMBER *epsilon);

/*
 * pseudo-seed generator
 */
extern NUMBER *pseudo_seed(void);

/*
 * external swap functions
 */
extern NUMBER *swap_b8_in_NUMBER(NUMBER *dest, NUMBER *src, bool all);
extern NUMBER *swap_b16_in_NUMBER(NUMBER *dest, NUMBER *src, bool all);
extern NUMBER *swap_HALF_in_NUMBER(NUMBER *dest, NUMBER *src, bool all);

/*
 * macro expansions to speed this thing up
 */
#  define qiszero(q) (ziszero((q)->num))
#  define qisneg(q) (zisneg((q)->num))
#  define qispos(q) (zispos((q)->num))
#  define qisint(q) (zisunit((q)->den))
#  define qisfrac(q) (!zisunit((q)->den))
#  define qisunit(q) (zisunit((q)->num) && zisunit((q)->den))
#  define qisone(q) (zisone((q)->num) && zisunit((q)->den))
#  define qisnegone(q) (zisnegone((q)->num) && zisunit((q)->den))
#  define qistwo(q) (zistwo((q)->num) && zisunit((q)->den))
#  define qiseven(q) (zisunit((q)->den) && ziseven((q)->num))
#  define qisodd(q) (zisunit((q)->den) && zisodd((q)->num))
#  define qistiny(q) (zistiny((q)->num))

#  define qhighbit(q) (zhighbit((q)->num))
#  define qlowbit(q) (zlowbit((q)->num))
#  define qdivcount(q1, q2) (zdivcount((q1)->num, (q2)->num))
#  define qisreciprocal(q) (zisunit((q)->num) && !ziszero((q)->den))
/* operation on #q may be undefined, so replace with an inline-function */
/* was: #define qlink(q)        ((q)->links++, (q)) */
static inline NUMBER *
qlink(NUMBER *q)
{
    if (q) {
        (q)->links++;
    }
    return q;
}

#  define qfree(q)                 \
      {                            \
          if (--((q)->links) <= 0) \
              qfreenum(q);         \
      }

/*
 * Flags for qparse calls
 */
#  define QPF_SLASH 0x1 /* allow slash for fractional number */
#  define QPF_IMAG 0x2  /* allow trailing 'i' for imaginary number */

/*
 * constants used often by the arithmetic routines
 */
extern NUMBER _qzero_, _qone_, _qnegone_, _qonehalf_, _qneghalf_, _qonesqbase_;
extern NUMBER _qtwo_, _qten_;
extern NUMBER *initnumbs[];

#endif
