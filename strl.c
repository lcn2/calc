/*
 * strl - size-bounded string copying and concatenation
 *
 * Copyright (C) 2021  Landon Curt Noll
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
 * Under source code control:	2021/03/08 21;58:10
 * File existed as early as:	2021
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#include "strl.h"
#include "alloc.h"

#if defined(STRL_TEST)
#include <stdio.h>
#include <stdlib.h>
#endif /* STRL_TEST */


#include "banned.h"	/* include after system header <> includes */


#if !defined(HAVE_STRLCPY) || defined(STRL_TEST)
/*
 * calc_strlcpy - calc STRL_TEST test version of strlcpy
 * strlcpy - size-bounded string copying
 *
 * The strlcpy() functions copies up to dstsize-1 characters from the
 * string src to dst, NUL-terminating the result if dstsize > 0.
 *
 * This function attempts to simulate strlcpy() on systems that
 * do not have this function in their C library.
 *
 * given:
 *	dst		destination string
 *	src		source string
 *	dstsize		size of the destination string buffer including NUL
 *
 * returns:
 *	total 		length of source string excluding NUL,
 *			0 ==> invalid arguments
 *
 * NOTE: If return value is >= dstsize, the result was truncated.
 *	 I.e., there was not enough room in dst and so the copy
 *	 was stopped before it would have overflowed and was NUL terminated.
 */
#if defined(STRL_TEST)
size_t
calc_strlcpy(char * dst, const char * src, size_t dstsize)
#else /* STRL_TEST */
size_t
strlcpy(char * dst, const char * src, size_t dstsize)
#endif /* STRL_TEST */
{
	size_t srclen;	/* src string length not including NUL */

	/*
	 * firewall
	 */
	if (dst == NULL || src == NULL || dstsize <= 0) {
		/* nothing can be copied */
#if defined(STRL_TEST)
		printf("in %s: return 0\n", __FUNCTION__);
#endif /* STRL_TEST */
		return 0;
	}

	/*
	 * determine how much string we could copy
	 */
	srclen = strlen(src);

	/*
	 * perform the size limited copy and NUL terminate
	 */
	if (srclen > dstsize-1) {
	    memcpy(dst, src, dstsize-1);
	    dst[dstsize-1] = '\0';
#if defined(STRL_TEST)
	    printf("in %s: if memcpy(\"%s\", \"%s\", %zu)\n",
		    __FUNCTION__, dst, src, dstsize-1);
#endif /* STRL_TEST */
	} else {
	    memcpy(dst, src, srclen);
	    dst[srclen] = '\0';
#if defined(STRL_TEST)
	    printf("in %s: else memcpy(\"%s\", \"%s\", %zu)\n",
		    __FUNCTION__, dst, src, srclen);
#endif /* STRL_TEST */
	}

	/*
	 * return the length we tried to copy, not including NUL
	 */
#if defined(STRL_TEST)
	printf("in %s: return %zu\n", __FUNCTION__, srclen);
#endif /* STRL_TEST */
	return srclen;
}
#endif /* !HAVE_STRLCPY || STRL_TEST */


#if !defined(HAVE_STRLCAT) || defined(STRL_TEST)
/*
 * calc_strlcat - calc STRL_TEST test version of strlcat
 * strlcat - size-bounded string cat
 *
 * The strlcat() appends string src to the end of dst.
 * At most, dstsize - strlen(dst) - 1 characters will of appended.
 *
 * This function attempts to simulate strlcat() on systems that
 * do not have this function in their C library.
 *
 * given:
 *	dst		destination string
 *	src		source string
 *	dstsize		size of the destination string buffer including NUL
 *
 * returns:
 *	total 		length of string could have been copied excluding NUL,
 *			0 ==> invalid arguments
 *
 * NOTE: If return value is >= dstsize, the output string was truncated.
 *	 I.e., there was not enough room and so the result was truncated
 *	 and NUL terminated.
 */
