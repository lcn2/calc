/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(__CALCSTRING_H__)
#define	__CALCSTRING_H__


#include "zmath.h"


struct string {
	char *s_str;
	long s_len;
	long s_links;
	struct string *s_next;
};

typedef struct string STRING;


typedef struct {
	char *h_list;	/* list of strings separated by nulls */
	long h_used;	/* characters used so far */
	long h_avail;	/* characters available for use */
	long h_count;	/* number of strings */
} STRINGHEAD;


extern void initstr(STRINGHEAD *hp);
extern char *addstr(STRINGHEAD *hp, char *str);
extern char *namestr(STRINGHEAD *hp, long n);
extern int findstr(STRINGHEAD *hp, char *str);
extern char *charstr(int ch);
extern char *addliteral(char *str);
extern long stringindex(char *str1, char *str2);
extern STRING *stralloc(void);
extern long addstring(char *str, long len);
extern STRING *charstring(int ch);
extern STRING *makestring(char *str);
extern STRING *makenewstring(char *str);
extern STRING *findstring(long index);
extern STRING *slink(STRING *);
extern void sfree(STRING *);
extern void fitstring(char *, long, long);
extern void showstrings(void);
extern void showliterals(void);
extern STRING _nullstring_;


#endif /* !__CALCSTRING_H__ */
