/*
 * const - constant number storage module
 *
 * Copyright (C) 1999-2007,2021  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:14
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "calc.h"
#include "qmath.h"


#include "banned.h"	/* include after system header <> includes */


#define CONSTALLOCSIZE 400	/* number of constants to allocate */

STATIC unsigned long constcount;/* number of constants defined */
STATIC long constavail;		/* number of constants available */
STATIC NUMBER **consttable;	/* table of constants */


void
initconstants(void)
{
	int i;

	consttable = (NUMBER **) malloc(sizeof(NUMBER *) * CONSTALLOCSIZE);
	if (consttable == NULL) {
		math_error("Unable to allocate constant table");
		/*NOTREACHED*/
	}
	for (i = 0; i < INITCONSTCOUNT; i++)
		consttable[i] = initnumbs[i];
	consttable[INITCONSTCOUNT] = NULL;	/* firewall */
	constcount = INITCONSTCOUNT;
	constavail = CONSTALLOCSIZE - INITCONSTCOUNT;
}


/*
 * Read in a literal real number and add it to the table of constant numbers,
 * creating a new entry if necessary.  The incoming number is a string
 * value which must have a correct format.
 * Returns the index of the number in the constant table.
 *
 * given:
 *	str		string representation of number
 */
long
addnumber(char *str)
{
	NUMBER *q;

	q = str2q(str);
	return addqconstant(q);
}


/*
 * Add a particular number to the constant table.
 * Returns the index of the number in the constant table, or zero
 * if the number could not be saved.  The incoming number if freed
 * if it is already in the table.
 *
 * XXX - we should hash the constant table
 *
 * given:
 *	q		number to be added
 */
long
addqconstant(NUMBER *q)
{
	register NUMBER **tp;	/* pointer to current number */
	register NUMBER *t;	/* number being tested */
	unsigned long index;	/* index into constant table */
	long numlen;		/* numerator length */
	long denlen;		/* denominator length */
	HALF numlow;		/* bottom value of numerator */
	HALF denlow;		/* bottom value of denominator */
	long first;		/* first non-null position found */
	BOOL havefirst;

	if (constavail <= 0) {
		if (consttable == NULL) {
			initconstants();
		} else {
			tp = (NUMBER **) realloc((char *) consttable,
			sizeof(NUMBER *) * (constcount + CONSTALLOCSIZE));
			if (tp == NULL) {
				math_error("Unable to reallocate const table");
				/*NOTREACHED*/
			}
			consttable = tp;
			constavail = CONSTALLOCSIZE;
		}
	}
	numlen = q->num.len;
	denlen = q->den.len;
	numlow = q->num.v[0];
	denlow = q->den.v[0];
	first = 0;
	havefirst = FALSE;
	tp = consttable;
	for (index = 0; index < constcount; index++, tp++) {
		t = *tp;
		if (t->links == 0) {
			if (!havefirst) {
				havefirst = TRUE;
				first = index;
			}
			continue;
		}
		if (q == t) {
			if (q->links == 1) {
				if (havefirst) {
					*tp = consttable[first];
					consttable[first] = q;
				} else {
					havefirst = TRUE;
					first = index;
				}
				continue;
			}
			return index;
		}

		if ((numlen != t->num.len) || (numlow != t->num.v[0]))
			continue;
		if ((denlen != t->den.len) || (denlow != t->den.v[0]))
			continue;
		if (q->num.sign != t->num.sign)
			continue;
		if (qcmp(q, t) == 0) {
			t->links++;
			qfree(q);
			return index;
		}
	}
	if (havefirst) {
		consttable[first] = q;
		return first;
	}
	constavail--;
	consttable[constcount++] = q;
	return index;
}


/*
 * Return the value of a constant number given its index.
 * Returns address of the number, or NULL if the index is illegal
 * or points to freed position.
 */
NUMBER *
constvalue(unsigned long index)
{
	if (index >= constcount) {
		math_error("Bad index value for constvalue");
		/*NOTREACHED*/
	}
	if (consttable[index]->links == 0) {
		math_error("Constvalue has been freed!!!");
		/*NOTREACHED*/
	}
	return consttable[index];
}


void
freeconstant(unsigned long index)
{
	NUMBER *q;

	if (index >= constcount) {
		math_error("Bad index value for freeconst");
		/*NOTREACHED*/
	}
	q = consttable[index];
	if (q->links == 0) {
		math_error("Attempting to free freed const location");
		/*NOTREACHED*/
	}
	qfree(q);
	if (index == constcount - 1) {
		trimconstants();
	}
}


void
trimconstants(void)
{
	while (constcount > 0 &&
	       (consttable[constcount-1] == NULL ||
		consttable[constcount-1]->links == 0)) {
		constcount--;
		constavail++;
	}
}

void
showconstants(void)
{
	unsigned long index;
	NUMBER **qp;
	long count;

	qp = consttable;
	count = 0;
	for (index = 0; index < constcount; index++, qp++) {
		if ((*qp)->links) {
			if (!count) {
				printf("\n   Index   Links   Value\n");
			}
			count++;
			printf("\n%8ld%8ld    ", index, (*qp)->links);
			fitprint(*qp, 40);
		}
	}
	printf("\n\nNumber = %ld\n", count);
}


/* END CODE */
