/*
 * qtrans - transcendental functions for real numbers
 *
 * Copyright (C) 1999-2007,2021-2023  David I. Bell, Landon Curt Noll and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:   1990/02/15 01:48:22
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Transcendental functions for real numbers.
 * These are sin, cos, exp, ln, power, cosh, sinh.
 */


#include "qmath.h"
#include "config.h"


#include "errtbl.h"
#include "banned.h"     /* include after system header <> includes */


HALF _qlgenum_[] = { 36744 };
HALF _qlgeden_[] = { 25469 };
NUMBER _qlge_ = { { _qlgenum_, 1, 0 }, { _qlgeden_, 1, 0 }, 1, NULL };

/*
 * cache the natural logarithm of 10 and 2
 */
STATIC NUMBER *ln_10 = NULL;
STATIC NUMBER *ln_10_epsilon = NULL;
STATIC NUMBER *ln_2 = NULL;
STATIC NUMBER *ln_2_epsilon = NULL;
STATIC NUMBER *ln_n = NULL;
STATIC NUMBER *ln_n_epsilon = NULL;

/*
 * cache pi
 *
 * pivalue[LAST_PI_EPSILON] - last epsilon used to calculate pi
 * pivalue[LAST_PI_VALUE] - last calculated pi
 *                            given pivalue[LAST_PI_EPSILON] epsilon
 * pivalue[LAST_PI_DIV_180_EPSILON] - last epsilon used to calculate pi/180
 * pivalue[LAST_PI_DIV_180_VALUE] - last calculated pi/180 given
 *                                    pivalue[LAST_PI_DIV_180_EPSILON] epsilon
 * pivalue[LAST_PI_DIV_200_EPSILON] - last epsilon used to calculate pi/200
 * pivalue[LAST_PI_DIV_200_VALUE] - last calculated pi/200 given
 *                                    pivalue[LAST_PI_DIV_200_EPSILON] epsilon
 */
enum pi_cache {
        LAST_PI_EPSILON = 0,
        LAST_PI_VALUE,
        LAST_PI_DIV_180_EPSILON,
        LAST_PI_DIV_180_VALUE,
        LAST_PI_DIV_200_EPSILON,
        LAST_PI_DIV_200_VALUE,
        PI_CACHE_LEN    /* must be last */
};
STATIC NUMBER *pivalue[PI_CACHE_LEN] = {
        NULL,   /* LAST_PI_EPSILON */
        NULL,   /* LAST_PI_VALUE */
        NULL,   /* LAST_PI_DIV_180_EPSILON */
        NULL,   /* LAST_PI_DIV_180_VALUE */
        NULL,   /* LAST_PI_DIV_200_EPSILON */
        NULL,   /* LAST_PI_DIV_200_VALUE */
};

/*
 * other static function declarations
 */
STATIC NUMBER *qexprel(NUMBER *q, long bitnum);

/*
 * Evaluate and store in specified locations the sin and cos of a given
 * number to accuracy corresponding to a specified number of binary digits.
 */
void
qsincos(NUMBER *q, long bitnum, NUMBER **vs, NUMBER **vc)
{
        long n, m, k, h, s, t, d;
        NUMBER *qtmp1, *qtmp2;
        ZVALUE X, cossum, sinsum, mul, ztmp1, ztmp2, ztmp3;

        qtmp1 = qqabs(q);
        h = qilog2(qtmp1);
        qfree(qtmp1);
        k = bitnum + h + 1;
        if (k < 0) {
                *vs = qlink(&_qzero_);
                *vc = qlink(&_qone_);
                return;
        }
        s = k;
        if (k) {
                do {
                        t = s;
                        s = (s + k/s)/2;
                }
                while (t > s);
        }               /* s is int(sqrt(k)) */
        s++;
        if (s < -h)
                s = -h;
        n = h + s;      /* n is number of squaring that will be required */
        m = bitnum + n;
        while (s > 0) {         /* increasing m by ilog2(s) */
                s >>= 1;
                m++;
        }                       /* m is working number of bits */
        qtmp1 = qscale(q, m - n);
        zquo(qtmp1->num, qtmp1->den, &X, conf->triground);
        qfree(qtmp1);
        if (ziszero(X)) {
                zfree(X);
                *vs = qlink(&_qzero_);
                *vc = qlink(&_qone_);
                return;
        }
        zbitvalue(m, &cossum);
        zcopy(X, &sinsum);
        zcopy(X, &mul);
        d = 1;
        for (;;) {
                X.sign = !X.sign;
                zmul(X, mul, &ztmp1);
                zfree(X);
                zshift(ztmp1, -m, &ztmp2);
                zfree(ztmp1);
                zdivi(ztmp2, ++d, &X);
                zfree(ztmp2);
                if (ziszero(X))
                        break;
                zadd(cossum, X, &ztmp1);
                zfree(cossum);
                cossum = ztmp1;
                zmul(X, mul, &ztmp1);
                zfree(X);
                zshift(ztmp1, -m, &ztmp2);
                zfree(ztmp1);
                zdivi(ztmp2, ++d, &X);
                zfree(ztmp2);
                if (ziszero(X))
                        break;
                zadd(sinsum, X, &ztmp1);
                zfree(sinsum);
                sinsum = ztmp1;
        }
        zfree(X);
        zfree(mul);
        while (n-- > 0) {
                zsquare(cossum, &ztmp1);
                zsquare(sinsum, &ztmp2);
                zsub(ztmp1, ztmp2, &ztmp3);
                zfree(ztmp1);
                zfree(ztmp2);
                zmul(cossum, sinsum, &ztmp1);
                zfree(cossum);
                zfree(sinsum);
                zshift(ztmp3, -m, &cossum);
                zfree(ztmp3);
                zshift(ztmp1, 1 - m, &sinsum);
                zfree(ztmp1);
        }
        h = zlowbit(cossum);
        qtmp1 = qalloc();
        if (m > h) {
                zshift(cossum, -h, &qtmp1->num);
                zbitvalue(m - h, &qtmp1->den);
        } else {
                zshift(cossum, - m, &qtmp1->num);
        }
        zfree(cossum);
        *vc = qtmp1;
        h = zlowbit(sinsum);
        qtmp2 = qalloc();
        if (m > h) {
                zshift(sinsum, -h, &qtmp2->num);
                zbitvalue(m - h, &qtmp2->den);
        } else {
                zshift(sinsum, -m, &qtmp2->num);
        }
        zfree(sinsum);
        *vs = qtmp2;
        return;
}


/*
 * Calculate the cosine of a number to a near multiple of epsilon.
 * This calls qsincos() and discards the value of sin.
 */
NUMBER *
qcos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *res;
        long n;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for cosine");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qone_);
        n = -qilog2(epsilon);
        if (n < 0)
                return qlink(&_qzero_);
        qsincos(q, n + 2, &sin, &cos);
        qfree(sin);
        res = qmappr(cos, epsilon, conf->triground);
        qfree(cos);
        return res;
}


/*
 * This calls qsincos() and discards the value of cos.
 */
NUMBER *
qsin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *res;
        long n;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for sine");
                not_reached();
        }
        n = -qilog2(epsilon);
        if (qiszero(q) || n < 0)
                return qlink(&_qzero_);
        qsincos(q, n + 2, &sin, &cos);
        qfree(cos);
        res = qmappr(sin, epsilon, conf->triground);
        qfree(sin);
        return res;
}


/*
 * Calculate the tangent function.
 */
NUMBER *
qtan(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *tan, *res;
        long n, k, m;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for tangent");
                not_reached();
        }
        if (qiszero(q))
                return qlink(q);
        n = qilog2(epsilon);
        k = (n > 0) ? 4 + n/2 : 4;
        for (;;) {
                qsincos(q, 2 * k - n, &sin, &cos);
                if (qiszero(cos)) {
                        qfree(sin);
                        qfree(cos);
                        k = 2 * k - n + 4;
                        continue;
                }
                m = -qilog2(cos);
                if (m < k)
                         break;
                qfree(sin);
                qfree(cos);
                k = m + 1;
        }
        tan = qqdiv(sin, cos);
        qfree(sin);
        qfree(cos);
        res = qmappr(tan, epsilon, conf->triground);
        qfree(tan);
        return res;
}


