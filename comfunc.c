/*
 * comfunc - extended precision complex arithmetic non-primitive routines
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
 * Under source code control:   1990/02/15 01:48:13
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "config.h"
#include "cmath.h"


#include "errtbl.h"
#include "banned.h"     /* include after system header <> includes */


/*
 * cache the natural logarithm of 10
 */
STATIC COMPLEX *cln_10 = NULL;
STATIC NUMBER *cln_10_epsilon = NULL;
STATIC COMPLEX *cln_2 = NULL;
STATIC NUMBER *cln_2_epsilon = NULL;
STATIC NUMBER _q10_ = { { _tenval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
STATIC NUMBER _q2_ = { { _twoval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
STATIC NUMBER _q0_ = { { _zeroval_, 1, 0 }, { _oneval_, 1, 0 }, 1, NULL };
COMPLEX _cten_ = { &_q10_, &_q0_, 1 };
COMPLEX _ctwo_ = { &_q2_, &_q0_, 1 };


/*
 * Compute the result of raising a complex number to an integer power.
 *
 * given:
 *      c               complex number to be raised
 *      q               power to raise it to
 */
COMPLEX *
c_powi(COMPLEX *c, NUMBER *q)
{
        COMPLEX *tmp, *res;     /* temporary values */
        long power;             /* power to raise to */
        FULL bit;               /* current bit value */
        int sign;

        if (qisfrac(q)) {
                math_error("Raising number to non-integral power");
                not_reached();
        }
        if (zge31b(q->num)) {
                math_error("Raising number to very large power");
                not_reached();
        }
        power = ztolong(q->num);
        if (ciszero(c) && (power == 0)) {
                math_error("Raising zero to zeroth power");
                not_reached();
        }
        sign = 1;
        if (qisneg(q))
                sign = -1;
        /*
         * Handle some low powers specially
         */
        if (power <= 4) {
                switch ((int) (power * sign)) {
                case 0:
                        return clink(&_cone_);
                case 1:
                        return clink(c);
                case -1:
                        return c_inv(c);
                case 2:
                        return c_square(c);
                case -2:
                        tmp = c_square(c);
                        res = c_inv(tmp);
                        comfree(tmp);
                        return res;
                case 3:
                        tmp = c_square(c);
                        res = c_mul(c, tmp);
                        comfree(tmp);
                        return res;
                case 4:
                        tmp = c_square(c);
                        res = c_square(tmp);
                        comfree(tmp);
                        return res;
                }
        }
        /*
         * Compute the power by squaring and multiplying.
         * This uses the left to right method of power raising.
         */
        bit = TOPFULL;
        while ((bit & power) == 0)
                bit >>= 1L;
        bit >>= 1L;
        res = c_square(c);
        if (bit & power) {
                tmp = c_mul(res, c);
                comfree(res);
                res = tmp;
        }
        bit >>= 1L;
        while (bit) {
                tmp = c_square(res);
                comfree(res);
                res = tmp;
                if (bit & power) {
                        tmp = c_mul(res, c);
                        comfree(res);
                        res = tmp;
                }
                bit >>= 1L;
        }
        if (sign < 0) {
                tmp = c_inv(res);
                comfree(res);
                res = tmp;
        }
        return res;
}


/*
 * Calculate the square root of a complex number to specified accuracy.
 * Type of rounding of each component specified by R as for qsqrt().
 */
COMPLEX *
c_sqrt(COMPLEX *c, NUMBER *epsilon, long R)
{
        COMPLEX *r;
        NUMBER *es, *aes, *bes, *u, *v, qtemp;
        NUMBER *ntmp;
        ZVALUE g, a, b, d, aa, cc;
        ZVALUE tmp1, tmp2, tmp3, mul1, mul2;
        long s1, s2, s3, up1, up2;
        int imsign, sign;

        if (ciszero(c))
                return clink(c);
        if (cisreal(c)) {
                r = comalloc();
                if (!qisneg(c->real)) {
                        qfree(r->real);
                        r->real = qsqrt(c->real, epsilon, R);
                        return r;
                }
                ntmp = qneg(c->real);
                qfree(r->imag);
                r->imag = qsqrt(ntmp, epsilon, R);
                qfree(ntmp);
                return r;
        }

        up1 = up2 = 0;
        sign = (R & 64) != 0;
        imsign = c->imag->num.sign;
        es = qsquare(epsilon);
        aes = qqdiv(c->real, es);
        bes = qqdiv(c->imag, es);
        qfree(es);
        zgcd(aes->den, bes->den, &g);
        zequo(bes->den, g, &tmp1);
        zmul(aes->num, tmp1, &a);
        zmul(aes->den, tmp1, &tmp2);
        zshift(tmp2, 1, &d);
        zfree(tmp1);
        zfree(tmp2);
        zequo(aes->den, g, &tmp1);
        zmul(bes->num, tmp1, &b);
        zfree(tmp1);
        zfree(g);
        qfree(aes);
        qfree(bes);
        zsquare(a, &tmp1);
        zsquare(b, &tmp2);
        zfree(b);
        zadd(tmp1, tmp2, &tmp3);
        zfree(tmp1);
        zfree(tmp2);
        if (R & 16) {
                zshift(tmp3, 4, &tmp1);
                zfree(tmp3);
                zshift(a, 2, &aa);
                zfree(a);
                s1 = zsqrt(tmp1, &cc, 16);
                zfree(tmp1);
                zadd(cc, aa, &tmp1);
                if (s1 == 0 && R & 32) {
                        zmul(tmp1, d, &tmp2);
                        zfree(tmp1);
                        s2 = zsqrt(tmp2, &tmp3, 16);
                        zfree(tmp2);
                        if (s2 == 0) {
                                aes = qalloc();
                                zshift(d, 1, &tmp1);
                                zreduce(tmp3, tmp1, &aes->num, &aes->den);
                                zfree(tmp1);
                                zfree(tmp3);
                                zfree(aa);
                                zfree(cc);
                                zfree(d);
                                r = comalloc();
                                qtemp = *aes;
                                qtemp.num.sign = sign;
                                qfree(r->real);
                                r->real = qmul(&qtemp, epsilon);
                                qfree(aes);
                                bes = qscale(r->real, 1);
                                qtemp = *bes;
                                qtemp.num.sign = sign ^ imsign;
                                qfree(r->imag);
                                r->imag = qqdiv(c->imag, &qtemp);
                                qfree(bes);
                                return r;
                        }
                        s3 = zquo(tmp3, d, &tmp1, s2 < 0);
                } else {
                        s2 = zquo(tmp1, d, &tmp3, s1 ? (s1 < 0) : 16);
                        zfree(tmp1);
                        s3 = zsqrt(tmp3,&tmp1,(s1||s2) ? (s1<0 || s2<0) : 16);
                }
                zfree(tmp3);
                zshift(tmp1, -1, &mul1);
                if (*tmp1.v & 1)
                        up1 = s1 + s2 + s3;
                else
                        up1 = -1;
                zfree(tmp1);
                zsub(cc, aa, &tmp1);
                s2 = zquo(tmp1, d, &tmp2, s1 ? (s1 < 0) : 16);
                zfree(tmp1);
                s3 = zsqrt(tmp2, &tmp1, (s1 || s2) ? (s1 < 0 || s2 < 0) : 16);
                zfree(tmp2);
                zshift(tmp1, -1, &mul2);
                if (*tmp1.v & 1)
                        up2 = s1 + s2 + s3;
                else
                        up2 = -1;
                zfree(tmp1);
                zfree(aa);
        } else {
                s1 = zsqrt(tmp3, &cc, 0);
                zfree(tmp3);
                zadd(cc, a, &tmp1);
                if (s1 == 0 && R & 32) {
                        zmul(tmp1, d, &tmp2);
                        zfree(tmp1);
                        s2 = zsqrt(tmp2, &tmp3, 0);
                        zfree(tmp2);
                        if (s2 == 0) {
                                aes = qalloc();
                                zreduce(tmp3, d, &aes->num, &aes->den);
                                zfree(tmp3);
                                zfree(a);
                                zfree(cc);
                                zfree(d);
                                r = comalloc();
                                qtemp = *aes;
                                qtemp.num.sign = sign;
                                qfree(r->real);
                                r->real = qmul(&qtemp, epsilon);
                                qfree(aes);
                                bes = qscale(r->real, 1);
                                qtemp = *bes;
                                qtemp.num.sign = sign ^ imsign;
                                qfree(r->imag);
                                r->imag = qqdiv(c->imag, &qtemp);
                                qfree(bes);
                                return r;
                        }
                        s3 = zquo(tmp3, d, &mul1, 0);
                } else {
                        s2 = zquo(tmp1, d, &tmp3, 0);
                        zfree(tmp1);
                        s3 = zsqrt(tmp3, &mul1, 0);
                }
                up1 = (s1 + s2 + s3) ? 0 : -1;
                zfree(tmp3);
                zsub(cc, a, &tmp1);
                s2 = zquo(tmp1, d, &tmp2, 0);
                zfree(tmp1);
                s3 = zsqrt(tmp2, &mul2, 0);
                up2 = (s1 + s2 + s3) ? 0 : -1;
                zfree(tmp2);
                zfree(a);
        }
        zfree(cc); zfree(d);
        if (up1 == 0) {
                if (R & 8)
                        up1 = (long)((R ^ *mul1.v) & 1);
                else
                        up1 = (R ^ epsilon->num.sign ^ sign) & 1;
                if (R & 2)
                        up1 ^= epsilon->num.sign ^ sign;
                if (R & 4)
                        up1 ^= epsilon->num.sign;
        }
        if (up2 == 0) {
                if (R & 8)
                        up2 = (long)((R ^ *mul2.v) & 1);
                else
                        up2 = (R ^ epsilon->num.sign ^ sign ^ imsign) & 1;
                if (R & 2)
                        up2 ^= epsilon->num.sign ^ imsign ^ sign;
                if (R & 4)
                        up2 ^= epsilon->num.sign;
        }
        if (up1 > 0) {
                zadd(mul1, _one_, &tmp1);
                zfree(mul1);
                mul1 = tmp1;
        }
        if (up2 > 0) {
                zadd(mul2, _one_, &tmp2);
                zfree(mul2);
                mul2 = tmp2;
        }
        if (ziszero(mul1)) {
                u = qlink(&_qzero_);
        } else {
                mul1.sign = sign ^ epsilon->num.sign;
                u = qalloc();
                zreduce(mul1, epsilon->den, &tmp2, &u->den);
                zmul(tmp2, epsilon->num, &u->num);
                zfree(tmp2);
        }
        zfree(mul1);
        if (ziszero(mul2)) {
                v = qlink(&_qzero_);
        } else {
                mul2.sign = imsign ^ sign ^ epsilon->num.sign;
                v = qalloc();
                zreduce(mul2, epsilon->den, &tmp2, &v->den);
                zmul(tmp2, epsilon->num, &v->num);
                zfree(tmp2);
        }
        zfree(mul2);
        if (qiszero(u) && qiszero(v)) {
                qfree(u);
                qfree(v);
                return clink(&_czero_);
        }
        r = comalloc();
        qfree(r->real);
        qfree(r->imag);
        r->real = u;
        r->imag = v;
        return r;
}


/*
 * Take the Nth root of a complex number, where N is a positive integer.
 * Each component of the result is within the specified error.
 */
COMPLEX *
c_root(COMPLEX *c, NUMBER *q, NUMBER *epsilon)
{
        COMPLEX *r;
        NUMBER *a2pb2, *root, *tmp1, *tmp2, *epsilon2;
        long n, m;

        if (qisneg(q) || qiszero(q) || qisfrac(q)) {
                math_error("Taking bad root of complex number");
                not_reached();
        }
        if (cisone(c) || qisone(q))
                return clink(c);
        if (qistwo(q))
                return c_sqrt(c, epsilon, conf->triground);
        if (cisreal(c) && !qisneg(c->real)) {
                tmp1 = qroot(c->real, q, epsilon);
                if (tmp1 == NULL)
                        return NULL;
                r = comalloc();
                qfree(r->real);
                r->real = tmp1;
                return r;
        }
        /*
         * Calculate the root using the formula:
         *      c_root(a + bi, n) =
         *              c_polar(qroot(a^2 + b^2, 2 * n), qatan2(b, a) / n).
         */
        n = qilog2(epsilon);
        epsilon2 = qbitvalue(n - 4);
        tmp1 = qsquare(c->real);
        tmp2 = qsquare(c->imag);
        a2pb2 = qqadd(tmp1, tmp2);
        qfree(tmp1);
        qfree(tmp2);
        tmp1 = qscale(q, 1L);
        root = qroot(a2pb2, tmp1, epsilon2);
        qfree(a2pb2);
        qfree(tmp1);
        qfree(epsilon2);
        if (root == NULL)
                return NULL;
        m = qilog2(root);
        if (m < n) {
                qfree(root);
                return clink(&_czero_);
        }
        epsilon2 = qbitvalue(n - m - 4);
        tmp1 = qatan2(c->imag, c->real, epsilon2);
        qfree(epsilon2);
        tmp2 = qqdiv(tmp1, q);
        qfree(tmp1);
        r = c_polar(root, tmp2, epsilon);
        qfree(root);
        qfree(tmp2);
        return r;
}


/*
 * Calculate the complex exponential function to the desired accuracy.
 * We use the formula:
 *      exp(a + bi) = exp(a) * (cos(b) + i * sin(b)).
 */
COMPLEX *
c_exp(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;
        NUMBER *sin, *cos, *tmp1, *tmp2, *epsilon1;
        long k, n;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex exp");
                not_reached();
        }
        if (cisreal(c)) {
                tmp1 = qexp(c->real, epsilon);
                if (tmp1 == NULL)
                        return NULL;
                r = comalloc();
                qfree(r->real);
                r->real = qexp(c->real, epsilon);
                return r;
        }
        n = qilog2(epsilon);
        epsilon1 = qbitvalue(n - 2);
        tmp1 = qexp(c->real, epsilon1);
        qfree(epsilon1);
        if (tmp1 == NULL)
                return NULL;
        if (qiszero(tmp1)) {
                qfree(tmp1);
                return clink(&_czero_);
        }
        k = qilog2(tmp1) + 1;
        if (k < n) {
                qfree(tmp1);
                return clink(&_czero_);
        }
        qsincos(c->imag, k - n + 2, &sin, &cos);
        tmp2 = qmul(tmp1, cos);
        qfree(cos);
        r = comalloc();
        qfree(r->real);
        r->real = qmappr(tmp2, epsilon, conf->triground);
        qfree(tmp2);
        tmp2 = qmul(tmp1, sin);
        qfree(tmp1);
        qfree(sin);
        qfree(r->imag);
        r->imag = qmappr(tmp2, epsilon, conf->triground);
        qfree(tmp2);
        return r;
}


/*
 * Calculate the natural logarithm of a complex number within the specified
 * error.  We use the formula:
 *      ln(a + bi) = ln(a^2 + b^2) / 2 + i * atan2(b, a).
 */
COMPLEX *
c_ln(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;
        NUMBER *a2b2, *tmp1, *tmp2, *epsilon1;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex ln");
                not_reached();
        }
        if (cisone(c))
                return clink(&_czero_);
        r = comalloc();
        if (cisreal(c) && !qisneg(c->real)) {
                qfree(r->real);
                r->real = qln(c->real, epsilon);
                return r;
        }
        tmp1 = qsquare(c->real);
        tmp2 = qsquare(c->imag);
        a2b2 = qqadd(tmp1, tmp2);
        qfree(tmp1);
        qfree(tmp2);
        epsilon1 = qscale(epsilon, 1L);
        tmp1 = qln(a2b2, epsilon1);
        qfree(a2b2);
        qfree(epsilon1);
        qfree(r->real);
        r->real = qscale(tmp1, -1L);
        qfree(tmp1);
        qfree(r->imag);
        r->imag = qatan2(c->imag, c->real, epsilon);
        return r;
}


/*
 * Calculate base 10 logarithm by:
 *
 *      log(c) = ln(c) / ln(10)
 */
COMPLEX *
c_log(COMPLEX *c, NUMBER *epsilon)
{
        int need_new_cln_10 = true;     /* false => use cached cln_10 value */
        COMPLEX *ln_c;                  /* ln(x) */
        COMPLEX *ret;                   /* base 10 logarithm of x */

        /*
         * compute ln(c) first
         */
        ln_c = c_ln(c, epsilon);
        /* quick return for log(1) == 0 */
        if (ciszero(ln_c)) {
                return ln_c;
        }

        /*
         * save epsilon for ln(10) if needed
         */
        if (cln_10_epsilon == NULL) {
                /* first time call */
                cln_10_epsilon = qcopy(epsilon);
        } else if (qcmp(cln_10_epsilon, epsilon) == true) {
                /* replaced cached value with epsilon arg */
                qfree(cln_10_epsilon);
                cln_10_epsilon = qcopy(epsilon);
        } else if (cln_10 != NULL) {
                /* the previously computed ln(2) is OK to use */
                need_new_cln_10 = false;
        }

        /*
         * compute ln(10) if needed
         */
        if (need_new_cln_10 == true) {
                if (cln_10 != NULL) {
                        comfree(cln_10);
                }
                cln_10 = c_ln(&_cten_, cln_10_epsilon);
        }

        /*
         * return ln(c) / ln(10)
         */
        ret = c_div(ln_c, cln_10);
        comfree(ln_c);
        return ret;
}


/*
 * Calculate base 2 logarithm by:
 *
 *      log(c) = ln(c) / ln(2)
 */
COMPLEX *
c_log2(COMPLEX *c, NUMBER *epsilon)
{
        int need_new_cln_2 = true;      /* false => use cached cln_2 value */
        COMPLEX *ln_c;                  /* ln(x) */
        COMPLEX *ret;                   /* base 2 of x */

        /*
         * compute ln(c) first
         */
        ln_c = c_ln(c, epsilon);
        /* quick return for log(1) == 0 */
        if (ciszero(ln_c)) {
                return ln_c;
        }

        /*
         * save epsilon for ln(2) if needed
         */
        if (cln_2_epsilon == NULL) {
                /* first time call */
                cln_2_epsilon = qcopy(epsilon);
        } else if (qcmp(cln_2_epsilon, epsilon) == true) {
                /* replaced cached value with epsilon arg */
                qfree(cln_2_epsilon);
                cln_2_epsilon = qcopy(epsilon);
        } else if (cln_2 != NULL) {
                /* the previously computed ln(2) is OK to use */
                need_new_cln_2 = false;
        }

        /*
         * compute ln(2) if needed
         */
        if (need_new_cln_2 == true) {
                if (cln_2 != NULL) {
                        comfree(cln_2);
                }
                cln_2 = c_ln(&_ctwo_, cln_2_epsilon);
        }

        /*
         * return ln(c) / ln(2)
         */
        ret = c_div(ln_c, cln_2);
        comfree(ln_c);
        return ret;
}


/*
 * Calculate the complex cosine within the specified accuracy.
 * This uses the formula:
 *      cos(x) = (exp(1i * x) + exp(-1i * x))/2;
 */
COMPLEX *
c_cos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r, *ctmp1, *ctmp2, *ctmp3;
        NUMBER *epsilon1;
        long n;
        bool neg;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex cos");
                not_reached();
        }
        n = qilog2(epsilon);
        ctmp1 = comalloc();
        qfree(ctmp1->real);
        qfree(ctmp1->imag);
        neg = qisneg(c->imag);
        ctmp1->real = neg ? qneg(c->imag) : qlink(c->imag);
        ctmp1->imag = neg ? qlink(c->real) : qneg(c->real);
        epsilon1 = qbitvalue(n - 2);
        ctmp2 = c_exp(ctmp1, epsilon1);
        comfree(ctmp1);
        qfree(epsilon1);
        if (ctmp2 == NULL)
                return NULL;
        if (ciszero(ctmp2)) {
                comfree(ctmp2);
                return clink(&_czero_);
        }
        ctmp1 = c_inv(ctmp2);
        ctmp3 = c_add(ctmp2, ctmp1);
        comfree(ctmp1);
        comfree(ctmp2);
        ctmp1 = c_scale(ctmp3, -1);
        comfree(ctmp3);
        r = comalloc();
        qfree(r->real);
        r->real = qmappr(ctmp1->real, epsilon, conf->triground);
        qfree(r->imag);
        r->imag = qmappr(ctmp1->imag, epsilon, conf->triground);
        comfree(ctmp1);
        return r;
}


