/*
 * string - string list routines
 *
 * Copyright (C) 1999-2004  David I. Bell and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * @(#) $Id: string.c,v 29.4 2004/02/23 14:04:01 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/string.c,v $
 *
 * Under source code control:	1990/02/15 01:48:10
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "calc.h"
#include "string.h"

#define STR_TABLECHUNK	100	/* how often to reallocate string table */
#define STR_CHUNK	2000	/* size of string storage allocation */
#define STR_UNIQUE	100	/* size of string to allocate separately */

STRING _nullstring_ = {"", 0, 1, NULL};

static char *chartable;		/* single character string table */

static struct {
	long l_count;		/* count of strings in table */
	long l_maxcount;	/* maximum strings storable in table */
	long l_avail;		/* characters available in current string */
	char *l_alloc;		/* next available string storage */
	char **l_table;		/* current string table */
} literals;


/*
 * Initialize or reinitialize a string header for use.
 *
 * given:
 *	hp		structure to be inited
 */
void
initstr(STRINGHEAD *hp)
{
	if (hp->h_list == NULL) {
		hp->h_list = (char *)malloc(2000);
		hp->h_avail = 2000;
		hp->h_used = 0;
	}
	hp->h_avail += hp->h_used;
	hp->h_used = 0;
	hp->h_count = 0;
	hp->h_list[0] = '\0';
	hp->h_list[1] = '\0';
}


/*
 * Copy a string to the end of a list of strings, and return the address
 * of the copied string.  Returns NULL if the string could not be copied.
 * No checks are made to see if the string is already in the list.
 * The string cannot be null or have imbedded nulls.
 *
 * given:
 *	hp		header of string storage
 *	str		string to be added
 */
char *
addstr(STRINGHEAD *hp, char *str)
{
	char *retstr;		/* returned string pointer */
	char *list;		/* string list */
	long newsize;		/* new size of string list */
	long len;		/* length of current string */

	if ((str == NULL) || (*str == '\0'))
		return NULL;
	len = (long)strlen(str) + 1;
	if (hp->h_avail <= len) {
		newsize = len + 2000 + hp->h_used + hp->h_avail;
		list = (char *)realloc(hp->h_list, newsize);
		if (list == NULL)
			return NULL;
		hp->h_list = list;
		hp->h_avail = newsize - hp->h_used;
	}
	retstr = hp->h_list + hp->h_used;
	hp->h_used += len;
	hp->h_avail -= len;
	hp->h_count++;
	strcpy(retstr, str);
	retstr[len] = '\0';
	return retstr;
}


/*
 * Return a null-terminated string which consists of a single character.
 * The table is initialized on the first call.
 */
char *
charstr(int ch)
{
	char *cp;
	int i;

	if (chartable == NULL) {
		cp = (char *)malloc(512);
		if (cp == NULL) {
			math_error("Cannot allocate character table");
			/*NOTREACHED*/
		}
		for (i = 0; i < 256; i++) {
			*cp++ = (char)i;
			*cp++ = '\0';
		}
		chartable = cp - 512;
	}
	return &chartable[(ch & 0xff) * 2];
}


/*
 * Find a string with the specified name and return its number in the
 * string list.	 The first string is numbered zero.  Minus one is returned
 * if the string is not found.
 *
 * given:
 *	hp		header of string storage
 *	str		string to be added
 */
int
findstr(STRINGHEAD *hp, char *str)
{
	register char *test;	/* string being tested */
	long len;		/* length of string being found */
	long testlen;		/* length of test string */
	int index;		/* index of string */

	if ((hp->h_count <= 0) || (str == NULL))
		return -1;
	len = (long)strlen(str);
	test = hp->h_list;
	index = 0;
	while (*test) {
		testlen = (long)strlen(test);
		if ((testlen == len) && (*test == *str) && (strcmp(test, str) == 0))
			return index;
		test += (testlen + 1);
		index++;
	}
	return -1;
}


/*
 * Return the name of a string with the given index.
 * If the index is illegal, a pointer to an empty string is returned.
 *
 * given:
 *	hp		header of string storage
 *	n
 */
char *
namestr(STRINGHEAD *hp, long n)
{
	register char *str;	/* current string */

	if (n >= hp->h_count)
		return "";
	str = hp->h_list;
	while (*str) {
		if (--n < 0)
			return str;
		str += (strlen(str) + 1);
	}
	return "";
}