/*
 * Calculate the cotangent function.
 */
NUMBER *
qcot(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *cot, *res;
        long n, k, m;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for cotangent");
                not_reached();
        }
        if (qiszero(q)) {
                math_error("Zero argument for cotangent");
                not_reached();
        }
        k = -qilog2(q);
        n = qilog2(epsilon);
        if (k < 0)
                k = (n > 0) ? n/2 : 0;
        k += 4;
        for (;;) {
                qsincos(q, 2 * k - n, &sin, &cos);
                if (qiszero(sin)) {
                        qfree(sin);
                        qfree(cos);
                        k = 2 * k - n + 4;
                        continue;
                }
                m = -qilog2(sin);
                if (m < k)
                         break;
                qfree(sin);
                qfree(cos);
                k = m + 1;
        }
        cot = qqdiv(cos, sin);
        qfree(sin);
        qfree(cos);
        res = qmappr(cot, epsilon, conf->triground);
        qfree(cot);
        return res;
}


/*
 * Calculate the secant function.
 */
NUMBER *
qsec(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *sec, *res;
        long n, k, m;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for secant");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qone_);
        n = qilog2(epsilon);
        k = (n > 0) ? 4 + n/2 : 4;
        for (;;) {
                qsincos(q, 2 * k - n, &sin, &cos);
                qfree(sin);
                if (qiszero(cos)) {
                        qfree(cos);
                        k = 2 * k - n + 4;
                        continue;
                }
                m = -qilog2(cos);
                if (m < k)
                         break;
                qfree(cos);
                k = m + 1;
        }
        sec = qinv(cos);
        qfree(cos);
        res = qmappr(sec, epsilon, conf->triground);
        qfree(sec);
        return res;
}


/*
 * Calculate the cosecant function.
 */
NUMBER *
qcsc(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin, *cos, *csc, *res;
        long n, k, m;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for cosecant");
                not_reached();
        }
        if (qiszero(q)) {
                math_error("Zero argument for cosecant");
                not_reached();
        }
        k = -qilog2(q);
        n = qilog2(epsilon);
        if (k < 0)
                k = (n > 0) ? n/2 : 0;
        k += 4;
        for (;;) {
                qsincos(q, 2 * k - n, &sin, &cos);
                qfree(cos);
                if (qiszero(sin)) {
                        qfree(sin);
                        k = 2 * k - n + 4;
                        continue;
                }
                m = -qilog2(sin);
                if (m < k)
                         break;
                qfree(sin);
                k = m + 1;
        }
        csc = qinv(sin);
        qfree(sin);
        res = qmappr(csc, epsilon, conf->triground);
        qfree(csc);
        return res;
}


/*
 * Calculate the arcsine function.
 * The result is in the range -pi/2 to pi/2.
 */
NUMBER *
qasin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *qtmp1, *qtmp2, *epsilon1;
        ZVALUE ztmp;
        bool neg;
        FLAG r;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for asin");
                not_reached();
        }
        if (qiszero(q)) {
                return qlink(&_qzero_);
        }
        ztmp = q->num;
        neg = ztmp.sign;
        ztmp.sign = 0;
        r = zrel(ztmp, q->den);
        if (r > 0) {
                return NULL;
        }
        if (r == 0) {
                epsilon1 = qscale(epsilon, 1L);
                qtmp2 = qpi(epsilon1);
                qtmp1 = qscale(qtmp2, -1L);
        } else {
                epsilon1 = qscale(epsilon, -2L);
                qtmp1 = qalloc();
                zsquare(q->num, &qtmp1->num);
                zsquare(q->den, &ztmp);
                zsub(ztmp, qtmp1->num, &qtmp1->den);
                zfree(ztmp);
                qtmp2 = qsqrt(qtmp1, epsilon1, conf->triground);
                qfree(qtmp1);
                qtmp1 = qatan(qtmp2, epsilon);
        }
        qfree(qtmp2);
        qfree(epsilon1);
        if (neg) {
                qtmp2 = qneg(qtmp1);
                qfree(qtmp1);
                return(qtmp2);
        }
        return qtmp1;
}



/*
 * Calculate the acos function.
 * The result is in the range 0 to pi.
 */
NUMBER *
qacos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *q1, *q2, *epsilon1;
        ZVALUE z;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for acos");
                not_reached();
        }
        if (qisone(q))
                return qlink(&_qzero_);
        if (qisnegone(q))
                return qpi(epsilon);

        z = q->num;
        z.sign = 0;
        if (zrel(z, q->den) > 0)
                return NULL;
        epsilon1 = qscale(epsilon, -3L);                /* ??? */
        q1 = qalloc();
        zsub(q->den, q->num, &q1->num);
        zadd(q->den, q->num, &q1->den);
        q2 = qsqrt(q1, epsilon1, conf->triground);
        qfree(q1);
        qfree(epsilon1);
        epsilon1 = qscale(epsilon, -1L);
        q1 = qatan(q2, epsilon1);
        qfree(epsilon1);
        qfree(q2);
        q2 = qscale(q1, 1L);
        qfree(q1)
        return q2;
}


/*
 * Calculate the arctangent function to the nearest or next to nearest
 * multiple of epsilon.  Algorithm uses
 *      atan(x) = 2 * atan(x/(1 + sqrt(1+x^2)))
 * to reduce x to a small value and then
 *      atan(x) = x - x^3/3 + ...
 */
NUMBER *
qatan(NUMBER *q, NUMBER *epsilon)
{
        long m, k, i;
        FULL d;
        ZVALUE X, D, DD, sum, mul, term, ztmp1, ztmp2;
        NUMBER *qtmp, *res;
        bool sign;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for arctangent");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qzero_);
        m = 12 - qilog2(epsilon);
                /* 4 bits for 4 doublings; 8 for rounding */
        if (m < 8)
                m = 8;          /* m is number of working binary digits */
        qtmp = qscale(q, m);
        zquo(qtmp->num, qtmp->den, &X, conf->triground);
        qfree(qtmp);
        zbitvalue(m, &D);       /* q has become X/D */
        zsquare(D, &DD);
        i = 4;                  /* maybe this should be larger */
        while (i-- > 0 && !ziszero(X)) {
                zsquare(X, &ztmp1);
                zadd(ztmp1, DD, &ztmp2);
                zfree(ztmp1);
                zsqrt(ztmp2, &ztmp1, conf->triground);
                zfree(ztmp2);
                zadd(ztmp1, D, &ztmp2);
                zfree(ztmp1);
                zshift(X, m, &ztmp1);
                zfree(X);
                zquo(ztmp1, ztmp2, &X, conf->triground);
                zfree(ztmp1);
                zfree(ztmp2);
        }
        zfree(DD);
        zfree(D);
        if (ziszero(X)) {
                zfree(X);
                return qlink(&_qzero_);
        }
        zcopy(X, &sum);
        zsquare(X, &ztmp1);
        zshift(ztmp1, -m, &mul);
        zfree(ztmp1);
        d = 3;
        sign = !X.sign;
        for (;;) {
                if (d > BASE) {
                        math_error("Too many terms required for atan");
                        not_reached();
                }
                zmul(X, mul, &ztmp1);
                zfree(X);
                zshift(ztmp1, -m, &X);  /* X now (original X)^d */
                zfree(ztmp1);
                zdivi(X, d, &term);
                if (ziszero(term)) {
                        zfree(term);
                        break;
                }
                term.sign = sign;
                zadd(sum, term, &ztmp1);
                zfree(sum);
                zfree(term);
                sum = ztmp1;
                sign = !sign;
                d += 2;
        }
        zfree(mul);
        zfree(X);
        qtmp = qalloc();
        k = zlowbit(sum);
        if (k) {
                zshift(sum, -k, &qtmp->num);
                zfree(sum);
        } else {
                qtmp->num = sum;
        }
        zbitvalue(m - 4 - k, &qtmp->den);
        res = qmappr(qtmp, epsilon, conf->triground);
        qfree(qtmp);
        return res;
}