/*
 * Calculate the complex sine within the specified accuracy.
 * This uses the formula:
 *      sin(x) = (exp(1i * x) - exp(-i1*x))/(2i).
 */
COMPLEX *
c_sin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r, *ctmp1, *ctmp2, *ctmp3;
        NUMBER *qtmp, *epsilon1;
        long n;
        bool neg;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex sin");
                not_reached();
        }
        if (ciszero(c)) {
                return clink(&_czero_);
        }
        n = qilog2(epsilon);
        ctmp1 = comalloc();
        neg = qisneg(c->imag);
        qfree(ctmp1->real);
        qfree(ctmp1->imag);
        ctmp1->real = neg ? qneg(c->imag) : qlink(c->imag);
        ctmp1->imag = neg ? qlink(c->real) : qneg(c->real);
        epsilon1 = qbitvalue(n - 2);
        ctmp2 = c_exp(ctmp1, epsilon1);
        comfree(ctmp1);
        qfree(epsilon1);
        if (ctmp2 == NULL) {
                return NULL;
        }
        if (ciszero(ctmp2)) {
                comfree(ctmp2);
                return clink(&_czero_);
        }
        ctmp1 = c_inv(ctmp2);
        ctmp3 = c_sub(ctmp2, ctmp1);
        comfree(ctmp1);
        comfree(ctmp2);
        ctmp1 = c_scale(ctmp3, -1);
        comfree(ctmp3);
        r = comalloc();
        qtmp = neg ? qlink(ctmp1->imag) : qneg(ctmp1->imag);
        qfree(r->real);
        r->real = qmappr(qtmp, epsilon, conf->triground);
        qfree(qtmp);
        qtmp = neg ? qneg(ctmp1->real) : qlink(ctmp1->real);
        qfree(r->imag);
        r->imag = qmappr(qtmp, epsilon, conf->triground);
        qfree(qtmp);
        comfree(ctmp1);
        return r;
}


