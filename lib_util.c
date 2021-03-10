/*
 * lib_util - calc library utility routines
 *
 * Copyright (C) 1999-2006,2021  Landon Curt Noll
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
 * Under source code control:	1997/04/19 21:38:30
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * These routines are here to support users of libcalc.a.  These routines
 * are not directly used by calc itself, however.
 */


#include "zmath.h"
#include "alloc.h"
#include "lib_util.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * lowhex2bin - quick low order ASCII hex to binary conversion
 *
 * We just use mod 16 for non-hex ASCII chars.	We use just mod 128
 * for non-ASCII to ASCII conversion.
 *
 *   | 00 nul | 01 soh | 02 stx | 03 etx | 04 eot | 05 enq | 06 ack | 07 bel |
 *   | 08 bs  | 09 ht  | 0a nl	| 0b vt	 | 0c np  | 0d cr  | 0e so  | 0f si  |
 *   | 10 dle | 11 dc1 | 12 dc2 | 13 dc3 | 14 dc4 | 15 nak | 16 syn | 17 etb |
 *   | 18 can | 19 em  | 1a sub | 1b esc | 1c fs  | 1d gs  | 1e rs  | 1f us  |
 *   | 20 sp  | 21 !   | 22 "	| 23 #	 | 24 $	  | 25 %   | 26 &   | 27 '   |
 *   | 28 (   | 29 )   | 2a *	| 2b +	 | 2c ,	  | 2d -   | 2e .   | 2f /   |
 *   | 30 0   | 31 1   | 32 2	| 33 3	 | 34 4	  | 35 5   | 36 6   | 37 7   |
 *   | 38 8   | 39 9   | 3a :	| 3b ;	 | 3c <	  | 3d =   | 3e >   | 3f ?   |
 *   | 40 @   | 41 A   | 42 B	| 43 C	 | 44 D	  | 45 E   | 46 F   | 47 G   |
 *   | 48 H   | 49 I   | 4a J	| 4b K	 | 4c L	  | 4d M   | 4e N   | 4f O   |
 *   | 50 P   | 51 Q   | 52 R	| 53 S	 | 54 T	  | 55 U   | 56 V   | 57 W   |
 *   | 58 X   | 59 Y   | 5a Z	| 5b [	 | 5c \	  | 5d ]   | 5e ^   | 5f _   |
 *   | 60 `   | 61 a   | 62 b	| 63 c	 | 64 d	  | 65 e   | 66 f   | 67 g   |
 *   | 68 h   | 69 i   | 6a j	| 6b k	 | 6c l	  | 6d m   | 6e n   | 6f o   |
 *   | 70 p   | 71 q   | 72 r	| 73 s	 | 74 t	  | 75 u   | 76 v   | 77 w   |
 *   | 78 x   | 79 y   | 7a z	| 7b {	 | 7c |	  | 7d }   | 7e ~   | 7f del |
 */
int lowhex2bin[256] = {
  /* 0	 1   2	 3   4	 5   6	 7   8	 9   a	 b   c	 d   e	 f  */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 0 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 1 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 2 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 3 */
    0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 4 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 5 */
    0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 6 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 7 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 8 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* 9 */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* a */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* b */
    0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* c */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* d */
    0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,	/* e */
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf	/* f */
};


/*
 * hex2bin - macro to convert two ASCII hex chars into binary value
 *
 * given:
 *	high - high order hex ASCII char
 *	low - low order hex ASCII char
 *
 * returns:
 *	numeric equivalent to 0x{high}{low} as an int
 */
#define hex2bin(high,low) \
	(lowhex2bin[(int)((char)(high))]<<4 | lowhex2bin[((int)(char)(low))])

/*
 * lowbin2hex - quick low order binary conversion to ASCII hex
 */
char lowbin2hex[256] = {
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
};


/*
 * convstr2z - convert a string into a ZVALUE
 *
 * NOTE: No attempt is make to deal with byte order.
 *
 * given:
 *	str	string to convert
 *
 * returns:
 *	ZVALUE
 */
ZVALUE
convstr2z(char *str)
{
	HALF *v;	/* storage for string as HALFs */
	ZVALUE ret;	/* return value */
	size_t len;	/* length in HALFs of our string rounded up */

	/*
	 * firewall
	 */
	if (str == NULL || *str == '\0') {
		/* NULL or empty strings return 0 */
		return _zero_;
	}

	/*
	 * allocate HALF storage
	 */
	len = (strlen(str)+sizeof(HALF)-1)/sizeof(HALF);
	v = (HALF *)malloc(len * sizeof(HALF));
	if (v == NULL) {
		math_error("convstr2z bad malloc");
		/*NOTREACHED*/
	}
	v[len-1] = 0;	/* deal with possible partial end of string HALF */

	/*
	 * initialize HALF array with string value
	 */
	memcpy((void *)v, (void *)str, strlen(str));

	/*
	 * setup the rest of the ZVALUE
	 */
	ret.v = v;
	ret.len = len;
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * convhex2z - convert hex string to ZVALUE
 *
 * usage:
 *	str	hex ASCII string with optional leading 0x
 *
 * returns:
 *	ZVALUE
 */
ZVALUE
convhex2z(char *hex)
{
	HALF *v;	/* storage for string as HALFs */
	HALF *hp;	/* HALF pointer */
	char *sp;	/* string pointer */
	ZVALUE ret;	/* return value */
	int len;	/* length in HALFs of our string rounded up */
	size_t slen;	/* hex string length */
	int i;

	/*
	 * firewall
	 */
	if (hex == NULL || hex[0] == '\0') {
		/* NULL or empty strings return 0 */
		return _zero_;
	}

	/*
	 * skip leading 0X or 0x if needed
	 */
	if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
		hex += 2;
	}
	if (hex[0] == '\0') {
		/* just 0X or 0x returns 0 */
		return _zero_;
	}

	/*
	 * allocate HALF storage
	 */
	slen = strlen(hex);
	len = ((slen*4)+BASEB-1)/BASEB;
	v = (HALF *)malloc(len * sizeof(HALF));
	if (v == NULL) {
		math_error("convhex2z bad malloc");
		/*NOTREACHED*/
	}
	v[len-1] = 0;	/* deal with possible partial end of string HALF */

	/*
	 * deal with the upper partial HALF value
	 */
	hp = v+len-1;
	sp = hex;
	if ((slen % (BASEB/4)) != 0) {

		/* deal with a odd length hex string first */
		if (slen % 2 == 1) {
			*hp = hex2bin('0', *sp++);
			--slen;

		/* even length - top top hex char to process */
		} else {
			*hp = 0;
		}
		/* slen is even now */

		/* eat two hex chars at a time until the HALF is full */
		for (; (slen % (BASEB/4)) != 0; slen -= 2, sp += 2) {
			*hp = ((*hp<<8) | hex2bin(sp[0], sp[1]));
		}

		/* move on to the next HALF */
		--hp;
	}
	/* slen is now a multiple of BASEB/4 */

	/*
	 * deal with full HALFs
	 */
	for (; slen > 0; slen -= (BASEB/4), --hp) {

		/* clear HALF */
		*hp = 0;

		/* eat two hex chars at a time until the HALF is full */
		for (i=0; i < (BASEB/4); i += 2) {
			*hp = ((*hp<<8) | hex2bin(sp[i], sp[i+1]));
		}
	}

	/*
	 * setup the rest of the ZVALUE
	 */
	ret.v = v;
	ret.len = len;
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * convz2hex - convert ZVALUE to hex string
 *
 * We will ignore the sign of the value.
 *
 * usage:
 *	z	ZVALUE
 *
 * returns:
 *	str	hex ASCII malloced string (without a leading 0x)
 */
char *
convz2hex(ZVALUE z)
{
	char *ret;	/* string to return */
	int slen;	/* string length (not counting \0) */
	HALF half;	/* HALF value to convert */
	int seen_nz;	/* 0 => we have not seen a non-zero hex char (yet) */
	char *p;
	int i;
	int j;

	/*
	 * firewall
	 */
	if (z.v == NULL || ziszero(z)) {
		/* malloc and return "0" */
		ret = (char *)malloc(sizeof("0"));
		if (ret == NULL) {
			math_error("convz2hex bad malloc of 0 value");
			/*NOTREACHED*/
		}
		ret[0] = '0';
		ret[1] = '\0';
		return ret;
	}

	/*
	 * malloc string storage
	 */
	slen = (z.len * BASEB/4);
	ret = (char *)calloc(slen+1+1, sizeof(char));
	if (ret == NULL) {
		math_error("convz2hex bad malloc of string");
		/*NOTREACHED*/
	}

	/*
	 * load in hex ASCII chars for each HALF
	 *
	 * We will not write leading '0' hex chars into the string.
	 */
	seen_nz = 0;
	for (p=ret, i=z.len-1; i >= 0; --i) {

		/*
		 * load in ASCII hex by ASCII hex
		 */
		for (half=z.v[i], j=BASEB-4; j >= 0; j-=4) {
			if (seen_nz) {
				/* we saw a non-zero, just load the rest */
				*p++ = lowbin2hex[(half >> j) & 0xff];
			} else {
				/* all zeros so far */
				*p = lowbin2hex[(half >> j) & 0xff];
				if (*p != '0') {
					/* we found our first non-zero char */
					++p;
					seen_nz = 1;
				}
			}
		}
	}
	if (seen_nz) {
		*p = '\0';
	} else {
		/* saw nothing but 0's, so just return 0 */
		*ret = '0';
		*(ret+1) = '\0';
	}

	/*
	 * return the new string
	 */
	return ret;
}