/*
 * Inverse secant function
 */
NUMBER *
qasec(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp, *res;

        tmp = qinv(q);
        res = qacos(tmp, epsilon);
        qfree(tmp);
        return res;
}


/*
 * Inverse cosecant function
 */
NUMBER *
qacsc(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp, *res;

        tmp = qinv(q);
        res = qasin(tmp, epsilon);
        qfree(tmp);
        return res;
}


/*
 * Inverse cotangent function
 */
NUMBER *
qacot(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *epsilon1;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon for acot");
                not_reached();
        }
        if (qiszero(q)) {
                epsilon1 = qscale(epsilon, 1L);
                tmp1 = qpi(epsilon1);
                qfree(epsilon1);
                tmp2 = qscale(tmp1, -1L);
                qfree(tmp1);
                return tmp2;
        }
        tmp1 = qinv(q);
        if (!qisneg(q)) {
                tmp2 = qatan(tmp1, epsilon);
                qfree(tmp1);
                return tmp2;
        }
        epsilon1 = qscale(epsilon, -2L);
        tmp2 = qatan(tmp1, epsilon1);
        qfree(tmp1);
        tmp1 = qpi(epsilon1);
        qfree(epsilon1);
        tmp3 = qqadd(tmp1, tmp2);
        qfree(tmp1);
        qfree(tmp2);
        tmp1 = qmappr(tmp3, epsilon, conf->triground);
        qfree(tmp3);
        return tmp1;
}


/*
 * Calculate the angle which is determined by the point (x,y).
 * This is the same as atan(y/x) for positive x, but is continuous
 * except for y = 0, x <= 0.  By convention, y is the first argument.
 * For all x, y, -pi < atan2 <= pi.  For example, qatan2(1, -1) = 3/4 * pi.
 */
NUMBER *
qatan2(NUMBER *qy, NUMBER *qx, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *epsilon2;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for atan2");
                not_reached();
        }
        if (qiszero(qy) && qiszero(qx)) {
                /* conform to 4.3BSD ANSI/IEEE 754-1985 math lib */
                return qlink(&_qzero_);
        }
        /*
         * If the point is on the negative real axis, then the answer is pi.
         */
        if (qiszero(qy) && qisneg(qx))
                return qpi(epsilon);
        /*
         * If the point is in the right half plane, then use the normal atan.
         */
        if (!qisneg(qx) && !qiszero(qx)) {
                if (qiszero(qy))
                        return qlink(&_qzero_);
                tmp1 = qqdiv(qy, qx);
                tmp2 = qatan(tmp1, epsilon);
                qfree(tmp1);
                return tmp2;
        }
        /*
         * The point is in the left half plane (x <= 0) with nonzero y.
         * Calculate the angle by using the formula:
         *      atan2(y,x) = 2 * atan(sgn(y) * sqrt((x/y)^2 + 1) - x/y).
         */
        epsilon2 = qscale(epsilon, -4L);
        tmp1 = qqdiv(qx, qy);
        tmp2 = qsquare(tmp1);
        tmp3 = qqadd(tmp2, &_qone_);
        qfree(tmp2);
        tmp2 = qsqrt(tmp3, epsilon2, 24L | (qy->num.sign * 64));
        qfree(tmp3);
        tmp3 = qsub(tmp2, tmp1);
        qfree(tmp2);
        qfree(tmp1);
        qfree(epsilon2);
        epsilon2 = qscale(epsilon, -1L);
        tmp1 = qatan(tmp3, epsilon2);
        qfree(epsilon2);
        qfree(tmp3);
        tmp2 = qscale(tmp1, 1L);
        qfree(tmp1);
        return tmp2;
}


/*
 * Calculate the value of pi to within the required epsilon.
 * This uses the following formula which only needs integer calculations
 * except for the final operation:
 *      pi = 1 / SUMOF(comb(2 * N, N) ^ 3 * (42 * N + 5) / 2 ^ (12 * N + 4)),
 * where the summation runs from N=0.  This formula gives about 6 bits of
 * accuracy per term.  Since the denominator for each term is a power of two,
 * we can simply use shifts to sum the terms.  The combinatorial numbers
 * in the formula are calculated recursively using the formula:
 *      comb(2*(N+1), N+1) = 2 * comb(2 * N, N) * (2 * N + 1) / N.
 */
NUMBER *
qpi(NUMBER *epsilon)
{
        ZVALUE comb;                    /* current combinatorial value */
        ZVALUE sum;                     /* current sum */
        ZVALUE tmp1, tmp2;
        NUMBER *r, *t1, qtmp;
        long shift;                     /* current shift of result */
        long N;                         /* current term number */
        long bits;                      /* needed number of bits of precision */
        long t;

        /* firewall */
        if (qiszero(epsilon)) {
                math_error("zero epsilon value for pi");
                not_reached();
        }

        /* use pi cache if epsilon marches, else flush if needed */
        if (pivalue[LAST_PI_EPSILON] != NULL &&
            pivalue[LAST_PI_VALUE] != NULL &&
            epsilon == pivalue[LAST_PI_EPSILON]) {
                return qlink(pivalue[LAST_PI_VALUE]);
        }
        if (pivalue[LAST_PI_EPSILON] != NULL) {
                qfree(pivalue[LAST_PI_EPSILON]);
        }
        if (pivalue[LAST_PI_VALUE] != NULL) {
                qfree(pivalue[LAST_PI_VALUE]);
        }

        bits = -qilog2(epsilon) + 4;
        if (bits < 4)
                bits = 4;
        comb = _one_;
        itoz(5L, &sum);
        N = 0;
        shift = 4;
        do {
                t = 1 + (++N & 0x1);
                (void) zdivi(comb, N / (3 - t), &tmp1);
                zfree(comb);
                zmuli(tmp1, t * (2 * N - 1), &comb);
                zfree(tmp1);
                zsquare(comb, &tmp1);
                zmul(comb, tmp1, &tmp2);
                zfree(tmp1);
                zmuli(tmp2, 42 * N + 5, &tmp1);
                zfree(tmp2);
                zshift(sum, 12L, &tmp2);
                zfree(sum);
                zadd(tmp1, tmp2, &sum);
                t = zhighbit(tmp1);
                zfree(tmp1);
                zfree(tmp2);
                shift += 12;
        } while ((shift - t) < bits);
        zfree(comb);
        qtmp.num = _one_;
        qtmp.den = sum;
        t1 = qscale(&qtmp, shift);
        zfree(sum);
        r = qmappr(t1, epsilon, conf->triground);
        qfree(t1);
        pivalue[LAST_PI_EPSILON] = qlink(epsilon);
        pivalue[LAST_PI_VALUE] = qlink(r);
        return r;
}


/*
 * qpidiv180 - calculate pi / 180
 *
 * This function returns pi/180 as used to covert between radians and degrees.
 */
NUMBER *
qpidiv180(NUMBER *epsilon)
{
        NUMBER *pi, *pidiv180;

        /* firewall */
        if (qiszero(epsilon)) {
                math_error("zero epsilon value for qpidiv180");
                not_reached();
        }

        /* use pi/180 cache if epsilon marches, else flush if needed */
        if (pivalue[LAST_PI_DIV_180_EPSILON] != NULL &&
            pivalue[LAST_PI_DIV_180_VALUE] != NULL &&
            epsilon == pivalue[LAST_PI_DIV_180_EPSILON]) {
                return qlink(pivalue[LAST_PI_DIV_180_VALUE]);
        }
        if (pivalue[LAST_PI_DIV_180_EPSILON] != NULL) {
                qfree(pivalue[LAST_PI_DIV_180_EPSILON]);
        }
        if (pivalue[LAST_PI_DIV_180_VALUE] != NULL) {
                qfree(pivalue[LAST_PI_DIV_180_VALUE]);
        }

        /* let qpi() returned cached pi or calculate new as needed */
        pi = qpi(epsilon);

        /* calculate pi/180 */
        pidiv180 = qdivi(pi, 180);

        /* cache epsilon and pi/180 */
        pivalue[LAST_PI_DIV_180_EPSILON] = qlink(epsilon);
        pivalue[LAST_PI_DIV_180_VALUE] = qlink(pidiv180);

        /* return pi/180 */
        return pidiv180;
}


