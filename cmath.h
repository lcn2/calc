/*
 * cmath - data structures for extended precision complex arithmetic
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
 * @(#) $Revision: 29.6 $
 * @(#) $Id: cmath.h,v 29.6 2002/03/12 09:38:26 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/cmath.h,v $
 *
 * Under source code control:	1993/07/30 19:42:45
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__CMATH_H__)
#define __CMATH_H__


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
extern COMPLEX *comalloc(void);
extern COMPLEX *qqtoc(NUMBER *q1, NUMBER *q2);
extern void comfree(COMPLEX *c);
extern void comprint(COMPLEX *c);
extern void cprintfr(COMPLEX *c);


/*
 * Basic numeric routines.
 */

extern COMPLEX *cadd(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *csub(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *cmul(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *cdiv(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *caddq(COMPLEX *c, NUMBER *q);
extern COMPLEX *csubq(COMPLEX *c, NUMBER *q);
extern COMPLEX *cmulq(COMPLEX *c, NUMBER *q);
extern COMPLEX *cdivq(COMPLEX *c, NUMBER *q);
extern COMPLEX *cscale(COMPLEX *c, long i);
extern COMPLEX *cshift(COMPLEX *c, long i);
extern COMPLEX *csquare(COMPLEX *c);
extern COMPLEX *cconj(COMPLEX *c);
extern COMPLEX *c_real(COMPLEX *c);
extern COMPLEX *c_imag(COMPLEX *c);
extern COMPLEX *cneg(COMPLEX *c);
extern COMPLEX *cinv(COMPLEX *c);
extern COMPLEX *cint(COMPLEX *c);
extern COMPLEX *cfrac(COMPLEX *c);
extern BOOL ccmp(COMPLEX *c1, COMPLEX *c2);


/*
 * More complicated functions.
 */
extern COMPLEX *cpowi(COMPLEX *c, NUMBER *q);
extern NUMBER *cilog(COMPLEX *c, ZVALUE base);


/*
 * Transcendental routines.  These all take an epsilon argument to
 * specify how accurately these are to be calculated.
 */
extern COMPLEX *cpower(COMPLEX *c1, COMPLEX *c2, NUMBER *epsilon);
extern COMPLEX *csqrt(COMPLEX *c, NUMBER *epsilon, long R);
extern COMPLEX *croot(COMPLEX *c, NUMBER *q, NUMBER *epsilon);
extern COMPLEX *cexp(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cln(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *ccos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *csin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *ccosh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *csinh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cpolar(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern COMPLEX *crel(COMPLEX *c1, COMPLEX *c2);
extern COMPLEX *casin(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacos(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *catan(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacot(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *casec(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacsc(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *casinh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacosh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *catanh(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacoth(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *casech(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cacsch(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cgd(COMPLEX *c, NUMBER *epsilon);
extern COMPLEX *cagd(COMPLEX *c, NUMBER *epsilon);



/*
 * external functions
 */
extern COMPLEX *swap_b8_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);
extern COMPLEX *swap_b16_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);
extern COMPLEX *swap_HALF_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all);


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
extern COMPLEX _czero_, _cone_, _conei_;


#endif /* !__CMATH_H__ */
