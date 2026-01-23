/*
 * c_pzasusb8 - print numerator as a string of uint8_ts
 *
 * Copyright (C) 1999-2004,2021-2023,2026  Ernest Bowen
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
 * Under source code control:   1999/10/06 03:12:25
 * File existed as early as:    1999
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * ISO C requires a translation unit to contain at least one declaration,
 * so we declare a global variable whose value is based on if CUSTOM is defined.
 */
#if defined(CUSTOM)
int c_pzasusb8_allowed = 1; /* CUSTOM defined */
#else
int c_pzasusb8_allowed = 0; /* CUSTOM undefined */
#endif

#if defined(CUSTOM)

/*
 * important <system> header includes
 */
#  include <stdio.h>
#  include <stdint.h>
#  include <stdbool.h>

/*
 * calc local src includes
 */
#  include "../value.h"
#  include "../custom.h"
#  include "../have_unused.h"
#  include "../attribute.h"
#  include "../errtbl.h"

#  include "../banned.h" /* include after all other includes */

/*
 * c_pzasusb8 - print numerator as a string of uint8_ts
 *
 * given:
 *    count = 1;
 *    vals[0]   real number;
 *
 * returns:
 *    null
 */
/*ARGSUSED*/
VALUE
c_pzasusb8(char *UNUSED(name), int UNUSED(count), VALUE **vals)
{
    VALUE result;  /* what we will return */
    ZVALUE z;      /* numerator of the value */
    long half_cnt; /* number of HALFs in the numerator */
    uint8_t *h;       /* octet pointer */
    long half_len; /* length of a half in octets */
    long i;
    long j;

    /*
     * arg check
     */
    result.v_type = V_NULL;
    if (vals[0]->v_type != V_NUM) {
        math_error("Non-real argument for pzasusb8");
        not_reached();
    }

    /*
     * look at the numerator
     */
    z = vals[0]->v_num->num;
    half_len = sizeof(HALF);
    half_cnt = z.len;

    /*
     * print the octets
     */
    h = (uint8_t *)z.v;
    for (i = 0; i < half_cnt; ++i) {
        printf("%ld:\t", i);
        for (j = 0; j < half_len; ++j) {
            printf("%02x", (int)(*h++));
        }
        putchar('\n');
    }
    return result;
}

#endif