/*
 * qpidiv200 - calculate pi / 200
 *
 * This function returns pi/200 as used to covert between radians and gradians.
 */
NUMBER *
qpidiv200(NUMBER *epsilon)
{
        NUMBER *pi, *pidiv200;

        /* firewall */
        if (qiszero(epsilon)) {
                math_error("zero epsilon value for qpidiv200");
                not_reached();
        }

        /* use pi/200 cache if epsilon marches, else flush if needed */
        if (pivalue[LAST_PI_DIV_200_EPSILON] != NULL &&
            pivalue[LAST_PI_DIV_200_VALUE] != NULL &&
            epsilon == pivalue[LAST_PI_DIV_200_EPSILON]) {
                return qlink(pivalue[LAST_PI_DIV_200_VALUE]);
        }
        if (pivalue[LAST_PI_DIV_200_EPSILON] != NULL) {
                qfree(pivalue[LAST_PI_DIV_200_EPSILON]);
        }
        if (pivalue[LAST_PI_DIV_200_VALUE] != NULL) {
                qfree(pivalue[LAST_PI_DIV_200_VALUE]);
        }

        /* let qpi() returned cached pi or calculate new as needed */
        pi = qpi(epsilon);

        /* calculate pi/200 */
        pidiv200 = qdivi(pi, 200);

        /* cache epsilon and pi/200 */
        pivalue[LAST_PI_DIV_200_EPSILON] = qlink(epsilon);
        pivalue[LAST_PI_DIV_200_VALUE] = qlink(pidiv200);

        /* return pi/200 */
        return pidiv200;
}


/*
 * Calculate the exponential function to the nearest or next to nearest
 * multiple of the positive number epsilon.
 */
NUMBER *
qexp(NUMBER *q, NUMBER *epsilon)
{
        long m, n;
        NUMBER *tmp1, *tmp2;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for exp");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qone_);
        tmp1 = qmul(q, &_qlge_);
        m = qtoi(tmp1);         /* exp(q) < 2^(m+1) or m == MAXLONG */
        qfree(tmp1);

        if (m > (1 << 30))
                return NULL;

        n = qilog2(epsilon);    /* 2^n <= epsilon < 2^(n+1) */
        if (m < n)
                return qlink(&_qzero_);
        tmp1 = qqabs(q);
        tmp2 = qexprel(tmp1, m - n + 1);
        qfree(tmp1);
        if (tmp2 == NULL)
                return NULL;
        if (qisneg(q)) {
                tmp1 = qinv(tmp2);
                qfree(tmp2);
                tmp2 = tmp1;
        }
        tmp1 = qmappr(tmp2, epsilon, conf->triground);
        qfree(tmp2);
        return tmp1;
}


/*
 * Calculate the exponential function with relative error corresponding
 * to a specified number of significant bits
 * Requires *q >= 0, bitnum >= 0.
 * This returns NULL if more than 2^30 working bits would be required.
 */
S_FUNC NUMBER *
qexprel(NUMBER *q, long bitnum)
{
        long n, m, k, h, s, t, d;
        NUMBER *qtmp1;
        ZVALUE X, B, sum, term, ztmp1, ztmp2;

        h = qilog2(q);
        k = bitnum + h + 1;
        if (k < 0)
                return qlink(&_qone_);
        s = k;
        if (k) {
                do {
                        t = s;
                        s = (s + k/s)/2;
                }
                while (t > s);
        }               /* s is int(sqrt(k)) */
        s++;
        if (s < -h)
                s = -h;
        n = h + s;      /* n is number of squarings that will be required */
        m = bitnum + n;
        if (m > (1 << 30))
                return NULL;
        while (s > 0) {         /* increasing m by ilog2(s) */
                s >>= 1;
                m++;
        }                       /* m is working number of bits */
        qtmp1 = qscale(q, m - n);
        zquo(qtmp1->num, qtmp1->den, &X, conf->triground);
        qfree(qtmp1);
        if (ziszero(X)) {
                zfree(X);
                return qlink(&_qone_);
        }
        zbitvalue(m, &sum);
        zcopy(X, &term);
        d = 1;
        do {
                zadd(sum, term, &ztmp1);
                zfree(sum);
                sum = ztmp1;
                zmul(term, X, &ztmp1);
                zfree(term);
                zshift(ztmp1, -m, &ztmp2);
                zfree(ztmp1);
                zdivi(ztmp2, ++d, &term);
                zfree(ztmp2);
        }
        while (!ziszero(term));
        zfree(term);
        zfree(X);
        k = 0;
        zbitvalue(2 * m + 1, &B);
        while (n-- > 0) {
                k *= 2;
                zsquare(sum, &ztmp1);
                zfree(sum);
                if (zrel(ztmp1, B) >= 0) {
                        zshift(ztmp1, -m - 1, &sum);
                        k++;
                } else {
                        zshift(ztmp1, -m, &sum);
                }
                zfree(ztmp1);
        }
        zfree(B);
        h = zlowbit(sum);
        qtmp1 = qalloc();
        if (m > h + k) {
                zshift(sum, -h, &qtmp1->num);
                zbitvalue(m - h - k, &qtmp1->den);
        }
        else
                zshift(sum, k - m, &qtmp1->num);
        zfree(sum);
        return qtmp1;
}


/*
 * Calculate the natural logarithm of a number accurate to the specified
 * positive epsilon.
 */
NUMBER *
qln(NUMBER *q, NUMBER *epsilon)
{
        long m, n, k, h, d;
        ZVALUE term, sum, mul, pow, X, D, B, ztmp;
        NUMBER *qtmp, *res;
        bool neg;

        if (qiszero(q)) {
                math_error("ln of 0");
                not_reached();
        }
        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for ln");
                not_reached();
        }
        if (qisunit(q))
                return qlink(&_qzero_);
        q = qqabs(q);                   /* Ignore sign of q */
        neg = (zrel(q->num, q->den) < 0);
        if (neg) {
                qtmp = qinv(q);
                qfree(q);
                q = qtmp;
        }
        k = qilog2(q);
        m = -qilog2(epsilon);           /* m will be number of working bits */
        if (m < 0)
                m = 0;
        h = k;
        while (h > 0) {
                h /= 2;
                m++;                    /* Add 1 for each sqrt until X < 2 */
        }
        m += 18;        /* 8 more sqrts, 8 for rounding, 2 for epsilon/4 */
        qtmp = qscale(q, m - k);
        zquo(qtmp->num, qtmp->den, &X, conf->triground);
        qfree(q);
        qfree(qtmp);

        zbitvalue(m, &D);               /* Now "q" = X/D */
        zbitvalue(m - 8, &ztmp);
        zadd(D, ztmp, &B);              /* Will take sqrts until X <= B */
        zfree(ztmp);

        n = 1;                  /* n is to count 1 + number of sqrts */

        while (k > 0 || zrel(X, B) > 0) {
                n++;
                zshift(X, m + (k & 1), &ztmp);
                zfree(X);
                zsqrt(ztmp, &X, conf->triground);
                zfree(ztmp)
                k /= 2;
        }
        zfree(B);
        zsub(X, D, &pow);       /* pow, mul used as tmps */
        zadd(X, D, &mul);
        zfree(X);
        zfree(D);
        zshift(pow, m, &ztmp);
        zfree(pow);
        zquo(ztmp, mul, &pow, conf->triground); /* pow now (X - D)/(X + D) */
        zfree(ztmp);
        zfree(mul);

        zcopy(pow, &sum);       /* pow is first term of sum */
        zsquare(pow, &ztmp);
        zshift(ztmp, -m, &mul); /* mul is now multiplier for powers */
        zfree(ztmp);

        d = 1;
        for (;;) {
                zmul(pow, mul, &ztmp);
                zfree(pow);
                zshift(ztmp, -m, &pow);
                zfree(ztmp);
                d += 2;
                zdivi(pow, d, &term);   /* Round down div should be round off */
                if (ziszero(term)) {
                        zfree(term);
                        break;
                }
                zadd(sum, term, &ztmp);
                zfree(term);
                zfree(sum);
                sum = ztmp;
        }
        zfree(pow);
        zfree(mul);
        qtmp = qalloc();        /* qtmp is to be 2^n * sum / 2^m */
        k = zlowbit(sum);
        sum.sign = neg;
        if (k + n >= m) {
                zshift(sum, n - m, &qtmp->num);
                zfree(sum);
        } else {
                if (k) {
                        zshift(sum, -k, &qtmp->num);
                        zfree(sum);
                } else {
                        qtmp->num = sum;
                }
                zbitvalue(m - k - n, &qtmp->den);
        }
        res = qmappr(qtmp, epsilon, conf->triground);
        qfree(qtmp);
        return res;
}


