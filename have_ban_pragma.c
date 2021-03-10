/*
 * have_ban_pragma.c - Determine if we have #pragma GCC poison func_name
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
 * Under source code control:	2021/03/08 01:02:34
 * File existed as early as:	2021
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_ban_pragma
 *
 * Not all systems have #pragma GCC poison func_name, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_PRAGMA_GCC_POSION
 *		defined ==> use #pragma GCC poison func_name
 *		undefined ==> do not use #pragma GCC poison func_name
 *
 * NOTE: Modern clang compilers allow for 'pragma GCC poison func_name'.
 *	 This is NOT simply a GCC feature.
 */

#include <stdio.h>


/* undef UNBAN to be undefined to force use of banned.h */
#undef UNBAN

/* prevent banned.h from including have_ban_pragma.h */
#define PRE_HAVE_BAN_PRAGMA_H

#include "banned.h"	/* include after system header <> includes */


int
main(void)
{
#if defined(HAVE_NO_PRAGMA_GCC_POSION)

	printf("#undef HAVE_PRAGMA_GCC_POSION /* no */\n");

#else /* HAVE_NO_PRAGMA_GCC_POSION */

	printf("#define HAVE_PRAGMA_GCC_POSION /* yes */\n");

#endif /* HAVE_NO_PRAGMA_GCC_POSION */

	/* exit(0); */
	return 0;
}
