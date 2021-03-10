/*
 * have_gettime - determine if we have clock_gettime()
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
 * Under source code control:	1999/10/20 23:43:42
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_gettime
 *
 * Not all systems have the clock_gettime() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETTIME
 *		defined ==> use clock_gettime() for either CLOCK_SGI_CYCLE
 *			    and/or CLOCK_REALTIME
 *		undefined ==> clock_gettime() is not available for both
 *			      CLOCK_SGI_CYCLE and CLOCK_REALTIME
 */

#include <stdio.h>
#include <time.h>


#include "banned.h"	/* include after system header <> includes */


int
main(void)
{
#if defined(HAVE_NO_GETTIME)

	printf("#undef HAVE_GETTIME /* no */\n");

#else /* HAVE_NO_GETTIME */

# if defined(CLOCK_SGI_CYCLE)

	struct timespec sgi_cycle;	/* SGI hardware clock */
	(void) clock_gettime(CLOCK_SGI_CYCLE, &sgi_cycle);
	printf("#define HAVE_GETTIME /* yes - w/CLOCK_SGI_CYCLE */\n");

# elif defined(CLOCK_REALTIME)

	struct timespec realtime;	/* POSIX realtime clock */
	(void) clock_gettime(CLOCK_REALTIME, &realtime);
	printf("#define HAVE_GETTIME /* yes - CLOCK_REALTIME only */\n");

# else

	printf("#undef HAVE_GETTIME /* no - no SGI_CYCLE and no REALTIME */\n");

# endif /* CLOCK_REALTIME */

#endif /* HAVE_NO_GETTIME */
	/* exit(0); */
	return 0;
}