/*
 * Calculate the base 10 logarithm
 *
 *      log(q) = ln(q) / ln(10)
 */
NUMBER *
qlog(NUMBER *q, NUMBER *epsilon)
{
        int need_new_ln_10 = true;      /* false => use cached ln_10 value */
        NUMBER *ln_q;                   /* ln(x) */
        NUMBER *ret;                    /* base 10 logarithm of x */

        /* firewall */
        if (qiszero(q)) {
                math_error("log of 0");
                not_reached();
        }
        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for log");
                not_reached();
        }

        /*
         * shortcut for small integer powers of 10
         */
        if (qisint(q) && qispos(q) && !zge8192b(q->num) && ziseven(q->num)) {
                bool is_10_power;       /* true ==> q is a power of 10 */
                long ilog_10;           /* integer log base 10 */

                /* try for a quick small power of 10 log */
                ilog_10 = zlog10(q->num, &is_10_power );
                if (is_10_power == true) {
                        /* is small power of 10, return log */
                        return itoq(ilog_10);
                }
                /* q is an even integer that is not a power of 10 */
        }

        /*
         * compute ln(c) first
         */
        ln_q = qln(q, epsilon);
        /* quick return for log(1) == 0 */
        if (qiszero(ln_q)) {
                return ln_q;
        }

        /*
         * save epsilon for ln(10) if needed
         */
        if (ln_10_epsilon == NULL) {
                /* first time call */
                ln_10_epsilon = qcopy(epsilon);
        } else if (qcmp(ln_10_epsilon, epsilon) == true) {
                /* replaced cached value with epsilon arg */
                qfree(ln_10_epsilon);
                ln_10_epsilon = qcopy(epsilon);
        } else if (ln_10 != NULL) {
                /* the previously computed ln(2) is OK to use */
                need_new_ln_10 = false;
        }

        /*
         * compute ln(10) if needed
         */
        if (need_new_ln_10 == true) {
                if (ln_10 != NULL) {
                        qfree(ln_10);
                }
                ln_10 = qln(&_qten_, ln_10_epsilon);
        }

        /*
         * return ln(q) / ln(10)
         */
        ret = qqdiv(ln_q, ln_10);
        qfree(ln_q);
        return ret;
}


/*
 * Calculate the base 2 logarithm
 *
 *      log(q) = ln(q) / ln(2)
 */
NUMBER *
qlog2(NUMBER *q, NUMBER *epsilon)
{
        int need_new_ln_2 = true;       /* false => use cached ln_2 value */
        NUMBER *ln_q;                   /* ln(x) */
        NUMBER *ret;                    /* base 2 logarithm of x */

        /* firewall */
        if (qiszero(q)) {
                math_error("log2 of 0");
                not_reached();
        }
        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for log2");
                not_reached();
        }

        /*
         * special case: q is integer power of 2
         */
        ret = qalloc();
        if (qispowerof2(q, &ret)) {
                return ret;
        }
        qfree(ret);

        /*
         * compute ln(c) first
         */
        ln_q = qln(q, epsilon);
        /* quick return for ln(1) == 0 */
        if (qiszero(ln_q)) {
                return ln_q;
        }

        /*
         * save epsilon for ln(2) if needed
         */
        if (ln_2_epsilon == NULL) {
                /* first time call */
                ln_2_epsilon = qcopy(epsilon);
        } else if (qcmp(ln_2_epsilon, epsilon) == true) {
                /* replaced cached value with epsilon arg */
                qfree(ln_2_epsilon);
                ln_2_epsilon = qcopy(epsilon);
        } else if (ln_2 != NULL) {
                /* the previously computed ln(2) is OK to use */
                need_new_ln_2 = false;
        }

        /*
         * compute ln(2) if needed
         */
        if (need_new_ln_2 == true) {
                if (ln_2 != NULL) {
                        qfree(ln_2);
                }
                ln_2 = qln(&_qtwo_, ln_2_epsilon);
        }

        /*
         * return ln(q) / ln(2)
         */
        ret = qqdiv(ln_q, ln_2);
        qfree(ln_q);
        return ret;
}


/*
 * Calculate the base n logarithm
 *
 *      logn(q, n) = ln(q) / ln(n)
 */
NUMBER *
qlogn(NUMBER *q, NUMBER *n, NUMBER *epsilon)
{
        int need_new_ln_n = true;       /* false => use cached ln_n value */
        NUMBER *ln_q;                   /* ln(x) */
        NUMBER *ret;                    /* base 2 logarithm of x */

        /* firewall */
        if (qiszero(q)) {
                math_error("log base n of 0");
                not_reached();
        }
        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for logn");
                not_reached();
        }
        if (qiszero(n)) {
                math_error("invalid logarithm base of 0 for logn");
                not_reached();
        }
        if (qisone(n)) {
                math_error("invalid logarithm base of 1 for logn");
                not_reached();
        }

        /*
         * special case: q is integer power of 2
         */
        ret = qalloc();
        if (qispowerof2(q, &ret)) {
                return ret;
        }
        /* XXX - deal with n is integer power of 2 - XXX */
        qfree(ret);

        /*
         * compute ln(q) first
         */
        ln_q = qln(q, epsilon);
        /* quick return for ln(1) == 0 */
        if (qiszero(ln_q)) {
                return ln_q;
        }

        /*
         * save epsilon for ln(n) if needed
         */
        if (ln_n_epsilon == NULL) {
                /* first time call */
                ln_n_epsilon = qcopy(epsilon);
        } else if (qcmp(ln_n_epsilon, epsilon) == true) {
                /* replaced cached value with epsilon arg */
                qfree(ln_n_epsilon);
                ln_n_epsilon = qcopy(epsilon);
        } else if (ln_n != NULL) {
                /* the previously computed ln(2) is OK to use */
                need_new_ln_n = false;
        }

        /*
         * compute ln(n) if needed
         */
        if (need_new_ln_n == true) {
                if (ln_n != NULL) {
                        qfree(ln_n);
                }
                ln_n = qln(&_qtwo_, ln_n_epsilon);
        }

        /*
         * return ln(q) / ln(2)
         */
        ret = qqdiv(ln_q, ln_n);
        qfree(ln_q);
        return ret;
}


/*
 * Calculate the result of raising one number to the power of another.
 * The result is calculated to the nearest or next to nearest multiple of
 * epsilon.
 */