/*
 * Useful routine to return the index of one string within another one
 * which has the format:  "str1\000str2\000str3\000...strn\0\0".  Index starts
 * at one for the first string.	 Returns zero if the string being checked
 * is not contained in the formatted string.
 *
 * Be sure to use \000 instead of \0.  ANSI-C compilers interpret "foo\0foo..."
 * as "foo\017oo...".
 *
 * given:
 *	format		string formatted into substrings
 *	test		string to be found in formatted string
 */
long
stringindex(char *format, char *test)
{
	long index;		/* found index */
	long len;		/* length of current piece of string */
	long testlen;		/* length of test string */

	testlen = (long)strlen(test);
	index = 1;
	while (*format) {
		len = (long)strlen(format);
		if ((len == testlen) && (*format == *test) &&
			(strcmp(format, test) == 0))
				return index;
		format += (len + 1);
		index++;
	}
	return 0;
}


/*
 * Add a possibly new literal string to the literal string pool.
 * Returns the new string address which is guaranteed to be always valid.
 * Duplicate strings will repeatedly return the same address.
 */
char *
addliteral(char *str)
{
	register char **table;	/* table of strings */
	char *newstr;		/* newly allocated string */
	long count;		/* number of strings */
	long len;		/* length of string to allocate */

	len = (long)strlen(str);
	if (len <= 1)
		return charstr(*str);
	/*
	 * See if the string is already in the table.
	 */
	table = literals.l_table;
	count = literals.l_count;
	while (count-- > 0) {
		if ((str[0] == table[0][0]) && (str[1] == table[0][1]) &&
			(strcmp(str, table[0]) == 0))
				return table[0];
		table++;
	}
	/*
	 * Make the table of string pointers larger if necessary.
	 */
	if (literals.l_count >= literals.l_maxcount) {
		count = literals.l_maxcount + STR_TABLECHUNK;
		if (literals.l_maxcount)
			table = (char **) realloc(literals.l_table, count * sizeof(char *));
		else
			table = (char **) malloc(count * sizeof(char *));
		if (table == NULL) {
			math_error("Cannot allocate string literal table");
			/*NOTREACHED*/
		}
		literals.l_table = table;
		literals.l_maxcount = count;
	}
	table = literals.l_table;
	/*
	 * If the new string is very long, allocate it manually.
	 */
	len = (len + 2) & ~1;	/* add room for null and round up to word */
	if (len >= STR_UNIQUE) {
		newstr = (char *)malloc(len);
		if (newstr == NULL) {
			math_error("Cannot allocate large literal string");
			/*NOTREACHED*/
		}
		strcpy(newstr, str);
		table[literals.l_count++] = newstr;
		return newstr;
	}
	/*
	 * If the remaining space in the allocate string is too small,
	 * then allocate a new one.
	 */
	if (literals.l_avail < len) {
		newstr = (char *)malloc(STR_CHUNK);
		if (newstr == NULL) {
			math_error("Cannot allocate new literal string");
			/*NOTREACHED*/
		}
		literals.l_alloc = newstr;
		literals.l_avail = STR_CHUNK;
	}
	/*
	 * Allocate the new string from the allocate string.
	 */
	newstr = literals.l_alloc;
	literals.l_avail -= len;
	literals.l_alloc += len;
	table[literals.l_count++] = newstr;
	strcpy(newstr, str);
	return newstr;
}


STRING *
stringadd(STRING *s1, STRING *s2)
{
	STRING *s;
	char *cfrom, *c;
	long len;

	if (s1->s_len == 0)
		return slink(s2);
	if (s2->s_len == 0)
		return slink(s1);
	len = s1->s_len + s2->s_len;
	s = stralloc();
	s->s_len = len;
	s->s_str = (char *) malloc(len + 1);
	if (s->s_str == NULL)
		return NULL;
	len = s1->s_len;
	cfrom = s1->s_str;
	c = s->s_str;
	while (len-- > 0)
		*c++ = *cfrom++;
	len = s2->s_len;
	cfrom = s2->s_str;
	while (len-- > 0)
		*c++ = *cfrom++;
	*c = '\0';
	return s;
}

/*
 * stringneg reverses the characters in a string, returns null if malloc fails
 */
