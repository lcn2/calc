/*
 * zfunc - extended precision integral arithmetic non-primitive routines
 *
 * Copyright (C) 1999-2007,2021  David I. Bell, Landon Curt Noll
 *				 and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:27
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "zmath.h"
#include "alloc.h"


#include "banned.h"	/* include after system header <> includes */


ZVALUE _tenpowers_[TEN_MAX+1];		/* table of 10^2^n */

STATIC long *power10 = NULL;
STATIC int max_power10_exp = 0;

/*
 * given:
 *
 *	unsigned long x
 * or:	unsigned long long x
 * or:	long x			and  x >= 0
 * or:	long long x		and  x >= 0
 *
 * If issq_mod4k[x & 0xfff] == 0, then x cannot be a perfect square
 * else x might be a perfect square.
 */
STATIC USB8 issq_mod4k[1<<12] = {
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
	0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
};


/*
 * Compute the factorial of a number.
 */
void
zfact(ZVALUE z, ZVALUE *dest)
{
	long ptwo;		/* count of powers of two */
	long n;			/* current multiplication value */
	long m;			/* reduced multiplication value */
	long mul;		/* collected value to multiply by */
	ZVALUE res, temp;

	if (zisneg(z)) {
		math_error("Negative argument for factorial");
		/*NOTREACHED*/
	}
	if (zge31b(z)) {
		math_error("Very large factorial");
		/*NOTREACHED*/
	}
	n = ztolong(z);
	ptwo = 0;
	mul = 1;
	res = _one_;
	/*
	 * Multiply numbers together, but squeeze out all powers of two.
	 * We will put them back in at the end.	 Also collect multiple
	 * numbers together until there is a risk of overflow.
	 */
	for (; n > 1; n--) {
		for (m = n; ((m & 0x1) == 0); m >>= 1)
			ptwo++;
		if (mul <= MAXLONG/m) {
			mul *= m;
			continue;
		}
		zmuli(res, mul, &temp);
		zfree(res);
		res = temp;
		mul = m;
	}
	/*
	 * Multiply by the remaining value, then scale result by
	 * the proper power of two.
	 */
	if (mul > 1) {
		zmuli(res, mul, &temp);
		zfree(res);
		res = temp;
	}
	zshift(res, ptwo, &temp);
	zfree(res);
	*dest = temp;
}


/*
 * Compute the permutation function  M! / (M - N)!.
 */
void
zperm(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	SFULL count;
	ZVALUE cur, tmp, ans;

	if (zisneg(z1) || zisneg(z2)) {
		math_error("Negative argument for permutation");
		/*NOTREACHED*/
	}
	if (zrel(z1, z2) < 0) {
		math_error("Second arg larger than first in permutation");
		/*NOTREACHED*/
	}
	if (zge31b(z2)) {
		math_error("Very large permutation");
		/*NOTREACHED*/
	}
	count = ztolong(z2);
	zcopy(z1, &ans);
	zsub(z1, _one_, &cur);
	while (--count > 0) {
		zmul(ans, cur, &tmp);
		zfree(ans);
		ans = tmp;
		zsub(cur, _one_, &tmp);
		zfree(cur);
		cur = tmp;
	}
	zfree(cur);
	*res = ans;
}

/*
 * docomb evaluates binomial coefficient when z1 >= 0, z2 >= 0
 */
S_FUNC int
docomb(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE ans;
	ZVALUE mul, div, temp;
	FULL count, i;
#if BASEB == 16
	HALF dh[2];
#else
	HALF dh[1];
#endif

	if (zrel(z2, z1) > 0)
		return 0;
	zsub(z1, z2, &temp);

	if (zge31b(z2) && zge31b(temp)) {
		zfree(temp);
		return -2;
	}
	if (zrel(temp, z2) < 0)
		count = ztofull(temp);
	else
		count = ztofull(z2);
	zfree(temp);
	if (count == 0)
		return 1;
	if (count == 1)
		return 2;
	div.sign = 0;
	div.v = dh;
	div.len = 1;
	zcopy(z1, &mul);
	zcopy(z1, &ans);
	for (i = 2; i <= count; i++) {
#if BASEB == 16
		dh[0] = (HALF)(i & BASE1);
		dh[1] = (HALF)(i >> BASEB);
		div.len = 1 + (dh[1] != 0);
#else
		dh[0] = (HALF) i;
#endif
		zsub(mul, _one_, &temp);
		zfree(mul);
		mul = temp;
		zmul(ans, mul, &temp);
		zfree(ans);
		zquo(temp, div, &ans, 0);
		zfree(temp);
	}
	zfree(mul);
	*res = ans;
	return 3;
}

/*
 * Compute the combinatorial function  M! / ( N! * (M - N)! ).
 * Returns 0 if result is 0
*	   1      1
*	   2      z1
*	  -1	  -1
*	  -2	if too complicated
*	   3	result stored at res
 */
int
zcomb(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE z3, z4;
	int r;

	if (z2.sign || (!z1.sign && zrel(z2, z1) > 0))
		return 0;
	if (zisone(z2))
		return 2;
	if (z1.sign) {
		z1.sign = 0;
		zsub(z1, _one_, &z3);
		zadd(z3, z2, &z4);
		zfree(z3);
		r = docomb(z4, z2, res);
		if (r == 2) {
			*res = z4;
			r = 3;
		}
		else
			zfree(z4);
		if (z2.v[0] & 1) {
			if (r == 1)
				r = -1;
			if (r == 3)
				res->sign = 1;
		}
		return r;
	}
	return docomb(z1, z2, res);
}


/*
 * Compute the Jacobi function (m / n) for odd n.
 *
 * The property of the Jacobi function is: If n>2 is prime then
 *
 *	the result is 1 if m == x^2 (mod n) for some x.
 *	otherwise the result is -1.
 *
 * If n is not prime, then the result does not prove that n is not prime
 * when the value of the Jacobi is 1.
 *
 * Jacobi evaluation of (m / n), where n > 0 is odd AND m > 0 is odd:
 *
 *	rule 0:	(0 / n) == 0
 *	rule 1:	(1 / n) == 1
 *	rule 2:	(m / n) == (a / n)	if m == a % n
 *	rule 3:	(m / n) == (2*m / n)	if n == 1 % 8 OR n == 7 % 8
 *	rule 4:	(m / n) == -(2*m / n)	if n != 1 & 8 AND n != 7 % 8
 *	rule 5:	(m / n) == (n / m)	if m == 3 % 4 AND n == 3 % 4
 *	rule 6:	(m / n) == -(n / m)	if m != 3 % 4 OR n != 3 % 4
 *
 * NOTE: This function returns 0 in invalid Jacobi parameters:
 *	 m < 0 OR n is even OR n < 1.
 */
