/*
 * math_error - a simple libcalc math error routine
 */
/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
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
 * chongo was here	/\../\
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
 *		printf("Ouch: %s\n", calc_error);
 *	}
 *	calc_jmp = 1;
 */

#include <stdio.h>
#include <setjmp.h>
#include "args.h"
#include "calc.h"

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
	exit(1);
}
