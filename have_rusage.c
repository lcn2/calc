/*
 * have_rusage - Determine if we getrusage()
 *
 * usage:
 *	have_rusage
 *
 * Not all systems have the getrusage() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETRUSAGE
 *		defined ==> use getrusage()
 *		undefined ==> do not call or cannot call getrusage()
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

#include <sys/time.h>
#include <sys/resource.h>

int
main(void)
{
#if defined(HAVE_NO_GETRUSAGE)

	printf("#undef HAVE_GETRUSAGE /* no */\n");

#else /* HAVE_NO_GETRUSAGE */

	struct rusage rusage;		/* resource utilization */

	(void) getrusage(RUSAGE_SELF, &rusage);

	printf("#define HAVE_GETRUSAGE /* yes */\n");

#endif /* HAVE_NO_GETRUSAGE */

	/* exit(0); */
	return 0;
}
