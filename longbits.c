/*
 * longbits - Determine the number if bits in a char, short, int or long
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll
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
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1994/03/18 03:06:18
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	longbits [long_bit_size]
 *
 *	long_bit_size	force size of long (must be 32 or 64)
 *
 * NOTE: If long_bit_size arg is empty (0 chars long) or it begins with
 *	 a whitespace character, it will be ignored and no forcing will
 *	 be done.
 *
 * Not all (in fact very few) C pre-processors can do:
 *
 *	#if sizeof(long) == 8
 *
 * so we have to form LONG_BITS ahead of time.
 *
 * This prog outputs several defines and typedefs:
 *
 *	LONG_BITS
 *		Number of bits in a long.  Not all (in fact very few) C
 *		pre-processors can do #if sizeof(long) == 8.
 *
 *	USB8	unsigned 8 bit value
 *	SB8	signed 8 bit value
 *
 *	USB16	unsigned 16 bit value
 *	SB16	signed 16 bit value
 *
 *	USB32	unsigned 32 bit value
 *	SB32	signed 32 bit value
 *
 *	HAVE_B64
 *		defined ==> ok to use USB64 (unsigned 64 bit value)
 *				  and SB64 (signed 64 bit value)
 *		undefined ==> do not use USB64 nor SB64
 *
 *	BOOL_B84
 *		If HAVE_B64 undefined ==> FALSE
 *		If HAVE_B64 defined ==> TRUE
 *
 *	USB64	unsigned 64 bit value if HAVE_B64 is defined
 *	SB64	signed 64 bit value if HAVE_B64 is defined
 *
 *	L(x)	form a 33 to 64 bit signed constant
 *	U(x)	form a 33 to 64 bit unsigned constant
 *
 * We hide the comments within strings to avoid complaints from some snitty
 * compilers.  We also hide 3 X's which is the calc symbol for "something bogus
 * this way comes".  In such error cases, we add -=*#*=- to force a syntax
 * error in the resulting .h file.
 *
 * We will exit 0 if all is well, non-zero with an error to stderr otherwise.
 */


#include <stdio.h>
#include <ctype.h>

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif


#include "banned.h"	/* include after system header <> includes */


char *program;		/* our name */

