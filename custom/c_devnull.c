/*
 * c_devnull - a custom function that does nothing
 *
 * Copyright (C) 1999-2004,2021,2023,2026  Landon Curt Noll
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
 * Under source code control:   1997/03/09 17:49:12
 * File existed as early as:    1997
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * ISO C requires a translation unit to contain at least one declaration,
 * so we declare a global variable whose value is based on if CUSTOM is defined.
 */
#if defined(CUSTOM)
int c_devnull_allowed = 1; /* CUSTOM defined */
#else
int c_devnull_allowed = 0; /* CUSTOM undefined */
#endif

#if defined(CUSTOM)

/*
 * important <system> header includes
 */
#  include <unistd.h>
#  include <stdint.h>
#  include <stdbool.h>

/*
 * calc local src includes
 */
#  include "../value.h"
#  include "../custom.h"
#  include "../have_unused.h"

#  include "../banned.h" /* include after all other includes */

/*
 * c_devnull - a custom function that does nothing
 *
 * This custom function does nothing.  It is useful as a test hook
 * for looking at the general interface.
 */
/*ARGSUSED*/
VALUE
c_devnull(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result; /* what we will return */

    /*
     * return NULL
     */
    result.v_type = V_NULL;
    return result;
}

#endif
