/*
 * have_strlcat - determine if we have strlcat()
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
 *	have_strlcat
 *
 * Not all systems have the strlcat() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_STRLCAT
 *		defined ==> use strlcat()
 *		undefined ==> do not or cannot call strlcat()
 */

#include <stdio.h>
#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif


#include "banned.h"	/* include after system header <> includes */


#define BUF_SIZ 5

char src[BUF_SIZ+1] = "abcde";


int
main(void)
{
#if defined(HAVE_NO_STRLCAT)

	printf("#undef HAVE_STRLCAT /* no */\n");

#else /* HAVE_NO_STRLCAT */
	char dst[BUF_SIZ+1+1];

	dst[0] = 'S';
	dst[1] = '\0';
	(void) strlcat(dst, src, sizeof(dst));

	printf("#define HAVE_STRLCAT /* yes */\n");

#endif /* HAVE_NO_STRLCAT */

	/* exit(0); */
	return 0;
}
