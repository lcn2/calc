/*
 * longbits - Determine the number if bits in a long
 *
 * We need LONG_BITS to be an integer, instead defined as sizeof(long)*CHAR_BIT
 * because we want the cpp to be able to perform #if comparison on the value.
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  Landon Curt Noll
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
 * Under source code control:   1994/03/18 03:06:18
 * File existed as early as:    1994
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*
 * calc local src includes
 */

#include "banned.h" /* include after all other includes */

int
main(void)
{
    /*
     * report size of a long
     */
    printf("#undef LONG_BITS\n");
    printf("#define LONG_BITS %ld\t\t/%s/\n", sizeof(long) * CHAR_BIT, "* bit length of a long *");

    /*
     * All Done!!! -- Jessica Noll, Age 2
     */
    exit(0); /*ooo*/
}