STRING *
stringneg(STRING *str)
{
	long len;
	STRING *s;
	char *c, *cfrom;

	len = str->s_len;
	if (len <= 1)
		return slink(str);
	c = (char *) malloc(len + 1);
	if (c == NULL)
		return NULL;
	s = stralloc();
	s->s_len = len;
	s->s_str = c;
	cfrom = str->s_str + len;
	while (len-- > 0)
		*c++ = *--cfrom;
	*c = '\0';
	return s;
}

STRING *
stringsub(STRING *s1, STRING *s2)
{
	STRING *tmp, *s;

	tmp = stringneg(s2);
	if (tmp == NULL)
		return NULL;
	s = stringadd(s1, tmp);
	if (s != NULL)
		sfree(tmp);
	return s;
}

/*
 * stringmul: repeated concatenation, reverse if negative multiplier
 * returns null if malloc fails
 */
STRING *
stringmul(NUMBER *q, STRING *str)
{
	long len;
	long j;
	NUMBER *tmp1, *tmp2;
	char *c, *c1;
	STRING *s;
	BOOL neg;

	if (str->s_len == 0)
		return slink(str);
	neg = qisneg(q);
	q = neg ? qneg(q): qlink(q);
	tmp1 = itoq(str->s_len);
	tmp2 = qmul(q, tmp1);
	qfree(tmp1);
	tmp1 = qint(tmp2);
	qfree(tmp2);
	if (zge31b(tmp1->num)) {
		qfree(q);
		qfree(tmp1);
		return NULL;
	}
	len = qtoi(tmp1);
	qfree(tmp1);
	qfree(q);
	if (len == 0) {
		s = stralloc();
		s->s_len = 0;
		s->s_str = charstr('\0');
		return s;
	}
	c = (char *) malloc(len + 1);
	if (c == NULL)
		return NULL;
	str = neg ? stringneg(str) : slink(str);
	s = stralloc();
	s->s_str = c;
	s->s_len = len;
	j = 0;
	c1 = str->s_str;
	while (len-- > 0) {
		*c++ = *c1++;
		if (++j == str->s_len) {
			j = 0;
			c1 = str->s_str;
		}
	}
	*c = '\0';
	sfree(str);
	return s;
}

STRING *
stringand(STRING *s1, STRING *s2)
{
	STRING *s;
	long len;
	char *c1, *c2, *c;

	if (s1->s_len == 0  || s2->s_len == 0)
		return slink(&_nullstring_);
	len = s1->s_len;
	if (s2->s_len < len)
		len = s2->s_len;
	s = stralloc();
	s->s_len = len;
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s->s_str = c;
	c1 = s1->s_str;
	c2 = s2->s_str;
	while (len-- > 0)
		*c++ = *c1++ & *c2++;
	return s;
}


STRING *
stringor(STRING *s1, STRING *s2)
{
	STRING *s;
	long len, i, j;
	char *c1, *c2, *c;

	if (s1->s_len < s2->s_len) {
		s = s1;
		s1 = s2;
		s2 = s;
	}		/* Now len(s1) >= len(s2) */
	if (s2->s_len == 0)
		return slink(s1);
	i = s1->s_len;
	if (i == 0)
		return slink(&_nullstring_);
	len = s1->s_len;
	s = stralloc();
	s->s_len = len;
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s->s_str = c;
	c1 = s1->s_str;
	c2 = s2->s_str;
	i = s2->s_len;
	j = s1->s_len - i;
	while (i-- > 0)
		*c++ = *c1++ | *c2++;
	while (j-- > 0)
		*c++ = *c1++;
	return s;
}


STRING *
stringxor(STRING *s1, STRING *s2)
{
	STRING *s;
	long len, i, j;
	char *c1, *c2, *c;

	if (s1->s_len < s2->s_len) {
		s = s1;
		s1 = s2;
		s2 = s;
	}		/* Now len(s1) >= len(s2) */
	if (s2->s_len == 0)
		return slink(s1);
	i = s1->s_len;
	if (i == 0)
		return slink(&_nullstring_);
	len = s1->s_len;
	s = stralloc();
	s->s_len = len;
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s->s_str = c;
	c1 = s1->s_str;
	c2 = s2->s_str;
	i = s2->s_len;
	j = s1->s_len - i;
	while (i-- > 0)
		*c++ = *c1++ ^ *c2++;
	while (j-- > 0)
		*c++ = *c1++;
	return s;
}


