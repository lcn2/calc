/*
 * have_gettime - Determine if we clock_gettime()
 *
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
/*
 * Copyright (c) 1999 by Landon Curt Noll.  All Rights Reserved.
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

#include <time.h>

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
