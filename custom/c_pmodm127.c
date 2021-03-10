/*
 * c_pmodm127 - calculate q mod 2^(2^127-1)
 *
 * Copyright (C) 2004-2007,2021  Landon Curt Noll
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
 * Under source code control:	2004/07/28 22:12:25
 * File existed as early as:	2004
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


#include "banned.h"	/* include after system header <> includes */


/* 2^255 */
STATIC HALF h255[] = {
#if BASEB == 32
	(HALF)0x00000000, (HALF)0x00000000, (HALF)0x00000000, (HALF)0x00000000,
	(HALF)0x00000000, (HALF)0x00000000, (HALF)0x00000000, (HALF)0x80000000
#else /* BASEB == 32 */
	(HALF)0x0000, (HALF)0x0000, (HALF)0x0000, (HALF)0x0000,
	(HALF)0x0000, (HALF)0x0000, (HALF)0x0000, (HALF)0x0000,
	(HALF)0x0000, (HALF)0x0000, (HALF)0x0000, (HALF)0x0000,
	(HALF)0x0000, (HALF)0x0000, (HALF)0x0000, (HALF)0x8000
#endif /* BASEB == 32 */
};
ZVALUE p255 = {
    h255, 8, 0
};


/* static declarations */
S_FUNC void zmod5_or_zmod(ZVALUE *zp);
STATIC BOOL havelastmod = FALSE;
STATIC ZVALUE lastmod[1];
STATIC ZVALUE lastmodinv[1];


/*
 * c_pmodm127 - calculate q mod 2^(2^127-1)
 *
 * given:
 *    count = 1;
 *    vals[0]	real number;	(q - potential factor)
 *
 * returns:
 *    result	real number;	(q mod 2^(2^127-1))
 */
/*ARGSUSED*/
VALUE
c_pmodm127(char *UNUSED(name), int UNUSED(count), VALUE **vals)
{
	VALUE result;		/* what we will return */
	ZVALUE q;		/* test factor */
	ZVALUE temp;		/* temp calculation value */
	int i;			/* exponent value */

	/*
	 * arg check
	 */
	result.v_type = V_NULL;
	if (vals[0]->v_type != V_NUM) {
		math_error("Non-numeric argument for pmodm127");
		/*NOTREACHED*/
	}
	if (qisfrac(vals[0]->v_num)) {
		math_error("Non-integer argument for pmodm127");
		/*NOTREACHED*/
	}
	if (qisneg(vals[0]->v_num) || qiszero(vals[0]->v_num)) {
		math_error("argument for pmodm127 <= 0");
		/*NOTREACHED*/
	}

	/*
	 * look at the numerator
	 */
	q = vals[0]->v_num->num;

	/*
	 * setup lastmod with q
	 */
	if (havelastmod && zcmp(q, *lastmod)) {
		zfree(*lastmod);
		zfree(*lastmodinv);
		havelastmod = FALSE;
	}
	if (!havelastmod) {
		zcopy(q, lastmod);
		zbitvalue(2 * q.len * BASEB, &temp);
		zquo(temp, q, lastmodinv, 0);
		zfree(temp);
		havelastmod = TRUE;
	}

	/*
	 * start with 2^255
	 */
	result.v_num = qalloc();
	zcopy(p255, &result.v_num->num);
	result.v_type = V_NUM;

	/*
	 * compute 2^(2^127-1) mod q by modular exponentiation
	 *
	 * We implement the following calc code in C:
	 *
	 *	(* given q, our test factor, as the arg to this function *)
	 *	result = 2^255;
	 *	for (i=8; i < 127; ++i) {
	 *		result %= q;	  (* mod *)
	 *		result *= result; (* square *)
	 *		result <<= 1;     (* times 2 *)
	 *	}
	 *	result %= q;	          (* result is now 2^(2^127-1) % q *)
	 */
	for (i=8; i<127; ++i) {
#if 0
		/* use of zmod is a bit slower than zmod5_or_zmod */
		(void) zmod(result.v_num->num, *lastmod, &temp, 0);
		zfree(result.v_num->num);
		result.v_num->num = temp;
#else
		zmod5_or_zmod(&result.v_num->num);	/* mod */
#endif
#if 0
		/* use of zmul is slightly slower than zsquare */
		zmul(result.v_num->num, result.v_num->num, &temp);  /* square */
#else
		zsquare(result.v_num->num, &temp);	/* square */
#endif
		/*
		 * We could manually shift here, but this would o speed
		 * up the operation only a very tiny bit at the expense
		 * of a bunch of special code.
		 */
		zfree(result.v_num->num);
		zshift(temp, 1, &result.v_num->num);	/* times 2 */
		zfree(temp);
	}
	zmod5_or_zmod(&result.v_num->num);	/* result = 2^(2^127-1) % q */

	/*
	 * cleanup and return result
	 */
	return result;
}


