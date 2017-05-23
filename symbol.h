/*
 * symbol - global and local symbol routines
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
 * Under source code control:	1990/02/15 01:48:37
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_SYMBOL_H)
#define INCLUDE_SYMBOL_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "zmath.h"
#else
# include <calc/zmath.h>
#endif


/*
 * Symbol Declarations.
 */
#define SYM_UNDEFINED	0	/* undefined symbol */
#define SYM_PARAM	1	/* parameter symbol */
#define SYM_LOCAL	2	/* local symbol */
#define SYM_GLOBAL	3	/* global symbol */
#define SYM_STATIC	4	/* static symbol */

#define SCOPE_GLOBAL	0	/* file scope level for global variables */
#define SCOPE_STATIC	1	/* lowest file scope for static variables */


typedef struct global GLOBAL;
struct global {
	size_t g_len;		/* length of symbol name */
	short g_filescope;	/* file scope level of symbol (0 if global) */
	short g_funcscope;	/* function scope level of symbol */
	char *g_name;		/* global symbol name */
	VALUE g_value;		/* global symbol value */
	GLOBAL *g_next;		/* next symbol in hash chain */
};


/*
 * Routines to search for global symbols.
 */
E_FUNC GLOBAL *addglobal(char *name, BOOL isstatic);
E_FUNC GLOBAL *findglobal(char *name);


/*
 * Routines to return names of variables.
 */
E_FUNC char *localname(long n);
E_FUNC char *paramname(long n);
E_FUNC char *globalname(GLOBAL *sp);


/*
 * Routines to handle entering and leaving of scope levels.
 */
E_FUNC void resetscopes(void);
E_FUNC void enterfilescope(void);
E_FUNC void exitfilescope(void);
E_FUNC void enterfuncscope(void);
E_FUNC void exitfuncscope(void);
E_FUNC void endscope (char *name, BOOL isglobal);


/*
 * Other routines.
 */
E_FUNC long addlocal(char *name);
E_FUNC long findlocal(char *name);
E_FUNC long addparam(char *name);
E_FUNC long findparam(char *name);
E_FUNC void initlocals(void);
E_FUNC void initglobals(void);
E_FUNC int writeglobals(char *name);
E_FUNC int symboltype(char *name);
E_FUNC void showglobals(void);
E_FUNC void showallglobals(void);
E_FUNC void freeglobals(void);
E_FUNC void showstatics(void);
E_FUNC void freestatics(void);


#endif /* !INCLUDE_SYMBOL_H */