FLAG
zjacobi(ZVALUE z1, ZVALUE z2)
{
	ZVALUE p, q, tmp;
	long lowbit;
	int val;

	/* firewall */
	if (ziszero(z1) || zisneg(z1))
		return 0;
	if (ziseven(z2) || zisneg(z2))
		return 0;

	/* assume a value of 1 unless we find otherwise */
	if (zisone(z1))
		return 1;
	val = 1;
	zcopy(z1, &p);
	zcopy(z2, &q);
	for (;;) {
		zmod(p, q, &tmp, 0);
		zfree(p);
		p = tmp;
		if (ziszero(p)) {
			zfree(p);
			zfree(q);
			return 0;
		}
		if (ziseven(p)) {
			lowbit = zlowbit(p);
			zshift(p, -lowbit, &tmp);
			zfree(p);
			p = tmp;
			if ((lowbit & 1) && (((*q.v & 0x7) == 3) ||
			    ((*q.v & 0x7) == 5)))
				val = -val;
		}
		if (zisunit(p)) {
			zfree(p);
			zfree(q);
			return val;
		}
		if ((*p.v & *q.v & 0x3) == 3)
			val = -val;
		tmp = q;
		q = p;
		p = tmp;
	}
}


/*
 * Return the Fibonacci number F(n).
 * This is evaluated by recursively using the formulas:
 *	F(2N+1) = F(N+1)^2 + F(N)^2
 * and
 *	F(2N) = F(N+1)^2 - F(N-1)^2
 */
void
zfib(ZVALUE z, ZVALUE *res)
{
	long n;
	int sign;
	ZVALUE fnm1, fn, fnp1;		/* consecutive Fibonacci values */
	ZVALUE t1, t2, t3;
	FULL i;

	if (zge31b(z)) {
		math_error("Very large Fibonacci number");
		/*NOTREACHED*/
	}
	n = ztolong(z);
	if (n == 0) {
		*res = _zero_;
		return;
	}
	sign = z.sign && ((n & 0x1) == 0);
	if (n <= 2) {
		*res = _one_;
		res->sign = (BOOL)sign;
		return;
	}
	i = TOPFULL;
	while ((i & n) == 0)
		i >>= (FULL)1;
	i >>= (FULL)1;
	fnm1 = _zero_;
	fn = _one_;
	fnp1 = _one_;
	while (i) {
		zsquare(fnm1, &t1);
		zsquare(fn, &t2);
		zsquare(fnp1, &t3);
		zfree(fnm1);
		zfree(fn);
		zfree(fnp1);
		zadd(t2, t3, &fnp1);
		zsub(t3, t1, &fn);
		zfree(t1);
		zfree(t2);
		zfree(t3);
		if (i & n) {
			fnm1 = fn;
			fn = fnp1;
			zadd(fnm1, fn, &fnp1);
		} else {
			zsub(fnp1, fn, &fnm1);
		}
		i >>= (FULL)1;
	}
	zfree(fnm1);
	zfree(fnp1);
	*res = fn;
	res->sign = (BOOL)sign;
}


/*
 * Compute the result of raising one number to the power of another
 * The second number is assumed to be non-negative.
 * It cannot be too large except for trivial cases.
 */
void
zpowi(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	int sign;		/* final sign of number */
	unsigned long power;	/* power to raise to */
	FULL bit;		/* current bit value */
	long twos;		/* count of times 2 is in result */
	ZVALUE ans, temp;

	sign = (z1.sign && zisodd(z2));
	z1.sign = 0;
	z2.sign = 0;
	if (ziszero(z2) && !ziszero(z1)) {	/* number raised to power 0 */
		*res = _one_;
		return;
	}
	if (zisabsleone(z1)) {	/* 0, 1, or -1 raised to a power */
		ans = _one_;
		ans.sign = (BOOL)sign;
		if (*z1.v == 0)
			ans = _zero_;
		*res = ans;
		return;
	}
	if (zge31b(z2)) {
		math_error("Raising to very large power");
		/*NOTREACHED*/
	}
	power = ztoulong(z2);
	if (zistwo(z1)) {	/* two raised to a power */
		zbitvalue((long) power, res);
		return;
	}
	/*
	 * See if this is a power of ten
	 */
	if (zistiny(z1) && (*z1.v == 10)) {
		ztenpow((long) power, res);
		res->sign = (BOOL)sign;
		return;
	}
	/*
	 * Handle low powers specially
	 */
	if (power <= 4) {
		switch ((int) power) {
		case 1:
			ans.len = z1.len;
			ans.v = alloc(ans.len);
			zcopyval(z1, ans);
			ans.sign = (BOOL)sign;
			*res = ans;
			return;
		case 2:
			zsquare(z1, res);
			return;
		case 3:
			zsquare(z1, &temp);
			zmul(z1, temp, res);
			zfree(temp);
			res->sign = (BOOL)sign;
			return;
		case 4:
			zsquare(z1, &temp);
			zsquare(temp, res);
			zfree(temp);
			return;
		}
	}
	/*
	 * Shift out all powers of twos so the multiplies are smaller.
	 * We will shift back the right amount when done.
	 */
	twos = 0;
	if (ziseven(z1)) {
		twos = zlowbit(z1);
		ans.v = alloc(z1.len);
		ans.len = z1.len;
		ans.sign = z1.sign;
		zcopyval(z1, ans);
		zshiftr(ans, twos);
		ztrim(&ans);
		z1 = ans;
		twos *= power;
	}
	/*
	 * Compute the power by squaring and multiplying.
	 * This uses the left to right method of power raising.
	 */
	bit = TOPFULL;
	while ((bit & power) == 0)
		bit >>= 1;
	bit >>= 1;
	zsquare(z1, &ans);
	if (bit & power) {
		zmul(ans, z1, &temp);
		zfree(ans);
		ans = temp;
	}
	bit >>= 1;
	while (bit) {
		zsquare(ans, &temp);
		zfree(ans);
		ans = temp;
		if (bit & power) {
			zmul(ans, z1, &temp);
			zfree(ans);
			ans = temp;
		}
		bit >>= 1;
	}
	/*
	 * Scale back up by proper power of two
	 */
	if (twos) {
		zshift(ans, twos, &temp);
		zfree(ans);
		ans = temp;
		zfree(z1);
	}
	ans.sign = (BOOL)sign;
	*res = ans;
}


/*
 * Compute ten to the specified power
 * This saves some work since the squares of ten are saved.
 */
void
ztenpow(long power, ZVALUE *res)
{
	long i;
	ZVALUE ans;
	ZVALUE temp;

	if (power <= 0) {
		*res = _one_;
		return;
	}
	ans = _one_;
	_tenpowers_[0] = _ten_;
	for (i = 0; power; i++) {
		if (_tenpowers_[i].len == 0) {
			if (i <= TEN_MAX) {
				zsquare(_tenpowers_[i-1], &_tenpowers_[i]);
			} else {
				math_error("cannot compute 10^2^(TEN_MAX+1)");
				/*NOTREACHED*/
			}
		}
		if (power & 0x1) {
			zmul(ans, _tenpowers_[i], &temp);
			zfree(ans);
			ans = temp;
		}
		power /= 2;
	}
	*res = ans;
}


