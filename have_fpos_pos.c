/*
 * have_fpos_pos - Determine if a __pos element in fpos_t
 *
 * Copyright (C) 2000,2021,2026  Landon Curt Noll
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
 * Under source code control:   2000/12/17 01:23
 * File existed as early as:    2000
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <string.h>

/*
 * calc local src includes
 */
#include "have_posscl.h"

#include "banned.h" /* include after all other includes */

int
main(void)
{
#if defined(HAVE_FPOS_T_SCALAR)

    printf("/* HAVE_FPOS_T_SCALAR is defined, we assume fpos_t is scalar */\n");
    printf("/* we assume we have no __pos in fpos_t */\n");
    printf("#undef HAVE_FPOS_POS\t/* no */\n");
    printf("#undef FPOS_POS_BITS\n");
    printf("#undef FPOS_POS_LEN\n");

#else

    printf("/* HAVE_FPOS_T_SCALAR is undefined, we assume fpos_t is not scalar */\n");
#  if defined(HAVE_NO_FPOS_POS)
    printf("/* HAVE_NO_FPOS_POS defiled, we assume we have no __pos in fpos_t */\n");
    printf("#undef HAVE_FPOS_POS\t/* no */\n");
    printf("#undef FPOS_POS_BITS\n");
    printf("#undef FPOS_POS_LEN\n");

#  elif defined(FPOS_POS_BITS)
    printf("/* FPOS_POS_BITS defiled, assume we have __pos in fpos_t */\n");
    printf("#undef HAVE_FPOS_POS\n");
    printf("#define HAVE_FPOS_POS 1  /* yes */\n");
    printf("#undef FPOS_POS_BITS\n");
    printf("#define FPOS_POS_BITS %d\n", FPOS_POS_BITS);
    printf("#undef FPOS_POS_LEN\n");
    printf("#define FPOS_POS_LEN %d\n", int(FPOS_POS_BITS / 8));
#  else
    fpos_t pos; /* file position */

    memset(&pos, 0, sizeof(pos)); /* zeroize pos to "set it" */
    printf("/* we successfully compiled with a fpos_t type wit an __pos element */\n");
    printf("#undef HAVE_FPOS_POS\n");
    printf("#define HAVE_FPOS_POS 1  /* yes */\n");
    printf("#undef FPOS_POS_BITS\n");
    printf("#define FPOS_POS_BITS %zu\n", sizeof(pos.__pos) * 8);
    printf("#undef FPOS_POS_LEN\n");
    printf("#define FPOS_POS_LEN %zu\n", sizeof(pos.__pos));
#  endif
#endif
    /* exit(0); */
    return 0;
}
