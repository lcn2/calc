/*
 * have_const - Determine if we want or can support ansi const
 *
 * usage:
 *	have_const
 *
 * Not all compilers support const, so this may not compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_CONST
 *		defined ==> ok to use const
 *		undefined ==> do not use const
 *
 *	CONST
 *		const ==> use const
 *		(nothing) ==> const not used
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

MAIN
main(void)
{
#if defined(HAVE_NO_CONST)
	printf("#undef HAVE_CONST /* no */\n");
	printf("#undef CONST\n");
	printf("#define CONST /* no */\n");
#else /* HAVE_NO_CONST */
	const char * const str = "const";

	printf("#define HAVE_CONST /* yes */\n");
	printf("#undef CONST\n");
	printf("#define CONST %s /* yes */\n", str);
#endif /* HAVE_NO_CONST */
	exit(0);
}
