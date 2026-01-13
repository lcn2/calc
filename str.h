/*
 * str - string list routines
 *
 * Copyright (C) 1999-2007,2014,2025-2026  David I. Bell
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
 * Under source code control:   1990/02/15 01:48:36
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_STR_H)
#  define INCLUDE_STR_H

struct string {
    char *s_str;
    size_t s_len;
    long s_links;
    struct string *s_next;
};

typedef struct string STRING;

typedef struct {
    char *h_list;   /* list of strings separated by nulls */
    size_t h_used;  /* characters used so far */
    size_t h_avail; /* characters available for use */
    long h_count;   /* number of strings */
} STRINGHEAD;

extern STRING _nullstring_;

extern void initstr(STRINGHEAD *hp);
extern char *addstr(STRINGHEAD *hp, char *str);
extern char *namestr(STRINGHEAD *hp, long n);
extern int findstr(STRINGHEAD *hp, char *str);
extern char *addliteral(char *str);
extern STRING *stralloc(void);
extern long addstring(char *str, size_t len);
extern STRING *charstring(int ch);
extern STRING *makestring(char *str);
extern STRING *makenewstring(char *str);
extern STRING *findstring(long index);
extern STRING *slink(STRING *);
extern void sfree(STRING *);
extern void fitstring(char *, long, long);
extern void strprint(STRING *);
extern void showstrings(void);
extern void showliterals(void);

#endif