/*
 * Calculate modular inverse suppressing unnecessary divisions.
 * This is based on the Euclidean algorithm for large numbers.
 * (Algorithm X from Knuth Vol 2, section 4.5.2. and exercise 17)
 * Returns TRUE if there is no solution because the numbers
 * are not relatively prime.
 */
BOOL
zmodinv(ZVALUE u, ZVALUE v, ZVALUE *res)
{
	FULL	q1, q2, ui3, vi3, uh, vh, A, B, C, D, T;
	ZVALUE	u2, u3, v2, v3, qz, tmp1, tmp2, tmp3;

	v.sign = 0;
	if (zisneg(u) || (zrel(u, v) >= 0))
		zmod(u, v, &v3, 0);
	else
		zcopy(u, &v3);
	zcopy(v, &u3);
	u2 = _zero_;
	v2 = _one_;

	/*
	 * Loop here while the size of the numbers remain above
	 * the size of a HALF.	Throughout this loop u3 >= v3.
	 */
	while ((u3.len > 1) && !ziszero(v3)) {
		vh = 0;
#if LONG_BITS == BASEB
		uh =  u3.v[u3.len - 1];
		if (v3.len == u3.len)
			vh = v3.v[v3.len - 1];
#else
		uh = (((FULL) u3.v[u3.len - 1]) << BASEB) + u3.v[u3.len - 2];
		if ((v3.len + 1) >= u3.len)
			vh = v3.v[v3.len - 1];
		if (v3.len == u3.len)
			vh = (vh << BASEB) + v3.v[v3.len - 2];
#endif
		A = 1;
		B = 0;
		C = 0;
		D = 1;

		/*
		 * Calculate successive quotients of the continued fraction
		 * expansion using only single precision arithmetic until
		 * greater precision is required.
		 */
		while ((vh + C) && (vh + D)) {
			q1 = (uh + A) / (vh + C);
			q2 = (uh + B) / (vh + D);
			if (q1 != q2)
				break;
			T = A - q1 * C;
			A = C;
			C = T;
			T = B - q1 * D;
			B = D;
			D = T;
			T = uh - q1 * vh;
			uh = vh;
			vh = T;
		}

		/*
		 * If B is zero, then we made no progress because
		 * the calculation requires a very large quotient.
		 * So we must do this step of the calculation in
		 * full precision
		 */
		if (B == 0) {
			zquo(u3, v3, &qz, 0);
			zmul(qz, v2, &tmp1);
			zsub(u2, tmp1, &tmp2);
			zfree(tmp1);
			zfree(u2);
			u2 = v2;
			v2 = tmp2;
			zmul(qz, v3, &tmp1);
			zsub(u3, tmp1, &tmp2);
			zfree(tmp1);
			zfree(u3);
			u3 = v3;
			v3 = tmp2;
			zfree(qz);
			continue;
		}
		/*
		 * Apply the calculated A,B,C,D numbers to the current
		 * values to update them as if the full precision
		 * calculations had been carried out.
		 */
		zmuli(u2, (long) A, &tmp1);
		zmuli(v2, (long) B, &tmp2);
		zadd(tmp1, tmp2, &tmp3);
		zfree(tmp1);
		zfree(tmp2);
		zmuli(u2, (long) C, &tmp1);
		zmuli(v2, (long) D, &tmp2);
		zfree(u2);
		zfree(v2);
		u2 = tmp3;
		zadd(tmp1, tmp2, &v2);
		zfree(tmp1);
		zfree(tmp2);
		zmuli(u3, (long) A, &tmp1);
		zmuli(v3, (long) B, &tmp2);
		zadd(tmp1, tmp2, &tmp3);
		zfree(tmp1);
		zfree(tmp2);
		zmuli(u3, (long) C, &tmp1);
		zmuli(v3, (long) D, &tmp2);
		zfree(u3);
		zfree(v3);
		u3 = tmp3;
		zadd(tmp1, tmp2, &v3);
		zfree(tmp1);
		zfree(tmp2);
	}

	/*
	 * Here when the remaining numbers become single precision in size.
	 * Finish the procedure using single precision calculations.
	 */
	if (ziszero(v3) && !zisone(u3)) {
		zfree(u3);
		zfree(v3);
		zfree(u2);
		zfree(v2);
		return TRUE;
	}
	ui3 = ztofull(u3);
	vi3 = ztofull(v3);
	zfree(u3);
	zfree(v3);
	while (vi3) {
		q1 = ui3 / vi3;
		zmuli(v2, (long) q1, &tmp1);
		zsub(u2, tmp1, &tmp2);
		zfree(tmp1);
		zfree(u2);
		u2 = v2;
		v2 = tmp2;
		q2 = ui3 - q1 * vi3;
		ui3 = vi3;
		vi3 = q2;
	}
	zfree(v2);
	if (ui3 != 1) {
		zfree(u2);
		return TRUE;
	}
	if (zisneg(u2)) {
		zadd(v, u2, res);
		zfree(u2);
		return FALSE;
	}
	*res = u2;
	return FALSE;
}


/*
 * Compute the greatest common divisor of a pair of integers.
 */
