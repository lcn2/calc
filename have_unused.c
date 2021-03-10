/*
 * have_unused - Determine if we want or can support the unused attribute
 *
 * Copyright (C) 2004,2021  Landon Curt Noll
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
 * Under source code control:	2004/02/22 22:36:10
 * File existed as early as:	2004
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_unused
 *
 * Not all compilers support the unused attribute, so this may not compile
 * on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_UNUSED
 *		defined ==> ok to use unused
 *		undefined ==> do not use unused
 *
 *	UNUSED
 *		unused ==> use unused
 *		(nothing) ==> unused not used
 */

#include <stdio.h>


#include "banned.h"	/* include after system header <> includes */


#if !defined(HAVE_NO_UNUSED)

/* make sure that we can use the __attribute__((x)) in #define's */
#undef UNUSED
#define UNUSED(x) UNUSED_ ## x __attribute__((unused))

/*
 * unused_str - a function with an unused argument
 */
static char *
unused_str(char *UNUSED(str))
{
    return "UNUSED_ ## x __attribute__((unused))";
}

#endif /* !HAVE_NO_UNUSED */


int
main(void)
{
#if defined(HAVE_NO_UNUSED)

	printf("#undef UNUSED\n");
	printf("#if defined(__LCLINT__)\n");
	printf("# define UNUSED(x) /*@unused@*/ x\n");
	printf("#else\n");
	printf("# define UNUSED(x) x\n");
	printf("#endif\n");

#else /* HAVE_NO_UNUSED */

	printf("#undef UNUSED\n");
	printf("#define UNUSED(x) %s\n", unused_str(NULL));

#endif /* HAVE_NO_UNUSED */

	/* exit(0); */
	return 0;
}
