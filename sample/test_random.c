/*
 * test_random - test the libcalc random number generator
 *
 * usage:
 *	test_random [[bits] seed_string]
 *
 *	seed_string 	something for which we can seed (def: default seed)
 *	bits		number of bits to generate
 */

/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
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


#if defined(__sgi)
# include "../longbits.h"
# if defined(HAVE_B64)
typedef USB64 k_sigset_t;
# else
typedef struct {
	USB32 sigbits[2];
} k_sigset_t;
# endif
#endif


#include <sys/types.h>
#include <stdio.h>
#include "../calc.h"
#include "../zrandom.h"
#include "../have_const.h"
#include "../lib_util.h"

#define DEF_CNT 128	/* default number of bits to generate */

extern char *program;	/* our name */


int
main(int argc, char **argv)
{
	RANDOM *prev_state;	/* previous random number state */
	ZVALUE seed;		/* seed for Blum-Blum-Shub */
	ZVALUE random_val;	/* random number produced */
	long cnt;		/* number of bits to generate */
	char *hexstr;		/* random number as hex string */

	/*
	 * parse args
	 */
	program = argv[0];
	switch (argc) {
	case 3:
		seed = convstr2z(argv[2]);
		cnt = strtol(argv[1], NULL, 0);
		break;
	case 2:
		seed = _zero_;	/* use the default seed */
		cnt = strtol(argv[1], NULL, 0);
		break;
	case 1:
		seed = _zero_;	/* use the default seed */
		cnt = DEF_CNT;
		break;
	default:
		fprintf(stderr, "usage: %s [[bits] seed_string]\n", program);
		exit(1);
	}
	if (cnt <= 0) {
		fprintf(stderr, "%s: cnt:%d must be > 0\n", program, (int)cnt);
		exit(2);
	}
	printf("seed= 0x%s\n", convz2hex(seed));

	/*
	 * libcalc setup
	 */
	libcalc_call_me_first();

	/*
	 * seed the generator
	 */
	prev_state = zsrandom2(seed, zconst[10]);
	if (prev_state == NULL) {
		math_error("previous random state is NULL");
		/*NOTREACHED*/
	}

	/*
	 * generate random bits
	 */
	zrandom(cnt, &random_val);

	/*
	 * convert into hex string
	 */
	hexstr = convz2hex(random_val);
	printf("random= 0x%s\n", hexstr);

	/*
	 * all done
	 */
	/* exit(0); */
	return 0;
}
