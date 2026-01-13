/*
 * charbit - determine what CHAR_BIT is and define CALC_CHARBIT
 *
 * Copyright (C) 2021,2026  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   2021/12/07 20:57:50
 * File existed as early as:    2021
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      charbit
 *
 * This prog outputs several defines:
 *
 *      CALC_CHARBIT
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <limits.h>

#include "banned.h" /* include after all other includes */

int
main(void)
{
    printf("#include <limits.h>\n");
#if defined(CHAR_BIT)

    printf("#define CALC_CHARBIT (CHAR_BIT) /* from <limits.h> */\n");

#else

    printf("#define CALC_CHARBIT (8) /* no CHAR_BIT, assume 8 */\n");

#endif

    /* exit(0); */
    return 0;
}
