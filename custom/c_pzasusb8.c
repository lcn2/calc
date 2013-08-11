/*
 * c_pzasusb8 - print numereator as a string of USB8s
 *
 * Copyright (C) 1999-2004  Ernest Bowen
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
 * @(#) $Id: c_pzasusb8.c,v 30.1 2007/03/16 11:10:04 chongo Exp $
 * @(#) $Source: /usr/local/src/bin/calc/custom/RCS/c_pzasusb8.c,v $
 *
 * Under source code control:	1999/10/06 03:12:25
 * File existed as early as:	1999
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if defined(CUSTOM)

#include <stdio.h>

#include "have_const.h"
#include "value.h"
#include "custom.h"
#include "zmath.h"

#include "have_unused.h"

/*
 * c_pzasusb8 - print numereator as a string of USB8s
 *
 * given:
 *    count = 1;
 *    vals[0]	real number;
 *
 * returns:
 *    null
 */
/*ARGSUSED*/
VALUE
c_pzasusb8(char UNUSED *name, int UNUSED count, VALUE **vals)
{
	VALUE result;		/* what we will return */
	ZVALUE z;		/* numerator of the value */
	long half_cnt;		/* number of HALFs in the numerator */
	USB8 *h;		/* octet pointer */
	long half_len;		/* length of a half in octets */
	long i;
	long j;

	/*
	 * arg check
	 */
	result.v_type = V_NULL;
	if (vals[0]->v_type != V_NUM) {
		math_error("Non-real argument for pzasusb8");
		/*NOTREACHED*/
	}

	/*
	 * look at the numerator
	 */
	z = vals[0]->v_num->num;
	half_len = sizeof(HALF);
	half_cnt = z.len;

	/*
	 * print the octets
	 */
	h = (USB8 *) z.v;
	for (i=0; i < half_cnt; ++i) {
		printf("%ld:\t", i);
		for (j=0; j < half_len; ++j) {
			printf("%02x", (int)(*h++));
		}
		putchar('\n');
	}
	return result;
}

#endif /* CUSTOM */
