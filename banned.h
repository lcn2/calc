/*
 * banned - indicate which functions are banned in calc source
 *
 * inspired by https://github.com/git/git/blob/master/banned.h
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


#if !defined(INCLUDE_BANNED_H)
#define INCLUDE_BANNED_H

#include "have_stdlib.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

/*
 * From: //github.com/git/git/blob/master/banned.h
 *
 * This header lists functions that have been banned from our code base,
 * because they're too easy to misuse (and even if used correctly,
 * complicate audits). Including this header turns them into compile-time
 * errors.
 */

#define BANNED(func,better) sorry_##func##_is_a_banned_function_use_##better##_instead

#undef strcpy
#define strcpy(x,y) BANNED(strcpy,strlcpy)
#undef strcat
#define strcat(x,y) BANNED(strcat,strlcat)
#undef strncpy
#define strncpy(x,y,n) BANNED(strncpy,memccpy)
#undef strncat
#define strncat(x,y,n) BANNED(strncat,memccpy)

#if defined(STDARG)
#define sprintf(...) BANNED(sprintf,snprintf)
#define vsprintf(...) BANNED(vsprintf,vsnprintf)
#else /* STDARG */
#define sprintf(buf,fmt,arg) BANNED(sprintf,snprintf)
#define vsprintf(buf,fmt,arg) BANNED(vsprintf,vsnprintf)
#endif /* STDARG */

#if 0 /* the XYtimeZZY_s() c11 functions are not yet universal - so do not ban XYtimeZZY() just yet - XXX */
#undef gmtime
#define gmtime(t) BANNED(gmtime,gmtime_s)
#undef localtime
#define localtime(t) BANNED(localtime,localtime_s)
#undef ctime
#define ctime(t) BANNED(ctime,ctime_s)
#undef ctime_r
#define ctime_r(t, buf) BANNED(ctime_r,ctime_s)
#undef asctime
#define asctime(t) BANNED(asctime,asctime_s)
#undef asctime_r
#define asctime_r(t, buf) BANNED(asctime_r,asctime_s)
#endif /* XXX */


#endif /* !INCLUDE_BANNED_H */
