/*
 * have_getprid - determine if we have getprid()
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.2 $
 * @(#) $Id: have_getprid.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_getprid.c,v $
 *
 * Under source code control:	1999/10/20 23:43:42
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_getprid
 *
 * Not all systems have the getprid() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETPRID
 *		defined ==> use getprid()
 *		undefined ==> do not or cannot call getprid()
 */


#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
#if defined(HAVE_NO_GETPRID)

	printf("#undef HAVE_GETPRID /* no */\n");

#else /* HAVE_NO_GETPRID */

	(void) getprid();
	printf("#define HAVE_GETPRID /* yes */\n");

#endif /* HAVE_NO_GETPRID */

	/* exit(0); */
	return 0;
}
