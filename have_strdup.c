/*
 * have_strdup - determine if we have strdup()
 *
 * Copyright (C) 1999  Landon Curt Noll
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
 * @(#) $Id: have_strdup.c,v 30.1 2007/03/16 11:09:46 chongo Exp $
 * @(#) $Source: /usr/local/src/bin/calc/RCS/have_strdup.c,v $
 *
 * Under source code control:	1999/11/09 12:46:54
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_strdup
 *
 * Not all systems have the strdup() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_STRDUP
 *		defined ==> use strdup()
 *		undefined ==> do not call or cannot call strdup()
 */


#include <string.h>

int
main(void)
{
#if defined(HAVE_NO_STRDUP)

	printf("#undef HAVE_STRDUP /* no */\n");

#else /* HAVE_NO_STRDUP */

	char *p;

	p = strdup("#define HAVE_STRDUP /* yes */");
	if (p != NULL) {
		printf("%s\n", p);
	}

#endif /* HAVE_NO_STRDUP */

	/* exit(0); */
	return 0;
}
