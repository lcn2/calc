/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(__ALLOC_H__)
#define __ALLOC_H__


#include "have_malloc.h"
#include "have_newstr.h"
#include "have_string.h"
#include "have_memmv.h"

#ifdef HAVE_MALLOC_H
# include <malloc.h>
#else
# if defined(__STDC__) && __STDC__ != 0
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
#  if defined(__STDC__) && __STDC__ != 0
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
# if defined(__STDC__) && __STDC__ != 0
#  define CALC_SIZE_T size_t
# else
#  define CALC_SIZE_T long
# endif
extern void *memmove(void *s1, const void *s2, CALC_SIZE_T n);
#endif

#endif /* !__ALLOC_H__ */
