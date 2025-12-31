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
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   2023/09/12 20:55:14
 * File existed as early as:    2023
 *
 * Share and enjoy!  :-)                http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_ERRTBL_H)
#  define INCLUDE_ERRTBL_H

#  include "attribute.h"
#  include "have_const.h"
#  include "bool.h"
#  include "decl.h"

/*
 * NOTE: See also errsym.h, the file that this code, via errcode -d, via the Makefile, creates
 *
 * We cannot use errsym.h when compiling for errcode (ERRCODE_SRC defined)
 */
#  if !defined(ERRCODE_SRC)
#    include "errsym.h"
#  endif /* !ERRCODE_SRC */

/*
 * primary error code defines
 */
#  define E__NONE 0        /* calc_errno cleared: libc errno codes above here */
#  define E__BASE 10000    /* calc computation error codes start above here */
#  define E__USERDEF 20000 /* user defined error codes start here */
#  define E__USERMAX 32767 /* maximum user defined error code */

#  define USERMAX_DIGITS 5 /* number of decimal digits in E__USERMAX */

/*
 * invalid errnum
 */
#  define NULL_ERRNUM (-1) /* errnum for the final table terminating NULL entry */

/*
 * The error routine.
 */
E_FUNC void math_error(char *, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));

/*
 * calc error code, error symbol and error message
 */
struct errtbl {
    int errnum;   /* calc computation error codes or -1 */
    char *errsym; /* E_STRING - must match regexp: ^E_[A-Z0-9_]+$ or NULL */
    char *errmsg; /* calc error message or NULL */
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
EXTERN CONST struct errtbl error_table[]; /* calc error codes, error symbols and error messages */

/*
 * external functions
 */
E_FUNC bool is_e_digits(CONST char *errsym);
E_FUNC bool is_valid_errnum(int errnum);
E_FUNC bool is_errnum_in_error_table(int errnum);
E_FUNC int e_digits_2_errnum(CONST char *errsym);
E_FUNC bool is_e_1string(CONST char *errsym);
E_FUNC bool is_e_2string(CONST char *errsym);
E_FUNC struct errtbl *find_errsym_in_errtbl(CONST char *errsym, CONST struct errtbl *tbl);
E_FUNC struct errtbl *find_errnum_in_errtbl(int errnum, CONST struct errtbl *tbl);
E_FUNC CONST struct errtbl *lookup_errnum_in_error_table(int errnum);
E_FUNC void verify_error_table(void);
E_FUNC int errsym_2_errnum(CONST char *errsym);
E_FUNC char *errnum_2_errsym(int errnum, bool *palloced);
E_FUNC char *errnum_2_errmsg(int errnum, bool *palloced);
E_FUNC char *errsym_2_errmsg(CONST char *errsym, bool *palloced);

#endif /* !INCLUDE_ERRTBL_H */
