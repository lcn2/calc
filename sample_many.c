/*
 * sample_many - generate many random values via random number generator
 *
 * Copyright (C) 1999-2007  Landon Curt Noll
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
 * @(#) $Revision: 30.1 $
 * @(#) $Id: sample_many.c,v 30.1 2007/03/16 11:09:46 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/sample_many.c,v $
 *
 * Under source code control:	1997/04/19 22:46:49
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	many_random [[bits] seed_string]
 *
 *	seed_string	something for which we can seed (def: default seed)
 *	bits		number of bits to generate
 */


#include <sys/types.h>
#include <stdio.h>
#include "calc.h"
#include "zrandom.h"
#include "have_const.h"
#include "lib_util.h"

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