NUMBER *
qpower(NUMBER *q1, NUMBER *q2, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *epsilon2;
        NUMBER *q1tmp, *q2tmp;
        long m, n;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon for power");
                not_reached();
        }
        if (qiszero(q1) && qisneg(q2)) {
                math_error("Negative power of zero");
                not_reached();
        }
        if (qiszero(q2) || qisone(q1))
                return qlink(&_qone_);
        if (qiszero(q1))
                return qlink(&_qzero_);
        if (qisneg(q1)) {
                math_error("Negative base for qpower");
                not_reached();
        }
        if (qisone(q2))
                return qmappr(q1, epsilon, conf->triground);
        if (zrel(q1->num, q1->den) < 0) {
                q1tmp = qinv(q1);
                q2tmp = qneg(q2);
        } else {
                q1tmp = qlink(q1);
                q2tmp = qlink(q2);
        }
        if (qisone(q2tmp)) {
                qfree(q2tmp);
                q2tmp = qmappr(q1tmp, epsilon, conf->triground);
                qfree(q1tmp);
                return q2tmp;
        }
        m = qilog2(q1tmp);
        n = qilog2(epsilon);
        if (qisneg(q2tmp)) {
                if (m > 0) {
                        tmp1 = itoq(m);
                        tmp2 = qmul(tmp1, q2tmp);
                        m = qtoi(tmp2);
                } else {
                        tmp1 = qdec(q1tmp);
                        tmp2 = qqdiv(tmp1, q1tmp);
                        qfree(tmp1);
                        tmp1 = qmul(tmp2, q2tmp);
                        qfree(tmp2);
                        tmp2 = qmul(tmp1, &_qlge_);
                        m = qtoi(tmp2);
                }
        } else {
                if (m > 0) {
                        tmp1 = itoq(m + 1);
                        tmp2 = qmul(tmp1, q2tmp);
                        m = qtoi(tmp2);
                } else {
                        tmp1 = qdec(q1tmp);
                        tmp2 = qmul(tmp1, q2tmp);
                        qfree(tmp1);
                        tmp1 = qmul(tmp2, &_qlge_);
                        m = qtoi(tmp1);
                }
        }
        qfree(tmp1);
        qfree(tmp2);
        if (m > (1 << 30)) {
                qfree(q1tmp);
                qfree(q2tmp);
                return NULL;
        }
        m += 1;
        if (m < n) {
                qfree(q1tmp);
                qfree(q2tmp);
                return qlink(&_qzero_);
        }
        tmp1 = qqdiv(epsilon, q2tmp);
        tmp2 = qscale(tmp1, -m - 4);
        epsilon2 = qqabs(tmp2);
        qfree(tmp1);
        qfree(tmp2);
        tmp1 = qln(q1tmp, epsilon2);
        qfree(epsilon2);
        tmp2 = qmul(tmp1, q2tmp);
        qfree(tmp1);
        qfree(q1tmp);
        qfree(q2tmp);
        if (qisneg(tmp2)) {
                tmp1 = qneg(tmp2);
                qfree(tmp2);
                tmp2 = qexprel(tmp1, m - n + 3);
                if (tmp2 == NULL) {
                        qfree(tmp1);
                        return NULL;
                }
                qfree(tmp1);
                tmp1 = qinv(tmp2);
        } else {
                tmp1 = qexprel(tmp2, m - n + 3) ;
                if (tmp1 == NULL) {
                        qfree(tmp2);
                        return NULL;
                }
        }
        qfree(tmp2);
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        return tmp2;
}


/*
 * Calculate the K-th root of a number to within the specified accuracy.
 */
NUMBER *
qroot(NUMBER *q1, NUMBER *q2, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2;
        int neg;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon for root");
                not_reached();
        }
        if (qisneg(q2) || qiszero(q2) || qisfrac(q2)) {
                math_error("Taking bad root of number");
                not_reached();
        }
        if (qiszero(q1) || qisone(q1) || qisone(q2))
                return qlink(q1);
        if (qistwo(q2))
                return qsqrt(q1, epsilon, conf->triground);
        neg = qisneg(q1);
        if (neg) {
                if (ziseven(q2->num)) {
                        math_error("Taking even root of negative number");
                        not_reached();
                }
                q1 = qqabs(q1);
        }
        tmp2 = qinv(q2);
        tmp1 = qpower(q1, tmp2, epsilon);
        qfree(tmp2);
        if (tmp1 == NULL)
                return NULL;
        if (neg) {
                tmp2 = qneg(tmp1);
                qfree(tmp1);
                tmp1 = tmp2;
        }
        return tmp1;
}


/* Calculate the hyperbolic cosine function to the nearest or next to
 * nearest multiple of epsilon.
 * This is calculated using cosh(x) = (exp(x) + 1/exp(x))/2;
 */
NUMBER *
qcosh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *epsilon1;

        epsilon1 = qscale(epsilon, -2);
        tmp1 = qqabs(q);
        tmp2 = qexp(tmp1, epsilon1);
        qfree(tmp1);
        qfree(epsilon1);
        if (tmp2 == NULL)
                return NULL;
        tmp1 = qinv(tmp2);
        tmp3 = qqadd(tmp1, tmp2);
        qfree(tmp1)
        qfree(tmp2)
        tmp1 = qscale(tmp3, -1);
        qfree(tmp3);
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        return tmp2;
}


/*
 * Calculate the hyperbolic sine to the nearest or next to nearest
 * multiple of epsilon.
 * This is calculated using sinh(x) = (exp(x) - 1/exp(x))/2.
 */
NUMBER *
qsinh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *epsilon1;

        if (qiszero(q))
                return qlink(&_qzero_);
        epsilon1 = qscale(epsilon, -3);
        tmp1 = qqabs(q);
        tmp2 = qexp(tmp1, epsilon1);
        qfree(tmp1);
        qfree(epsilon1);
        if (tmp2 == NULL)
                return NULL;
        tmp1 = qinv(tmp2);
        tmp3 = qispos(q) ? qsub(tmp2, tmp1) : qsub(tmp1, tmp2);
        qfree(tmp1)
        qfree(tmp2)
        tmp1 = qscale(tmp3, -1);
        qfree(tmp3);
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        return tmp2;
}


/*
 * Calculate the hyperbolic tangent to the nearest or next to nearest
 * multiple of epsilon.
 * This is calculated using the formulae:
 *      tanh(x) = 1 or -1 for very large abs(x)
 *      tanh(x) = (+ or -)(1 - 2 * exp(2 * x))  for large abx(x)
 *      tanh(x) = (exp(2*x) - 1)/(exp(2*x) + 1) otherwise
 */
NUMBER *
qtanh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3;
        long n, m;

        n = qilog2(epsilon);
        if (n > 0 || qiszero(q))
                return qlink(&_qzero_);
        n = -n;
        tmp1 = qqabs(q);
        tmp2 = qmul(tmp1, &_qlge_);
        m = qtoi(tmp2);         /* exp(|q|) < 2^(m+1) or m == MAXLONG */
        qfree(tmp2);
        if (m > 1 + n/2) {
                qfree(tmp1);
                return q->num.sign ? qlink(&_qnegone_) : qlink(&_qone_);
        }
        tmp2 = qscale(tmp1, 1);
        qfree(tmp1);
        tmp1 = qexprel(tmp2, 2 + n);
        qfree(tmp2);
        if (m > 1 + n/4) {
                tmp2 = qqdiv(&_qtwo_, tmp1);
                qfree(tmp1);
                tmp1 = qsub(&_qone_, tmp2);
                qfree(tmp2);
        } else {
                tmp2 = qdec(tmp1);
                tmp3 = qinc(tmp1);
                qfree(tmp1);
                tmp1 = qqdiv(tmp2, tmp3);
                qfree(tmp2);
                qfree(tmp3);
        }
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        if (qisneg(q)) {
                tmp1 = qneg(tmp2);
                qfree(tmp2);
                return tmp1;
        }
        return tmp2;
}


