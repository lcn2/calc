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
 */

#include "cmath.h"
#include "byteswap.h"


/*
 * swap_b8_in_HALFs - swap 8 and if needed, 16 bits in an array of HALFs
 *
 * given:
 *      dest    - pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src     - pointer to a HALF array to swap
 *	len	- length of the src HALF array
 *
 * returns:
 *	pointer to where the swapped src has been put
 */
HALF *
swap_b8_in_HALFs(HALF *dest, HALF *src, LEN len)
{
	LEN i;

	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {
		dest = alloc(len);
	}

	/*
	 * swap the array
	 */
	for (i=0; i < len; ++i, ++dest, ++src) {
		SWAP_B8_IN_HALF(dest, src);
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b8_in_ZVALUE - swap 8 and if needed, 16 bits in a ZVALUE
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a ZVALUE to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a ZVALUE.
 */
ZVALUE *
swap_b8_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(ZVALUE));
		if (dest == NULL) {
			math_error("swap_b8_in_ZVALUE: swap_b8_in_ZVALUE: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b8_in_ZVALUE) and swap storage
		 */
		dest->v = swap_b8_in_HALFs(NULL, src->v, src->len);

	} else {

		/*
		 * swap storage
		 */
		if (dest->v != NULL) {
			zfree(*dest);
		}
		dest->v = swap_b8_in_HALFs(NULL, src->v, src->len);
	}

	/*
	 * swap or copy the rest of the ZVALUE elements
	 */
	if (all) {
		dest->len = (LEN)SWAP_B8_IN_LEN(&dest->len, &src->len);
		dest->sign = (BOOL)SWAP_B8_IN_BOOL(&dest->sign, &src->sign);
	} else {
		dest->len = src->len;
		dest->sign = src->sign;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b8_in_NUMBER - swap 8 and if needed, 16 bits in a NUMBER
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a NUMBER to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a NUMBER.
 */
NUMBER *
swap_b8_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(NUMBER));
		if (dest == NULL) {
			math_error("swap_b8_in_NUMBER: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b8_in_ZVALUE) and swap storage
		 */
		dest->num = *swap_b8_in_ZVALUE(NULL, &src->num, all);
		dest->den = *swap_b8_in_ZVALUE(NULL, &src->den, all);

	} else {

		/*
		 * swap storage
		 */
		dest->num = *swap_b8_in_ZVALUE(&dest->num, &src->num, all);
		dest->den = *swap_b8_in_ZVALUE(&dest->den, &src->den, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_B8_IN_LONG(&dest->links, &src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b8_in_COMPLEX - swap 8 and if needed, 16 bits in a COMPLEX
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a COMPLEX to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a COMPLEX.
 */
COMPLEX *
swap_b8_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(COMPLEX));
		if (dest == NULL) {
			math_error("swap_b8_in_COMPLEX: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b8_in_ZVALUE) and swap storage
		 */
		dest->real = swap_b8_in_NUMBER(NULL, src->real, all);
		dest->imag = swap_b8_in_NUMBER(NULL, src->imag, all);

	} else {

		/*
		 * swap storage
		 */
		dest->real = swap_b8_in_NUMBER(dest->real, src->real, all);
		dest->imag = swap_b8_in_NUMBER(dest->imag, src->imag, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_B8_IN_LONG(&dest->links, &src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b16_in_HALFs - swap 16 bits in an array of HALFs
 *
 * given:
 *      dest    - pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src     - pointer to a HALF array to swap
 *	len	- length of the src HALF array
 *
 * returns:
 *	pointer to where the swapped src has been put
 */
HALF *
swap_b16_in_HALFs(HALF *dest, HALF *src, LEN len)
{
	LEN i;

	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {
		dest = alloc(len);
	}

	/*
	 * swap the array
	 */
	for (i=0; i < len; ++i, ++dest, ++src) {
		SWAP_B16_IN_HALF(dest, src);
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b16_in_ZVALUE - swap 16 bits in a ZVALUE
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a ZVALUE to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a ZVALUE.
 */
ZVALUE *
swap_b16_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(ZVALUE));
		if (dest == NULL) {
			math_error("swap_b16_in_ZVALUE: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b16_in_ZVALUE) and swap storage
		 */
		dest->v = swap_b16_in_HALFs(NULL, src->v, src->len);

	} else {

		/*
		 * swap storage
		 */
		if (dest->v != NULL) {
			zfree(*dest);
		}
		dest->v = swap_b16_in_HALFs(NULL, src->v, src->len);
	}

	/*
	 * swap or copy the rest of the ZVALUE elements
	 */
	if (all) {
		dest->len = (LEN)SWAP_B16_IN_LEN(&dest->len, &src->len);
		dest->sign = (BOOL)SWAP_B16_IN_BOOL(&dest->sign, &src->sign);
	} else {
		dest->len = src->len;
		dest->sign = src->sign;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b16_in_NUMBER - swap 16 bits in a NUMBER
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a NUMBER to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a NUMBER.
 */
NUMBER *
swap_b16_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(NUMBER));
		if (dest == NULL) {
			math_error("swap_b16_in_NUMBER: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b16_in_ZVALUE) and swap storage
		 */
		dest->num = *swap_b16_in_ZVALUE(NULL, &src->num, all);
		dest->den = *swap_b16_in_ZVALUE(NULL, &src->den, all);

	} else {

		/*
		 * swap storage
		 */
		dest->num = *swap_b16_in_ZVALUE(&dest->num, &src->num, all);
		dest->den = *swap_b16_in_ZVALUE(&dest->den, &src->den, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_B16_IN_LONG(&dest->links, &src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_b16_in_COMPLEX - swap 16 bits in a COMPLEX
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a COMPLEX to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a COMPLEX.
 */
COMPLEX *
swap_b16_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(COMPLEX));
		if (dest == NULL) {
			math_error("swap_b16_in_COMPLEX: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_b16_in_ZVALUE) and swap storage
		 */
		dest->real = swap_b16_in_NUMBER(NULL, src->real, all);
		dest->imag = swap_b16_in_NUMBER(NULL, src->imag, all);

	} else {

		/*
		 * swap storage
		 */
		dest->real = swap_b16_in_NUMBER(dest->real, src->real, all);
		dest->imag = swap_b16_in_NUMBER(dest->imag, src->imag, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_B16_IN_LONG(&dest->links, &src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_HALF_in_ZVALUE - swap HALFs in a ZVALUE
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a ZVALUE to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a ZVALUE.
 */
ZVALUE *
swap_HALF_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(ZVALUE));
		if (dest == NULL) {
			math_error("swap_HALF_in_ZVALUE: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * copy storage because we are dealing with HALFs
		 */
		dest->v = (HALF *) zcopyval(*src, *dest);

	} else {

		/*
		 * copy storage because we are dealing with HALFs
		 */
		if (dest->v != NULL) {
			zfree(*dest);
			dest->v = alloc(src->len);
		}
		zcopyval(*src, *dest);
	}

	/*
	 * swap or copy the rest of the ZVALUE elements
	 */
	if (all) {
		dest->len = (LEN)SWAP_HALF_IN_LEN(&dest->len, &src->len);
		dest->sign = (BOOL)SWAP_HALF_IN_BOOL(&dest->sign, &src->sign);
	} else {
		dest->len = src->len;
		dest->sign = src->sign;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_HALF_in_NUMBER - swap HALFs in a NUMBER
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a NUMBER to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a NUMBER.
 */
NUMBER *
swap_HALF_in_NUMBER(NUMBER *dest, NUMBER *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(NUMBER));
		if (dest == NULL) {
			math_error("swap_HALF_in_NUMBER: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_HALF_in_ZVALUE) and swap storage
		 */
		dest->num = *swap_HALF_in_ZVALUE(NULL, &src->num, all);
		dest->den = *swap_HALF_in_ZVALUE(NULL, &src->den, all);

	} else {

		/*
		 * swap storage
		 */
		dest->num = *swap_HALF_in_ZVALUE(&dest->num, &src->num, all);
		dest->den = *swap_HALF_in_ZVALUE(&dest->den, &src->den, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_HALF_IN_LONG(&dest->links,&src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}


/*
 * swap_HALF_in_COMPLEX - swap HALFs in a COMPLEX
 *
 * given:
 *	dest	- pointer to where the swapped src wil be put or
 *		  NULL to allocate the storage
 *	src	- pointer to a COMPLEX to swap
 *	all	- TRUE => swap every element, FALSE => swap only the
 *		  multiprecision storage
 *
 * returns:
 *	pointer to where the swapped src has been put
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) the elements of a COMPLEX.
 */
COMPLEX *
swap_HALF_in_COMPLEX(COMPLEX *dest, COMPLEX *src, BOOL all)
{
	/*
	 * allocate storage if needed
	 */
	if (dest == NULL) {

		/*
		 * allocate the storage
		 */
		dest = malloc(sizeof(COMPLEX));
		if (dest == NULL) {
			math_error("swap_HALF_in_COMPLEX: Not enough memory");
			/*NOTREACHED*/
		}

		/*
		 * allocate (by forcing swap_HALF_in_ZVALUE) and swap storage
		 */
		dest->real = swap_HALF_in_NUMBER(NULL, src->real, all);
		dest->imag = swap_HALF_in_NUMBER(NULL, src->imag, all);

	} else {

		/*
		 * swap storage
		 */
		dest->real = swap_HALF_in_NUMBER(dest->real, src->real, all);
		dest->imag = swap_HALF_in_NUMBER(dest->imag, src->imag, all);
	}

	/*
	 * swap or copy the rest of the NUMBER elements
	 */
	if (all) {
		dest->links = (long)SWAP_HALF_IN_LONG(&dest->links,&src->links);
	} else {
		dest->links = src->links;
	}

	/*
	 * return the result
	 */
	return dest;
}
