/*
 * sample_rand - test the libcalc random number generator
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
 * @(#) $Id: sample_rand.c,v 30.1 2007/03/16 11:09:46 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/sample_rand.c,v $
 *
 * Under source code control:	1997/04/19 22:46:49
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	test_random [[bits] seed_string]
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
