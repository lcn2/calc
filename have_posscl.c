/*
 * have_posscl - determine if have a scalar FILEPOS element
 *
 * usage:
 *	have_posscl
 *
 * On some systems, FILEPOS is a scalar value on which one can perform
 * arithmetic operations, assignments and comparisons.  On some systems
 * FILEPOS is some sort of union or struct which must be converted into
 * a ZVALUE in order to perform arithmetic operations, assignments and
 * comparisons.
 *
 *
 * This prog outputs several defines:
 *
 *	HAVE_FILEPOS_SCALAR
 *		defined ==> ok to perform arithmetic ops, = and comparisons
 *		undefined ==> convert to ZVALUE first
 */
/*
 * Copyright (c) 1996 by Landon Curt Noll.  All Rights Reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include "have_fpos.h"

MAIN
main(void)
{
#if !defined(FILEPOS_NON_SCALAR)
	FILEPOS value;	/* an FILEPOS to perform arithmatic on */
	FILEPOS value2;	/* an FILEPOS to perform arithmatic on */

	/*
	 * do some math opts on an FILEPOS
	 */
	value = (FILEPOS)getpid();
	value2 = (FILEPOS)-1;
	if (value > (FILEPOS)1) {
		--value;
	}
	if (value <= (FILEPOS)getppid()) {
		--value;
	}
	if (value == value2) {
		value += value2;
	}
	value <<= 1;
	if (!value) {
		printf("/* something for the FILEPOS to do */\n");
	}

	/*
	 * report FILEPOS as a scalar
	 */
	printf("#undef HAVE_FILEPOS_SCALAR\n");
	printf("#define HAVE_FILEPOS_SCALAR /* FILEPOS is a simple value */\n");
#else
	printf("#undef HAVE_FILEPOS_SCALAR /* FILEPOS is not a simple value */\n");
#endif
	exit(0);
}
