/*
 * have_stdvs - try <stdarg.h> to see if it really works with vsprintf()
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
 * @(#) $Id: have_stdvs.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_stdvs.c,v $
 *
 * Under source code control:	1995/09/09 22:41:10
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * On some systems that have both <stdarg.h> and <varargs.h>, vsprintf()
 * does not work well under one type of include file.
 *
 * Some systems (such as UMIPS) have bugs in the <stdarg.h> implementation
 * that show up in vsprintf(), so we may have to try to use sprintf()
 * as if it were vsprintf() and hope for the best.
 *
 * This program will output #defines and exits 0 if vsprintf() (or sprintf())
 * produces the results that we expect.	 This program exits 1 if vsprintf()
 * (or sprintf()) produces unexpected results while using the <stdarg.h>
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

char buf[BUFSIZ];


void
try_this(char *fmt, ...)
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
	    /* <stdarg.h> with vsprintf() didn't work */
#else
	    /* <stdarg.h> with sprintf() simulating vsprintf() didn't work */
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
#if defined(DONT_HAVE_VSPRINTF)
	puts("/*");
	puts(" * SIMULATE_STDARG");
	puts(" *");
	puts(" * WARNING: This type of stdarg makes assumptions about the stack");
	puts(" *	    that may not be true on your system.  You may want to");
	puts(" *	    define STDARG (if using ANSI C) or VARARGS.");
	puts(" */");
	puts("typedef char *va_list;");
	puts("#define va_start(ap,parmn) (void)((ap) = (char*)(&(parmn) + 1))");
	puts("#define va_end(ap) (void)((ap) = 0)");
	puts("#define va_arg(ap, type) \\");
	puts("	      (((type*)((ap) = ((ap) + sizeof(type))))[-1])");
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
	/* exit(0); */
	return 0;
}
