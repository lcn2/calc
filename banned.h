/*
 * banned - optionally ban dqngerious functions
 *
 * Unless UNBAN is defined, this file will turn the use
 * of certain dangerous functions into syntax errors.
 *
 * In the case of calc, we are motivated in part by the desire for
 * calc to correctly calculate: even durings extremely long calculations.
 *
 * If UNBAN is NOT defined, then calling certain functions
 * will result in a syntaxc error.
 *
 * If we define UNBAN, then the effect of this file is disabled.
 *
 * The banned.h attempts to ban the use of certain dangerous functions
 * that, if improperly used, could compromise the computational integrity
 * if calculations.
 *
 * In the case of calc, we are motivated in part by the desire for calc
 * to correctly calculate: even durings extremely long calculations.
 *
 * If UNBAN is NOT defined, then calling certain functions
 * will result in a call to a non-existent function (link error).
 *
 * While we do NOT encourage defining UNBAN, there may be
 * a system / compiler environment where re-defining a
 * function may lead to a fatal compiler complication.
 * If that happens, consider compiling as:
 *
 *	make clobber all chk CCBAN=-DUNBAN
 *
 * as see if this is a work-a-round.
 *
 * If YOU discover a need for the -DUNBAN work-a-round, PLEASE tell us!
 * Please send us a bug report.  See the file:
 *
 *	BUGS
 *
 * or the URL:
 *
 *	http://www.isthe.com/chongo/tech/comp/calc/calc-bugrept.html
 *
 * for how to send us such a bug report.
 *
 * Copyright (C) 2021  Landon Curt Noll
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
 * Under source code control:	2021/03/06 21:07:31
 * File existed as early as:	2021
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(PRE_HAVE_BAN_PRAGMA_H)
#include "have_ban_pragma.h"
#endif /* ! PRE_HAVE_BAN_PRAGMA_H */


#if !defined(INCLUDE_BANNED_H)
#define INCLUDE_BANNED_H

/*
 * If we define UNBAN, then the effect of this file is disabled.
 */
#if !defined(UNBAN)

/*
 * In the spirit of:
 *
 *	https://github.com/git/git/blob/master/banned.h
 *
 * we will ban the use of certain unsafe functions by turning
 * then into function calls that do not exist.
 *
 * In the case of calc, we are motivated in part by the desire
 * for calc to correctly calculate: even durings extremely long
 * calculations.
 *
 * If UNBAN is NOT defined, then calling certain functions
 * will result in a syntaxc error.
 *
 * Unlike the above URL, we suggest an alternative function.
 * In many cases, additional logic is required to use the
 * alternative function, we cannot simply replace one function
 * with another.
 */

/*
 * If one is not careful, strcpy() can lead to buffer overflows.
 * Use strlcpy() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef strcpy
#pragma GCC poison strcpy
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * If one is not careful, strcat() can lead to buffer overflows.
 * Use strlcat() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef strcat
#pragma GCC poison strcat
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * If one is not careful, strncpy() can lead to buffer overflows.
 * Use memccpy() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef strncpy
#pragma GCC poison strncpy
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * If one is not careful, strncat() can lead to buffer overflows.
 * Use memccpy() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef strncat
#pragma GCC poison strncat
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * If one is not careful, sprintf() can lead to buffer overflows.
 * Use snprintf() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef sprintf
#pragma GCC poison sprintf
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * If one is not careful, vsprintf() can lead to buffer overflows.
 * Use vsnprintf() instead.
 */
#if defined(HAVE_PRAGMA_GCC_POSION)
#undef vsprintf
#pragma GCC poison vsprintf
#endif /* HAVE_PRAGMA_GCC_POSION */

/*
 * XXX - As of 2021, functions such as:
 *
 *	gmtime_s
 *	localtime_s
 *	ctime_s
 *	asctime_s
 *
 * are not universal.  We cannot yet ban the following
 * functions because we do not have a portable AND
 * widely available alternative.  Therefore we just
 * have to be extra careful when using:
 *
 *	gmtime
 *	localtime
 *	ctime
 *	ctime_r
 *	asctime
 *	asctime_r
 */

#endif /* !UNBAN */

#endif /* !INCLUDE_BANNED_H */
