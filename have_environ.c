/*
 * have_environ - Determine if we have the environ user environment synbol
 *
 * Copyright (C) 2021  Landon Curt Noll
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
 * Under source code control:   2021/12/06 19:34:32
 * File existed as early as:    2021
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      have_environ
 *
 * Not all enviroments have the user environment external:
 *
 *      extern char **environ;
 *
 * so this may not compile on your system.
 *
 * This prog outputs:
 *
 *      HAVE_ENVIRON
 *              defined ==> environ is an non-NULL extern symbol
 *              undefined ==> environ is NOT an extern symbol or is NULL
 */

#include <stdio.h>


#include "banned.h"     /* include after system header <> includes */


int
main(void)
{
#if defined(HAVE_NO_ENVIRON)

        printf("#undef HAVE_ENVIRON /* no */\n");

#else /* HAVE_NO_ENVIRON */

        extern char **environ;

        if (environ == NULL) {
                printf("#define HAVE_ENVIRON /* no */\n");
        } else {
                printf("#define HAVE_ENVIRON /* yes */\n");
        }

#endif /* HAVE_NO_ENVIRON */

        /* exit(0); */
        return 0;
}
