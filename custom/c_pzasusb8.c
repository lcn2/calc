/*
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted.
 *
 *	      Ernest Bowen, following Landon Curt Noll
 */

#if defined(CUSTOM)

#include <stdio.h>

#include "../have_const.h"
#include "../value.h"
#include "../custom.h"
#include "../zmath.h"

/*
 * c_pzasusb8 - print numereator of real as if its array of HALFs were
 *    a string of USB8s
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
c_pzasusb8(char *name, int count, VALUE **vals)
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
