/*
 * have_varvs - try <varargs.h> to see if it really works with vsprintf()
 *
 * Some systems have bugs in the <varargs.h> implementation that show up in
 * vsprintf(), so we may have to try to use sprintf() as if it were vsprintf()
 * and hope for the best.
 *
 * This program will output #defines and exits 0 if vsprintf() (or sprintf())
 * produces the results that we expect.	 This program exits 1 if vsprintf()
 * (or sprintf()) produces unexpected results while using the <stdarg.h>
 * include file.
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

#include <stdio.h>

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif

char buf[BUFSIZ];

#if !defined(STDARG) && !defined(SIMULATE_STDARG)
#include <varargs.h>

void
try_this(char *fmt, ...)
{
    va_list ap;

    va_start(ap);

#if !defined(DONT_HAVE_VSPRINTF)
    vsprintf(buf, fmt, ap);
#else
    sprintf(buf, fmt, ap);
#endif

    va_end(ap);
}

#else

void
try_this(char *a, int b, char *c, int d)
{
    return;
}

#endif


int
main(void)
{
	/*
	 * setup
	 */
	buf[0] = '\0';

	/*
	 * test variable args and vsprintf/sprintf
	 */
	try_this("@%d:%s:%d@", 1, "hi", 2);
	if (strcmp(buf, "@1:hi:2@") != 0) {
#if !defined(DONT_HAVE_VSPRINTF)
	    /* <varargs.h> with vsprintf() didn't work */
#else
	    /* <varargs.h> with sprintf() simulating vsprintf() didn't work */
#endif
	    exit(1);
	}
	try_this("%s %d%s%d%d %s",
	    "Landon Noll 1st proved that", 2, "^", 23209, -1, "was prime");
	if (strcmp(buf,
		   "Landon Noll 1st proved that 2^23209-1 was prime") != 0) {
#if !defined(DONT_HAVE_VSPRINTF)
	    /* <stdarg.h> with vsprintf() didn't work */
#else
	    /* <stdarg.h> with sprintf() simulating vsprintf() didn't work */
#endif
	    exit(1);
	}

	/*
	 * report the result
	 */
	puts("/* what type of variable args do we have? */");
	puts("#define VARARGS /* use <varargs.h> */");
	puts("#include <varargs.h>");
	puts("\n/* should we use vsprintf()? */");
#if !defined(DONT_HAVE_VSPRINTF)
	puts("#define HAVE_VS /* yes */");
#else
	puts("/*");
	puts(" * Hack aleart!!!");
	puts(" *");
	puts(" * Systems that do not have vsprintf() need something.  In some");
	puts(" * cases the sprintf function will deal correctly with the");
	puts(" * va_alist 3rd arg.  Hope for the best!");
	puts(" */");
	puts("#define vsprintf sprintf");
	puts("#undef HAVE_VS");
#endif
	/* exit(0); */
	return 0;
}
