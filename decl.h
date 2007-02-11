/*
 * decl - variable and function declaration macros
 *
 * Copyright (C) 2007  Landon Curt Noll
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.2 $
 * @(#) $Id: decl.h,v 29.2 2007/02/11 10:19:14 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/decl.h,v $
 *
 * Under source code control:	2007/02/09 05:24:25
 * File existed as early as:	2007
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "have_const.h"


#if !defined(__DECL_H__)
#define __DECL_H__


/*
 * Mac OS X macros that promote thread safety
 */
#if defined(MACOSX_TLS)


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
# if defined(_EXPORTING)
#  define DLL __declspec(dllexport)
# else
#  define DLL __declspec(dllimport)
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

#endif /* !__DECL_H__ */
