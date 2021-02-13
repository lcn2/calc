/*
 * custom - interface for custom software and hardware interfaces
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll
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
 * Under source code control:	1997/03/03 04:53:08
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
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
 * Keep this file down to a minimum.   Don't put custom builtin function
 * stuff in this file!
 */


#if !defined(CUSTOM_H)
#define CUSTOM_H


/*
 * arg count definitions
 */
#define MAX_CUSTOM_ARGS 100	/* maximum number of custom arguments */


/*
 * register count
 */
#define CUSTOM_REG_MAX 31	/* maximum custom register number */


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
E_FUNC VALUE custom(char*, int, VALUE**);	/* master custom interface */
EXTERN BOOL allow_custom;		/* TRUE => custom builtins allowed */
E_FUNC void showcustom(void);		/* print custom functions */
E_FUNC void customhelp(char *);		/* direct custom help */
EXTERN CONST struct custom cust[];	/* custom interface table */
E_FUNC void init_custreg(void);		/* initialize custom registers */

#endif /* !CUSTOM_H */