/*
 * zmod5_or_zmod - fast integer modulo the modulus computation
 *
 * "borrowed" from ../zmod.c
 *
 * Given the address of a positive integer whose word count does not
 * exceed twice that of the modulus stored at lastmod, to evaluate and store
 * at that address the value of the integer modulo the modulus.
 *
 * Unlike the static routine in ../zmod.c, we will call the zmod and return
 * the result of the zmod5_or_zmod conditions do not apply to the argument
 * and saved mod.
 */
S_FUNC void
zmod5_or_zmod(ZVALUE *zp)
{
	LEN len, modlen, j;
	ZVALUE tmp1, tmp2;
	ZVALUE z1, z2, z3;
	HALF *a, *b;
	FULL f;
	HALF u;

	int subcount = 0;

	if (zrel(*zp, *lastmod) < 0)
		return;
	modlen = lastmod->len;
	len = zp->len;
	z1.v = zp->v + modlen - 1;
	z1.len = len - modlen + 1;
	z1.sign = z2.sign = z3.sign = 0;
	if (z1.len > modlen + 1) {
		/* in ../zmod.c we did a math_error("Bad call to zmod5!!!"); */
		/* here we just call the slower zmod and return the result */
		(void) zmod(*zp, *lastmod, &tmp1, 0);
		/* replace zp with tmp1 mod result */
		zfree(*zp);
		*zp = tmp1;
		return;
	}
	z2.v = lastmodinv->v + modlen + 1 - z1.len;
	z2.len = lastmodinv->len - modlen - 1 + z1.len;
	zmul(z1, z2, &tmp1);
	z3.v = tmp1.v + z1.len;
	z3.len = tmp1.len - z1.len;
	if (z3.len > 0) {
		zmul(z3, *lastmod, &tmp2);
		j = modlen;
		a = zp->v;
		b = tmp2.v;
		u = 0;
		len = modlen;
		while (j-- > 0) {
			f = (FULL) *a - (FULL) *b++ - (FULL) u;
			*a++ = (HALF) f;
			u = - (HALF) (f >> BASEB);
		}
		if (z1.len > 1) {
			len++;
			if (tmp2.len > modlen)
				f = (FULL) *a - (FULL) *b - (FULL) u;
			else
				f = (FULL) *a - (FULL) u;
			*a++ = (HALF) f;
		}
		while (len > 0 && *--a == 0)
			len--;
		zp->len = len;
		zfree(tmp2);
	}
	zfree(tmp1);
	while (len > 0 && zrel(*zp, *lastmod) >= 0) {
		subcount++;
		if (subcount > 2) {
			math_error("Too many subtractions in zmod5_or_zmod");
			/*NOTREACHED*/
		}
		j = modlen;
		a = zp->v;
		b = lastmod->v;
		u = 0;
		while (j-- > 0) {
			f = (FULL) *a - (FULL) *b++ - (FULL) u;
			*a++ = (HALF) f;
			u = - (HALF) (f >> BASEB);
		}
		if (len > modlen) {
			f = (FULL) *a - (FULL) u;
			*a++ = (HALF) f;
		}
		while (len > 0 && *--a == 0)
			len--;
		zp->len = len;
	}
	if (len == 0)
		zp->len = 1;
}

#endif /* CUSTOM */