void
zgcd(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	int h, i, j, k;
	LEN len, l, m, n, o, p, q;
	HALF u, v, w, x;
	HALF *a, *a0, *A, *b, *b0, *B, *c, *d;
	FULL f, g;
	ZVALUE gcd;
	BOOL needw;

	if (zisunit(z1) || zisunit(z2)) {
		*res = _one_;
		return;
	}
	z1.sign = 0;
	z2.sign = 0;
	if (ziszero(z1) || !zcmp(z1, z2)) {
		zcopy(z2, res);
		return;
	}
	if (ziszero(z2)) {
		zcopy(z1, res);
		return;
	}

	o = 0;
	while (!(z1.v[o] | z2.v[o])) o++;	/* Count common zero digits */

	c = z1.v + o;
	d = z2.v + o;

	m = z1.len - o;
	n = z2.len - o;
	u = *c | *d;			/* Count common zero bits */
	v = 1;
	p = 0;
	while (!(u & v)) {
		v <<= 1;
		p++;
	}

	while (!*c) {				/* Removing zero digits */
		c++;
		m--;
	}

	while (!*d) {
		d++;
		n--;
	}


	u = *d;			/* Count zero bits for *d */
	v = 1;
	q = 0;
	while (!(u & v)) {
		v <<= 1;
		q++;
	}

	a0 = A = alloc(m);
	b0 = B = alloc(n);

	memcpy(A, c, m * sizeof(HALF));			/* Copy c[] to A[] */

				/* Copy d[] to B[], shifting if necessary */
	if (q) {
		i = n;
		b = B + n;
		d += n;
		f = 0;
		while (i--) {
			f = f << BASEB | *--d;
			*--b = (HALF) (f >> q);
		}
		if (B[n-1] == 0) n--;
	}
	else memcpy(B, d, n * sizeof(HALF));

	if (n == 1) {		/* One digit case; use Euclid's algorithm */
		n = m;
		b0 = A;
		m = 1;
		a0 = B;
		if (m == 1) {				/* a has one digit */
			v = *a0;
			if (v > 1) {			/* Euclid's algorithm */
				b = b0 + n;
				i = n;
				u = 0;
				while (i--) {
					f = (FULL) u << BASEB | *--b;
					u = (HALF) (f % v);
				}
				while (u) { w = v % u; v = u; u = w; }
			}
			*b0 = v;
			n = 1;
		}
		len = n + o;
		gcd.v = alloc(len + 1);
		/* Common zero digits */
		if (o) memset(gcd.v, 0, o * sizeof(HALF));
		/* Left shift for common zero bits */
		if (p) {
			i = n;
			f = 0;
			b = b0;
			a = gcd.v + o;
			while (i--) {
				f = f >> BASEB | (FULL) *b++ << p;
				*a++ = (HALF) f;
			}
			if (f >>= BASEB) {len++; *a = (HALF) f;}
		} else {
			memcpy(gcd.v + o, b0, n * sizeof(HALF));
		}
		gcd.len = len;
		gcd.sign = 0;
		freeh(A);
		freeh(B);
		*res = gcd;
		return;
	}

	u = B[n-1];				/* Bit count for b */
	k = (n	- 1) * BASEB;
	while (u >>= 1) k++;

	needw = TRUE;

	w = 0;
	j = 0;
	while (m) {				/* START OF MAIN LOOP */
	   if (m - n < 2 || needw) {
		q = 0;
		u = *a0;
		v = 1;
		while (!(u & v)) {		/* count zero bits for *a0 */
			q++;
			v <<= 1;
		}

		if (q) {			/* right-justify a */
			a = a0 + m;
			i = m;
			f = 0;
			while (i--) {
				f = f << BASEB | *--a;
				*a = (HALF) (f >> q);
			}
			if (!a0[m-1]) m--;	/* top digit vanishes */
		}

		if (m == 1) break;

		u = a0[m-1];
		j = (m - 1) * BASEB;
		while (u >>= 1) j++;		/* counting bits for a */
		h = j - k;
		if (h < 0) {			/* swapping to get h > 0 */
			l = m;
			m = n;
			n = l;
			a = a0;
			a0 = b0;
			b0 = a;
			k = j;
			h = -h;
			needw = TRUE;
		}
		if (h > 1) {
			if (needw) {		/* find w = minv(*b0, h0) */
				u = 1;
				v = *b0;
				w = 0;
				x = 1;
				i = h;
				while (i-- && x) {
					if (u & x) { u -= v * x; w |= x;}
					x <<= 1;
				}
				needw = FALSE;
			}
			g = *a0 * w;
			if (h < BASEB) {
			    g &= (1 << h) - 1;
			} else {
			    g &= BASE1;
			}
		} else {
			g = 1;
		}
	    } else {
		g = (HALF) *a0 * w;
	    }
	    a = a0;
	    b = b0;
	    i = n;
	    if (g > 1) {			/* a - g * b case */
		    f = 0;
		    while (i--) {
			    f = (FULL) *a - g * *b++ - f;
			    *a++ = (HALF) f;
			    f >>= BASEB;
			    f = -f & BASE1;
		    }
		    if (f) {
			    i = m - n;
			    while (i-- && f) {
				    f = *a - f;
				    *a++ = (HALF) f;
				    f >>= BASEB;
				    f = -f & BASE1;
			    }
		    }
		    while (m && !*a0) { /* Removing trailing zeros */
			    m--;
			    a0++;
		    }
		    if (f) {		/* a - g * b < 0 */
			    while (m > 1 && a0[m-1] == BASE1) m--;
			    *a0 = - *a0;
			    a = a0;
			    i = m;
			    while (--i) {
				    a++;
				    *a = ~*a;
			    }
		    }
	    } else {				/* abs(a - b) case */
		    while (i && *a++ == *b++) i--;
		    q = n - i;
		    if (m == n) {		/* a and b same length */
			    if (i) {	 /* a not equal to b */
				    while (m && a0[m-1] == b0[m-1]) m--;
				    if (a0[m-1] < b0[m-1]) {
					    /* Swapping since a < b */
					    a = a0;
					    a0 = b0;
					    b0 = a;
					    k = j;
				    }
				    a = a0 + q;
				    b = b0 + q;
				    i = m - q;
				    f = 0;
				    while (i--) {
					    f = (FULL) *a - *b++ - f;
					    *a++ = (HALF) f;
					    f >>= BASEB;
					    f = -f & BASE1;
				    }
			    }
		    } else {		/* a has more digits than b */
			    a = a0 + q;
			    b = b0 + q;
			    i = n - q;
			    f = 0;
			    while (i--) {
				    f = (FULL) *a - *b++ - f;
				    *a++ = (HALF) f;
				    f >>= BASEB;
				    f = -f & BASE1;
			    }
			    if (f) { while (!*a) *a++ = BASE1;
				    (*a)--;
			    }
		    }
		    a0 += q;
		    m -= q;
		    while (m && !*a0) { /* Removing trailing zeros */
			    m--;
			    a0++;
		    }
	    }
	    while (m && !a0[m-1]) m--;	/* Removing leading zeros */
	}
	if (m == 1) {				/* a has one digit */
		v = *a0;
		if (v > 1) {			/* Euclid's algorithm */
			b = b0 + n;
			i = n;
			u = 0;
			while (i--) {
				f = (FULL) u << BASEB | *--b;
				u = (HALF) (f % v);
			}
			while (u) { w = v % u; v = u; u = w; }
		}
		*b0 = v;
		n = 1;
	}
	len = n + o;
	gcd.v = alloc(len + 1);
	if (o) memset(gcd.v, 0, o * sizeof(HALF));	/* Common zero digits */
	if (p) {			/* Left shift for common zero bits */
		i = n;
		f = 0;
		b = b0;
		a = gcd.v + o;
		while (i--) {
			f = (FULL) *b++ << p | f;
			*a++ = (HALF) f;
			f >>= BASEB;
		}
		if (f) {
			len++; *a = (HALF) f;
		}
	} else {
		memcpy(gcd.v + o, b0, n * sizeof(HALF));
	}
	gcd.len = len;
	gcd.sign = 0;
	freeh(A);
	freeh(B);
	*res = gcd;
	return;
}

/*
 * Compute the lcm of two integers (least common multiple).
 * This is done using the formula:  gcd(a,b) * lcm(a,b) = a * b.
 */
