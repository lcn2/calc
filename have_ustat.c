/*
 * have_ustat - Determine if we have ustat()
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
 * @(#) $Id: have_ustat.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_ustat.c,v $
 *
 * Under source code control:	1999/10/20 22:51:13
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_ustat
 *
 * Not all systems have the ustat() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_USTAT
 *		defined ==> use ustat()
 *		undefined ==> do not call or cannot call ustat()
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>

int
main(void)
{
#if defined(HAVE_NO_USTAT)

	printf("#undef HAVE_USTAT /* no */\n");

#else /* HAVE_NO_USTAT */

	struct stat stat_dot;		/* stat of "." */
	struct ustat ustat_dot;		/* usage stat of "." */

	(void) stat(".", &stat_dot);
	(void) ustat(stat_dot.st_dev, &ustat_dot);

	printf("#define HAVE_USTAT /* yes */\n");

#endif /* HAVE_NO_USTAT */

	/* exit(0); */
	return 0;
}
