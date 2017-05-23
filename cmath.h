/*
 * cmath - data structures for extended precision complex arithmetic
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
 * Under source code control:	1993/07/30 19:42:45
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_CMATH_H)
#define INCLUDE_CMATH_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "qmath.h"
#else
# include <calc/qmath.h>
#endif


/*
 * Complex arithmetic definitions.
 */
typedef struct {
	NUMBER *real;		/* real part of number */
	NUMBER *imag;		/* imaginary part of number */
	long links;		/* link count */
} COMPLEX;


/*
 * Input, output, and conversion routines.
 */
E_FUNC COMPLEX *comalloc(void);
E_FUNC COMPLEX *qqtoc(NUMBER *q1, NUMBER *q2);
E_FUNC void comfree(COMPLEX *c);
E_FUNC void comprint(COMPLEX *c);
E_FUNC void cprintfr(COMPLEX *c);


/*
 * Basic numeric routines.
 */

E_FUNC COMPLEX *c_add(COMPLEX *c1, COMPLEX *c2);
E_FUNC COMPLEX *c_sub(COMPLEX *c1, COMPLEX *c2);
E_FUNC COMPLEX *c_mul(COMPLEX *c1, COMPLEX *c2);
E_FUNC COMPLEX *c_div(COMPLEX *c1, COMPLEX *c2);
E_FUNC COMPLEX *c_addq(COMPLEX *c, NUMBER *q);
E_FUNC COMPLEX *c_subq(COMPLEX *c, NUMBER *q);
E_FUNC COMPLEX *c_mulq(COMPLEX *c, NUMBER *q);
E_FUNC COMPLEX *c_divq(COMPLEX *c, NUMBER *q);
E_FUNC COMPLEX *c_scale(COMPLEX *c, long i);
E_FUNC COMPLEX *c_shift(COMPLEX *c, long i);
E_FUNC COMPLEX *c_square(COMPLEX *c);
E_FUNC COMPLEX *c_conj(COMPLEX *c);
E_FUNC COMPLEX *c_real(COMPLEX *c);
E_FUNC COMPLEX *c_imag(COMPLEX *c);
E_FUNC COMPLEX *c_neg(COMPLEX *c);
E_FUNC COMPLEX *c_inv(COMPLEX *c);
E_FUNC COMPLEX *c_int(COMPLEX *c);
E_FUNC COMPLEX *c_frac(COMPLEX *c);
E_FUNC BOOL c_cmp(COMPLEX *c1, COMPLEX *c2);


/*
 * More complicated functions.
 */
E_FUNC COMPLEX *c_powi(COMPLEX *c, NUMBER *q);
E_FUNC NUMBER *c_ilog(COMPLEX *c, ZVALUE base);


/*
 * Transcendental routines.  These all take an epsilon argument to
 * specify how accurately these are to be calculated.
 */
E_FUNC COMPLEX *c_power(COMPLEX *c1, COMPLEX *c2, NUMBER *epsilon);
E_FUNC COMPLEX *c_sqrt(COMPLEX *c, NUMBER *epsilon, long R);
E_FUNC COMPLEX *c_root(COMPLEX *c, NUMBER *q, NUMBER *epsilon);
E_FUNC COMPLEX *c_exp(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_ln(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_log(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_cos(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_sin(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_cosh(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_sinh(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_polar(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
E_FUNC COMPLEX *c_rel(COMPLEX *c1, COMPLEX *c2);
E_FUNC COMPLEX *c_asin(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acos(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_atan(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acot(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_asec(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acsc(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_asinh(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acosh(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_atanh(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acoth(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_asech(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_acsch(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_gd(COMPLEX *c, NUMBER *epsilon);
E_FUNC COMPLEX *c_agd(COMPLEX *c, NUMBER *epsilon);



/*
 * external functions
 */
E_FUNC COMPLEX *swap_b8_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);
E_FUNC COMPLEX *swap_b16_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);
E_FUNC COMPLEX *swap_HALF_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);


/*
 * macro expansions to speed this thing up
 */
#define cisreal(c)	(qiszero((c)->imag))
#define cisimag(c)	(qiszero((c)->real) && !cisreal(c))
#define ciszero(c)	(cisreal(c) && qiszero((c)->real))
#define cisone(c)	(cisreal(c) && qisone((c)->real))
#define cisnegone(c)	(cisreal(c) && qisnegone((c)->real))
#define cisrunit(c)	(cisreal(c) && qisunit((c)->real))
#define cisiunit(c)	(qiszero((c)->real) && qisunit((c)->imag))
#define cisunit(c)	(cisrunit(c) || cisiunit(c))
#define cistwo(c)	(cisreal(c) && qistwo((c)->real))
#define cisint(c)	(qisint((c)->real) && qisint((c)->imag))
#define ciseven(c)	(qiseven((c)->real) && qiseven((c)->imag))
#define cisodd(c)	(qisodd((c)->real) || qisodd((c)->imag))
#define clink(c)	((c)->links++, (c))


/*
 * Pre-defined values.
 */
EXTERN COMPLEX _czero_, _cone_, _conei_;


#endif /* !INCLUDE_CMATH_H */