void
zlcm(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE temp1, temp2;

	zgcd(z1, z2, &temp1);
	zequo(z1, temp1, &temp2);
	zfree(temp1);
	zmul(temp2, z2, res);
	zfree(temp2);
}


/*
 * Return whether or not two numbers are relatively prime to each other.
 */
BOOL
zrelprime(ZVALUE z1, ZVALUE z2)
{
	FULL rem1, rem2;		/* remainders */
	ZVALUE rem;
	BOOL result;

	z1.sign = 0;
	z2.sign = 0;
	if (ziseven(z1) && ziseven(z2)) /* false if both even */
		return FALSE;
	if (zisunit(z1) || zisunit(z2)) /* true if either is a unit */
		return TRUE;
	if (ziszero(z1) || ziszero(z2)) /* false if either is zero */
		return FALSE;
	if (zistwo(z1) || zistwo(z2))	/* true if either is two */
		return TRUE;
	/*
	 * Try reducing each number by the product of the first few odd primes
	 * to see if any of them are a common factor.
	 */
	rem1 = zmodi(z1, (FULL)3 * 5 * 7 * 11 * 13);
	rem2 = zmodi(z2, (FULL)3 * 5 * 7 * 11 * 13);
	if (((rem1 % 3) == 0) && ((rem2 % 3) == 0))
		return FALSE;
	if (((rem1 % 5) == 0) && ((rem2 % 5) == 0))
		return FALSE;
	if (((rem1 % 7) == 0) && ((rem2 % 7) == 0))
		return FALSE;
	if (((rem1 % 11) == 0) && ((rem2 % 11) == 0))
		return FALSE;
	if (((rem1 % 13) == 0) && ((rem2 % 13) == 0))
		return FALSE;
	/*
	 * Try a new batch of primes now
	 */
	rem1 = zmodi(z1, (FULL)17 * 19 * 23);
	rem2 = zmodi(z2, (FULL)17 * 19 * 23);
	if (((rem1 % 17) == 0) && ((rem2 % 17) == 0))
		return FALSE;
	if (((rem1 % 19) == 0) && ((rem2 % 19) == 0))
		return FALSE;
	if (((rem1 % 23) == 0) && ((rem2 % 23) == 0))
		return FALSE;
	/*
	 * Yuk, we must actually compute the gcd to know the answer
	 */
	zgcd(z1, z2, &rem);
	result = zisunit(rem);
	zfree(rem);
	return result;
}


/*
 * Compute the integer floor of the log of an integer to a specified base.
 * The signs of the integers and base are ignored.
 * Example:  zlog(123456, 10) = 5.
 */
long
zlog(ZVALUE z, ZVALUE base)
{
	ZVALUE *zp;			/* current square */
	long power;			/* current power */
	ZVALUE temp;			/* temporary */
	ZVALUE squares[32];		/* table of squares of base */

	/* ignore signs */

	z.sign = 0;
	base.sign = 0;

	/*
	 * Make sure that the numbers are nonzero and the base is > 1
	 */
	if (ziszero(z) || ziszero(base) || zisone(base)) {
		math_error("Zero or too small argument argument for zlog!!!");
		/*NOTREACHED*/
	}

	/*
	 * Some trivial cases.
	 */
	power = zrel(z, base);
	if (power <= 0)
		return (power + 1);

	/* base - power of two */
	if (zisonebit(base))
		return (zhighbit(z) / zlowbit(base));

	/* base = 10 */
	if (base.len == 1 && base.v[0] == 10)
		return zlog10(z, NULL);
	/*
	 * Now loop by squaring the base each time, and see whether or
	 * not each successive square is still smaller than the number.
	 */
	zp = &squares[0];
	*zp = base;
	while (zp->len * 2 - 1 <= z.len  && zrel(z, *zp) > 0) {
		/* while square not too large */
		zsquare(*zp, zp + 1);
		zp++;
	}

	/*
	 * Now back down the squares,
	 */
	power = 0;
	for (; zp > squares; zp--) {
		if (zrel(z, *zp) >= 0) {
			zquo(z, *zp, &temp, 0);
			if (power)
				zfree(z);
			z = temp;
			power++;
		}
		zfree(*zp);
		power <<= 1;
	}
	if (zrel(z, *zp) >= 0)
		power++;
	if (power > 1)
		zfree(z);
	return power;
}


/*
 * Return the integral log base 10 of a number.
 *
 * If was_10_power != NULL, then this flag is set to TRUE if the
 * value was a power of 10, FALSE otherwise.
 */
long
zlog10(ZVALUE z, BOOL *was_10_power)
{
	ZVALUE *zp;			/* current square */
	long power;			/* current power */
	ZVALUE temp;			/* temporary */
	ZVALUE pow10;			/* power of 10 */
	FLAG rel;			/* relationship */
	int i;

	if (ziszero(z)) {
		math_error("Zero argument argument for zlog10");
		/*NOTREACHED*/
	}

	/* Ignore sign of z */
	z.sign = 0;

	/* preload power10 table if missing */
	if (power10 == NULL) {
		long v;

		/* determine power10 table size */
		for (v=1, max_power10_exp=0;
		     v <= (long)(MAXLONG/10L);
		     v *= 10L, ++max_power10_exp) {
		}

		/* create power10 table */
		power10 = malloc(sizeof(long) * (max_power10_exp+1));
		if (power10 == NULL) {
			math_error("cannot malloc power10 table");
			/*NOTREACHED*/
		}

		/* load power10 table */
		for (i=0, v = 1L; i <= max_power10_exp; ++i, v *= 10L) {
			power10[i] = v;
		}
	}

	/* assume not a power of ten unless we find out otherwise */
	if (was_10_power != NULL) {
		*was_10_power = FALSE;
	}

	/* quick exit for small values */
	if (! zgtmaxlong(z)) {
		long value = ztolong(z);

		for (i=0; i <= max_power10_exp; ++i) {
			if (value == power10[i]) {
				if (was_10_power != NULL) {
					*was_10_power = TRUE;
				}
				return i;
			} else if (value < power10[i]) {
				return i-1;
			}
		}
	}

	/*
	 * Loop by squaring the base each time, and see whether or
	 * not each successive square is still smaller than the number.
	 */
	zp = &_tenpowers_[0];
	*zp = _ten_;
	while (((zp->len * 2) - 1) <= z.len) {	/* while square not too large */
		if (zp >= &_tenpowers_[TEN_MAX]) {
			math_error("Maximum storable power of 10 reached!");
			/*NOTREACHED*/
		}
		if (zp[1].len == 0)
			zsquare(*zp, zp + 1);
		zp++;
	}

	/*
	 * Now back down the squares, and multiply them together to see
	 * exactly how many times the base can be raised by.
	 */
	/* find the tenpower table entry < z */
	do {
		rel = zrel(*zp, z);
		if (rel == 0) {
			/* quick exit - we match a tenpower entry */
			if (was_10_power != NULL) {
				*was_10_power = TRUE;
			}
			return (1L << (zp - _tenpowers_));
		}
	} while (rel > 0 && --zp >= _tenpowers_);
	if (zp < _tenpowers_) {
		math_error("fell off bottom of tenpower table!");
		/*NOTREACHED*/
	}

	/* the tenpower value is now our starting comparison value */
	zcopy(*zp, &pow10);
	power = (1L << (zp - _tenpowers_));

	/* try to build up a power of 10 from tenpower table entries */
	while (--zp >= _tenpowers_) {

		/* try the next lower tenpower value */
		zmul(pow10, *zp, &temp);
		rel = zrel(temp, z);
		if (rel == 0) {
			/* exact power of 10 match */
			power += (1L << (zp - _tenpowers_));
			if (was_10_power != NULL) {
				*was_10_power = TRUE;
			}
			return power;

		/* ignore this entry if we went too large */
		} else if (rel > 0) {
			zfree(temp);

		/* otherwise increase power and keep going */
		} else {
			power += (1L << (zp - _tenpowers_));
			zfree(pow10);
			pow10 = temp;
		}
	}
	return power;
}


