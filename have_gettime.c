/*
 * have_gettime - determine if we have clock_gettime()
 *
 * Copyright (C) 1999,2021,2026  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1999/10/20 23:43:42
 * File existed as early as:    1999
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      have_gettime
 *
 * Not all systems have the clock_gettime() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *      HAVE_GETTIME
 *              defined ==> use clock_gettime() from CLOCK_REALTIME
 *              undefined ==> clock_gettime() is not available from
 *                            CLOCK_REALTIME
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <time.h>

/*
 * calc local src includes
 */

#include "banned.h" /* include after all other includes */

int
main(void)
{
#if defined(HAVE_NO_GETTIME) || defined(_WIN32) || defined(_WIN64)

    printf("#undef HAVE_GETTIME /* no */\n");

#else

#  if defined(CLOCK_REALTIME)

    struct timespec realtime; /* POSIX realtime clock */
    (void)clock_gettime(CLOCK_REALTIME, &realtime);
    printf("#define HAVE_GETTIME /* yes - CLOCK_REALTIME only */\n");

#  else

    printf("#undef HAVE_GETTIME /* no - no CLOCK_REALTIME */\n");

#  endif
#endif
    /* exit(0); */
    return 0;
}
