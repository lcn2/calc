/*
 * longlong - Determine the number if bits in a long long, if is exists
 *
 * usage:
 *	longlong [bits]
 *
 *	bits if empty or missing causes this prog to compute its length,
 *	     if 0, this prog will output nothing
 *	     otherwise this prog will assume it is the long long bit length
 *
 * Not all compilers support the long long type, so this may not compile
 * on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_LONGLONG
 *		defined ==> ok to use long long
 *		undefined ==> do not use long long, even if they exist
 *
 *	LONGLONG_BITS
 *		0 ==> do not use long long, even if they exist
 *		!= 0 ==> bits in an unsigned long long
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

#include "have_stdlib.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "have_string.h"
#if defined(HAVE_STRING_H)
#include <string.h>
#endif


/*
 * have the compiler try its hand with unsigned and signed long longs
 */
unsigned long long val = 4294967297ULL;
long long val2 = -4294967297LL;


MAIN
main(int argc, char **argv)
{
	int longlong_bits;	/* bits in a long long, or <=0 => dont use */

	/*
	 * parse args
	 */
	if (argc < 2) {
		/* no arg means compute the length */
		longlong_bits = sizeof(unsigned long long)*8;
	} else if (strcmp(argv[1], "") == 0) {
		/* empty arg means compute the length */
		longlong_bits = sizeof(unsigned long long)*8;
	} else {
		longlong_bits = atoi(argv[1]);
	}

	/*
	 * length is preset, or 0 ==> do not use
	 */
	if (longlong_bits > 0) {

		/*
		 * if size is longer than an unsigned long, use it
		 */
		if (longlong_bits > sizeof(unsigned long)*8) {

			/* use long long length */
			printf("#define HAVE_LONGLONG\n");
			printf("#define LONGLONG_BITS %d  /* yes */\n",
			    longlong_bits);
		}
	}
	exit(0);
}