STRING *
stringdiff(STRING *s1, STRING *s2)
{
	STRING *s;
	long i;
	char *c2, *c;

	i = s1->s_len;
	if (i == 0)
		return slink(s1);
	s = stringcopy(s1);
	if (i > s2->s_len)
		i = s2->s_len;
	c = s->s_str;
	c2 = s2->s_str;
	while (i-- > 0)
		*c++ &= ~*c2++;
	return s;
}

STRING *
stringcomp(STRING *s1)
{
	long len;
	STRING *s;
	char *c1, *c;

	len = s1->s_len;
	if (len == 0)
		return slink(&_nullstring_);
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s = stralloc();
	s->s_len = len;
	s->s_str = c;
	c1 = s1->s_str;
	while (len-- > 0)
		*c++ = ~*c1++;
	*c = '\0';
	return s;
}

STRING *
stringsegment(STRING *s1, long n1, long n2)
{
	STRING *s;
	char *c, *c1;
	long len;

	if ((n1 < 0 && n2 < 0) || (n1 >= s1->s_len && n2 >= s1->s_len))
		return slink(&_nullstring_);
	if (n1 < 0)
		n1 = 0;
	if (n2 < 0)
		n2 = 0;
	if (n1 >= s1->s_len)
		n1 = s1->s_len - 1;
	if (n2 >= s1->s_len)
		n2 = s1->s_len - 1;
	len = (n1 >= n2) ? n1 - n2 + 1 : n2 - n1 + 1;
	s = stralloc();
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s->s_len = len;
	s->s_str = c;
	c1 = s1->s_str + n1;
	if (n1 >= n2) {
		/*
		 * We prevent the c1 pointer from walking behind s1_s_str
		 * by stopping one short of the end and running the loop one
		 * more time.
		 *
		 * We could stop the loop with just len-- > 0, but stopping
		 * short and running the loop one last time manually helps make
		 * code checkers such as insure happy.
		 */
		while (len-- > 1) {
			*c++ = *c1--;
		}
		/* run the loop manually one last time */
		*c++ = *c1;
	} else {
		while (len-- > 0)
			*c++ = *c1++;
	}
	*c = '\0';
	return s;
}

/*
 * stringshift shifts s1 n bits to left if n > 0, -n to the right if n < 0;
 * octets in string considered to be in decreasing order of index, as in
 * ... a_3 a_2 a_1 a_0.	 Returned string has same length as s1.
 * Vacated bits are filled with '\0'; bits shifted off end are lost
 */
STRING *
stringshift(STRING *s1, long n)
{
	char *c1, *c;
	STRING *s;
	long len, i, j, k;
	BOOL right;
	char ch;

	len = s1->s_len;
	if (len == 0 || n == 0)
		return slink(s1);
	right = (n < 0);
	if (right) n = -n;
	j = n & 7;
	k = 8 - j;
	n >>= 3;
	c = malloc(len + 1);
	if (c == NULL)
		return NULL;
	s = stralloc();
	s->s_len = len;
	s->s_str = c;
	c[len] = '\0';
	if (n > len)
		n = len;
	ch = '\0';
	c1 = s1->s_str;
	i = n;
	if (right) {
		c += len;
		c1 += len;
		while (i-- > 0)
			*--c = '\0';
		i = len - n;
		while (i-- > 0) {
			*--c = ((unsigned char) *--c1 >> j) | ch;
			ch = (unsigned char) *c1 << k;
		}
	} else {
		while (i-- > 0)
			*c++ = '\0';
		i = len - n;
		while (i-- > 0) {
			*c++ = ((unsigned char) *c1 << j) | ch;
			ch = (unsigned char) *c1++ >> k;
		}
	}
	return s;
}

/*
 * stringcpy copies as many characters as possible up to and including
 * the first '\0' from s2 to s1 and returns s1
 */
STRING *
stringcpy(STRING *s1, STRING *s2)
{
	char *c1, *c2;
	long len;

	c1 = s1->s_str;
	c2 = s2->s_str;
	len = s1->s_len;
	while (len-- > 0 && *c2 != 0)
		*c1++ = *c2++;
	*c1 = '\0';
	return slink(s1);
}