COMPLEX *
c_cosh(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        tmp1 = c_exp(c, epsilon);
        if (tmp1 == NULL)
                return NULL;
        tmp2 = c_neg(c);
        tmp3 = c_exp(tmp2, epsilon);
        comfree(tmp2);
        if (tmp3 == NULL)
                return NULL;
        tmp2 = c_add(tmp1, tmp3);
        comfree(tmp1);
        comfree(tmp3);
        tmp1 = c_scale(tmp2, -1);
        comfree(tmp2);
        return tmp1;
}


COMPLEX *
c_sinh(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        tmp1 = c_exp(c, epsilon);
        if (tmp1 == NULL)
                return NULL;
        tmp2 = c_neg(c);
        tmp3 = c_exp(tmp2, epsilon);
        comfree(tmp2);
        if (tmp3 == NULL)
                return NULL;
        tmp2 = c_sub(tmp1, tmp3);
        comfree(tmp1);
        comfree(tmp3);
        tmp1 = c_scale(tmp2, -1);
        comfree(tmp2);
        return tmp1;
}


COMPLEX *
c_asin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_mul(&_conei_, c);
        tmp2 = c_asinh(tmp1, epsilon);
        comfree(tmp1);
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        return tmp1;
}


COMPLEX *
c_acos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_square(c);
        tmp2 = c_sub(&_cone_, tmp1);
        comfree(tmp1);
        tmp1 = c_sqrt(tmp2, epsilon, conf->triground);
        comfree(tmp2);
        tmp2 = c_mul(&_conei_, tmp1);
        comfree(tmp1);
        tmp1 = c_add(c, tmp2);
        comfree(tmp2);
        tmp2 = c_ln(tmp1, epsilon);
        comfree(tmp1);
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        return tmp1;
}


