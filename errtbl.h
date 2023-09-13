/*
 * errtbl - calc error code table entry
 *
 * Copyright (C) 2023  Ernest Bowen and Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	2023/09/12 20:55:14
 * File existed as early as:	2023
 *
 * Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_ERRTBL_H)
#define INCLUDE_ERRTBL_H


#include "have_const.h"
#include "decl.h"


/*
 * primary error code defines
 */
#define E__NONE 0		/* calc_errno cleared: libc errno codes above here */
#define E__BASE 10000		/* calc errors start above here */
#define E__USERDEF 20000	/* user defined error codes start here */
#define E__USERMAX 32767	/* maximum user defined error code */


/*
 * calc error code, error symbol and error message
 */
struct errtbl {
	int errnum;		/* calc error code or -1 */
	char *errsym;		/* error symbol string - must match the regular expression: ^E_[A-Z0-9_]+$ or NULL */
	char *errmsg;		/* calc error message or NULL */
};


/*
 * The error_table[] array represents the calc computation error related
 * error codes, symbols and messages.
 *
 * The errnum of the 1st entry error_table[0] must be E__BASE.
 *
 * All errnum for the following entries just be consecutive,
 * except for the final NULL entry.
 *
 * The final entry must have an errnum of -1, errsym of NULL and errmsg of NULL.
 */
EXTERN CONST struct errtbl error_table[];	/* calc error codes, error symbols and error messages */


#endif /* !INCLUDE_ERRTBL_H */
