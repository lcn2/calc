/*
 * cmath - data structures for extended precision complex arithmetic
 *
 * Copyright (C) 1999-2007,2014,2023,2026  David I. Bell and Landon Curt Noll
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
 * Under source code control:   1993/07/30 19:42:45
 * File existed as early as:    1993
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_CMATH_H)
#  define INCLUDE_CMATH_H

/*
 * Complex arithmetic definitions.
 */
typedef struct {
    NUMBER *real; /* real part of number */
    NUMBER *imag; /* imaginary part of number */
    long links;   /* link count */
} COMPLEX;

/*
 * Input, output, and conversion routines.
 */
extern COMPLEX *cmappr(COMPLEX *c, NUMBER *e, long rnd, bool cfree);
extern COMPLEX *comalloc(void);
extern COMPLEX *qqtoc(NUMBER *q1, NUMBER *q2);
extern void comfree(COMPLEX *c);
extern void comprint(COMPLEX *c);
extern void cprintfr(COMPLEX *c);

/*
 * Basic numeric routines.
 */

extern COMPLEX *c_add(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *c_sub(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *c_mul(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *c_div(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *c_addq(COMPLEX *c, NUMBER *q);
extern COMPLEX *c_subq(COMPLEX *c, NUMBER *q);
extern COMPLEX *c_mulq(COMPLEX *c, NUMBER *q);
extern COMPLEX *c_divq(COMPLEX *c, NUMBER *q);
extern COMPLEX *c_scale(COMPLEX *c, long i);
extern COMPLEX *c_shift(COMPLEX *c, long i);
extern COMPLEX *c_square(COMPLEX *c);
extern COMPLEX *c_conj(COMPLEX *c);
extern COMPLEX *c_real(COMPLEX *c);
extern NUMBER *c_to_q(COMPLEX *c, bool cfree);
extern COMPLEX *q_to_c(NUMBER *q);
extern COMPLEX *c_imag(COMPLEX *c);
extern COMPLEX *c_neg(COMPLEX *c);
extern COMPLEX *c_inv(COMPLEX *c);
extern COMPLEX *c_int(COMPLEX *c);
extern COMPLEX *c_frac(COMPLEX *c);
extern bool c_cmp(COMPLEX *c1, COMPLEX *c2);

/*
 * More complicated functions.
 */
extern COMPLEX *c_powi(COMPLEX *c, NUMBER *q);
extern NUMBER *c_ilog(COMPLEX *c, ZVALUE base);

/*
 * Transcendental routines.  These all take an epsilon argument to
 * specify how accurately these are to be calculated.
 */
extern COMPLEX *c_power(COMPLEX *c1, COMPLEX *c2, NUMBER *epsilon);
extern COMPLEX *c_sqrt(COMPLEX *c, NUMBER *epsilon, long R);
extern COMPLEX *c_root(COMPLEX *c, NUMBER *q, NUMBER *epsilon);
extern COMPLEX *c_exp(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_ln(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_log(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_log2(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_cos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_sin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_cosh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_sinh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_polar(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern COMPLEX *c_rel(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *c_asin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_tan(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_atan(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_cot(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acot(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_sec(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_asec(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_csc(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acsc(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_asinh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acosh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_atanh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acoth(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_asech(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acsch(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_gd(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_agd(COMPLEX *c, NUMBER *epsilon);

/*
 * historical trig functions
 */
extern COMPLEX *c_versin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_aversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_coversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acoversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_vercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_avercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_covercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acovercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_haversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_ahaversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_hacoversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_ahacoversin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_havercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_ahavercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_hacovercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_ahacovercos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_exsec(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_aexsec(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_excsc(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_aexcsc(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_crd(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_acrd(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_cas(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *c_cis(COMPLEX *c, NUMBER *epsilon);

/*
 * external functions
 */
extern COMPLEX *swap_b8_in_COMPLEX(COMPLEX *dest, COMPLEX *src, bool all);
extern COMPLEX *swap_b16_in_COMPLEX(COMPLEX *dest, COMPLEX *src, bool all);
extern COMPLEX *swap_HALF_in_COMPLEX(COMPLEX *dest, COMPLEX *src, bool all);

/*
 * macro expansions to speed this thing up
 */
#  define cisreal(c) (qiszero((c)->imag))
#  define cisimag(c) (qiszero((c)->real) && !cisreal(c))
#  define ciszero(c) (cisreal(c) && qiszero((c)->real))
#  define cisone(c) (cisreal(c) && qisone((c)->real))
#  define cisnegone(c) (cisreal(c) && qisnegone((c)->real))
#  define cisrunit(c) (cisreal(c) && qisunit((c)->real))
#  define cisiunit(c) (qiszero((c)->real) && qisunit((c)->imag))
#  define cisunit(c) (cisrunit(c) || cisiunit(c))
#  define cistwo(c) (cisreal(c) && qistwo((c)->real))
#  define cisint(c) (qisint((c)->real) && qisint((c)->imag))
#  define ciseven(c) (qiseven((c)->real) && qiseven((c)->imag))
#  define cisodd(c) (qisodd((c)->real) || qisodd((c)->imag))
#  define clink(c) ((c)->links++, (c))

/*
 * Pre-defined values.
 */
extern COMPLEX _czero_, _cone_, _conei_;

#endif