COMPLEX *
c_asinh(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;
        bool neg;

        neg = qisneg(c->real);
        tmp1 = neg ? c_neg(c) : clink(c);
        tmp2 = c_square(tmp1);
        tmp3 = c_add(&_cone_, tmp2);
        comfree(tmp2);
        tmp2 = c_sqrt(tmp3, epsilon, conf->triground);
        comfree(tmp3);
        tmp3 = c_add(tmp2, tmp1);
        comfree(tmp1);
        comfree(tmp2);
        tmp1 = c_ln(tmp3, epsilon);
        comfree(tmp3);
        if (neg) {
                tmp2 = c_neg(tmp1);
                comfree(tmp1);
                return tmp2;
        }
        return tmp1;
}


COMPLEX *
c_acosh(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_square(c);
        tmp2 = c_sub(tmp1, &_cone_);
        comfree(tmp1);
        tmp1 = c_sqrt(tmp2, epsilon, conf->triground);
        comfree(tmp2);
        tmp2 = c_add(c, tmp1);
        comfree(tmp1);
        tmp1 = c_ln(tmp2, epsilon);
        comfree(tmp2);
        return tmp1;
}


/*
 * c_tan - complex trigonometric tangent
 *
 * This uses the formula:
 *
 *      tan(x) = sin(x) / cos(x)
 *
 * given:
 *      c           argument to the trigonometric function
 *      epsilon     precision of the trigonometric calculation
 *
 * returns:
 *      != NULL ==> allocated pointer to COMPLEX result
 *      NULL ==> invalid trigonometric argument
 *
 * NOTE: When the trigonometric result is returned as non-NULL result,
 *       the value may be a real value.  The caller may wish to:
 *
 *              COMPLEX *c;     (* return result of this function *)
 *              NUMBER *q;      (* COMPLEX result when c is a real number *)
 *
 *              if (c == NULL) {
 *                      math_error("... some error message");
 *                      not_reached();
 *              }
 *              if (cisreal(c)) {
 *                      q = c_to_q(c, ok_to_free);
 *              }
 */
