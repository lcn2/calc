/*
 * math_error - a simple libcalc math error routine
 *
 * Copyright (C) 1999,2021  Landon Curt Noll
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
 * If one sets up calc_matherr_jmpbuf, and then sets calc_use_matherr_jmpbuf
 * to non-zero then this routine will longjmp back with the return value of
 * calc_use_matherr_jmpbuf.  In addition, the last calc error message will be
 * found in calc_use_matherr_jmpbuf_msg.  This error is not printed to stderr.
 *
 * For example:
 *
 *	#include <setjmp.h>
 *	#include "lib_calc.h"
 *
 *	int error;
 *
 *	...
 *
 *	if ((error = setjmp(calc_matherr_jmpbuf)) != 0) {
 *
 *		(* reinitialize calc after a longjmp *)
 *		reinitialize();
 *
 *		(* report the error *)
 *		printf("Ouch: %s\n", calc_err_msg);
 *	}
 *	calc_use_matherr_jmpbuf = 1;
 */


#include <stdio.h>
#include <setjmp.h>
#include "args.h"
#include "calc.h"
#include "lib_calc.h"


#include "banned.h"	/* include after system header <> includes */


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
	vsnprintf(calc_err_msg, MAXERROR, fmt, ap);
	va_end(ap);
	calc_err_msg[MAXERROR] = '\0';	/* paranoia */

	/*
	 * if we should longjmp, so do
	 */
	if (calc_use_matherr_jmpbuf != 0) {
		if (conf->calc_debug & CALCDBG_RUNSTATE)
			printf("math_error: longjmp calc_matherr_jmpbuf\n");
		longjmp(calc_matherr_jmpbuf, calc_use_matherr_jmpbuf);
	}

	/*
	 * print error message and edit
	 */
	(void) fflush(stdout);
	(void) fflush(stderr);
	fprintf(stderr, "%s\n\n", calc_err_msg);

	/*
	 * if interactive, return to main via longjmp()
	 */
	if (calc_use_scanerr_jmpbuf != 0) {
		if (conf->calc_debug & CALCDBG_RUNSTATE)
			printf("math_error: longjmp calc_scanerr_jmpbuf\n");
		longjmp(calc_scanerr_jmpbuf, calc_use_scanerr_jmpbuf);
	}

	/*
	 * exit
	 */
	if (conf->calc_debug & CALCDBG_RUNSTATE)
		printf("math_error: about to exit\n");
	libcalc_call_me_last();
	exit(40);
}
