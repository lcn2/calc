/*
 * have_ustat - Determine if we ustat()
 *
 * usage:
 *	have_ustat
 *
 * Not all systems have the ustat() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_USTAT
 *		defined ==> use ustat()
 *		undefined ==> do not call or cannot call ustat()
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

#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>

int
main(void)
{
#if defined(HAVE_NO_USTAT)

	printf("#undef HAVE_USTAT /* no */\n");

#else /* HAVE_NO_USTAT */

	struct stat stat_dot;           /* stat of "." */
	struct ustat ustat_dot;         /* usage stat of "." */

	(void) stat(".", &stat_dot);
	(void) ustat(stat_dot.st_dev, &ustat_dot);

	printf("#define HAVE_USTAT /* yes */\n");

#endif /* HAVE_NO_USTAT */

	/* exit(0); */
	return 0;
}
