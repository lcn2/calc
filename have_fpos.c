/*
 * have_fpos - Determine if have fgetpos and fsetpos functions
 *
 * If the symbol HAVE_NO_FPOS is defined, we will output nothing.
 * If we are able to compile this program, then we must have the
 * fgetpos and fsetpos functions and we will output the
 * appropriate have_fpos.h file body.
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

int
main(void)
{
#if !defined(HAVE_NO_FPOS)
	fpos_t pos;		/* file position */

	/* get the current position */
	(void) fgetpos(stdin, &pos);

	/* set the current position */
	(void) fsetpos(stdin, &pos);

	/* print a have_fpos.h body that says we have the functions */
	printf("#undef HAVE_FPOS\n");
	printf("#define HAVE_FPOS 1  /* yes */\n\n");
	printf("typedef fpos_t FILEPOS;\n");
#endif
	/* exit(0); */
	return 0;
}