/*
 * stringncpy copies up to n characters from s2 to s1 and returns s1
 */
STRING *
stringncpy(STRING *s1, STRING *s2, long num)
{
	char *c1, *c2;

	if (num > s1->s_len)
		num = s1->s_len;
	c1 = s1->s_str;
	c2 = s2->s_str;
	while (num-- > 0 && *c2 != 0)
		*c1++ = *c2++;
	while (num-- > 0)
		*c1++ = '\0';
	*c1 = '\0';
	return slink(s1);
}


/*
 * stringcontent counts the number of set bits in s
 */
long
stringcontent(STRING *s)
{
	char *c;
	unsigned char ch;
	long count;
	long len;

	len = s->s_len;
	count = 0;
	c = s->s_str;
	while (len-- > 0) {
		ch = *c++;
		while (ch) {
			count += (ch & 1);
			ch >>= 1;
		}
	}
	return count;
}

long
stringhighbit(STRING *s)
{
	char *c;
	unsigned char ch;
	long i;

	i = s->s_len;
	c = s->s_str + i;
	while (--i >= 0 && *--c == '\0');
	if (i < 0)
		return -1;
	i <<= 3;
	for (ch = *c; ch >>= 1; i++);
	return i;
}

long
stringlowbit(STRING *s)
{
	char *c;
	unsigned char ch;
	long i;

	for (i = s->s_len, c = s->s_str; i > 0 && *c ==	 '\0'; i--, c++);
	if (i == 0)
		return -1;
	i = (s->s_len - i) << 3;
	for (ch = *c; !(ch & 1); ch >>= 1, i++);
	return i;
}


/*
 * stringcompare compares first len characters of strings starting at c1, c2
 * Returns TRUE if and only if a difference is encountered.
 * Essentially a local version of memcmp.
 */
static BOOL
stringcompare(char *c1, char *c2, long len)
{
	while (len-- > 0) {
		if (*c1++ != *c2++)
			return TRUE;
	}
	return FALSE;
}

/*
 * stringcmp returns TRUE if strings differ, FALSE if strings equal
 */
BOOL
stringcmp(STRING *s1, STRING *s2)
{
	if (s1->s_len != s2->s_len)
		return TRUE;
	return stringcompare(s1->s_str, s2->s_str, s1->s_len);
}


/*
 * stringrel returns 0 if strings are equal; otherwise 1 or -1 according
 * as the greater of the first unequal characters are in the first or
 * second string, or in the case of unequal-length strings when the compared
 * characters are all equal, 1 or -1 according as the first or second string
 * is longer.
 */
FLAG
stringrel(STRING *s1, STRING *s2)
{
	char *c1, *c2;
	long i1, i2;

	i1 = s1->s_len;
	i2 = s2->s_len;
	if (i1 == 0)
		return (i2 > 0);
	if (i2 == 0)
		return -1;
	c1 = s1->s_str;
	c2 = s2->s_str;
	while (i1 > 0 && i2 > 0 && *c1 == *c2) {
		i1--;
		i2--;
		c1++;
		c2++;
	}
	if ((unsigned char) *c1 > (unsigned char) *c2) return 1;
	if ((unsigned char) *c1 < (unsigned char) *c2) return -1;
	if (i1 < i2) return -1;
	return (i1 > i2);
}


/*
 * str with characters c0, c1, ... is considered as a bitstream, 8 bits
 * per character; within a character the bits ordered from low order to
 * high order.	For 0 <= i < 8 * length of str, stringbit returns 1 or 0
 * according as the bit with index i is set or not set; other values of i
 * return -1.
 */
int
stringbit(STRING *s, long index)
{
	unsigned int ch;
	int res;

	if (index < 0)
		return -1;
	res = index & 7;
	index >>= 3;
	if (index >= s->s_len)
		return -1;
	ch = s->s_str[index];
	return (ch >> res) & 1;
}


BOOL
stringtest(STRING *s)
{
	long i;
	char *c;

	i = s->s_len;
	c = s->s_str;
	while (i-- > 0) {
		if (*c++)
			return TRUE;
	}
	return FALSE;
}

/*
 * If index is in acceptable range, stringsetbit sets or resets specified
 * bit in string s according as val is TRUE or FALSE, and returns 0.
 * Returns 1 if index < 0; 2 if index too large.
 */