COMPLEX *
c_tan(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *denom; /* trigonometric identity numerator */
        COMPLEX *numer; /* trigonometric identity denominator */
        COMPLEX *res;   /* trigonometric result */

        /*
         * firewall - check args
         */
        if (c == NULL) {
                return NULL;
        }
        if (check_epsilon(epsilon) == false) {
                return NULL;
        }

        /*
         * evaluate the cos(x) denominator
         *
         * Return NULL if cos(x) failed or we otherwise divide by zero.
         */
        denom = c_cos(c, epsilon);
        if (denom == NULL || ciszero(denom)) {
                return NULL;
        }

        /*
         * evaluate the sin(x) numerator
         *
         * Return NULL if sin(x) failed.
         */
        numer = c_sin(c, epsilon);
        if (numer == NULL) {
                comfree(denom);
                return NULL;
        }

        /*
         * catch the special case of numerator of 0
         */
        if (ciszero(numer)) {
                comfree(denom);
                comfree(numer);
                return clink(&_czero_);
        }

        /*
         * compute the trigonometric function value
         */
        res = c_div(numer, denom);
        comfree(denom);
        comfree(numer);

        /*
         * return the trigonometric result
         */
        return res;
}


COMPLEX *
c_atan(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        if (qiszero(c->real) && qisunit(c->imag))
                return NULL;
        tmp1 = c_sub(&_conei_, c);
        tmp2 = c_add(&_conei_, c);
        tmp3 = c_div(tmp1, tmp2);
        comfree(tmp1);
        comfree(tmp2);
        tmp1 = c_ln(tmp3, epsilon);
        comfree(tmp3);
        tmp2 = c_scale(tmp1, -1);
        comfree(tmp1);
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        return tmp1;
}


/*
 * c_cot - complex trigonometric cotangent
 *
 * This uses the formula:
 *
 *      cot(x) = cos(x) / sin(x)
 *
 * given:
 *      c           argument to the trigonometric function
 *      epsilon     precision of the trigonometric calculation
 *
 * returns:
 *      != NULL ==> allocated pointer to COMPLEX result
 *      NULL ==> invalid trigonometric argument
 *
 * NOTE: When the trigonometric result is returned as non-NULL result,
 *       the value may be a real value.  The caller may wish to:
 *
 *              COMPLEX *c;     (* return result of this function *)
 *              NUMBER *q;      (* COMPLEX result when c is a real number *)
 *
 *              if (c == NULL) {
 *                      math_error("... some error message");
 *                      not_reached();
 *              }
 *              if (cisreal(c)) {
 *                      q = c_to_q(c, ok_to_free);
 *              }
 */
COMPLEX *
c_cot(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *denom; /* trigonometric identity numerator */
        COMPLEX *numer; /* trigonometric identity denominator */
        COMPLEX *res;   /* trigonometric result */

        /*
         * firewall - check args
         */
        if (c == NULL) {
                return NULL;
        }
        if (check_epsilon(epsilon) == false) {
                return NULL;
        }

        /*
         * evaluate the sin(x) denominator
         *
         * Return NULL if sin(x) failed or we otherwise divide by zero.
         */
        denom = c_sin(c, epsilon);
        if (denom == NULL || ciszero(denom)) {
                return NULL;
        }

        /*
         * evaluate the cos(x) numerator
         *
         * Return NULL if cos(x) failed.
         */
        numer = c_cos(c, epsilon);
        if (numer == NULL) {
                comfree(denom);
                return NULL;
        }

        /*
         * catch the special case of numerator of 0
         */
        if (ciszero(numer)) {
                comfree(denom);
                comfree(numer);
                return clink(&_czero_);
        }

        /*
         * compute the trigonometric function value
         */
        res = c_div(numer, denom);
        comfree(denom);
        comfree(numer);

        /*
         * return the trigonometric result
         */
        return res;
}


COMPLEX *
c_acot(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        if (qiszero(c->real) && qisunit(c->imag))
                return NULL;
        tmp1 = c_add(c, &_conei_);
        tmp2 = c_sub(c, &_conei_);
        tmp3 = c_div(tmp1, tmp2);
        comfree(tmp1);
        comfree(tmp2);
        tmp1 = c_ln(tmp3, epsilon);
        comfree(tmp3);
        tmp2 = c_scale(tmp1, -1);
        comfree(tmp1);
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        return tmp1;
}


/*
 * c_sec - complex trigonometric tangent
 *
 * This uses the formula:
 *
 *      sec(x) = 1 / cos(x)
 *
 * given:
 *      c           argument to the trigonometric function
 *      epsilon     precision of the trigonometric calculation
 *
 * returns:
 *      != NULL ==> allocated pointer to COMPLEX result
 *      NULL ==> invalid trigonometric argument
 *
 * NOTE: When the trigonometric result is returned as non-NULL result,
 *       the value may be a real value.  The caller may wish to:
 *
 *              COMPLEX *c;     (* return result of this function *)
 *              NUMBER *q;      (* COMPLEX result when c is a real number *)
 *
 *              if (c == NULL) {
 *                      math_error("... some error message");
 *                      not_reached();
 *              }
 *              if (cisreal(c)) {
 *                      q = c_to_q(c, ok_to_free);
 *              }
 */
COMPLEX *
c_sec(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *denom; /* trigonometric identity numerator */
        COMPLEX *res;   /* trigonometric result */

        /*
         * firewall - check args
         */
        if (c == NULL) {
                return NULL;
        }
        if (check_epsilon(epsilon) == false) {
                return NULL;
        }

        /*
         * evaluate the cos(x) denominator
         *
         * Return NULL if cos(x) failed or we otherwise divide by zero.
         */
        denom = c_cos(c, epsilon);
        if (denom == NULL || ciszero(denom)) {
                return NULL;
        }

        /*
         * compute the trigonometric function value
         */
        res = c_div(&_cone_, denom);
        comfree(denom);

        /*
         * return the trigonometric result
         */
        return res;
}


COMPLEX *
c_asec(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_inv(c);
        tmp2 = c_acos(tmp1, epsilon);
        comfree(tmp1);
        return tmp2;
}


/*
 * c_sec - complex trigonometric cosecant
 *
 * This uses the formula:
 *
 *      csc(x) = 1 / sin(x)
 *
 * given:
 *      c           argument to the trigonometric function
 *      epsilon     precision of the trigonometric calculation
 *
 * returns:
 *      != NULL ==> allocated pointer to COMPLEX result
 *      NULL ==> invalid trigonometric argument
 *
 * NOTE: When the trigonometric result is returned as non-NULL result,
 *       the value may be a real value.  The caller may wish to:
 *
 *              COMPLEX *c;     (* return result of this function *)
 *              NUMBER *q;      (* COMPLEX result when c is a real number *)
 *
 *              if (c == NULL) {
 *                      math_error("... some error message");
 *                      not_reached();
 *              }
 *              if (cisreal(c)) {
 *                      q = c_to_q(c, ok_to_free);
 *              }
 */