int
main(int argc, char **argv)
{
	int exitcode = 0;	/* how we will exit */
	size_t long_bits = 0;	/* bit length of a long */
	int forced_size = 0;	/* 1 => size of long was forced via arg */
	char value;		/* signed or maybe unsigned character */

	/*
	 * parse args
	 */
	program = argv[0];
	switch (argc) {
	case 1:
		long_bits = sizeof(long)*8;
		break;
	case 2:
		/* ignore empty or leading space args */
		if (argv[1][0] == '\0' ||
		    (isascii((int)argv[1][0]) && isspace((int)argv[1][0]))) {
			long_bits = sizeof(long)*8;
		/* process the forced size arg */
		} else {
			forced_size = 1;
			long_bits = atoi(argv[1]);
			if (long_bits != 32 && long_bits != 64) {
				fprintf(stderr,
				    "usage: %s [32 or 64]\n", program);
				exit(1);
			}
		}
		break;
	default:
		fprintf(stderr, "usage: %s [32 or 64]\n", program);
		exit(2);
	}

	/*
	 * report size of long
	 */
	printf("#undef LONG_BITS\n");
	printf("#define LONG_BITS %ld\t\t/%s/\n",
	  (long int)long_bits, "* bit length of a long *");
	putchar('\n');

	/*
	 * If we are forcing the size of a long, then do not check
	 * sizes of other values but instead assume that the user
	 * knows what they are doing.
	 */
	if (forced_size) {

		/*
		 * note that the size was forced
		 */
		printf("/%s/\n\n", "* size of long was forced *");

		/*
		 * forced forming of USB8, SB8, USB16 and SB16
		 */
		printf("typedef unsigned char USB8;\t/%s/\n",
		    "* unsigned 8 bits *");
		printf("typedef signed char SB8;\t/%s/\n\n",
		    "* signed 8 bits *");

		printf("typedef unsigned short USB16;\t/%s/\n",
		    "* unsigned 16 bits *");
		printf("typedef short SB16;\t\t/%s/\n\n",
		    "* signed 16 bits *");

		/*
		 * forced forming of USB32 and SB32
		 */
		if (long_bits == 32) {
			/* forced 32 bit long mode assumptions */
			printf("typedef unsigned long USB32;\t/%s/\n",
			    "* unsigned 32 bits *");
			printf("typedef long SB32;\t\t/%s/\n\n",
			    "* signed 32 bits *");
		} else {
			/* forced 64 bit long mode assumptions */
			printf("typedef unsigned int USB32;\t/%s/\n",
			    "* unsigned 32 bits *");
			printf("typedef int SB32;\t\t/%s/\n\n",
			    "* signed 32 bits *");
		}

		/*
		 * forced forming of HAVE_B64, USB64, SB64, U(x) and L(x)
		 */
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t/%s/\n",
		  "* have USB64 and SB64 types *");
		printf("typedef unsigned long long USB64;\t/%s/\n",
		  "* unsigned 64 bits *");
		printf("typedef long long SB64;\t\t/%s/\n",
		  "* signed 64 bits *");
		putchar('\n');
		printf("/%s/\n","* how to form 64 bit constants *");
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || \
    defined(__cplusplus)
		printf("#define U(x) x ## ULL\n");
		printf("#define L(x) x ## LL\n");
#else
		printf("#define U(x) ((unsigned long long)x)\n");
		printf("#define L(x) ((long long)x)\n");
#endif

		/*
		 * all done
		 */
		exit(0);
	}

	/*
	 * look for 8 bit values
	 */
	value = (char)-1;
	if (sizeof(char) != 1) {
		fprintf(stderr,
		  "%s: OUCH!!! - char is not a single byte!\n", program);
		fprintf(stderr,
		  "%s: fix the USB8 typedef by hand\n", program);
		printf("typedef unsigned char USB8;\t/* XX%s/ -=*#*=-\n",
		  "X - should be 8 unsigned bits but is not *");
		if (value < 1) {
			printf("typedef char SB8;\t\t/* XX%s/ -=*#*=-\n",
			  "X - should be 8 signed bits but is not *");
		} else {
			printf("typedef signed char SB8;\t\t/* XX%s/ -=*#*=-\n",
			  "X - should be 8 signed bits but is not *");
		}
		exitcode = 3;
	} else {
		printf("typedef unsigned char USB8;\t/%s/\n",
		  "* unsigned 8 bits *");
		if (value < 1) {
			printf("typedef char SB8;\t\t/%s/\n",
			  "* signed 8 bits *");
		} else {
			printf("typedef signed char SB8;\t\t/%s/\n",
			  "* signed 8 bits *");
		}
	}
	putchar('\n');

	/*
	 * look for 16 bit values
	 */
	if (sizeof(short) != 2) {
		fprintf(stderr,
		  "%s: OUCH!!! - short is not a 2 bytes!\n", program);
		fprintf(stderr,
		  "%s: fix the USB16 typedef by hand\n", program);
		printf("typedef unsigned short USB16;\t/* XX%s/ -=*#*=-\n",
		  "X - should be 16 unsigned bits but is not *");
		printf("typedef signed char SB16;\t/* XX%s/ -=*#*=-\n",
		  "X - should be 16 signed bits but is not *");
		exitcode = 4;
	} else {
		printf("typedef unsigned short USB16;\t/%s/\n",
		  "* unsigned 16 bits *");
		printf("typedef short SB16;\t\t/%s/\n",
		  "* signed 16 bits *");
	}
	putchar('\n');

	/*
	 * look for 32 bit values
	 */
	if (sizeof(long) == 4) {
		printf("typedef unsigned long USB32;\t/%s/\n",
		  "* unsigned 32 bits *");
		printf("typedef long SB32;\t\t/%s/\n",
		  "* signed 32 bits *");
	} else if (sizeof(int) == 4) {
		printf("typedef unsigned int USB32;\t/%s/\n",
		  "* unsigned 32 bits *");
		printf("typedef int SB32;\t\t/%s/\n",
		  "* signed 32 bits *");
	} else {
		fprintf(stderr,
		  "%s: OUCH!!! - neither int nor long are 4 bytes!\n", program);
		printf("typedef unsigned int USB32;\t/* XX%s/ -=*#*=-\n",
		  "X - should be 32 unsigned bits but is not *");
		printf("typedef signed int SB32;\t/* XX%s/ -=*#*=-\n",
		  "X - should be 32 signed bits but is not *");
		exitcode = 5;
	}
	putchar('\n');

	/*
	 * look for 64 bit values
	 */
	if (sizeof(long) == 8) {
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t/%s/\n",
		  "* have USB64 and SB64 types *");
		printf("typedef unsigned long USB64;\t/%s/\n",
		  "* unsigned 64 bits *");
		printf("typedef long SB64;\t\t/%s/\n",
		  "* signed 64 bits *");
		putchar('\n');
		printf("/%s/\n","* how to form 64 bit constants *");
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || \
    defined(__cplusplus)
		printf("#define U(x) x ## UL\n");
		printf("#define L(x) x ## L\n");
#else
		printf("#define U(x) ((unsigned long)x)\n");
		printf("#define L(x) ((long)x)\n");
#endif
	} else {
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t/%s/\n",
		  "* have USB64 and SB64 types *");
		printf("typedef unsigned long long USB64;\t/%s/\n",
		  "* unsigned 64 bits *");
		printf("typedef long long SB64;\t\t/%s/\n",
		  "* signed 64 bits *");
		putchar('\n');
		printf("/%s/\n","* how to form 64 bit constants *");
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || \
    defined(__cplusplus)
		printf("#define U(x) x ## ULL\n");
		printf("#define L(x) x ## LL\n");
#else
		printf("#define U(x) ((unsigned long long)x)\n");
		printf("#define L(x) ((long long)x)\n");
#endif
	}

	/* all done */
	/* exit(exitcode); */
	return exitcode;
}
