/*
 * have_newstr - Determine if we have a system without ANSI C string functions
 *
 * Copyright (C) 1999,2021  Landon Curt Noll
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
 * Under source code control:	1995/05/02 03:55:08
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_newstr
 *
 * Not all systems support all ANSI C string functions, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_NEWSTR
 *		defined ==> use memcpy(), memset(), strchr()
 *		undefined ==> use bcopy() instead of memcpy(),
 *			      use bfill() instead of memset(),
 *			      use index() instead of strchr()
 */

#include <stdio.h>
#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif


#include "banned.h"	/* include after system header <> includes */


#define MOVELEN 3

char src[] = "chongo was here";
char dest[MOVELEN+1];

int
main(void)
{
#if defined(HAVE_NO_NEWSTR)

	printf("#undef HAVE_NEWSTR /* no */\n");

#else /* HAVE_NO_NEWSTR */

	(void) memcpy(dest, src, MOVELEN);
	(void) memset(dest, 0, MOVELEN);
	(void) strchr(src, 'e');

	printf("#define HAVE_NEWSTR /* yes */\n");

#endif /* HAVE_NO_NEWSTR */

	/* exit(0); */
	return 0;
}
