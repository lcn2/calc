/*
 * math_error - a simple libcalc math error routine
 *
 * Copyright (C) 1999  Landon Curt Noll
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
 * @(#) $Revision: 29.2 $
 * @(#) $Id: math_error.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/math_error.c,v $
 *
 * Under source code control:	1994/08/03 05:08:22
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Your program MUST provide a function called math_error.  This is called
 * by the math routines on an error condition, such as malloc failures or a
 * division by zero.  The routine is called in the manner of printf, with a
 * format string and optional arguments.
 *
 * By default, this routine simply prints a message to stderr and then exits.
 *
 * If one sets up calc_jmp_buf, and then sets calc_jmp to non-zero then
 * this routine will longjmp back (with the value of calc_jmp) instead.
 * In addition, the last calc error message will be found in calc_error;
 * this error is not printed to sttderr.
 *
 * For example:
 *
 *	#include <setjmp.h>
 *
 *	extern jmp_buf calc_jmp_buf;
 *	extern int calc_jmp;
 *	extern char *calc_error;
 *	int error;
 *
 *	...
 *
 *	if ((error = setjmp(calc_jmp_buf)) != 0) {
 *
 *		(* reinitialize calc after a longjmp *)
 *		reinitialize();
 *
 *		(* report the error *)
 *		printf("Ouch: %s\n", calc_error);
 *	}
 *	calc_jmp = 1;
 */


#include <stdio.h>
#include <setjmp.h>
#include "args.h"
#include "calc.h"
#include "math_error.h"


/*
 * error jump point we will longjmp to this jmp_buf if calc_jmp is non-zero
 */
jmp_buf calc_jmp_buf;
int calc_jmp = 0;		/* non-zero => use calc_jmp_buf */
char calc_error[MAXERROR+1];	/* last calc error message */


/*
 * math_error - print a math error and exit
 */
void
math_error(char *fmt, ...)
{
	va_list ap;

	/*
	 * format the error
	 */
#ifdef VARARGS
	va_start(ap);
#else
	va_start(ap, fmt);
#endif
	vsprintf(calc_error, fmt, ap);
	va_end(ap);

	/*
	 * if we should longjmp, so do
	 */
	if (calc_jmp != 0) {
		longjmp(calc_jmp_buf, calc_jmp);
	}

	/*
	 * print error message and edit
	 */
	(void) fflush(stdout);
	(void) fflush(stderr);
	fprintf(stderr, "%s\n", calc_error);
	fputc('\n', stderr);
	libcalc_call_me_last();
	exit(1);
}