/*
 * Return the number of times that one number will divide another.
 * This works similarly to zlog, except that divisions must be exact.
 * For example, zdivcount(540, 3) = 3, since 3^3 divides 540 but 3^4 won't.
 */
long
zdivcount(ZVALUE z1, ZVALUE z2)
{
	long count;		/* number of factors removed */
	ZVALUE tmp;		/* ignored return value */

	if (ziszero(z1) || ziszero(z2) || zisunit(z2))
		return 0;
	count = zfacrem(z1, z2, &tmp);
	zfree(tmp);
	return count;
}


/*
 * Remove all occurrences of the specified factor from a number.
 * Also returns the number of factors removed as a function return value.
 * Example:  zfacrem(540, 3, &x) returns 3 and sets x to 20.
 */
long
zfacrem(ZVALUE z1, ZVALUE z2, ZVALUE *rem)
{
	register ZVALUE *zp;		/* current square */
	long count;			/* total count of divisions */
	long worth;			/* worth of current square */
	long lowbit;			/* for zlowbit(z2) */
	ZVALUE temp1, temp2, temp3;	/* temporaries */
	ZVALUE squares[32];		/* table of squares of factor */

	z1.sign = 0;
	z2.sign = 0;
	/*
	 * Reject trivial cases.
	 */
	if ((z1.len < z2.len) || (zisodd(z1) && ziseven(z2)) ||
		ziszero(z2) || zisone(z2) ||
		((z1.len == z2.len) && (z1.v[z1.len-1] < z2.v[z2.len-1]))) {
		rem->v = alloc(z1.len);
		rem->len = z1.len;
		rem->sign = 0;
		zcopyval(z1, *rem);
		return 0;
	}
	/*
	 * Handle any power of two special.
	 */
	if (zisonebit(z2)) {
		lowbit = zlowbit(z2);
		count = zlowbit(z1) / lowbit;
		rem->v = alloc(z1.len);
		rem->len = z1.len;
		rem->sign = 0;
		zcopyval(z1, *rem);
		zshiftr(*rem, count * lowbit);
		ztrim(rem);
		return count;
	}
	/*
	 * See if the factor goes in even once.
	 */
	zdiv(z1, z2, &temp1, &temp2, 0);
	if (!ziszero(temp2)) {
		zfree(temp1);
		zfree(temp2);
		rem->v = alloc(z1.len);
		rem->len = z1.len;
		rem->sign = 0;
		zcopyval(z1, *rem);
		return 0;
	}
	zfree(temp2);
	z1 = temp1;
	/*
	 * Now loop by squaring the factor each time, and see whether
	 * or not each successive square will still divide the number.
	 */
	count = 1;
	worth = 1;
	zp = &squares[0];
	*zp = z2;
	while (((zp->len * 2) - 1) <= z1.len) { /* while square not too large */
		zsquare(*zp, &temp1);
		zdiv(z1, temp1, &temp2, &temp3, 0);
		if (!ziszero(temp3)) {
			zfree(temp1);
			zfree(temp2);
			zfree(temp3);
			break;
		}
		zfree(temp3);
		zfree(z1);
		z1 = temp2;
		*++zp = temp1;
		worth *= 2;
		count += worth;
	}

	/*
	 * Now back down the list of squares, and see if the lower powers
	 * will divide any more times.
	 */
	/*
	 * We prevent the zp pointer from walking behind squares
	 * by stopping one short of the end and running the loop one
	 * more time.
	 *
	 * We could stop the loop with just zp >= squares, but stopping
	 * short and running the loop one last time manually helps make
	 * code checkers such as insure happy.
	 */
	for (; zp > squares; zp--, worth /= 2) {
		if (zp->len <= z1.len) {
			zdiv(z1, *zp, &temp1, &temp2, 0);
			if (ziszero(temp2)) {
				temp3 = z1;
				z1 = temp1;
				temp1 = temp3;
				count += worth;
			}
			zfree(temp1);
			zfree(temp2);
		}
		if (zp != squares)
			zfree(*zp);
	}
	/* run the loop manually one last time */
	if (zp == squares) {
		if (zp->len <= z1.len) {
			zdiv(z1, *zp, &temp1, &temp2, 0);
			if (ziszero(temp2)) {
				temp3 = z1;
				z1 = temp1;
				temp1 = temp3;
				count += worth;
			}
			zfree(temp1);
			zfree(temp2);
		}
		if (zp != squares)
			zfree(*zp);
	}

	*rem = z1;
	return count;
}


/*
 * Keep dividing a number by the gcd of it with another number until the
 * result is relatively prime to the second number.  Returns the number
 * of divisions made, and if this is positive, stores result at res.
 */
long
zgcdrem(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
	ZVALUE tmp1, tmp2;
	long count, onecount;
	long sh;

	if (ziszero(z1) || ziszero(z2)) {
		math_error("Zero argument in call to zgcdrem!!!");
		/*NOTREACHED*/
	}
	/*
	 * Begin by taking the gcd for the first time.
	 * If the number is already relatively prime, then we are done.
	 */
	z1.sign = 0;
	z2.sign = 0;
	if (zisone(z2))
		return 0;
	if (zisonebit(z2)) {
		sh = zlowbit(z1);
		if (sh == 0)
			return 0;
		zshift(z1, -sh, res);
		return 1 + (sh - 1)/zlowbit(z2);
	}
	if (zisonebit(z1)) {
		if (zisodd(z2))
			return 0;
		*res = _one_;
		return zlowbit(z1);
	}

	zgcd(z1, z2, &tmp1);
	if (zisunit(tmp1) || ziszero(tmp1))
		return 0;
	zequo(z1, tmp1, &tmp2);
	count = 1;
	z1 = tmp2;
	z2 = tmp1;
	/*
	 * Now keep alternately taking the gcd and removing factors until
	 * the gcd becomes one.
	 */
	while (!zisunit(z2)) {
		onecount = zfacrem(z1, z2, &tmp1);
		if (onecount) {
			count += onecount;
			zfree(z1);
			z1 = tmp1;
		}
		zgcd(z1, z2, &tmp1);
		zfree(z2);
		z2 = tmp1;
	}
	*res = z1;
	return count;
}


