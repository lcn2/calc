/*
 * have_const - Determine if we want or can support ansi const
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
 * @(#) $Id: have_const.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_const.c,v $
 *
 * Under source code control:	1995/04/22 13:18:44
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_const
 *
 * Not all compilers support const, so this may not compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_CONST
 *		defined ==> ok to use const
 *		undefined ==> do not use const
 *
 *	CONST
 *		const ==> use const
 *		(nothing) ==> const not used
 */


#include <stdio.h>

int
main(void)
{
#if defined(HAVE_NO_CONST)

	printf("#undef HAVE_CONST /* no */\n");
	printf("#undef CONST\n");
	printf("#define CONST /* no */\n");

#else /* HAVE_NO_CONST */

	const char * const str = "const";

	printf("#define HAVE_CONST /* yes */\n");
	printf("#undef CONST\n");
	printf("#define CONST %s /* yes */\n", str);

#endif /* HAVE_NO_CONST */

	/* exit(0); */
	return 0;
}
