/*
 * Copyright (c) 1993 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */

#ifndef	CALCSTRING_H
#define	CALCSTRING_H

#include "zmath.h"


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

#endif

/* END CODE */
