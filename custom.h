/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Prior to calc 2.9.3t9, these routines existed as a calc library called
 * cryrand.cal.  They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
 *
 * Comments, suggestions, bug fixes and questions about these routines
 * are welcome.  Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *			Landon Curt Noll
 *
 *			chongo@toad.com
 *			...!{pyramid,sun,uunet}!hoptoad!chongo
 *
 * chongo was here	/\../\
 */


/*
 * Be careful what you put in this file, upper .c files include
 * this file even when CUSTOM is not defined (ALLOW_CUSTOM is empty).
 *
 * Don't include anything, let the including .c file bring in:
 *
 *	have_const.h
 *	value.h
 *
 * before they include this file.
 *
 * Keep this file down to a minimum.   Don't put custom builtin funcion
 * stuff in this file!
 */


#if !defined(CUSTOM_H)
#define CUSTOM_H


/*
 * arg count definitons
 */
#define MAX_CUSTOM_ARGS 100	/* maximum number of custom arguments */


/*
 * custom function interface
 */
struct custom {
	char *name;		/* name of the custom builtin */
	char *desc;		/* very brief description of custom builtin */
	short minargs;		/* minimum number of arguments */
	short maxargs;		/* maximum number of arguments */
	VALUE (*func)(char *name, int argc, VALUE **argv);  /* custom func */
};


/*
 * external declarations
 *
 * These are the required interfaces.  The dummy.c stubs these interfaces too.
 */
extern VALUE custom(char*, int, VALUE**);	/* master custom interface */
extern int allow_custom;		/* TRUE => custom builtins allowed */
extern void showcustom(void);		/* print custom functions */
extern void customhelp(char *);		/* direct custom help */
extern CONST struct custom cust[];	/* custom interface table */

#endif /* !CUSTOM_H */