/*
 * Hyperbolic cotangent.
 * Calculated using coth(x) = 1 + 2/(exp(2*x) - 1)
 */
NUMBER *
qcoth(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *res;
        long n, k;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for coth");
                not_reached();
        }
        if (qiszero(q)) {
                math_error("Zero argument for coth");
                not_reached();
        }
        tmp1 = qscale(q, 1);
        tmp2 = qqabs(tmp1);
        qfree(tmp1);
        k = qilog2(tmp2);
        n = qilog2(epsilon);
        if (k > 0) {
                tmp1 = qmul(&_qlge_, tmp2);
                k = qtoi(tmp1);
                qfree(tmp1);
        } else {
                k = 2 * k;
        }
        k = 4 - k - n;
        if (k < 4)
                k = 4;
        tmp1 = qexprel(tmp2,  k);
        qfree(tmp2);
        tmp2 = qdec(tmp1);
        qfree(tmp1);
        if (qiszero(tmp2)) {
                math_error("This should not happen ????");
                not_reached();
        }
        tmp1 = qinv(tmp2);
        qfree(tmp2);
        tmp2 = qscale(tmp1, 1);
        qfree(tmp1);
        tmp1 = qinc(tmp2);
        qfree(tmp2);
        if (qisneg(q)) {
                tmp2 = qneg(tmp1);
                qfree(tmp1);
                tmp1 = tmp2;
        }
        res = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        return res;
}


NUMBER *
qsech(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *res;
        long n, k;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for sech");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qone_);

        tmp1 = qqabs(q);
        k = 0;
        if (zrel(tmp1->num, tmp1->den) >= 0) {
                tmp2 = qmul(&_qlge_, tmp1);
                k = qtoi(tmp2);
                qfree(tmp2);
        }
        n = qilog2(epsilon);
        if (k + n > 1) {
                qfree(tmp1);
                return qlink(&_qzero_);
        }
        tmp2 = qexprel(tmp1, 4 - k - n);
        qfree(tmp1);
        tmp1 = qinv(tmp2);
        tmp3 = qqadd(tmp1, tmp2);
        qfree(tmp1);
        qfree(tmp2);
        tmp1 = qinv(tmp3);
        qfree(tmp3);
        tmp2 = qscale(tmp1, 1);
        qfree(tmp1);
        res = qmappr(tmp2, epsilon, conf->triground);
        qfree(tmp2);
        return res;
}


NUMBER *
qcsch(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *res;
        long n, k;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for csch");
                not_reached();
        }
        if (qiszero(q)) {
                math_error("Zero argument for csch");
                not_reached();
        }

        n = qilog2(epsilon);
        tmp1 = qqabs(q);
        if (zrel(tmp1->num, tmp1->den) >= 0) {
                tmp2 = qmul(&_qlge_, tmp1);
                k = qtoi(tmp2);
                qfree(tmp2);
        } else {
                k = 2 * qilog2(tmp1);
        }
        if (k + n >= 1) {
                qfree(tmp1);
                return qlink(&_qzero_);
        }
        tmp2 = qexprel(tmp1, 4 - k - n);
        qfree(tmp1);
        tmp1 = qinv(tmp2);
        if (qisneg(q))
                tmp3 = qsub(tmp1, tmp2);
        else
                tmp3 = qsub(tmp2, tmp1);
        qfree(tmp1);
        qfree(tmp2);
        tmp1 = qinv(tmp3);
        qfree(tmp3)
        tmp2 = qscale(tmp1, 1);
        qfree(tmp1);
        res = qmappr(tmp2, epsilon, conf->triground);
        qfree(tmp2);
        return res;
}


/*
 * Compute the hyperbolic arccosine within the specified accuracy.
 * This is calculated using the formula:
 *      acosh(x) = ln(x + sqrt(x^2 - 1)).
 */
NUMBER *
qacosh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *epsilon1;
        long n;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for acosh");
                not_reached();
        }
        if (qisone(q))
                return qlink(&_qzero_);
        if (zrel(q->num, q->den) < 0)
                return NULL;
        n = qilog2(epsilon);
        epsilon1 = qbitvalue(n - 3);
        tmp1 = qsquare(q);
        tmp2 = qdec(tmp1);
        qfree(tmp1);
        tmp1 = qsqrt(tmp2, epsilon1, conf->triground);
        qfree(tmp2);
        tmp2 = qqadd(tmp1, q);
        qfree(tmp1);
        tmp1 = qln(tmp2, epsilon1);
        qfree(tmp2);
        qfree(epsilon1);
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        qfree(tmp1);
        return tmp2;
}


/*
 * Compute the hyperbolic arcsine within the specified accuracy.
 * This is calculated using the formula:
 *      asinh(x) = ln(x + sqrt(x^2 + 1)).
 */
NUMBER *
qasinh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *epsilon1;
        long n;
        bool neg;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for asinh");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qzero_);
        neg = qisneg(q);
        q = qqabs(q);
        n = qilog2(epsilon);
        epsilon1 = qbitvalue(n - 3);
        tmp1 = qsquare(q);
        tmp2 = qinc(tmp1);
        qfree(tmp1);
        tmp1 = qsqrt(tmp2, epsilon1, conf->triground);
        qfree(tmp2);
        tmp2 = qqadd(tmp1, q);
        qfree(tmp1);
        tmp1 = qln(tmp2, epsilon1);
        qfree(tmp2);
        qfree(q);
        qfree(epsilon1);
        tmp2 = qmappr(tmp1, epsilon, conf->triground);
        if (neg) {
                tmp1 = qneg(tmp2);
                qfree(tmp2);
                return tmp1;
        }
        return tmp2;
}


/*
 * Compute the hyperbolic arctangent within the specified accuracy.
 * This is calculated using the formula:
 *      atanh(x) = ln((1 + x) / (1 - x)) / 2.
 */
NUMBER *
qatanh(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp1, *tmp2, *tmp3, *epsilon1;
        ZVALUE z;

        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for atanh");
                not_reached();
        }
        if (qiszero(q))
                return qlink(&_qzero_);
        z = q->num;
        z.sign = 0;
        if (zrel(z, q->den) >= 0)
                return NULL;
        tmp1 = qinc(q);
        tmp2 = qsub(&_qone_, q);
        tmp3 = qqdiv(tmp1, tmp2);
        qfree(tmp1);
        qfree(tmp2);
        epsilon1 = qscale(epsilon, 1L);
        tmp1 = qln(tmp3, epsilon1);
        qfree(tmp3);
        tmp2 = qscale(tmp1, -1L);
        qfree(tmp1);
        qfree(epsilon1);
        return tmp2;
}


/*
 * Inverse hyperbolic secant function
 */
NUMBER *
qasech(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp, *res;

        tmp = qinv(q);
        res = qacosh(tmp, epsilon);
        qfree(tmp);
        return res;
}


/*
 * Inverse hyperbolic cosecant function
 */
NUMBER *
qacsch(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp, *res;

        tmp = qinv(q);
        res = qasinh(tmp, epsilon);
        qfree(tmp);
        return res;
}


/*
 * Inverse hyperbolic cotangent function
 */
NUMBER *
qacoth(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *tmp, *res;

        tmp = qinv(q);
        res = qatanh(tmp, epsilon);
        qfree(tmp);
        return res;
}