int
stringsetbit(STRING *s, long index, BOOL val)
{
	char *c;
	int bit;

	if (index < 0)
		return 1;
	bit = 1 << (index & 7);
	index >>= 3;
	if (index >= s->s_len)
		return 2;
	c = &s->s_str[index];
	*c &= ~bit;
	if (val)
		*c |= bit;
	return 0;
}

/*
 * stringsearch returns 0 and sets index = i if the first occurrence
 * of s2 in s1 for start <= i < end is at index i.  If no such occurrence
 * is found, -1 is returned.
 */
int
stringsearch(STRING *s1, STRING *s2, long start, long end, ZVALUE *index)
{
	long len2, i, j;
	char *c1, *c2, *c;
	char ch;

	len2 = s2->s_len;
	if (start < 0)
		start = 0;
	if (end < start + len2)
		return -1;
	if (len2 == 0) {
		itoz(start, index);
		return 0;
	}
	i = end - start - len2;
	c1 = s1->s_str + start;
	ch = *s2->s_str;
	while (i-- >= 0) {
		if (*c1++ == ch) {
			c = c1;
			c2 = s2->s_str;
			j = len2;
			while (--j > 0 && *c++ == *++c2);
			if (j == 0) {
				itoz(end - len2 - i - 1, index);
				return 0;
			}
		}
	}
	return -1;
}

int
stringrsearch(STRING *s1, STRING *s2, long start, long end, ZVALUE *index)
{
	long len1, len2, i, j;
	char *c1, *c2, *c, *c2top;
	char ch;

	len1 = s1->s_len;
	len2 = s2->s_len;
	if (start < 0)
		start = 0;
	if (end > len1)
		end = len1;
	if (end < start + len2)
		return -1;
	if (len2 == 0) {
		itoz(start, index);
		return 0;
	}
	i = end - start - len2 + 1;
	c1 = s1->s_str + end - 1;
	c2top = s2->s_str + len2 - 1;
	ch = *c2top;

	while (--i >= 0) {
		if (*c1-- == ch) {
			c = c1;
			j = len2;
			c2 = c2top;
			while (--j > 0 && *c-- == *--c2);
			if (j == 0) {
				itoz(start + i, index);
				return 0;
			}
		}
	}
	return -1;
}


/*
 * String allocation routines
 */

#define STRALLOC	100


static STRING	*freeStr = NULL;
static STRING	**firstStrs = NULL;
static long	blockcount = 0;


STRING *
stralloc(void)
{
	STRING *temp;
	STRING **newfn;

	if (freeStr == NULL) {
		freeStr = (STRING *) malloc(sizeof (STRING) * STRALLOC);
		if (freeStr == NULL) {
			math_error("Unable to allocate memory for stralloc");
			/*NOTREACHED*/
		}
		freeStr[STRALLOC - 1].s_next = NULL;
		freeStr[STRALLOC - 1].s_links = 0;

		/*
		 * We prevent the temp pointer from walking behind freeStr
		 * by stopping one short of the end and running the loop one
		 * more time.
		 *
		 * We would stop the loop with just temp >= freeStr, but
		 * doing this helps make code checkers such as insure happy.
		 */
		for (temp = freeStr + STRALLOC - 2; temp > freeStr; --temp) {
			temp->s_next = temp + 1;
			temp->s_links = 0;
		}
		/* run the loop manually one last time */
		temp->s_next = temp + 1;
		temp->s_links = 0;

		blockcount++;
		if (firstStrs == NULL) {
		    newfn = (STRING **) malloc( blockcount * sizeof(STRING *));
		} else {
		    newfn = (STRING **)
			    realloc(firstStrs, blockcount * sizeof(STRING *));
		}
		if (newfn == NULL) {
			math_error("Cannot allocate new string block");
			/*NOTREACHED*/
		}
		firstStrs = newfn;
		firstStrs[blockcount - 1] = freeStr;
	}
	temp = freeStr;
	freeStr = temp->s_next;
	temp->s_links = 1;
	temp->s_str = NULL;
	return temp;
}


/*
 * makestring to be called only when str is the result of a malloc
 */
