/*
 * longlong - determine the number of bits in a long long, if is exists
 *
 * Copyright (C) 1999  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.1 $
 * @(#) $Id: longlong.c,v 29.1 1999/12/14 09:16:12 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/longlong.c,v $
 *
 * Under source code control:	1994/08/05 01:09:19
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://reality.sgi.com/chongo/
 * Share and enjoy!  :-)	http://reality.sgi.com/chongo/tech/comp/calc/
 */

/*
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
unsigned long long val = 0x1234567890123456ULL;
long long val2 = -1311768467284833366LL;	/* -0x1234567890123456 */


int
main(int argc, char **argv)
{
	int longlong_bits;	/* bits in a long long, or <=0 => dont use */
	char buf[BUFSIZ+1];	/* scan buffer */

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
		 * if size is longer than an unsigned long,
		 * and the negative 'long long' works, then use long long's
		 */
		if (longlong_bits > sizeof(unsigned long)*8 && val2 < 0) {

			/* use long long length */
			printf("#define HAVE_LONGLONG\n");
			printf("#define LONGLONG_BITS %d  /* yes */\n",
			    longlong_bits);

			printf("\n/*\n");
			printf(" * how should 64 bit values be formatted?\n");
			printf(" *\n");

			/* it is OK to get a printf format type warning here */
			sprintf(buf, "%ld", val);

			printf(" * sprintf \"%%ld\" of 0x1234567890123456ULL "
			       "is %s\n", buf);
			printf(" *\n");
			printf(" * if defined(L64_FORMAT), ok to use %%ld\n");
			printf(" * if !defined(L64_FORMAT), use %%lld\n");
			printf(" */\n");
			if (buf[0] == '-') {
				printf("#undef L64_FORMAT\n");
			} else {
				printf("#define L64_FORMAT\n");
			}

		/*
		 * We have no useful 64 bit values
		 */
		} else {
			if (longlong_bits <= sizeof(unsigned long)*8) {
				printf("/* long long size <= long size */\n");
			} else {
				printf("/* unsigned long long constants "
				       "don't work */\n");
			}
			printf("#undef HAVE_LONGLONG\n");
			printf("#define LONGLONG_BITS 0\t/%s/\n", "* no *");
			printf("#undef L64_FORMAT\n");
		}
	}
	/* exit(0); */
	return 0;
}
