/*
 * have_arc4random - Determine if we have the arc4random_buf() RNG
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
 * Under source code control:   2021/12/06 23:58:51
 * File existed as early as:    2021
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      have_arc4random
 *
 * Not all enviroments have the arc4random_buf() function,
 * so this may not compile on your system.
 *
 * This prog outputs:
 *
 *      HAVE_ARC4RANDOM
 *              defined ==> have arc4random_buf() call
 *              undefined ==> do not have arc4random_buf() call
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * calc local src includes
 */

#include "banned.h" /* include after system header <> includes */

#define BUFLEN (32) /* length of the buffer to fill */

int
main(void)
{
#if defined(HAVE_NO_ARC4RANDOM)

    printf("#undef HAVE_ARC4RANDOM /* no */\n");

#else

    /* buffer for arc4random_buf() to fill */
    static char buf[BUFLEN];

    arc4random_buf(buf, BUFLEN);
    printf("#define HAVE_ARC4RANDOM /* yes */\n");

#endif

    /* exit(0); */
    return 0;
}
