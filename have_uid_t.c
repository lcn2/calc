/*
 * have_uid_t - Determine if we want or can support uid_t
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
 * @(#) $Id: have_uid_t.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_uid_t.c,v $
 *
 * Under source code control:	1995/05/02 03:03:24
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_uid_t
 *
 * Not all compilers support uid_t, so this may not compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_UID_T
 *		defined ==> ok to use uid_t
 *		undefined ==> do not use uid_t
 */


#include <stdio.h>

#if !defined(HAVE_NO_UID_T)
#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <pwd.h>
#include <sys/types.h>
#endif /* ! HAVE_NO_UID_T */

int
main(void)
{
#if defined(HAVE_NO_UID_T)

	printf("#undef HAVE_UID_T /* no */\n");

#else /* HAVE_NO_UID_T */

	uid_t curds;
	extern uid_t geteuid();

	curds = geteuid();

	printf("#define HAVE_UID_T /* yes */\n");

#endif /* HAVE_NO_UID_T */

	/* exit(0); */
	return 0;
}
