/*
 * symbol - global and local symbol routines
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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: symbol.h,v 29.4 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/symbol.h,v $
 *
 * Under source code control:	1990/02/15 01:48:37
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__SYMBOL_H__)
#define __SYMBOL_H__


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
	int g_len;		/* length of symbol name */
	short g_filescope;	/* file scope level of symbol (0 if global) */
	short g_funcscope;	/* function scope level of symbol */
	char *g_name;		/* global symbol name */
	VALUE g_value;		/* global symbol value */
	GLOBAL *g_next;		/* next symbol in hash chain */
};


/*
 * Routines to search for global symbols.
 */
extern GLOBAL *addglobal(char *name, BOOL isstatic);
extern GLOBAL *findglobal(char *name);


/*
 * Routines to return names of variables.
 */
extern char *localname(long n);
extern char *paramname(long n);
extern char *globalname(GLOBAL *sp);


/*
 * Routines to handle entering and leaving of scope levels.
 */
extern void resetscopes(void);
extern void enterfilescope(void);
extern void exitfilescope(void);
extern void enterfuncscope(void);
extern void exitfuncscope(void);
extern void endscope (char *name, BOOL isglobal);


/*
 * Other routines.
 */
extern long addlocal(char *name);
extern long findlocal(char *name);
extern long addparam(char *name);
extern long findparam(char *name);
extern void initlocals(void);
extern void initglobals(void);
extern int writeglobals(char *name);
extern int symboltype(char *name);
extern void showglobals(void);
extern void showallglobals(void);
extern void freeglobals(void);
extern void showstatics(void);
extern void freestatics(void);


#endif /* !__SYMBOL_H__ */