COMPLEX *
c_csc(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *denom; /* trigonometric identity numerator */
        COMPLEX *res;   /* trigonometric result */

        /*
         * firewall - check args
         */
        if (c == NULL) {
                return NULL;
        }
        if (check_epsilon(epsilon) == false) {
                return NULL;
        }

        /*
         * evaluate the sin(x) denominator
         *
         * Return NULL if sin(x) failed or we otherwise divide by zero.
         */
        denom = c_sin(c, epsilon);
        if (denom == NULL || ciszero(denom)) {
                return NULL;
        }

        /*
         * compute the trigonometric function value
         */
        res = c_div(&_cone_, denom);
        comfree(denom);

        /*
         * return the trigonometric result
         */
        return res;
}


COMPLEX *
c_acsc(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_inv(c);
        tmp2 = c_asin(tmp1, epsilon);
        comfree(tmp1);
        return tmp2;
}


COMPLEX *
c_atanh(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        if (qiszero(c->imag) && qisunit(c->real))
                return NULL;
        tmp1 = c_add(&_cone_, c);
        tmp2 = c_sub(&_cone_, c);
        tmp3 = c_div(tmp1, tmp2);
        comfree(tmp1);
        comfree(tmp2);
        tmp1 = c_ln(tmp3, epsilon);
        comfree(tmp3);
        tmp2 = c_scale(tmp1, -1);
        comfree(tmp1);
        return tmp2;
}


COMPLEX *
c_acoth(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;

        if (qiszero(c->imag) && qisunit(c->real))
                return NULL;
        tmp1 = c_add(c, &_cone_);
        tmp2 = c_sub(c, &_cone_);
        tmp3 = c_div(tmp1, tmp2);
        comfree(tmp1);
        comfree(tmp2);
        tmp1 = c_ln(tmp3, epsilon);
        comfree(tmp3);
        tmp2 = c_scale(tmp1, -1);
        comfree(tmp1);
        return tmp2;
}


COMPLEX *
c_asech(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_inv(c);
        tmp2 = c_acosh(tmp1, epsilon);
        comfree(tmp1);
        return tmp2;
}


COMPLEX *
c_acsch(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_inv(c);
        tmp2 = c_asinh(tmp1, epsilon);
        comfree(tmp1);
        return tmp2;
}


COMPLEX *
c_gd(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2, *tmp3;
        NUMBER *q1, *q2;
        NUMBER *sin, *cos;
        NUMBER *eps;
        int n, n1;
        bool neg;

        if (cisreal(c)) {
                q1 = qscale(c->real, -1);
                eps = qscale(epsilon, -1);
                q2 = qtanh(q1, eps);
                qfree(q1);
                q1 = qatan(q2, eps);
                qfree(eps);
                qfree(q2);
                tmp1 = comalloc();
                qfree(tmp1->real);
                tmp1->real = qscale(q1, 1);
                qfree(q1);
                return tmp1;
        }
        if (qiszero(c->real)) {
                n = - qilog2(epsilon);
                qsincos(c->imag, n + 8, &sin, &cos);
                if (qiszero(cos) || (n1 = -qilog2(cos)) > n) {
                        qfree(sin);
                        qfree(cos);
                        return NULL;
                }
                neg = qisneg(sin);
                q1 = neg ? qsub(&_qone_, sin) : qqadd(&_qone_, sin);
                qfree(sin);
                if (n1 > 8) {
                        qfree(q1);
                        qfree(cos);
                        qsincos(c->imag, n + n1, &sin, &cos);
                        q1 = neg ? qsub(&_qone_, sin) : qqadd(&_qone_, sin);
                        qfree(sin);
                }
                q2 = qqdiv(q1, cos);
                qfree(q1);
                q1 = qln(q2, epsilon);
                qfree(q2);
                if (neg) {
                        q2 = qneg(q1);
                        qfree(q1);
                        q1 = q2;
                }
                tmp1 = comalloc();
                qfree(tmp1->imag);
                tmp1->imag = q1;
                if (qisneg(cos)) {
                        qfree(tmp1->real);
                        q1 = qpi(epsilon);
                        if (qisneg(c->imag)) {
                                q2 = qneg(q1);
                                qfree(q1);
                                q1 = q2;
                        }
                        tmp1->real = q1;
                }
                qfree(cos);
                return tmp1;
        }
        neg = qisneg(c->real);
        tmp1 = neg ? c_neg(c) : clink(c);
        tmp2 = c_exp(tmp1, epsilon);
        comfree(tmp1);
        if (tmp2 == NULL)
                return NULL;
        tmp1 = c_mul(&_conei_, tmp2);
        tmp3 = c_add(&_conei_, tmp2);
        comfree(tmp2);
        tmp2 = c_add(tmp1, &_cone_);
        comfree(tmp1);
        if (ciszero(tmp2) || ciszero(tmp3)) {
                comfree(tmp2);
                comfree(tmp3);
                return NULL;
        }
        tmp1 = c_div(tmp2, tmp3);
        comfree(tmp2);
        comfree(tmp3);
        tmp2 = c_ln(tmp1, epsilon);
        comfree(tmp1);
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        if (neg) {
                tmp2 = c_neg(tmp1);
                comfree(tmp1);
                return tmp2;
        }
        return tmp1;
}


COMPLEX *
c_agd(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *tmp1, *tmp2;

        tmp1 = c_mul(&_conei_, c);
        tmp2 = c_gd(tmp1, epsilon);
        comfree(tmp1);
        if (tmp2 == NULL)
                return NULL;
        tmp1 = c_div(tmp2, &_conei_);
        comfree(tmp2);
        return tmp1;
}


/*
 * Convert a number from polar coordinates to normal complex number form
 * within the specified accuracy.  This produces the value:
 *      q1 * cos(q2) + q1 * sin(q2) * i.
 */
COMPLEX *
c_polar(NUMBER *q1, NUMBER *q2, NUMBER *epsilon)
{
        COMPLEX *r;
        NUMBER *tmp, *cos, *sin;
        long m, n;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex polar");
                not_reached();
        }
        if (qiszero(q1))
                return clink(&_czero_);
        m = qilog2(q1) + 1;
        n = qilog2(epsilon);
        if (m < n)
                return clink(&_czero_);
        r = comalloc();
        if (qiszero(q2)) {
                qfree(r->real);
                r->real = qlink(q1);
                return r;
        }
        qsincos(q2, m - n + 2, &sin, &cos);
        tmp = qmul(q1, cos);
        qfree(cos);
        qfree(r->real);
        r->real = qmappr(tmp, epsilon, conf->triground);
        qfree(tmp);
        tmp = qmul(q1, sin);
        qfree(sin);
        qfree(r->imag);
        r->imag = qmappr(tmp, epsilon, conf->triground);
        qfree(tmp);
        return r;
}


