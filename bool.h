/*
 * bool - calc standard truth :-)
 *
 * Copyright (C) 2023  David I. Bell and Landon Curt Noll
 *
 * Primary author:  David I. Bell
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
 * Under source code control:	2023/07/19 17:58:42
 * File existed as early as:	1990
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_BOOL_H)
#define INCLUDE_BOOL_H


#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */


/*
 * standard truth :-)
 */
#if !defined(HAVE_STDBOOL_H)

/* fake a <stdbool.h> header file */
typedef unsigned char bool;	/* fake boolean value */
#undef true
#define true ((bool)(1))
#undef false
#define false ((bool)(0))

#endif /* !HAVE_STDBOOL_H */


/*
 * calc historic booleans
 */
#undef TRUE
#define TRUE (true)
#undef FALSE
#define FALSE (false)
#undef BOOL
#define BOOL bool


/*
 * booltostr - convert a boolean to a string
 */
#if !defined(booltostr)
#define booltostr(x) ((x) ? "true" : "false")
#endif


/*
 * strtobool - convert a string to a boolean
 */
#if !defined(strtobool)
#define strtobool(x) ((bool) ((x) != NULL && strcmp((x), "true") == 0))
#endif


#endif /* !INCLUDE_BOOL_H*/
