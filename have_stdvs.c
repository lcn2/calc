/*
 * have_stdvs - try <stdarg.h> to see if it really works with vsprintf()
 *
 * On some systems that have both <stdarg.h> and <varargs.h>, vsprintf()
 * does not work well under one type of include file.
 *
 * Some systems (such as UMIPS) have bugs in the <stdarg.h> implementation
 * that show up in vsprintf(), so we may have to try to use sprintf()
 * as if it were vsprintf() and hope for the best.
 *
 * This program will output #defines and exits 0 if vsprintf() (or sprintf())
 * produces the results that we expect.  This program exits 1 if vsprintf()
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
#include <stdarg.h>

#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif

char buf[BUFSIZ];


void
try(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
#if !defined(DONT_HAVE_VSPRINTF)
    vsprintf(buf, fmt, ap);
#else
    sprintf(buf, fmt, ap);
#endif
    va_end(ap);
}


MAIN
main(void)
{
    /*
     * setup
     */
    buf[0] = '\0';

    /*
     * test variable args and vsprintf/sprintf
     */
    try("@%d:%s:%d@", 1, "hi", 2);
    if (strcmp(buf, "@1:hi:2@") != 0) {
#if !defined(DONT_HAVE_VSPRINTF)
	/* <stdarg.h> with vsprintf() didn't work */
#else
	/* <stdarg.h> with sprintf() simulating vsprintf() didn't work */
#endif
	exit(1);
    }
    try("%s %d%s%d%d %s",
	"Landon Noll 1st proved that", 2, "^", 23209, -1, "was prime");
    if (strcmp(buf, "Landon Noll 1st proved that 2^23209-1 was prime") != 0) {
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
#if defined(DONT_HAVE_VSPRINTF)
    puts("/*");
    puts(" * SIMULATE_STDARG");
    puts(" *");
    puts(" * WARNING: This type of stdarg makes assumptions about the stack");
    puts(" * 	    that may not be true on your system.  You may want to");
    puts(" *	    define STDARG (if using ANSI C) or VARARGS.");
    puts(" */");
    puts("typedef char *va_list;");
    puts("#define va_start(ap,parmn) (void)((ap) = (char*)(&(parmn) + 1))");
    puts("#define va_end(ap) (void)((ap) = 0)");
    puts("#define va_arg(ap, type) \\");
    puts("        (((type*)((ap) = ((ap) + sizeof(type))))[-1])");
    puts("#define SIMULATE_STDARG /* use std_arg.h to simulate <stdarg.h> */");
#else
    puts("#define STDARG /* use <stdarg.h> */");
    puts("#include <stdarg.h>");
#endif
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
    exit(0);
}