/*
 * Raise one complex number to the power of another one to within the
 * specified error.
 */
COMPLEX *
c_power(COMPLEX *c1, COMPLEX *c2, NUMBER *epsilon)
{
        COMPLEX *ctmp1, *ctmp2;
        long k1, k2, k, m1, m2, m, n;
        NUMBER *a2b2, *qtmp1, *qtmp2, *epsilon1;

        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon value for complex power");
                not_reached();
        }
        if (ciszero(c1)) {
                if (cisreal(c2) && qisneg(c2->real)) {
                        math_error ("Non-positive real exponent of zero for complex power");
                        not_reached();
                }
                return clink(&_czero_);
        }
        n = qilog2(epsilon);
        m1 = m2 = -1000000;
        k1 = k2 = 0;
        if (!qiszero(c2->real)) {
                qtmp1 = qsquare(c1->real);
                qtmp2 = qsquare(c1->imag);
                a2b2 = qqadd(qtmp1, qtmp2);
                qfree(qtmp1);
                qfree(qtmp2);
                m1 = qilog2(c2->real);
                epsilon1 = qbitvalue(-m1 - 1);
                qtmp1 = qln(a2b2, epsilon1);
                qfree(epsilon1);
                qfree(a2b2);
                qtmp2 = qmul(qtmp1, c2->real);
                qfree(qtmp1);
                qtmp1 = qmul(qtmp2, &_qlge_);
                qfree(qtmp2);
                k1 = qtoi(qtmp1);
                qfree(qtmp1);
        }
        if (!qiszero(c2->imag)) {
                m2 = qilog2(c2->imag);
                epsilon1 = qbitvalue(-m2 - 1);
                qtmp1 = qatan2(c1->imag, c1->real, epsilon1);
                qfree(epsilon1);
                qtmp2 = qmul(qtmp1, c2->imag);
                qfree(qtmp1);
                qtmp1 = qscale(qtmp2, -1);
                qfree(qtmp2);
                qtmp2 = qmul(qtmp1, &_qlge_);
                qfree(qtmp1);
                k2 = qtoi(qtmp2);
                qfree(qtmp2);
        }
        m = (m2 > m1) ? m2 : m1;
        k = k1 - k2 + 1;
        if (k < n)
                return clink(&_czero_);
        epsilon1 = qbitvalue(n - k - m - 2);
        ctmp1 = c_ln(c1, epsilon1);
        qfree(epsilon1);
        ctmp2 = c_mul(ctmp1, c2);
        comfree(ctmp1);
        ctmp1 = c_exp(ctmp2, epsilon);
        comfree(ctmp2);
        return ctmp1;
}


/*
 * Print a complex number in the current output mode.
 */
void
comprint(COMPLEX *c)
{
        NUMBER qtmp;

        if (conf->outmode == MODE_FRAC) {
                cprintfr(c);
                return;
        }
        if (!qiszero(c->real) || qiszero(c->imag))
                qprintnum(c->real, MODE_DEFAULT, conf->outdigits);
        qtmp = c->imag[0];
        if (qiszero(&qtmp))
                return;
        if (conf->complex_space) {
            math_chr(' ');
        }
        if (!qiszero(c->real) && !qisneg(&qtmp))
                math_chr('+');
        if (qisneg(&qtmp)) {
                math_chr('-');
                qtmp.num.sign = 0;
        }
        if (conf->complex_space) {
            math_chr(' ');
        }
        qprintnum(&qtmp, MODE_DEFAULT, conf->outdigits);
        math_chr('i');
}


/*
 * Print a complex number in rational representation.
 * Example:  2/3-4i/5
 */
void
cprintfr(COMPLEX *c)
{
        NUMBER *r;
        NUMBER *i;

        r = c->real;
        i = c->imag;
        if (!qiszero(r) || qiszero(i))
                qprintfr(r, 0L, false);
        if (qiszero(i))
                return;
        if (!qiszero(r) && !qisneg(i))
                math_chr('+');
        zprintval(i->num, 0L, 0L);
        math_chr('i');
        if (qisfrac(i)) {
                if (conf->fraction_space) {
                    math_chr(' ');
                }
                math_chr('/');
                if (conf->fraction_space) {
                    math_chr(' ');
                }
                zprintval(i->den, 0L, 0L);
        }
}


NUMBER *
c_ilog(COMPLEX *c, ZVALUE base)
{
        NUMBER *qr, *qi;

        qr = qilog(c->real, base);
        qi = qilog(c->imag, base);

        if (qr == NULL) {
                if (qi == NULL)
                        return NULL;
                return qi;
        }
        if (qi == NULL)
                return qr;
        if (qrel(qr, qi) >= 0) {
                qfree(qi);
                return qr;
        }
        qfree(qr);
        return qi;
}


/*
 * c_versin - COMPLEX valued versed trigonometric sine
 *
 * This uses the formula:
 *
 *      versin(x) = 1 - cos(x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_versin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex cos(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_cos(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex cosine for complex versin");
                not_reached();
        }
        r = c_sub(&_cone_, ctmp);

        /*
         * return trigonometric result
         */
        comfree(ctmp);
        return r;
}