STRING *
makestring(char *str)
{
	STRING *s;
	long len;

	len = (long)strlen(str);
	s = stralloc();
	s->s_str = str;
	s->s_len = len;
	return s;
}

STRING *
charstring(int ch)
{
	STRING *s;
	char *c;

	c = (char *) malloc(2);
	if (c == NULL) {
		math_error("Allocation failure for charstring");
		/*NOTREACHED*/
	}
	s = stralloc();
	s->s_len = 1;
	s->s_str = c;
	*c++ = (char) ch;
	*c = '\0';
	return s;
}


/*
 * makenewstring creates a new string by copying null-terminated str;
 * str is not freed
 */
STRING *
makenewstring(char *str)
{
	STRING *s;
	char *c;
	long len;

	len = (long)strlen(str);
	if (len == 0)
		return slink(&_nullstring_);
	c = (char *) malloc(len + 1);
	if (c == NULL) {
		math_error("malloc for makenewstring failed");
		/*NOTREACHED*/
	}
	s = stralloc();
	s->s_str = c;
	s->s_len = len;
	memcpy(c, str, len);
	c[len] = '\0';
	return s;
}


STRING *
stringcopy (STRING *s1)
{
	STRING *s;
	char *c;
	long len;

	len = s1->s_len;
	if (len == 0)
		return slink(s1);
	c = malloc(len + 1);
	if (c == NULL) {
		math_error("Malloc failed for stringcopy");
		/*NOTREACHED*/
	}
	s = stralloc();
	s->s_len = len;
	s->s_str = c;
	memcpy(c, s1->s_str, len);
	c[len] = '\0';
	return s;
}


STRING *
slink(STRING *s)
{
	if (s->s_links <= 0) {
		math_error("Argument for slink has nonpositive links!!!");
		/*NOTREACHED*/
	}
	++s->s_links;
	return s;
}


void
sfree(STRING *s)
{
	if (s->s_links <= 0) {
		math_error("Argument for sfree has nonpositive links!!!");
		/*NOTREACHED*/
	}
	if (--s->s_links > 0 || s->s_len == 0)
		return;
	free(s->s_str);
	s->s_next = freeStr;
	freeStr = s;
}

static long	stringconstcount = 0;
static long	stringconstavail = 0;
static STRING	**stringconsttable;
#define STRCONSTALLOC 100

void
initstrings(void)
{
	stringconsttable = (STRING **) malloc(sizeof(STRING *) * STRCONSTALLOC);
	if (stringconsttable == NULL) {
		math_error("Unable to allocate constant table");
		/*NOTREACHED*/
	}
	stringconsttable[0] = &_nullstring_;
	stringconstcount = 1;
	stringconstavail = STRCONSTALLOC - 1;
}

/*
 * addstring is called only from token.c
 * When called, len is length of string including '\0'
 */
long
addstring(char *str, long len)
{
	STRING **sp;
	STRING *s;
	char *c;
	long index;		/* index into constant table */
	long first;		/* first non-null position found */
	BOOL havefirst;

	if (stringconstavail <= 0) {
		if (stringconsttable == NULL) {
			initstrings();
		} else {
			sp = (STRING **) realloc((char *) stringconsttable,
			sizeof(STRING *) * (stringconstcount + STRCONSTALLOC));
			if (sp == NULL) {
				math_error("Unable to reallocate string const table");
				/*NOTREACHED*/
			}
			stringconsttable = sp;
			stringconstavail = STRCONSTALLOC;
		}
	}
	len--;
	first = 0;
	havefirst = FALSE;
	sp = stringconsttable;
	for (index = 0; index < stringconstcount; index++, sp++) {
		s = *sp;
		if (s->s_links == 0) {
			if (!havefirst) {
				havefirst = TRUE;
				first = index;
			}
			continue;
		}
		if (s->s_len == len && stringcompare(s->s_str, str, len) == 0) {
			s->s_links++;
			return index;
		}
	}
	s = stralloc();
	c = (char *) malloc(len + 1);
	if (c == NULL) {
		math_error("Unable to allocate string constant memory");
		/*NOTREACHED*/
	}
	s->s_str = c;
	s->s_len = len;
	memcpy(s->s_str, str, len+1);
	if (havefirst) {
		stringconsttable[first] = s;
		return first;
	}
	stringconstavail--;
	stringconsttable[stringconstcount++] = s;
	return index;
}


