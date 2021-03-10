/*
 * c_help - custom help function
 *
 * Copyright (C) 1999-2004,2021  Landon Curt Noll
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
 * Under source code control:	1997/03/09 05:25:41
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if defined(CUSTOM)

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_const.h"
#include "value.h"
#include "custom.h"

#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * c_help - custom help function
 *
 * This function assumes that a help file with the same name as
 * the custom function has been installed by the custom/Makefile
 * (as listed in the CUSTOM_HELP makefile variable) under the
 * CUSTOMHELPDIR == HELPDIR/custhelp directory.
 *
 * The help command first does a search in HELPDIR and later
 * in CUSTOMHELPDIR.  If a custom help file has the same name
 * as a file under HELPDIR then help will display the HELPDIR
 * file and NOT the custom file.  This function will ignore
 * and HELPDIR file and work directly with the custom help file.
 *
 * given:
 *	vals[0]	   name of the custom help file to directly access
 */
/*ARGSUSED*/
VALUE
c_help(char *UNUSED(name), int UNUSED(count), VALUE **vals)
{
	VALUE result;		/* what we will return */

	/*
	 * parse args
	 */
	if (vals[0]->v_type != V_STR) {
		math_error("custom help arg 1 must be a string");
		/*NOTREACHED*/
	}

	/*
	 * give the help
	 */
	customhelp((char *)vals[0]->v_str);

	/*
	 * return NULL
	 */
	result.v_type = V_NULL;
	return result;
}

#endif /* CUSTOM */