/*
 * Return the number of digits (base 10) in a number, ignoring the sign.
 */
long
zdigits(ZVALUE z1)
{
	long count, val;

	z1.sign = 0;
	if (!zge16b(z1)) {	/* do small numbers ourself */
		count = 1;
		val = 10;
		while (*z1.v >= (HALF)val) {
			count++;
			val *= 10;
		}
		return count;
	}
	return (zlog10(z1, NULL) + 1);
}


/*
 * Return the single digit at the specified decimal place of a number,
 * where 0 means the rightmost digit.  Example:	 zdigit(1234, 1) = 3.
 */
long
zdigit(ZVALUE z1, long n)
{
	ZVALUE tmp1, tmp2;
	long res;

	z1.sign = 0;
	if (ziszero(z1) || (n < 0) || (n / BASEDIG >= z1.len))
		return 0;
	if (n == 0)
		return zmodi(z1, 10L);
	if (n == 1)
		return zmodi(z1, 100L) / 10;
	if (n == 2)
		return zmodi(z1, 1000L) / 100;
	if (n == 3)
		return zmodi(z1, 10000L) / 1000;
	ztenpow(n, &tmp1);
	zquo(z1, tmp1, &tmp2, 0);
	res = zmodi(tmp2, 10L);
	zfree(tmp1);
	zfree(tmp2);
	return res;
}


/*
 * z is to be a nonnegative integer
 * If z is the square of a integer stores at dest the square root of z;
 *	otherwise stores at z an integer differing from the square root
 *	by less than 1.	 Returns the sign of the true square root minus
 *	the calculated integer.	 Type of rounding is determined by
 *	rnd as follows: rnd = 0 gives round down, rnd = 1
 *	rounds up, rnd = 8 rounds to even integer, rnd = 9 rounds to odd
 *	integer, rnd = 16 rounds to nearest integer.
 */
FLAG
zsqrt(ZVALUE z, ZVALUE *dest, long rnd)
{
	HALF *a, *A, *b, *a0, u;
	int i, j, j1, j2, k, k1, m, m0, m1, n, n0, o;
	FULL d, e, f, g, h, s, t, x, topbit;
	int remsign;
	BOOL up, onebit;
	ZVALUE sqrt;

	if (z.sign) {
		math_error("Square root of negative number");
		/*NOTREACHED*/
	}
	if (ziszero(z)) {
		*dest = _zero_;
		return 0;
	}
	m0 = z.len;
	o = m0 & 1;
	m = m0 + o;		/* m is smallest even number >= z.len */
	n0 = n = m / 2;
	f = z.v[z.len - 1];
	k = 1;
	while (f >>= 2)
		k++;
	if (!o)
		k += BASEB/2;
	j = BASEB - k;
	m1 = m;
	if (k == BASEB) {
		m1 += 2;
		n0++;
	}
	A = alloc(m1);
	A[m1] = 0;
	a0 = A + n0;
	memcpy(A, z.v, m0 * sizeof(HALF));
	if (o)
		A[m - 1] = 0;
	if (n == 1) {
		if (j)
			f = (FULL) A[1] << j | A[0] >> k;
		else
			f = A[1];
		g = (FULL) A[0] << (j + BASEB);
		d = e = topbit = (FULL)1 << (k - 1);
	} else {
		if (j)
			f = (FULL) A[m-1] << (j + BASEB) | (FULL) A[m-2] << j |
				A[m-3] >> k;
		else
			f = (FULL) A[m-1] << BASEB | A[m-2];
		g = (FULL) A[m-3] << (j + BASEB) | (FULL) A[m-4] << j;
		d = e = topbit = (FULL)1 << (BASEB + k - 1);
	}

	s = (f & topbit);
	f <<= 1;
	if (g & TOPFULL)
		f++;
	g <<= 1;
	if (s) {
		f -= 4 * d;
		e = 2 * d - 1;
	}
	else
		f -= d;
	while (d >>= 1) {
		if (!(s | f | g))
			break;
		while (d && (f & topbit) == s) {
			d >>= 1;
			f <<= 1;
			if (g & TOPFULL)
				f++;
			g <<= 1;
		}
		if (d == 0)
			break;
		if (s)
			f += e + 1;
		else
			f -= e;
		t = f & topbit;
		f <<= 1;
		if (g & TOPFULL)
			f++;
		g <<= 1;
		if (t == 0 && f < d)
			t = topbit;
		f -= d;
		if (s)
			e -= d - !t;
		else
			e += d - (t > 0);
		s = t;
	}
	if (n0 == 1) {
		A[1] = (HALF)e;
		A[0] = (HALF)f;
		m = 1;
		goto done;
	}
	if (n0 == 2) {
		A[3] = (HALF)(e >> BASEB);
		A[2] = (HALF)e;
		A[1] = (HALF)(f >> BASEB);
		A[0] = (HALF)f;
		m = 2;
		goto done;
	}
	u = (HALF)(s ? BASE1 : 0);
	if (k < BASEB) {
		A[m1 - 1] = (HALF)(e >> (BASEB - 1));
		A[m1 - 2] = ((HALF)(e << 1) | (HALF)(s > 0));
		A[m1 - 3] = (HALF)(f >> BASEB);
		A[m1 - 4] = (HALF)f;
		m = m1 - 2;
		k1 = k + 1;
	} else {
		A[m1 - 1] = 1;
		A[m1 - 2] = (HALF)(e >> (BASEB - 1));
		A[m1 - 3] = ((HALF)(e << 1) | (HALF)(s > 0));
		A[m1 - 4] = u;
		A[m1 - 5] = (HALF)(f >> BASEB);
		A[m1 - 6] = (HALF)f;
		m = m1 - 3;
		k1 = 1;
	}
	h = e >> k;
	onebit = ((e & ((FULL)1 << (k - 1))) ? TRUE : FALSE);
	j2 = BASEB - k1;
	j1 = BASEB + j2;
	while (m > n0) {
		a = A + m - 1;
		if (j2)
			f = (FULL) *a << j1 | (FULL) a[-1] << j2 | a[-2] >> k1;
		else
			f = (FULL) *a << BASEB | a[-1];
		if (u)
			f = ~f;
		x = f / h;
		if (x) {
			if (onebit && x > 2 * (f % h) + 2)
				x--;
			b = a + 1;
			i = m1 - m;
			a -= i + 1;
			if (u) {
				f = *a + x * (BASE - x);
				*a++ = (HALF)f;
				u = (HALF)(f >> BASEB);
				while (i--) {
					f = *a + x * *b++ + u;
					*a++ = (HALF)f;
					u = (HALF)(f >> BASEB);
				}
				u += *a;
				x = ~x + !u;
				if (!(x & TOPHALF))
					a[1] -= 1;
			} else {
				f = *a - x * x;
				*a++ = (HALF)f;
				u = -(HALF)(f >> BASEB);
				while (i--) {
					f = *a - x * *b++ - u;
					*a++ = (HALF)f;
					u = -(HALF)(f >> BASEB);
				}
				u = *a - u;
				x = x + u;
				if (x & TOPHALF)
					a[1] |= 1;
			}
			*a = ((HALF)(x << 1) | (HALF)(u > 0));
		} else {
			*a = u;
		}
		m--;
		if (*--a == u) {
			while (m > 1 && *--a == u)
				m--;
		}
	}
	i = n;
	a = a0;
	while (i--) {
		*a >>= 1;
		if (a[1] & 1) *a |= TOPHALF;
		a++;
	}
	s = u;
done:	if (s == 0) {
		while (m > 0 && A[m - 1] == 0)
			m--;
		if (m == 0) {
			remsign = 0;
			sqrt.v = alloc(n);
			sqrt.len = n;
			sqrt.sign = 0;
			memcpy(sqrt.v, a0, n * sizeof(HALF));
			freeh(A);
			*dest = sqrt;
			return remsign;
		}
	}
	if (rnd & 16) {
		if (s == 0) {
			if (m != n) {
				up = (m > n);
			} else {
				i = n;
				b = a0 + n;
				a = A + n;
				while (i > 0 && *--a == *--b)
					i--;
				up = (i > 0 && *a > *b);
			}
		} else {
			while (m > 1 && A[m - 1] == BASE1)
				m--;
			if (m != n) {
				up = (m < n);
			} else {
				i = n;
				b = a0 + n;
				a = A + n;
				while (i > 0 && *--a + *--b == BASE1)
					i--;
				up = ((FULL) *a + *b >= BASE);
			}
		}
	}
	else
	if (rnd & 8)
		up = (((rnd ^ *a0) & 1) ? TRUE : FALSE);
	else
		up = ((rnd & 1) ? TRUE : FALSE);
	if (up) {
		remsign = -1;
		i = n;
		a = a0;
		while (i-- && *a == BASE1)
			*a++ = 0;
		if (i >= 0) {
			(*a)++;
		} else {
			n++;
			*a = 1;
		}
	} else {
		remsign = 1;
	}
	sqrt.v = alloc(n);
	sqrt.len = n;
	sqrt.sign = 0;
	memcpy(sqrt.v, a0, n * sizeof(HALF));
	freeh(A);
	*dest = sqrt;
	return remsign;

}

