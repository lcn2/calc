/*
 * decl - variable and function declaration macros
 *
 * Copyright (C) 2007,2014,2021  Landon Curt Noll
 *
 * Primary author:  Landon Curt Noll
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
 * Under source code control:	2007/02/09 05:24:25
 * File existed as early as:	2007
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "have_const.h"


#if !defined(INCLUDE_DECL_H)
#define INCLUDE_DECL_H


/*
 * Thread Local Storage macros
 *
 * NOTE: The use of -DWITH_TLS is extremely experimental.  Calc may not
 *	 compile with WITH_TLS defined.
 */
#if defined(WITH_TLS)


  /* variable related macros */
# define EXTERN extern __thread
# define STATIC static __thread

  /* function related macros */
# define E_FUNC extern
# define S_FUNC static


/*
 * MS windoz macros
 */
#elif defined(_WIN32) || defined(WINDOZ)


  /* determine which type of DLL we must generate */
# if !defined(STATIC_ONLY)
#  if defined(_EXPORTING)
#   define DLL __declspec(dllexport)
#  else
#   define DLL __declspec(dllimport)
#  endif
# else
#  define DLL
# endif

  /* variable related macros */
# define EXTERN extern DLL
# define STATIC static

  /* function related macros */
# define E_FUNC extern DLL
# define S_FUNC static


/*
 * default macros
 */
#else


  /* variable related macros */
# define EXTERN extern
# define STATIC static

  /* function related macros */
# define E_FUNC extern
# define S_FUNC static


#endif

/* Perform printf-style argument type checking for known compilers */
#ifdef __GNUC__
# define PRINTF_FORMAT(fmt_idx, arg_idx) __attribute__ \
         ((format (printf, fmt_idx, arg_idx)))
#else
# define PRINTF_FORMAT(fmt_idx, arg_idx)
#endif

#endif /* !INCLUDE_DECL_H */
