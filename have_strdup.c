/*
 * have_strdup - Determine if we strdup()
 *
 * usage:
 *	have_strdup
 *
 * Not all systems have the strdup() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_STRDUP
 *		defined ==> use strdup()
 *		undefined ==> do not call or cannot call strdup()
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

#include <string.h>

int
main(void)
{
#if defined(HAVE_NO_STRDUP)

	printf("#undef HAVE_STRDUP /* no */\n");

#else /* HAVE_NO_STRDUP */

	char *p;

	p = strdup("#define HAVE_STRDUP /* yes */");
	if (p != NULL) {
		printf("%s\n", p);
	}

#endif /* HAVE_NO_STRDUP */

	/* exit(0); */
	return 0;
}