/*
 * Take an arbitrary root of a number (to the greatest integer).
 * This uses the following iteration to get the Kth root of N:
 *	x = ((K-1) * x + N / x^(K-1)) / K
 */
void
zroot(ZVALUE z1, ZVALUE z2, ZVALUE *dest)
{
	ZVALUE ztry, quo, old, temp, temp2;
	ZVALUE k1;			/* holds k - 1 */
	int sign;
	long i;
	LEN highbit, k;
	SIUNION sival;

	sign = z1.sign;
	if (sign && ziseven(z2)) {
		math_error("Even root of negative number");
		/*NOTREACHED*/
	}
	if (ziszero(z2) || zisneg(z2)) {
		math_error("Non-positive root");
		/*NOTREACHED*/
	}
	if (ziszero(z1)) {	/* root of zero */
		*dest = _zero_;
		return;
	}
	if (zisunit(z2)) {	/* first root */
		zcopy(z1, dest);
		return;
	}
	if (zge31b(z2)) {	/* humongous root */
		*dest = _one_;
		dest->sign = (BOOL)((HALF)sign);
		return;
	}
	k = (LEN)ztolong(z2);
	highbit = zhighbit(z1);
	if (highbit < k) {	/* too high a root */
		*dest = _one_;
		dest->sign = (BOOL)((HALF)sign);
		return;
	}
	sival.ivalue = k - 1;
	k1.v = &sival.silow;
	/* ignore Saber-C warning #112 - get ushort from uint */
	/*	  ok to ignore on name zroot`sival */
	k1.len = 1 + (sival.sihigh != 0);
	k1.sign = 0;
	z1.sign = 0;
	/*
	 * Allocate the numbers to use for the main loop.
	 * The size and high bits of the final result are correctly set here.
	 * Notice that the remainder of the test value is rubbish, but this
	 * is unimportant.
	 */
	highbit = (highbit + k - 1) / k;
	ztry.len = (highbit / BASEB) + 1;
	ztry.v = alloc(ztry.len);
	zclearval(ztry);
	ztry.v[ztry.len-1] = ((HALF)1 << (highbit % BASEB));
	ztry.sign = 0;
	old.v = alloc(ztry.len);
	old.len = 1;
	zclearval(old);
	old.sign = 0;
	/*
	 * Main divide and average loop
	 */
	for (;;) {
		zpowi(ztry, k1, &temp);
		zquo(z1, temp, &quo, 0);
		zfree(temp);
		i = zrel(ztry, quo);
		if (i <= 0) {
			/*
			 * Current try is less than or equal to the root since
			 * it is less than the quotient. If the quotient is
			 * equal to the try, we are all done.  Also, if the
			 * try is equal to the old value, we are done since
			 * no improvement occurred.  If not, save the improved
			 * value and loop some more.
			 */
			if ((i == 0) || (zcmp(old, ztry) == 0)) {
				zfree(quo);
				zfree(old);
				ztry.sign = (BOOL)((HALF)sign);
				zquicktrim(ztry);
				*dest = ztry;
				return;
			}
			old.len = ztry.len;
			zcopyval(ztry, old);
		}
		/* average current try and quotient for the new try */
		zmul(ztry, k1, &temp);
		zfree(ztry);
		zadd(quo, temp, &temp2);
		zfree(temp);
		zfree(quo);
		zquo(temp2, z2, &ztry, 0);
		zfree(temp2);
	}
}


/*
 * Test to see if a number is an exact square or not.
 */
BOOL
zissquare(ZVALUE z)
{
	long n;
	ZVALUE tmp;

	/* negative values are never perfect squares */
	if (zisneg(z)) {
		return FALSE;
	}

	/* ignore trailing zero words */
	while ((z.len > 1) && (*z.v == 0)) {
		z.len--;
		z.v++;
	}

	/* zero or one is a perfect square */
	if (zisabsleone(z)) {
		return TRUE;
	}

	/* check mod 4096 values */
	if (issq_mod4k[(int)(*z.v & 0xfff)] == 0) {
		return FALSE;
	}

	/* must do full square root test now */
	n = !zsqrt(z, &tmp, 0);
	zfree(tmp);
	return (n ? TRUE : FALSE);
}