#if defined(STRL_TEST)
size_t
calc_strlcat(char * dst, const char * src, size_t dstsize)
#else /* STRL_TEST */
size_t
strlcat(char * dst, const char * src, size_t dstsize)
#endif /* STRL_TEST */
{
	size_t srclen;	/* src string length not including NUL */
	size_t dstlen;	/* dst string length not including NUL */
	size_t catlen;	/* amount from src we can copy */

	/*
	 * firewall
	 */
	if (dst == NULL || src == NULL || dstsize <= 0) {
		/* nothing can be concatinated */
#if defined(STRL_TEST)
		printf("in %s: return 0\n", __FUNCTION__);
#endif /* STRL_TEST */
		return 0;
	}

	/*
	 * prep to determine the room we have
	 */
	dstlen = strlen(dst);
	srclen = strlen(src);
#if defined(STRL_TEST)
	printf("in %s: dst = ((%s)) src = ((%s)) dstsize = %zu\n",
		__FUNCTION__, dst, src, dstsize);
	printf("in %s: dstlen = %zu srclen = %zu\n",
		__FUNCTION__, dstlen, srclen);
#endif /* STRL_TEST */
	if (dstsize <= dstlen+1) {
		/* dst is already full */
#if defined(STRL_TEST)
		printf("in %s: dstsize: %zu <= dstlen+1: %zu\n",
			__FUNCTION__, dstsize, dstlen+1);
		printf("in %s: already full return %zu\n",
			__FUNCTION__, srclen+dstlen);
#endif /* STRL_TEST */
		return srclen+dstlen;
	}

	/*
	 * determine the amount data we can copy
	 */
	catlen = dstsize - dstlen;

	/*
	 * perform the cancatimation
	 */
	dst += dstlen;
#if defined(STRL_TEST)
	printf("in %s: catlen = %zu\n", __FUNCTION__, catlen);
#endif /* STRL_TEST */
	if (catlen > srclen+1) {
	    memcpy(dst, src, srclen);
	    dst[srclen] = '\0';
#if defined(STRL_TEST)
	    printf("in %s: if memcpy(\"%s\", \"%s\", %zu)\n",
		    __FUNCTION__, dst, src, srclen);
#endif /* STRL_TEST */
	} else {
	    memcpy(dst, src, catlen);
	    dst[catlen] = '\0';
#if defined(STRL_TEST)
	    printf("in %s: if memcpy(\"%s\", \"%s\", %zu)\n",
		    __FUNCTION__, dst, src, catlen);
#endif /* STRL_TEST */
	}

	/*
	 * return the length, not including NUL, of what could have been formed
	 */
#if defined(STRL_TEST)
	printf("in %s: return %zu\n", __FUNCTION__, srclen+dstlen);
#endif /* STRL_TEST */
	return srclen+dstlen;
}
#endif /* !HAVE_STRLCAT || STRL_TEST */

#if defined(STRL_TEST)

static char src[] = "abcde";	/* test source string */

/*
 * Main routine to test the strlcpy() and strlcat() functions.
 */
int
main(int argc, char **argv)
{
	char dst[sizeof(src)];	/* test destination string */
	size_t ret;		/* strlcpy() or strlcat() return */

	/*
	 * normal copy
	 */
	memset(dst, 0, sizeof(dst));
	printf("src: %s\n", src);
	ret = calc_strlcpy(dst, src, sizeof(dst));
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * truncated copy
	 */
	memset(dst, 0, sizeof(dst));
	printf("src: %s\n", src);
	ret = calc_strlcpy(dst, src, sizeof(dst)-3);
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * overcat
	 */
	printf("src: %s\n", src);
	ret = calc_strlcat(dst, src, sizeof(dst));
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * fullcat
	 */
	printf("src: %s\n", src);
	ret = calc_strlcat(dst, src, sizeof(dst));
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * nilcat
	 */
	dst[0] = '\0';
	printf("src: %s\n", src);
	ret = calc_strlcat(dst, src, sizeof(dst));
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * shortcat
	 */
	dst[1] = '\0';
	printf("src: %s\n", src);
	ret = calc_strlcat(dst, src, sizeof(dst)-3);
	printf("ret: %zu\n", ret);
	printf("dst: %s\n\n", dst);

	/*
	 * all done
	 */
	exit(0);
}

#endif /* STRL_TEST */
