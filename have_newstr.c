/*
 * have_newstr - Determine if we have a system without ANSI C string functions
 *
 * usage:
 *	have_newstr
 *
 * Not all systems support all ANSI C string functions, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_NEWSTR
 *		defined ==> use memcpy(), memset(), strchr()
 *		undefined ==> use bcopy() instead of memcpy(),
 *			      use bfill() instead of memset(),
 *			      use index() instead of strchr()
 */
/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
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

#include <stdio.h>

#define MOVELEN 3

char src[] = "chongo was here";
char dest[MOVELEN+1];

int
main(void)
{
#if defined(HAVE_NO_NEWSTR)
	printf("#undef HAVE_NEWSTR /* no */\n");
#else /* HAVE_NO_NEWSTR */
	(void) memcpy(dest, src, MOVELEN);
	(void) memset(dest, 0, MOVELEN);
	(void) strchr(src, 'e');

	printf("#define HAVE_NEWSTR /* yes */\n");
#endif /* HAVE_NO_NEWSTR */
	/* exit(0); */
	return 0;
}
