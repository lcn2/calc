/*
 * Copyright (c) 1995 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(ALLOC_H)
#define ALLOC_H

#include "have_malloc.h"
#include "have_newstr.h"
#include "have_string.h"

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
extern long strlen();	/* should be size_t, but old systems don't have it */
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

#endif /* !ALLOC_H */
