/*
 * c_devnull - a custom function that does nothing
 *
 * Copyright (C) 1999-2004  Landon Curt Noll
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
 * @(#) $Revision: 30.1 $
 * @(#) $Id: c_devnull.c,v 30.1 2007/03/16 11:10:04 chongo Exp $
 * @(#) $Source: /usr/local/src/bin/calc/custom/RCS/c_devnull.c,v $
 *
 * Under source code control:	1997/03/09 17:49:12
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


/*
 * c_devnull - a custom function that does nothing
 *
 * This custom function does nothing.  It is useful as a test hook
 * for looking at the general interface.
 */
/*ARGSUSED*/
VALUE
c_devnull(char UNUSED *name, int UNUSED count, VALUE UNUSED **vals)
{
	VALUE result;		/* what we will return */

	/*
	 * return NULL
	 */
	result.v_type = V_NULL;
	return result;
}

#endif /* CUSTOM */
