/*
 * have_getsid - determine if we have getsid()
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
 * @(#) $Id: have_getsid.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_getsid.c,v $
 *
 * Under source code control:	1999/10/20 22:51:13
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_getsid
 *
 * Not all systems have the getsid() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_GETSID
 *		defined ==> use getsid()
 *		undefined ==> do not call or cannot call getsid()
 */


#include <sys/types.h>

int
main(void)
{
#if defined(HAVE_NO_GETSID)

	printf("#undef HAVE_GETSID /* no */\n");

#else /* HAVE_NO_GETSID */

	(void) getsid((pid_t)0);

	printf("#define HAVE_GETSID /* yes */\n");

#endif /* HAVE_NO_GETSID */

	/* exit(0); */
	return 0;
}