/*
 * qversin - versed trigonometric sine
 *
 * This uses the formula:
 *
 *      versin(x) = 1 - cos(x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qcos(q, epsilon);
        res = qsub(&_qone_, qtmp);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qaversin_or_NULL - return NULL or non-complex inverse versed trigonometric sine
 *
 * This uses the formula:
 *
 *      aversin(x) = acos(1 - x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qaversin_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(&_qone_, q);
        res = qacos(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qaversin - non-complex inverse versed trigonometric sine
 *
 * This uses the formula:
 *
 *      aversin(x) = acos(1 - x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qaversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qaversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cosine for aversin");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qcoversin - coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      coversin((x) = 1 - sin(x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qcoversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qsin(q, epsilon);
        res = qsub(&_qone_, qtmp);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qacoversin_or_NULL - return NULL or non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      acoversin(x) = asin(1 - x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qacoversin_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(&_qone_, q);
        res = qasin(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qacoversin - non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      acoversin(x) = asin(1 - x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qacoversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qacoversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse sine for acoversin");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qvercos - versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      vercos(x) = 1 + cos(x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qvercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qcos(q, epsilon);
        res = qqadd(&_qone_, qtmp);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qavercos_or_NULL - return NULL or non-complex inverse versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      avercos(x) = acos(x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qavercos_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(q, &_qone_);
        res = qacos(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qavercos - non-complex inverse versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      avercos(x) = acos(x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qavercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qavercos_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cosine for avercos");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qcovercos - coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      covercos((x) = 1 + sin(x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qcovercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qsin(q, epsilon);
        res = qqadd(&_qone_, qtmp);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qacovercos_or_NULL - return NULL or non-complex inverse coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      acovercos(x) = asin(x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qacovercos_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(&_qone_, q);
        res = qasin(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qacovercos - non-complex inverse coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      acovercos(x) = asin(x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qacovercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qacovercos_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse sine for acovercos");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qhaversin - half versed trigonometric sine
 *
 * This uses the formula:
 *
 *      haversin(x) = versin(x) / 2
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qhaversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qversin(q, epsilon);
        res = qdivi(qtmp, 2);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qahaversin_or_NULL - return NULL or non-complex inverse half versed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahaversin(x) = acos(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qahaversin_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */
        NUMBER *x2;     /* twice x */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        x2 = qmuli(q, 2);
        qtmp = qsub(&_qone_, x2);
        qfree(x2);
        res = qacos(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qahaversin - non-complex inverse half versed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahaversin(x) = acos(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qahaversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qahaversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cosine for ahaversin");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qhacoversin - coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      hacoversin((x) = coversin(x) / 2
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qhacoversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qcoversin(q, epsilon);
        res = qdivi(qtmp, 2);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qahacoversin_or_NULL - return NULL or non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahacoversin(x) = asin(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qahacoversin_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(&_qone_, q);
        res = qasin(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qahacoversin - non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahacoversin(x) = asin(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qahacoversin(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qahacoversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse sine for ahacoversin");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qhavercos - half versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      havercos(x) = vercos(x) / 2
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qhavercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qvercos(q, epsilon);
        res = qdivi(qtmp, 2);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qahavercos_or_NULL - return NULL or non-complex inverse half versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      ahavercos(x) = acos(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qahavercos_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */
        NUMBER *x2;     /* twice x */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        x2 = qmuli(q, 2);
        qtmp = qsub(&_qone_, x2);
        qfree(x2);
        res = qacos(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qahavercos - non-complex inverse half versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      ahavercos(x) = acos(1 - 2*x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qahavercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qahaversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cocosine for ahavercos");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qhacovercos - half coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      hacovercos((x) = covercos(x) / 2
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qhacovercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qcovercos(q, epsilon);
        res = qdivi(qtmp, 2);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qahacovercos_or_NULL - return NULL or non-complex inverse half coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      ahacovercos(x) = acos(2*x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qahacovercos_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qsub(&_qone_, q);
        res = qacos(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qahacovercos - non-complex inverse half coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      ahacovercos(x) = acos(2*x - 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qahacovercos(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qahacoversin_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cosine for ahacovercos");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qexsec - exterior trigonometric secant
 *
 * This uses the formula:
 *
 *      exsec(x) = sec(x) - 1
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qexsec(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qsec(q, epsilon);
        res = qsub(qtmp, &_qone_);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qaexsec_or_NULL - return NULL or non-complex inverse versed trigonometric sine
 *
 * This uses the formula:
 *
 *      aexsec(x) = asec(x + 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qaexsec_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qaddi(q, 1);
        res = qasec(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qaexsec - non-complex inverse versed trigonometric sine
 *
 * This uses the formula:
 *
 *      aexsec(x) = asec(x + 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qaexsec(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qaexsec_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse cosine for aexsec");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qexcsc - coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      excsc(x) = csc(x) - 1
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qexcsc(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qtmp = qcsc(q, epsilon);
        res = qsub(qtmp, &_qone_);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qaexcsc_or_NULL - return NULL or non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      aexcsc(x) = acsc(x + 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qaexcsc_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qtmp = qaddi(q, 1);
        res = qacsc(qtmp, epsilon);
        qfree(qtmp);
        if (res == NULL) {
                return NULL;
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qaexcsc - non-complex inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      aexcsc(x) = acsc(x + 1)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qaexcsc(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qaexcsc_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse sine for aexcsc");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qcrd - trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      crd(x) = 2 * sin(x / 2)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qcrd(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qdiv2;  /* q/2 */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate trig function value
         */
        qdiv2 = qdivi(q, 2);
        qtmp = qsin(qdiv2, epsilon);
        qfree(qdiv2);
        res = qmuli(qtmp, 2);
        qfree(qtmp);

        /*
         * return trigonometric result
         */
        return res;
}


/*
 * qacrd_or_NULL - return NULL or non-complex inverse trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      acrd(x) = 2 * asin(x / 2)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 *
 * returns:
 *      != NULL ==> real value result of trig function on q with error epsilon,
 *      NULL    ==> trig function value cannot be expressed as a NUMBER
 *
 * NOTE: If this function returns NULL, consider calling the equivalent
 *       COMPLEX function from comfunc.c.  See the help file for the
 *       related builtin for details.
 */
NUMBER *
qacrd_or_NULL(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */
        NUMBER *qdiv2;  /* q/2 */
        NUMBER *qtmp;   /* argument to inverse trig function */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        qdiv2 = qdivi(q, 2);
        qtmp = qasin(qdiv2, epsilon);
        qfree(qdiv2);
        if (qtmp == NULL) {
                return NULL;
        }
        res = qmuli(qtmp, 2);
        qfree(qtmp);

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qacrd - non-complex inverse trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      acrd(x) = 2 * asin(x / 2)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qacrd(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *res;    /* inverse trig value result */

        /*
         * firewall
         */
        if (q == NULL) {
                math_error("q is NULL for %s", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate inverse trig function value
         */
        res = qacrd_or_NULL(q, epsilon);
        if (res == NULL) {
                math_error("cannot compute inverse sine for acrd");
                not_reached();
        }

        /*
         * return inverse trigonometric result
         */
        return res;
}


/*
 * qcas - trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      cas(x) = cos(x) + sin(x)
 *
 * given:
 *      q           real value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      real value result of trig function on q with error epsilon
 */
NUMBER *
qcas(NUMBER *q, NUMBER *epsilon)
{
        NUMBER *sin;    /* sin(x) */
        NUMBER *tsin;   /* sin(x) rounded to nearest epsilon multiple */
        NUMBER *cos;    /* cos(x) */
        NUMBER *tcos;   /* cos(x) rounded to nearest epsilon multiple */
        NUMBER *res;
        long n;

        /*
         * firewall
         */
        if (qiszero(epsilon)) {
                math_error("Zero epsilon value for cosine");
                not_reached();
        }

        /*
         * case 0: quick return 1
         */
        if (qiszero(q)) {
                return qlink(&_qone_);
        }

        /*
         * case epsilon > 1: quick return 0
         */
        n = -qilog2(epsilon);
        if (n < 0) {
                return qlink(&_qzero_);
        }

        /*
         * compute cosine and sine
         */
        qsincos(q, n + 2, &sin, &cos);
        tcos = qmappr(cos, epsilon, conf->triground);
        qfree(cos);
        tsin = qmappr(sin, epsilon, conf->triground);
        qfree(sin);
        res = qqadd(tcos, tsin);
        qfree(tcos);
        qfree(tsin);

        /*
         * return trigonometric result
         */
        return res;
}
