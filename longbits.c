/*
 * longbits - Determine the number if bits in a char, short, int or long
 *
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
 * We will also note if we have a standard 64 bit type (i.e., long).  If we
 * do, we will typedef it and define HAVE_B64.  If we do not then if longlong.h
 * says we can use long long types, we will use that.  If we cannot use a
 * long long type, then HAVE_B64 will not be defined.
 *
 * We hide the comments within strings to avoid complaints from some snitty
 * compilers.  We also hide 3 X's which is the calc symbol for "something bogus
 * this way comes".  In such error cases, we add -=*#*=- to force a syntax
 * error in the resulting .h file.
 *
 * We will exit 0 if all is well, non-zero with an error to stderr otherwise.
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
#include <ctype.h>

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "longlong.h"

#if defined(__linux)
# if !defined(isascii)
   extern int isascii(int c);
# endif /* !isascii */
#endif /* __linux */

char *program;		/* our name */

MAIN
main(int argc, char **argv)
{
	int exitcode = 0;	/* how we will exit */
	int long_bits = 0;	/* bit length of a long */
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
		    (isascii(argv[1][0]) && isspace(argv[1][0]))) {
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
	printf("#define LONG_BITS %d\t\t%c%s%c\n",
	  long_bits, '/', "* bit length of a long *", '/');
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
		printf("%c%s%c\n\n", '/', "* size of long was forced *", '/');

		/*
		 * forced forming of USB8, SB8, USB16 and SB16
		 */
		printf("typedef unsigned char USB8;\t%c%s%c\n",
		    '/', "* unsigned 8 bits *", '/');
		printf("typedef signed char SB8;\t%c%s%c\n\n",
		    '/', "* signed 8 bits *", '/');

		printf("typedef unsigned short USB16;\t%c%s%c\n",
		    '/', "* unsigned 16 bits *", '/');
		printf("typedef short SB16;\t\t%c%s%c\n\n",
		    '/', "* signed 16 bits *", '/');

		/*
		 * forced forming of USB32 and SB32
		 */
		if (long_bits == 32) {
			/* forced 32 bit long mode assumptions */
			printf("typedef unsigned long USB32;\t%c%s%c\n",
			    '/', "* unsigned 32 bits *", '/');
			printf("typedef long SB32;\t\t%c%s%c\n\n",
			    '/', "* signed 32 bits *", '/');
		} else {
			/* forced 64 bit long mode assumptions */
			printf("typedef unsigned int USB32;\t%c%s%c\n",
			    '/', "* unsigned 32 bits *", '/');
			printf("typedef int SB32;\t\t%c%s%c\n\n",
			    '/', "* signed 32 bits *", '/');
		}

		/*
		 * forced forming of HAVE_B64, USB64, SB64, U(x) and L(x)
		 */
#if defined(HAVE_LONGLONG) && LONGLONG_BITS == 64
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t%c%s%c\n",
		  '/', "* have USB64 and SB64 types *", '/');
		printf("#define BOOL_B64 TRUE\n");
		printf("typedef unsigned long long USB64;\t%c%s%c\n",
		  '/', "* unsigned 64 bits *", '/');
		printf("typedef long long SB64;\t\t%c%s%c\n",
		  '/', "* signed 64 bits *", '/');
		putchar('\n');
		printf("%c%s%c\n", '/',"* how to form 64 bit constants *",'/');
#if defined(__STDC__) && __STDC__ != 0
		printf("#define U(x) x ## ULL\n");
		printf("#define L(x) x ## LL\n");
#else
		printf("#define U(x) ((unsigned long long)x)\n");
		printf("#define L(x) ((long long)x)\n");
