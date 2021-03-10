/*
 * str - string list routines
 *
 * Copyright (C) 1999-2007,2014  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:36
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_STR_H)
#define INCLUDE_STR_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "zmath.h"
#else
# include <calc/zmath.h>
#endif



struct string {
	char *s_str;
	size_t s_len;
	long s_links;
	struct string *s_next;
};

typedef struct string STRING;


typedef struct {
	char *h_list;	/* list of strings separated by nulls */
	size_t h_used;	/* characters used so far */
	size_t h_avail;	/* characters available for use */
	long h_count;	/* number of strings */
} STRINGHEAD;

EXTERN STRING _nullstring_;

E_FUNC void initstr(STRINGHEAD *hp);
E_FUNC char *addstr(STRINGHEAD *hp, char *str);
E_FUNC char *namestr(STRINGHEAD *hp, long n);
E_FUNC int findstr(STRINGHEAD *hp, char *str);
E_FUNC char *charstr(int ch);
E_FUNC char *addliteral(char *str);
E_FUNC long stringindex(char *str1, char *str2);
E_FUNC STRING *stralloc(void);
E_FUNC long addstring(char *str, size_t len);
E_FUNC STRING *charstring(int ch);
E_FUNC STRING *makestring(char *str);
E_FUNC STRING *makenewstring(char *str);
E_FUNC STRING *findstring(long index);
E_FUNC STRING *slink(STRING *);
E_FUNC void sfree(STRING *);
E_FUNC void fitstring(char *, long, long);
E_FUNC void strprint(STRING *);
E_FUNC void showstrings(void);
E_FUNC void showliterals(void);


#endif /* !INCLUDE_STR_H */