STRING *
findstring(long index)
{
	if (index < 0 || index >= stringconstcount) {
		math_error("Bad index for findstring");
		/*NOTREACHED*/
	}
	return stringconsttable[index];
}


void
freestringconstant(long index)
{
	STRING *s;
	STRING **sp;

	if (index >= 0) {
		s = findstring(index);
		sfree(s);
		if (index == stringconstcount - 1) {
			sp = &stringconsttable[index];
			while (stringconstcount > 0 && (*sp)->s_links == 0) {
				stringconstcount--;
				stringconstavail++;
				sp--;
			}
		}
	}
}

long
printechar(char *c)
{
	long n;
	unsigned char ch;
	unsigned char ech;	/* for escape sequence */
	unsigned char nch;	/* for next character */
	BOOL three;

	ch = *c;
	if (ch >= ' ' && ch < 127 && ch != '\\' && ch != '\"') {
		putchar(ch);
		return 1;
	}
	putchar('\\');
	ech = 0;
	switch (ch) {
		case '\n': ech = 'n'; break;
		case '\r': ech = 'r'; break;
		case '\t': ech = 't'; break;
		case '\b': ech = 'b'; break;
		case '\f': ech = 'f'; break;
		case '\v': ech = 'v'; break;
		case '\\': ech = '\\'; break;
		case '\"': ech = '\"'; break;
		case 7: ech = 'a'; break;
		case 27: ech = 'e'; break;
	}
	if (ech) {
		putchar(ech);
		return 2;
	}
	nch = *(c + 1);
	three = (nch >= '0' && nch < '8');
	n = 2;
	if (three || ch >= 64) {
		putchar('0' + ch/64);
		n++;
	}
	ch %= 64;
	if (three || ch >= 8) {
		putchar('0' + ch/8);
		n++;
	}
	ch %= 8;
	putchar('0' + ch);
	return n;
}


void
fitstring(char *str, long len, long width)
{
	long i, j, n, max;
	char *c;
	unsigned char ch, nch;

	max = (width - 3)/2;
	if (len == 0)
		return;
	c = str;
	for (i = 0, n = 0; i < len && n < max; i++) {
		n += printechar(c++);
	}
	if (i >= len)
		return;
	c = str + len;
	nch = '\0';
	for (n = 0, j = len ; j > i && n < max ; --j, nch = ch) {
		ch = *--c;
		n++;
		if (ch >= ' ' && ch <= 127 && ch != '\\' && ch != '\"')
			continue;
		n++;
		switch (ch) {
			case '\n': case '\r': case '\t': case '\b': case '\f':
			case '\v': case '\\': case '\"': case 7: case 27:
				continue;
		}
		if (ch >= 64 || (nch >= '0' && nch <= '7')) {
			n += 2;
			continue;
		}
		if (ch >= 8)
			n++;
	}
	if (j > i)
		printf("...");
	while (j++ < len)
		(void) printechar(c++);
}

void
showstrings(void)
{
	STRING *sp;
	long i, j, k;
	long count;


	printf("Index  Links  Length  String\n");
	printf("-----  -----  ------  ------\n");
	sp = &_nullstring_;
	printf("    0  %5ld	  0  \"\"\n", sp->s_links);
	for (i = 0, k = 1, count = 1; i < blockcount; i++) {
		sp = firstStrs[i];
		for (j = 0; j < STRALLOC; j++, k++, sp++) {
			if (sp->s_links > 0) {
				++count;
				printf("%5ld  %5ld  %6ld  \"",
					k, sp->s_links, sp->s_len);
				fitstring(sp->s_str, sp->s_len, 50);
				printf("\"\n");
			}
		}
	}
	printf("\nNumber: %ld\n", count);
}


void
showliterals(void)
{
	STRING *sp;
	long i;
	long count = 0;


	printf("Index  Links  Length  String\n");
	printf("-----  -----  ------  ------\n");
	for (i = 0; i < stringconstcount; i++) {
		sp = stringconsttable[i];
		if (sp->s_links > 0) {
			++count;
			printf("%5ld  %5ld  %6ld  \"",
				i, sp->s_links, sp->s_len);
			fitstring(sp->s_str, sp->s_len, 50);
			printf("\"\n");
		}

	}
	printf("\nNumber: %ld\n", count);
}
