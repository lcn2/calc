/*
 * have_getpgid - Determine if we getpgid()
 *
 * usage:
 *	have_getpgid
 *
 * Not all systems have the getpgid() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETPGID
 *		defined ==> use getpgid()
 *		undefined ==> do not or cannot call getpgid()
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
#include <unistd.h>

int
main(void)
{
#if defined(HAVE_NO_GETPGID)

	printf("#undef HAVE_GETPGID /* no */\n");

#else /* HAVE_NO_GETPGID */

	(void) getpgid((pid_t)0);

	printf("#define HAVE_GETPGID /* yes */\n");

#endif /* HAVE_NO_GETPGID */

	/* exit(0); */
	return 0;
}
