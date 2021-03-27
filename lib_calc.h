/*
 * lib_calc - calc link library initialization and shutdown routines
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll
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
 * Under source code control:	1997/03/23 18:37:10
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_MATH_ERROR_H)
#define INCLUDE_MATH_ERROR_H

#include <setjmp.h>

#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
#else
# include <calc/decl.h>
#endif

/*
 * error buffer definitions
 */
#define MAXERROR	512	/* maximum length of error message string */

/*
 * calc math error control
 */
/* non-zero => use calc_use_matherr_jmpbuf */
EXTERN int calc_use_matherr_jmpbuf;
/* math_error() control jump point when calc_use_matherr_jmpbuf != 0 */
EXTERN jmp_buf calc_matherr_jmpbuf;

/*
 * calc parse/scan error control
 */
/* non-zero => calc_scanerr_jmpbuf is ready */
EXTERN int calc_use_scanerr_jmpbuf;
/* scanerror() control jump point when calc_use_scanerr_jmpbuf != 0 */
EXTERN jmp_buf calc_scanerr_jmpbuf;

/*
 * last calc math error, parse/scan error message
 */
EXTERN char calc_err_msg[MAXERROR+1];
/* 0 ==> do not print parse/scan errors */
EXTERN int calc_print_scanerr_msg;

/*
 * calc parse/scan warning control
 */
/* last parse/scan warning message */
EXTERN char calc_warn_msg[MAXERROR+1];
/* 0 ==> do not print parse/scan warnings */
EXTERN int calc_print_scanwarn_msg;
/* number of parse/scan warnings found */
EXTERN unsigned long calc_warn_cnt;

/*
 * calc history file
 */
EXTERN char *calc_history;

/*
 * calc help
 */
EXTERN char *calc_helpdir;
EXTERN char *calc_customhelpdir;

#endif /* !INCLUDE_MATH_ERROR_H */
