/*
 * opcodes - opcode execution module definition
 *
 * Copyright (C) 1999-2007,2014,2021  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:35
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_OPCODES_H)
#define INCLUDE_OPCODES_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
#else
# include <calc/decl.h>
#endif


/*
 * Opcodes
 */
#define OP_NOP		0L	/* no operation */
#define OP_LOCALADDR	1L	/* load address of local variable */
#define OP_GLOBALADDR	2L	/* load address of global variable */
#define OP_PARAMADDR	3L	/* load address of parameter variable */
#define OP_LOCALVALUE	4L	/* load value of local variable */
#define OP_GLOBALVALUE	5L	/* load value of global variable */
#define OP_PARAMVALUE	6L	/* load value of parameter variable */
#define OP_NUMBER	7L	/* load constant real numeric value */
#define OP_INDEXADDR	8L	/* load array index address */
#define OP_PRINTRESULT	9L	/* print result of top-level expression */
#define OP_ASSIGN	10L	/* assign value to variable */
#define OP_ADD		11L	/* add top two values */
#define OP_SUB		12L	/* subtract top two values */
#define OP_MUL		13L	/* multiply top two values */
#define OP_DIV		14L	/* divide top two values */
#define OP_MOD		15L	/* take mod of top two values */
#define OP_SAVE		16L	/* save value for later use */
#define OP_NEGATE	17L	/* negate top value */
#define OP_INVERT	18L	/* invert top value */
#define OP_INT		19L	/* take integer part of top value */
#define OP_FRAC		20L	/* take fraction part of top value */
#define OP_NUMERATOR	21L	/* take numerator of top value */
#define OP_DENOMINATOR	22L	/* take denominator of top value */
#define OP_DUPLICATE	23L	/* duplicate top value on stack */
#define OP_POP		24L	/* pop top value from stack */
#define OP_RETURN	25L	/* return value of function */
#define OP_JUMPZ	26L	/* jump if top value is zero */
#define OP_JUMPNZ	27L	/* jump if top value is nonzero */
#define OP_JUMP		28L	/* jump unconditionally */
#define OP_USERCALL	29L	/* call a user-defined function */
#define OP_GETVALUE	30L	/* convert address to value */
#define OP_EQ		31L	/* test top two elements for equality */
#define OP_NE		32L	/* test top two elements for inequality */
#define OP_LE		33L	/* test top two elements for <= */
#define OP_GE		34L	/* test top two elements for >= */
#define OP_LT		35L	/* test top two elements for < */
#define OP_GT		36L	/* test top two elements for > */
#define OP_PREINC	37L	/* add one to variable (++x) */
#define OP_PREDEC	38L	/* subtract one from variable (--x) */
#define OP_POSTINC	39L	/* add one to variable (x++) */
#define OP_POSTDEC	40L	/* subtract one from variable (x--) */
#define OP_DEBUG	41L	/* debugging point */
#define OP_PRINT	42L	/* print value */
#define OP_ASSIGNPOP	43L	/* assign to variable and remove it */
#define OP_ZERO		44L	/* put zero on the stack */
#define OP_ONE		45L	/* put one on the stack */
#define OP_PRINTEOL	46L	/* print end of line */
#define OP_PRINTSPACE	47L	/* print a space */
#define OP_PRINTSTRING	48L	/* print constant string */
#define OP_DUPVALUE	49L	/* duplicate value of top value */
#define OP_OLDVALUE	50L	/* old calculation value */
#define OP_QUO		51L	/* integer quotient of top two values */
#define OP_POWER	52L	/* number raised to a power */
#define OP_QUIT		53L	/* quit program */
#define OP_CALL		54L	/* call built-in routine */
#define OP_GETEPSILON	55L	/* get allowed error for calculations */
#define OP_AND		56L	/* arithmetic and */
#define OP_OR		57L	/* arithmetic or */
#define OP_NOT		58L	/* logical not */
#define OP_ABS		59L	/* absolute value */
#define OP_SGN		60L	/* sign of number */
#define OP_ISINT	61L	/* whether top value is integer */
#define OP_CONDORJUMP	62L	/* conditional or jump */
#define OP_CONDANDJUMP	63L	/* conditional and jump */
#define OP_SQUARE	64L	/* square top value */
#define OP_STRING	65L	/* load constant string value */
#define OP_ISNUM	66L	/* whether top value is a number */
#define OP_UNDEF	67L	/* load undefined value on stack */
#define OP_ISNULL	68L	/* whether variable is the null value */
#define OP_ARGVALUE	69L	/* load value of argument (parameter) n */
#define OP_MATCREATE	70L	/* create matrix */
#define OP_ISMAT	71L	/* whether variable is a matrix */
#define OP_ISSTR	72L	/* whether variable is a string */
#define OP_GETCONFIG	73L	/* get value of configuration parameter */
#define OP_LEFTSHIFT	74L	/* left shift of integer */
#define OP_RIGHTSHIFT	75L	/* right shift of integer */
#define OP_CASEJUMP	76L	/* test case and jump if not matched */
#define OP_ISODD	77L	/* whether value is an odd integer */
#define OP_ISEVEN	78L	/* whether value is even integer */
#define OP_FIADDR	79L	/* 'fast index' matrix value address */
#define OP_FIVALUE	80L	/* 'fast index' matrix value */
#define OP_ISREAL	81L	/* test value for real number */
#define OP_IMAGINARY	82L	/* load imaginary numeric constant */
#define OP_RE		83L	/* real part of complex number */
#define OP_IM		84L	/* imaginary part of complex number */
#define OP_CONJUGATE	85L	/* complex conjugate of complex number */
#define OP_OBJCREATE	86L	/* create object */
#define OP_ISOBJ	87L	/* whether value is an object */
#define OP_NORM		88L	/* norm of value (square of abs) */
#define OP_ELEMADDR	89L	/* address of element of object */
#define OP_ELEMVALUE	90L	/* value of element of object */
#define OP_ISTYPE	91L	/* whether two values are the same type */
#define OP_SCALE	92L	/* scale value by a power of two */
#define OP_ISLIST	93L	/* whether value is a list */
#define OP_SWAP		94L	/* swap values of two variables */
#define OP_ISSIMPLE	95L	/* whether value is a simple type */
#define OP_CMP		96L	/* compare values returning -1, 0, or 1 */
#define OP_SETCONFIG	97L	/* set configuration parameter */
#define OP_SETEPSILON	98L	/* set allowed error for calculations */
#define OP_ISFILE	99L	/* whether value is a file */
#define OP_ISASSOC	100L	/* whether value is an association */
#define OP_INITSTATIC	101L	/* once only code for static initialization */
#define OP_ELEMINIT	102L	/* assign element of matrix or object */
#define OP_ISCONFIG	103L	/* whether value is a configuration state */
#define OP_ISHASH	104L	/* whether value is a hash state */
#define OP_ISRAND	105L	/* whether value is additive 55 random state */
#define OP_ISRANDOM	106L	/* whether value is a Blum random state */
#define OP_SHOW		107L	/* show data about current state */
#define OP_INITFILL	108L	/* fill new matrix with copies of a value */
#define OP_ASSIGNBACK	109L	/* assign in reverse order */
#define OP_TEST		110L	/* test whether value is "nonzero" */
#define OP_ISDEFINED	111L	/* whether string names a function */
#define OP_ISOBJTYPE	112L	/* whether string names an object type */
#define OP_ISBLK	113L	/* whether value is a block */
#define OP_PTR		114L	/* octet pointer */
#define OP_DEREF	115L	/* dereference an octet pointer */
#define OP_ISOCTET	116L	/* whether value is an octet */
#define OP_ISPTR	117L	/* whether value is a pointer */
#define OP_SAVEVAL	118L	/* activate updating */
#define OP_LINKS	119L	/* return links for numbers and strings */
#define OP_BIT		120L	/* whether specified bit is set */
#define OP_COMP		121L	/* complement value */
#define OP_XOR		122L	/* xor (~) of values */
#define OP_HIGHBIT	123L	/* index of high bit of value */
#define OP_LOWBIT	124L	/* index of low bit of value */
#define OP_CONTENT	125L	/* value returned by unary # */
#define OP_HASHOP	126L	/* binary # */
#define OP_BACKSLASH	127L	/* unary backslash */
#define OP_SETMINUS	128L	/* binary backslash */
#define OP_PLUS		129L	/* unary + */
#define OP_JUMPNN	130L	/* jump if top value is non-null */
#define OP_ABORT	131L	/* abort operation */
#define MAX_OPCODE	131L	/* highest legal opcode */


/*
 * external declarations
 */
EXTERN char *funcname;		/* function being executed */
EXTERN long funcline;		/* function line being executed */


#endif /* !INCLUDE_OPCODES_H */
