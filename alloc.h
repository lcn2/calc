/*
 * alloc - storage allocation and storage debug macros
 *
 * Copyright (C) 1999  David I. Bell
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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: alloc.h,v 29.4 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/alloc.h,v $
 *
 * Under source code control:	1990/02/15 01:48:29
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__ALLOC_H__)
#define __ALLOC_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "have_malloc.h"
# include "have_newstr.h"
# include "have_string.h"
# include "have_memmv.h"
#else
# include <calc/have_malloc.h>
# include <calc/have_newstr.h>
# include <calc/have_string.h>
# include <calc/have_memmv.h>
#endif

#ifdef HAVE_MALLOC_H
# include <malloc.h>
#else
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || defined(__cplusplus)
   extern void *malloc();
   extern void *realloc();
   extern void free();
# else
   extern char *malloc();
   extern char *realloc();
   extern void free();
# endif
#endif

#ifdef HAVE_STRING_H
# include <string.h>

#else

# if defined(HAVE_NEWSTR)
extern void *memcpy();
extern void *memset();
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || defined(__cplusplus)
extern size_t strlen();
#  else
extern long strlen();
#  endif
# else /* HAVE_NEWSTR */
extern void bcopy();
extern void bfill();
extern char *index();
# endif /* HAVE_NEWSTR */
extern char *strchr();
extern char *strcpy();
extern char *strncpy();
extern char *strcat();
extern int strcmp();

#endif

#if !defined(HAVE_NEWSTR)
#undef memcpy
#define memcpy(s1, s2, n) bcopy(s2, s1, n)
#undef memset
#define memset(s, c, n) bfill(s, n, c)
#undef strchr
#define strchr(s, c) index(s, c)
#endif /* HAVE_NEWSTR */

#if !defined(HAVE_MEMMOVE)
# undef CALC_SIZE_T
#if defined(FORCE_STDC) || (defined(__STDC__) && __STDC__ != 0) || defined(__cplusplus)
#  define CALC_SIZE_T size_t
# else
#  define CALC_SIZE_T long
# endif
extern void *memmove(void *s1, const void *s2, CALC_SIZE_T n);
#endif

#endif /* !__ALLOC_H__ */
