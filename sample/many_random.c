/*
 * many_random - generate many random values via random number generator
 *
 * usage:
 *	many_random [[bits] seed_string]
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
#define RESEED 1000	/* number of random numbers to generate */
#define MANY 100	/* number of random numbers to generate */

extern char *program;	/* our name */


int
main(int argc, char **argv)
{
	RANDOM *prev_state;	/* previous random number state */
	ZVALUE seed;		/* seed for Blum-Blum-Shub */
	ZVALUE tmp;		/* temp value */
	ZVALUE tmp2;		/* temp value */
	ZVALUE random_val;	/* random number produced */
	long cnt;		/* number of bits to generate */
	char *hexstr;		/* random number as hex string */
	int i;
	int j;

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

	/*
	 * libcalc setup
	 */
	libcalc_call_me_first();

	/*
	 * reseed every so often
 	 */
	for (j=0; j < RESEED; ++j) {

		/*
		 * seed the generator
		 */
		prev_state = zsrandom2(seed, zconst[1]);
		if (prev_state == NULL) {
			math_error("previous random state is NULL");
			/*NOTREACHED*/
		}
		randomfree(prev_state);

		/*
		 * generate random values forever
		 */
		for (i=0; i < MANY; ++i) {

			/*
			 * generate random bits
			 */
			zrandom(cnt, &random_val);

			/*
			 * convert into hex string
			 */
			hexstr = convz2hex(random_val);
			printf("%s\n", hexstr);

			/*
			 * free
			 */
			if (i < MANY-1) {
				zfree(random_val);
			}
			free(hexstr);
		}

		/*
		 * increment the seed to better test different seeds
		 *
		 * NOTE: It is generally a bad idea to use the
		 *	 same random number generator to modify
		 *	 the seed.  We only do this below to
		 *	 try different seeds for debugging.
		 *
		 *	 Don't do this in real life applications!
		 *
		 * We want to add at least 2^32 to the seed, so
		 * we do the effect of:
		 *
		 *	seed += ((last_val<<32) + last_val);
		 */
		zshift(random_val, 32, &tmp);
		zadd(tmp, random_val, &tmp2);
		zfree(random_val);
		zfree(tmp);
		zadd(seed, tmp2, &tmp);
		zfree(tmp2);
		zfree(seed);
		seed = tmp;
	}

	/*
	 * libcalc shutdown
	 */
	libcalc_call_me_last();

	/*
	 * all done
	 */
	/* exit(0); */
	return 0;
}
