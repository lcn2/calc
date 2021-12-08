/*
 * charbit - determine what CHAR_BIT is and define CALC_CHARBIT
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
 * Under source code control:	2021/12/07 20:57:50
 * File existed as early as:	2021
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	charbit
 *
 * This prog outputs several defines:
 *
 *	CALC_CHARBIT
 *		after including have_limits.h and perhaps <limits.h>,
 *		output CALC_CHARBIT as CHAR_BIT (from <limits.h>, or as 8.
 */

#include <stdio.h>
#include "have_limits.h"
#if defined(HAVE_LIMITS_H)
#include <limits.h>
#endif


#include "banned.h"	/* include after system header <> includes */


int
main(void)
{
	printf("#include \"have_limits.h\"\n");
	printf("#if defined(HAVE_LIMITS_H)\n");
	printf("#include <limits.h>\n");
	printf("#endif\n\n");
#if defined(CHAR_BIT)

	printf("#define CALC_CHARBIT (CHAR_BIT) /* from <limits.h> */\n");

#else /* CHAR_BIT */

	printf("#define CALC_CHARBIT (8) /* no CHAR_BIT, assume 8 */\n");

#endif /* CHAR_BIT */

	/* exit(0); */
	return 0;
}
