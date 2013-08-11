/*
 * have_getpgid - determine if we have getpgid()
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
 * @(#) $Id: have_getpgid.c,v 30.1 2007/03/16 11:09:46 chongo Exp $
 * @(#) $Source: /usr/local/src/bin/calc/RCS/have_getpgid.c,v $
 *
 * Under source code control:	1999/10/20 22:51:13
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_getpgid
 *
 * Not all systems have the getpgid() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETPGID
 *		defined ==> use getpgid()
 *		undefined ==> do not or cannot call getpgid()
 */

#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
#if defined(HAVE_NO_GETPGID)

	printf("#undef HAVE_GETPGID /* no */\n");

#else /* HAVE_NO_GETPGID */

	(void) getpgid((pid_t)0);

	printf("#define HAVE_GETPGID /* yes */\n");

#endif /* HAVE_NO_GETPGID */

	/* exit(0); */
	return 0;
}
