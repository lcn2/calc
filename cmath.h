/*
 * Copyright (c) 1993 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Data structure declarations for extended precision complex arithmetic.
 */

#ifndef	CMATH_H
#define	CMATH_H

#include "qmath.h"


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
extern COMPLEX *creal(COMPLEX *c);
extern COMPLEX *cimag(COMPLEX *c);
extern COMPLEX *cneg(COMPLEX *c);
extern COMPLEX *cinv(COMPLEX *c);
extern COMPLEX *cint(COMPLEX *c);
extern COMPLEX *cfrac(COMPLEX *c);
extern BOOL ccmp(COMPLEX *c1, COMPLEX *c2);


/*
 * More complicated functions.
 */
extern COMPLEX *cpowi(COMPLEX *c, NUMBER *q);


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
extern COMPLEX *cpolar(NUMBER *q1, NUMBER *q2, NUMBER *epsilon);
extern COMPLEX *crel(COMPLEX *c1, COMPLEX *c2);


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
#define	cisunit(c)	(cisrunit(c) || cisiunit(c))
#define cistwo(c)	(cisreal(c) && qistwo((c)->real))
#define cisint(c)	(qisint((c)->real) && qisint((c)->imag))
#define ciseven(c)	(qiseven((c)->real) && qiseven((c)->imag))
#define cisodd(c)	(qisodd((c)->real) || qisodd((c)->imag))
#define clink(c)	((c)->links++, (c))


/*
 * Pre-defined values.
 */
extern COMPLEX _czero_, _cone_, _conei_;

#endif

/* END CODE */