#endif
#else
		printf("#undef HAVE_B64\t\t\t%c%s%c\n",
		  '/', "* we have no USB64 and no SB64 types *", '/');
		printf("#define BOOL_B64 FALSE\n");
		putchar('\n');
		printf("%c%s%c\n", '/', "* no 64 bit constants *", '/');
		printf("#define U(x) no 33 to 64 bit constants %s\n",
		  "- do not use this macro!");
		printf("#define L(x) no 33 to 64 bit constants %s\n",
		  "- do not use this macro!");
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
		printf("typedef unsigned char USB8;\t%c* XX%s%c -=*#*=-\n",
		  '/', "X - should be 8 unsigned bits but is not *", '/');
		if (value < 1) {
			printf("typedef char SB8;\t%c* XX%s%c -=*#*=-\n",
			  '/', "X - should be 8 signed bits but is not *", '/');
		} else {
			printf("typedef signed char SB8;\t%c* XX%s%c -=*#*=-\n",
			  '/', "X - should be 8 signed bits but is not *", '/');
		}
		exitcode = 3;
	} else {
		printf("typedef unsigned char USB8;\t%c%s%c\n",
		  '/', "* unsigned 8 bits *", '/');
		if (value < 1) {
			printf("typedef char SB8;\t%c%s%c\n",
			  '/', "* signed 8 bits *", '/');
		} else {
			printf("typedef signed char SB8;\t%c%s%c\n",
			  '/', "* signed 8 bits *", '/');
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
		printf("typedef unsigned short USB16;\t%c* XX%s%c -=*#*=-\n",
		  '/', "X - should be 16 unsigned bits but is not *", '/');
		printf("typedef signed char SB16;\t%c* XX%s%c -=*#*=-\n",
		  '/', "X - should be 16 signed bits but is not *", '/');
		exitcode = 4;
	} else {
		printf("typedef unsigned short USB16;\t%c%s%c\n",
		  '/', "* unsigned 16 bits *", '/');
		printf("typedef short SB16;\t\t%c%s%c\n",
		  '/', "* signed 16 bits *", '/');
	}
	putchar('\n');

	/*
	 * look for 32 bit values
	 */
	if (sizeof(long) == 4) {
		printf("typedef unsigned long USB32;\t%c%s%c\n",
		  '/', "* unsigned 32 bits *", '/');
		printf("typedef long SB32;\t\t%c%s%c\n",
		  '/', "* signed 32 bits *", '/');
	} else if (sizeof(int) == 4) {
		printf("typedef unsigned int USB32;\t%c%s%c\n",
		  '/', "* unsigned 32 bits *", '/');
		printf("typedef int SB32;\t\t%c%s%c\n",
		  '/', "* signed 32 bits *", '/');
	} else {
		fprintf(stderr,
		  "%s: OUCH!!! - neither int nor long are 4 bytes!\n", program);
		printf("typedef unsigned int USB32;\t%c* XX%s%c -=*#*=-\n",
		  '/', "X - should be 32 unsigned bits but is not *", '/');
		printf("typedef signed int SB32;\t%c* XX%s%c -=*#*=-\n",
		  '/', "X - should be 32 signed bits but is not *", '/');
		exitcode = 5;
	}
	putchar('\n');

	/*
	 * look for 64 bit values
	 */
	if (sizeof(long) == 8) {
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t%c%s%c\n",
		  '/', "* have USB64 and SB64 types *", '/');
		printf("#define BOOL_B64 TRUE\n");
		printf("typedef unsigned long USB64;\t%c%s%c\n",
		  '/', "* unsigned 64 bits *", '/');
		printf("typedef long SB64;\t\t%c%s%c\n",
		  '/', "* signed 64 bits *", '/');
		putchar('\n');
		printf("%c%s%c\n", '/',"* how to form 64 bit constants *",'/');
#if defined(__STDC__) && __STDC__ != 0
		printf("#define U(x) x ## UL\n");
		printf("#define L(x) x ## L\n");
#else
		printf("#define U(x) ((unsigned long)x)\n");
		printf("#define L(x) ((long)x)\n");
#endif
	} else {
#if defined(HAVE_LONGLONG) && LONGLONG_BITS == 64
		printf("#undef HAVE_B64\n");
		printf("#define HAVE_B64\t\t%c%s%c\n",
		  '/', "* have USB64 and SB64 types *", '/');
		printf("#define BOOL_B64 TRUE\n");
		printf("typedef unsigned long long USB64;\t%c%s%c\n",
		  '/', "* unsigned 64 bits *", '/');
		printf("typedef long long SB64;\t\t%c%s%c\n",
		  '/', "* signed 64 bits *", '/');
		putchar('\n');
		printf("%c%s%c\n", '/',"* how to form 64 bit constants *",'/');
#if defined(__STDC__) && __STDC__ != 0
		printf("#define U(x) x ## ULL\n");
		printf("#define L(x) x ## LL\n");
#else
		printf("#define U(x) ((unsigned long long)x)\n");
		printf("#define L(x) ((long long)x)\n");
#endif
#else
		printf("#undef HAVE_B64\t\t\t%c%s%c\n",
		  '/', "* we have no USB64 and no SB64 types *", '/');
		printf("#define BOOL_B64 FALSE\n");
		putchar('\n');
		printf("%c%s%c\n", '/', "* no 64 bit constants *", '/');
		printf("#define U(x) no 33 to 64 bit constants %s\n",
		  "- do not use this macro!");
		printf("#define L(x) no 33 to 64 bit constants %s\n",
		  "- do not use this macro!");
#endif
	}

	/* all done */
	exit(exitcode);
}
