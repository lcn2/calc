/*
 * have_strlcpy - determine if we have strlcpy()
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
 * Under source code control:	2021/03/08 20:34:13
 * File existed as early as:	2021
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_strlcpy
 *
 * Not all systems have the strlcpy() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_STRLCPY
 *		defined ==> use strlcpy()
 *		undefined ==> do not or cannot call strlcpy()
 */

#include <stdio.h>
#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif


#include "banned.h"	/* include after system header <> includes */


#define BUF_SIZ (sizeof("abcde")-1)

char src[BUF_SIZ+1] = "abcde";


int
main(void)
{
#if defined(HAVE_NO_STRLCPY)

	printf("#undef HAVE_STRLCPY /* no */\n");

#else /* HAVE_NO_STRLCPY */
	char dst[BUF_SIZ+1];

	(void) strlcpy(dst, src, sizeof(dst));

	printf("#define HAVE_STRLCPY /* yes */\n");

#endif /* HAVE_NO_STRLCPY */

	/* exit(0); */
	return 0;
}
