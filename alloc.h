/*
 * alloc - storage allocation and storage debug macros
 *
 * Copyright (C) 1999-2007,2014,2025  David I. Bell
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
 * Under source code control:   1990/02/15 01:48:29
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_ALLOC_H)
#  define INCLUDE_ALLOC_H

#  if defined(CALC_SRC) /* if we are building from the calc source tree */
#    include "decl.h"
#    include "have_newstr.h"
#    include "have_string.h"
#    include "have_const.h"
#  else
#    include <calc/decl.h>
#    include <calc/have_newstr.h>
#    include <calc/have_string.h>
#    include <calc/have_const.h>
#  endif

#  ifdef HAVE_STRING_H
#    include <string.h>

#  else
#    include <stdio.h>

#    if defined(HAVE_NEWSTR)
E_FUNC void *memcpy();
E_FUNC void *memset();
#      if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || defined(__cplusplus)
E_FUNC size_t strlen();
#      else
E_FUNC long strlen();
#      endif
#    else  /* HAVE_NEWSTR */
E_FUNC void bcopy();
E_FUNC void bfill();
E_FUNC char *index();
#    endif /* HAVE_NEWSTR */
E_FUNC char *strchr();
E_FUNC char *strcpy();
E_FUNC char *strncpy();
E_FUNC char *strcat();
E_FUNC int strcmp();

#  endif

#  if !defined(HAVE_NEWSTR)
#    undef memcpy
#    define memcpy(s1, s2, n) bcopy(s2, s1, n)
#    undef memset
#    define memset(s, c, n) bfill(s, n, c)
#    undef strchr
#    define strchr(s, c) index(s, c)
#  endif /* HAVE_NEWSTR */

#endif /* !INCLUDE_ALLOC_H */
