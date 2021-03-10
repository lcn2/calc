/*
 * have_stdvs - try <stdarg.h> to see if it really works with vsnprintf()
 *
 * Copyright (C) 1999,2014,2018,2021  Landon Curt Noll
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
 * Under source code control:	1995/09/09 22:41:10
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * On some systems that have both <stdarg.h> and <varargs.h>, vsnprintf()
 * does not work well under one type of include file.
 *
 * Some systems (such as UMIPS) have bugs in the <stdarg.h> implementation
 * that show up in vsnprintf(), so we may have to try to use snprintf()
 * as if it were vsnprintf() and hope for the best.
 *
 * This program will output #defines and exits 0 if vsnprintf() (or snprintf())
 * produces the results that we expect.	 This program exits 1 if vsnprintf()
 * (or snprintf()) produces unexpected results while using the <stdarg.h>
 * include file.
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

#include "have_stdlib.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#undef VSNPRINTF_SIZE_T
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || \
    defined(__cplusplus)
# define VSNPRINTF_SIZE_T size_t
#else
# define VSNPRINTF_SIZE_T long
#endif


#include "banned.h"	/* include after system header <> includes */


char buf[BUFSIZ+1];


void
try_nthis(char *fmt, VSNPRINTF_SIZE_T size, ...)
{
	va_list ap;

	va_start(ap, size);
#if !defined(DONT_HAVE_VSNPRINTF)
	vsnprintf(buf, size, fmt, ap);
#else
	snprintf(buf, size, fmt, ap);
#endif
	va_end(ap);
}


int
main(void)
{
	/*
	 * setup
	 */
	buf[0] = '\0';

	/*
	 * test variable args and vsnprintf/snprintf
	 */
	try_nthis("@%d:%s:%d@", sizeof(buf)-1, 1, "hello", 5);
	if (strcmp(buf, "@1:hello:5@") != 0) {
#if !defined(DONT_HAVE_VSNPRINTF)
	    /* <stdarg.h> with vsnprintf() didn't work */
#else
	    /* <stdarg.h> with snprintf() simulating vsnprintf() didn't work */
#endif
	    exit(1);
	}
	try_nthis("%s %d%s%d%d %s", sizeof(buf)-1,
	    "Landon Noll 1st proved that", 2, "^", 23209, -1, "was prime");
	if (strcmp(buf,
		   "Landon Noll 1st proved that 2^23209-1 was prime") != 0) {
#if !defined(DONT_HAVE_VSNPRINTF)
	    /* <stdarg.h> with vsnprintf() didn't work */
#else
	    /* <stdarg.h> with snprintf() simulating vsnprintf() didn't work */
#endif
	    exit(1);
	}

	/*
	 * report the result
	 */
	puts("/* what type of variable args do we have? */");
#if defined(DONT_HAVE_VSNPRINTF)
	puts("/*");
	puts(" * SIMULATE_STDARG");
	puts(" *");
	puts(" * WARNING: This type of stdarg makes assumptions "
	     "about the stack");
	puts(" *	    that may not be true on your system.  "
	     "You may want to");
	puts(" *	    define STDARG (if using ANSI C) or VARARGS.");
	puts(" */");
	puts("typedef char *va_list;");
	puts("#define va_start(ap,parmn) (void)((ap) = (char*)(&(parmn) + 1))");
	puts("#define va_end(ap) (void)((ap) = 0)");
	puts("#define va_arg(ap, type) \\");
	puts("	      (((type*)((ap) = ((ap) + sizeof(type))))[-1])");
	puts("#define SIMULATE_STDARG "
	     "/* use std_arg.h to simulate <stdarg.h> */");
#else
	puts("#define STDARG /* use <stdarg.h> */");
	puts("#include <stdarg.h>");
#endif
	puts("\n/* should we use vsnprintf() and vsnprintf()? */");
#if !defined(DONT_HAVE_VSNPRINTF)
	puts("#define HAVE_VSNPRINTF /* yes */");
#else
	puts("/*");
	puts(" * Hack aleart!!!");
	puts(" *");
	puts(" * Systems that do not have vsnprintf() need something. In some");
	puts(" * cases the snprintf function will deal correctly with the");
	puts(" * va_alist 4th arg.  Same gors for a lack of an vsnprintf()");
	puts(" * function.  In either case we use the #defines below and");
	puts(" * hope for the best!");
	puts(" */");
	puts("#define vsnprintf snprintf");
	puts("#undef HAVE_VSNPRINTF");
#endif
	/* exit(0); */
	return 0;
}
