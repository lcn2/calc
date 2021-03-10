/*
 * have_memmv - Determine if we have memmove()
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
 * Under source code control:	1997/04/16 02:02:34
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_memmv
 *
 * Not all systems with memcpy() have memmove() functions, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_MEMMOVE
 *		defined ==> use memmove()
 *		undefined ==> use internal slow memmove() instead
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
#if defined(HAVE_NO_MEMMOVE)

	printf("#undef HAVE_MEMMOVE /* no */\n");

#else /* HAVE_NO_MEMMOVE */

	(void) memmove(dest, src, MOVELEN);

	printf("#define HAVE_MEMMOVE /* yes */\n");

#endif /* HAVE_NO_MEMMOVE */

	/* exit(0); */
	return 0;
}
