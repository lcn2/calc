/*
 * have_offscl - determine if we have a scalar off_t element
 *
 * Copyright (C) 1999,2021  Landon Curt Noll
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
 * Under source code control:   1996/07/13 12:57:22
 * File existed as early as:    1996
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      have_offscl
 *
 * On some systems, off_t is a scalar value on which one can perform
 * arithmetic operations, assignments and comparisons.  On some systems
 * off_t is some sort of union or struct which must be converted into
 * a ZVALUE in order to perform arithmetic operations, assignments and
 * comparisons.
 *
 *
 * This prog outputs several defines:
 *
 *      HAVE_OFF_T_SCALAR
 *              defined ==> OK to perform arithmetic ops, = and comparisons
 *              undefined ==> convert to ZVALUE first
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif


#include "banned.h"     /* include after system header <> includes */


int
main(void)
{
#if !defined(OFF_T_NON_SCALAR)
        off_t value;    /* an off_t to perform arithmetic on */
        off_t value2;   /* an off_t to perform arithmetic on */

        /*
         * do some math opts on an off_t
         */
        value = (off_t)getpid();
        value2 = (off_t)-1;
        if (value > (off_t)1) {
                --value;
        }
#if !defined(_WIN32) && !defined(_WIN64)
        if (value <= (off_t)getppid()) {
                --value;
        }
#endif
        if (value == value2) {
                value += value2;
        }
        value <<= 1;
        if (!value) {
                printf("/* something for the off_t to do */\n");
        }

        /*
         * report off_t as a scalar
         */
        printf("#undef HAVE_OFF_T_SCALAR\n");
        printf("#define HAVE_OFF_T_SCALAR /* off_t is a simple value */\n");
#else
        printf("#undef HAVE_OFF_T_SCALAR /* off_t is not a simple value */\n");
#endif
        /* exit(0); */
        return 0;
}