/*
 * c_aversin - COMPLEX valued inverse versed trigonometric sine
 *
 * This uses the formula:
 *
 *      aversin(x) = acos(1 - x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_aversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_sub(&_cone_, c);
        r = c_acos(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_coversin - COMPLEX valued coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      coversin(x) = 1 - cos(x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_coversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_sin(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex sine for complex coversin");
                not_reached();
        }
        r = c_sub(&_cone_, ctmp);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_acoversin - COMPLEX valued inverse coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      acoversin(x) = asin(1 - x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_acoversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_sub(&_cone_, c);
        r = c_asin(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_vercos - COMPLEX valued versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      vercos(x) = 1 + cos(x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_vercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex cos(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_cos(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex cosine for complex vercos");
                not_reached();
        }
        r = c_add(&_cone_, ctmp);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_avercos - COMPLEX valued inverse versed trigonometric cosine
 *
 * This uses the formula:
 *
 *      avercos(x) = acos(x - 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_avercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_sub(c, &_cone_);
        r = c_acos(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_covercos - COMPLEX valued coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      covercos(x) = 1 + sin(x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_covercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_sin(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex sine for complex covercos");
                not_reached();
        }
        r = c_add(&_cone_, ctmp);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_acovercos - COMPLEX valued inverse coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      acovercos(x) = asin(x - 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_acovercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_sub(c, &_cone_);
        r = c_asin(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_haversin - COMPLEX valued half versed trigonometric sine
 *
 * This uses the formula:
 *
 *      haversin(x) = versin(x) / 2
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_haversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex cos(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_versin(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex versed sine for complex haversin");
                not_reached();
        }
        r = c_divq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_ahaversin - COMPLEX valued inverse half versed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahaversin(x) = acos(1 - 2*x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_ahaversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */
        COMPLEX *x2;    /* twice x */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        x2 = c_mulq(c, &_qtwo_);
        ctmp = c_sub(&_cone_, x2);
        comfree(x2);
        r = c_acos(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_hacoversin - COMPLEX valued half coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      hacoversin(x) = coversin(x) / 2
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_hacoversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_coversin(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex coversed sine for complex hacoversin");
                not_reached();
        }
        r = c_divq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_ahacoversin - COMPLEX valued inverse half coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahacoversin(x) = asin(1 - 2*x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_ahacoversin(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */
        COMPLEX *x2;    /* twice x */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        x2 = c_mulq(c, &_qtwo_);
        ctmp = c_sub(&_cone_, x2);
        comfree(x2);
        r = c_asin(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_havercos - COMPLEX valued half coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      havercos(x) = vercos(x) / 2
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_havercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_vercos(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex versed cosine for complex havercos");
                not_reached();
        }
        r = c_divq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_ahavercos - COMPLEX valued inverse half coversed trigonometric sine
 *
 * This uses the formula:
 *
 *      ahavercos(x) = acos(2*x - 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_ahavercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */
        COMPLEX *x2;    /* twice x */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        x2 = c_mulq(c, &_qtwo_);
        ctmp = c_sub(&_cone_, x2);
        comfree(x2);
        r = c_acos(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_hacovercos - COMPLEX valued half coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      hacovercos(x) = covercos(x) / 2
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_hacovercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_covercos(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex coversed cosine for complex hacovercos");
                not_reached();
        }
        r = c_divq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_ahacovercos - COMPLEX valued inverse half coversed trigonometric cosine
 *
 * This uses the formula:
 *
 *      ahacovercos(x) = asin(2*x - 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_ahacovercos(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */
        COMPLEX *x2;    /* twice x */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        x2 = c_mulq(c, &_qtwo_);
        ctmp = c_sub(&_cone_, x2);
        comfree(x2);
        r = c_asin(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_exsec - COMPLEX valued exterior trigonometric secant
 *
 * This uses the formula:
 *
 *      exsec(x) = sec(x) - 1
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_exsec(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sec(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_sec(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex cosine for complex exsec");
                not_reached();
        }
        r = c_sub(ctmp, &_cone_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_aexsec - COMPLEX valued inverse exterior trigonometric secant
 *
 * This uses the formula:
 *
 *      aexsec(x) = asec(x + 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_aexsec(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_addq(c, &_qone_);
        r = c_asec(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_excsc - COMPLEX valued exterior trigonometric cosecant
 *
 * This uses the formula:
 *
 *      excsc(x) = csc(x) - 1
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_excsc(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* complex sin(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_csc(c, epsilon);
        if (ctmp == NULL) {
                math_error("Failed to compute complex sine for complex excsc");
                not_reached();
        }
        r = c_sub(ctmp, &_cone_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_aexcsc - COMPLEX valued inverse exterior trigonometric cosecant
 *
 * This uses the formula:
 *
 *      aexcsc(x) = acsc(x + 1)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_aexcsc(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;     /* inverse trig value result */
        COMPLEX *ctmp;  /* argument to inverse trig function */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        ctmp = c_addq(c, &_qone_);
        r = c_acsc(ctmp, epsilon);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_crd - COMPLEX valued trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      crd(x) = 2 * sin(x / 2)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_crd(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *cdiv2;         /* complex c/2 */
        COMPLEX *ctmp;          /* complex sin(c/2) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        cdiv2 = c_divq(c, &_qtwo_);
        ctmp = c_sin(cdiv2, epsilon);
        comfree(cdiv2);
        if (ctmp == NULL) {
                math_error("Failed to compute complex sine for complex crd");
                not_reached();
        }
        r = c_mulq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_acrd - COMPLEX valued inverse trigonometric chord of a unit circle
 *
 * This uses the formula:
 *
 *      acrd(x) = 2 * asin(x / 2)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_acrd(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* inverse trig value result */
        COMPLEX *cdiv2;         /* complex c/2 */
        COMPLEX *ctmp;          /* complex asin(c/2) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex inverse trig function value
         */
        cdiv2 = c_divq(c, &_qtwo_);
        ctmp = c_asin(cdiv2, epsilon);
        comfree(cdiv2);
        r = c_mulq(ctmp, &_qtwo_);
        comfree(ctmp);

        /*
         * return inverse trigonometric result
         */
        return r;
}


/*
 * c_cas - COMPLEX valued cosine plus sine
 *
 * This uses the formula:
 *
 *      cas(x) = cos(x) + sin(x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_cas(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *csin;          /* complex sin(c) */
        COMPLEX *ccos;          /* complex cos(c) */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        csin = c_sin(c, epsilon);
        if (csin == NULL) {
                math_error("Failed to compute complex sine for complex cas");
                not_reached();
        }
        ccos = c_cos(c, epsilon);
        if (ccos == NULL) {
                comfree(csin);
                math_error("Failed to compute complex cosine for complex cas");
                not_reached();
        }
        r = c_add(csin, ccos);
        comfree(csin);
        comfree(ccos);

        /*
         * return trigonometric result
         */
        return r;
}


/*
 * c_cis - COMPLEX valued Euler's formula
 *
 * This uses the formula:
 *
 *      cis(x) = cos(x) + 1i*sin(x)
 *      cis(x) = exp(1i * x)
 *
 * given:
 *      c           complex value to pass to the trig function
 *      epsilon     error tolerance / precision for trig calculation
 *
 * returns:
 *      complex value result of trig function on q with error epsilon
 */
COMPLEX *
c_cis(COMPLEX *c, NUMBER *epsilon)
{
        COMPLEX *r;             /* return COMPLEX value */
        COMPLEX *ctmp;          /* 1i * c */

        /*
         * firewall
         */
        if (c == NULL) {
                math_error("%s: c is NULL", __func__);
                not_reached();
        }
        if (check_epsilon(epsilon) == false) {
                math_error("Invalid epsilon arg for %s", __func__);
                not_reached();
        }

        /*
         * calculate complex trig function value
         */
        ctmp = c_mul(c, &_conei_);
        r = c_exp(ctmp, epsilon);
        comfree(ctmp);
        if (r == NULL) {
                math_error("Failed to compute complex exp for complex cis");
                not_reached();
        }

        /*
         * return trigonometric result
         */
        return r;
}
