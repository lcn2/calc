/*
 * listfunc - list handling routines
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
 * Under source code control:	1990/02/15 01:48:18
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * List handling routines.
 * Lists can be composed of any types of values, mixed if desired.
 * Lists are doubly linked so that elements can be inserted or
 * deleted efficiently at any point in the list.  A pointer is
 * kept to the most recently indexed element so that sequential
 * accesses are fast.
 */


#include "value.h"
#include "zrand.h"


#include "banned.h"	/* include after system header <> includes */


E_FUNC long irand(long s);

S_FUNC LISTELEM *elemalloc(void);
S_FUNC void elemfree(LISTELEM *ep);
S_FUNC void removelistelement(LIST *lp, LISTELEM *ep);


/*
 * Insert an element before the first element of a list.
 *
 * given:
 *	lp		list to put element onto
 *	vp		value to be inserted
 */
void
insertlistfirst(LIST *lp, VALUE *vp)
{
	LISTELEM *ep;		/* list element */

	ep = elemalloc();
	copyvalue(vp, &ep->e_value);
	if (lp->l_count == 0) {
		lp->l_last = ep;
	} else {
		lp->l_cacheindex++;
		lp->l_first->e_prev = ep;
		ep->e_next = lp->l_first;
	}
	lp->l_first = ep;
	lp->l_count++;
}


/*
 * Insert an element after the last element of a list.
 *
 * given:
 *	lp		list to put element onto
 *	vp		value to be inserted
 */
void
insertlistlast(LIST *lp, VALUE *vp)
{
	LISTELEM *ep;		/* list element */

	ep = elemalloc();
	copyvalue(vp, &ep->e_value);
	if (lp->l_count == 0) {
		lp->l_first = ep;
	} else {
		lp->l_last->e_next = ep;
		ep->e_prev = lp->l_last;
	}
	lp->l_last = ep;
	lp->l_count++;
}


/*
 * Insert an element into the middle of list at the given index (zero based).
 * The specified index will select the new element, so existing elements
 * at or beyond the index will be shifted down one position.  It is legal
 * to specify an index which is right at the end of the list, in which
 * case the element is appended to the list.
 *
 * given:
 *	lp		list to put element onto
 *	index		element number to insert in front of
 *	vp		value to be inserted
 */
void
insertlistmiddle(LIST *lp, long index, VALUE *vp)
{
	LISTELEM *ep;		/* list element */
	LISTELEM *oldep;	/* old list element at desired index */

	if (index == 0) {
		insertlistfirst(lp, vp);
		return;
	}
	if (index == lp->l_count) {
		insertlistlast(lp, vp);
		return;
	}
	oldep = NULL;
	if ((index >= 0) && (index < lp->l_count))
		oldep = listelement(lp, index);
	if (oldep == NULL) {
		math_error("Index out of bounds for list insertion");
		/*NOTREACHED*/
	}
	ep = elemalloc();
	copyvalue(vp, &ep->e_value);
	ep->e_next = oldep;
	ep->e_prev = oldep->e_prev;
	ep->e_prev->e_next = ep;
	oldep->e_prev = ep;
	lp->l_cache = ep;
	lp->l_cacheindex = index;
	lp->l_count++;
}


/*
 * Remove the first element from a list, returning its value.
 * Returns the null value if no more elements exist.
 *
 * given:
 *	lp		list to have element removed
 *	vp		location of the value
 */
void
removelistfirst(LIST *lp, VALUE *vp)
{
	if (lp->l_count == 0) {
		vp->v_type = V_NULL;
		vp->v_subtype = V_NOSUBTYPE;
		return;
	}
	*vp = lp->l_first->e_value;
	lp->l_first->e_value.v_type = V_NULL;
	lp->l_first->e_value.v_type = V_NOSUBTYPE;
	removelistelement(lp, lp->l_first);
}


/*
 * Remove the last element from a list, returning its value.
 * Returns the null value if no more elements exist.
 *
 * given:
 *	lp		list to have element removed
 *	vp		location of the value
 */
void
removelistlast(LIST *lp, VALUE *vp)
{
	if (lp->l_count == 0) {
		vp->v_type = V_NULL;
		vp->v_subtype = V_NOSUBTYPE;
		return;
	}
	*vp = lp->l_last->e_value;
	lp->l_last->e_value.v_type = V_NULL;
	lp->l_last->e_value.v_subtype = V_NOSUBTYPE;
	removelistelement(lp, lp->l_last);
}


/*
 * Remove the element with the given index from a list, returning its value.
 *
 * given:
 *	lp		list to have element removed
 *	index		list element to be removed
 *	vp		location of the value
 */
void
removelistmiddle(LIST *lp, long index, VALUE *vp)
{
	LISTELEM *ep;		/* element being removed */

	ep = NULL;
	if ((index >= 0) && (index < lp->l_count))
		ep = listelement(lp, index);
	if (ep == NULL) {
		math_error("Index out of bounds for list deletion");
		/*NOTREACHED*/
	}
	*vp = ep->e_value;
	ep->e_value.v_type = V_NULL;
	ep->e_value.v_subtype = V_NOSUBTYPE;
	removelistelement(lp, ep);
}


/*
 * Remove an arbitrary element from a list.
 * The value contained in the element is freed.
 *
 * given:
 *	lp		list header
 *	ep		list element to remove
 */
S_FUNC void
removelistelement(LIST *lp, LISTELEM *ep)
{
	if ((ep == lp->l_cache) || ((ep != lp->l_first) && (ep != lp->l_last)))
		lp->l_cache = NULL;
	if (ep->e_next)
		ep->e_next->e_prev = ep->e_prev;
	if (ep->e_prev)
		ep->e_prev->e_next = ep->e_next;
	if (ep == lp->l_first) {
		lp->l_first = ep->e_next;
		lp->l_cacheindex--;
	}
	if (ep == lp->l_last)
		lp->l_last = ep->e_prev;
	lp->l_count--;
	elemfree(ep);
}


LIST *
listsegment(LIST *lp, long n1, long n2)
{
	LIST *newlp;
	LISTELEM *ep;
	long i;

	newlp = listalloc();
	if ((n1 >= lp->l_count && n2 >= lp->l_count) || (n1 < 0 && n2 < 0))
		return newlp;
	if (n1 >= lp->l_count)
		n1 = lp->l_count - 1;
	if (n2 >= lp->l_count)
		n2 = lp->l_count - 1;
	if (n1 < 0)
		n1 = 0;
	if (n2 < 0)
		n2 = 0;

	ep = lp->l_first;
	if (n1 <= n2) {
		i = n2 - n1 + 1;
		while(n1-- > 0 && ep)
			ep = ep->e_next;
		while(i-- > 0 && ep) {
			insertlistlast(newlp, &ep->e_value);
			ep = ep->e_next;
		}
	} else {
		i = n1 - n2 + 1;
		while(n2-- > 0 && ep)
			ep = ep->e_next;
		while(i-- > 0 && ep) {
			insertlistfirst(newlp, &ep->e_value);
			ep = ep->e_next;
		}
	}
	return newlp;
}


/*
 * Search a list for the specified value starting at the specified index.
 * Returns 0 and stores the element number (zero based) if the value is
 * found, otherwise returns 1.
 */
int
listsearch(LIST *lp, VALUE *vp, long i, long j, ZVALUE *index)
{
	register LISTELEM *ep;

	if (i < 0 || j > lp->l_count) {
		math_error("This should not happen in call to listsearch");
		/*NOTREACHED*/
	}

	ep = listelement(lp, i);
	while (i < j) {
		if (!ep) {
			math_error("This should not happen in listsearch");
			/*NOTREACHED*/
		}
		if (acceptvalue(&ep->e_value, vp)) {
			lp->l_cache = ep;
			lp->l_cacheindex = i;
			utoz(i, index);
			return 0;
		}
		ep = ep->e_next;
		i++;
	}
	return 1;
}


/*
 * Search a list backwards for the specified value starting at the
 * specified index.  Returns 0 and stores i if the value is found at
 * index i; otherwise returns 1.
 */
int
listrsearch(LIST *lp, VALUE *vp, long i, long j, ZVALUE *index)
{
	register LISTELEM *ep;

	if (i < 0 || j > lp->l_count) {
		math_error("This should not happen in call to listrsearch");
		/*NOTREACHED*/
	}

	ep = listelement(lp, --j);
	while (j >= i) {
		if (!ep) {
			math_error("This should not happen in listsearch");
			/*NOTREACHED*/
		}
		if (acceptvalue(&ep->e_value, vp)) {
			lp->l_cache = ep;
			lp->l_cacheindex = j;
			utoz(j, index);
			return 0;
		}
		ep = ep->e_prev;
		j--;
	}
	return 1;
}


/*
 * Index into a list and return the address for the value corresponding
 * to that index.  Returns NULL if the element does not exist.
 *
 * given:
 *	lp		list to index into
 *	index		index of desired element
 */
VALUE *
listfindex(LIST *lp, long index)
{
	LISTELEM *ep;

	ep = listelement(lp, index);
	if (ep == NULL)
		return NULL;
	return &ep->e_value;
}


/*
 * Return the element at a specified index number of a list.
 * The list is indexed starting at zero, and negative indices
 * indicate to index from the end of the list.	This routine finds
 * the element by chaining through the list from the closest one
 * of the first, last, and cached elements.  Returns NULL if the
 * element does not exist.
 *
 * given:
 *	lp		list to index into
 *	index		index of desired element
 */
LISTELEM *
listelement(LIST *lp, long index)
{
	register LISTELEM *ep;	/* current list element */
	long dist;		/* distance to element */
	long temp;		/* temporary distance */
	BOOL forward;		/* TRUE if need to walk forwards */

	if (index < 0)
		index += lp->l_count;
	if ((index < 0) || (index >= lp->l_count))
		return NULL;
	/*
	 * Check quick special cases first.
	 */
	if (index == 0)
		return lp->l_first;
	if (index == 1)
		return lp->l_first->e_next;
	if (index == lp->l_count - 1)
		return lp->l_last;
	if ((index == lp->l_cacheindex) && lp->l_cache)
		return lp->l_cache;
	/*
	 * Calculate whether it is better to go forwards from
	 * the first element or backwards from the last element.
	 */
	forward = ((index * 2) <= lp->l_count);
	if (forward) {
		dist = index;
		ep = lp->l_first;
	} else {
		dist = (lp->l_count - 1) - index;
		ep = lp->l_last;
	}
	/*
	 * Now see if we have a cached element and if so, whether or
	 * not the distance from it is better than the above distance.
	 */
	if (lp->l_cache) {
		temp = index - lp->l_cacheindex;
		if ((temp >= 0) && (temp < dist)) {
			dist = temp;
			ep = lp->l_cache;
			forward = TRUE;
		}
		if ((temp < 0) && (-temp < dist)) {
			dist = -temp;
			ep = lp->l_cache;
			forward = FALSE;
		}
	}
	/*
	 * Now walk forwards or backwards from the selected element
	 * until we reach the correct element.	Cache the location of
	 * the found element for future use.
	 */
	if (forward) {
		while (dist-- > 0)
			ep = ep->e_next;
	} else {
		while (dist-- > 0)
			ep = ep->e_prev;
	}
	lp->l_cache = ep;
	lp->l_cacheindex = index;
	return ep;
}


/*
 * Compare two lists to see if they are identical.
 * Returns TRUE if they are different.
 */
BOOL
listcmp(LIST *lp1, LIST *lp2)
{
	LISTELEM *e1, *e2;
	long count;

	if (lp1 == lp2)
		return FALSE;
	if (lp1->l_count != lp2->l_count)
		return TRUE;
	e1 = lp1->l_first;
	e2 = lp2->l_first;
	count = lp1->l_count;
	while (count-- > 0) {
		if (comparevalue(&e1->e_value, &e2->e_value))
			return TRUE;
		e1 = e1->e_next;
		e2 = e2->e_next;
	}
	return FALSE;
}


/*
 * Copy a list
 */
LIST *
listcopy(LIST *oldlp)
{
	LIST *lp;
	LISTELEM *oldep;

	lp = listalloc();
	oldep = oldlp->l_first;
	while (oldep) {
		insertlistlast(lp, &oldep->e_value);
		oldep = oldep->e_next;
	}
	return lp;
}


/*
 * Round elements of a list to a specified number of decimal digits
 */
LIST *
listround(LIST *oldlp, VALUE *v2, VALUE *v3)
{
	LIST *lp;
	LISTELEM *oldep, *ep, *eq;

	lp = listalloc();
	oldep = oldlp->l_first;
	lp->l_count = oldlp->l_count;
	if (oldep) {
		ep = elemalloc();
		lp->l_first = ep;
		for (;;) {
			roundvalue(&oldep->e_value, v2, v3, &ep->e_value);
			oldep = oldep->e_next;
			if (!oldep)
				break;
			eq = elemalloc();
			ep->e_next = eq;
			eq->e_prev = ep;
			ep = eq;
		}
		lp->l_last = ep;
	}
	return lp;
}


/*
 * Round elements of a list to a specified number of binary digits
 */
LIST *
listbround(LIST *oldlp, VALUE *v2, VALUE *v3)
{
	LIST *lp;
	LISTELEM *oldep, *ep, *eq;

	lp = listalloc();
	oldep = oldlp->l_first;
	lp->l_count = oldlp->l_count;
	if (oldep) {
		ep = elemalloc();
		lp->l_first = ep;
		for (;;) {
			broundvalue(&oldep->e_value, v2, v3, &ep->e_value);
			oldep = oldep->e_next;
			if (!oldep)
				break;
			eq = elemalloc();
			ep->e_next = eq;
			eq->e_prev = ep;
			ep = eq;
		}
		lp->l_last = ep;
	}
	return lp;
}


/*
 * Approximate a list by approximating elements by multiples of v2,
 * type of rounding determined by v3.
 */
LIST *
listappr(LIST *oldlp, VALUE *v2, VALUE *v3)
{
	LIST *lp;
	LISTELEM *oldep, *ep, *eq;

	lp = listalloc();
	oldep = oldlp->l_first;
	lp->l_count = oldlp->l_count;
	if (oldep) {
		ep = elemalloc();
		lp->l_first = ep;
		for (;;) {
			apprvalue(&oldep->e_value, v2, v3, &ep->e_value);
			oldep = oldep->e_next;
			if (!oldep)
				break;
			eq = elemalloc();
			ep->e_next = eq;
			eq->e_prev = ep;
			ep = eq;
		}
		lp->l_last = ep;
	}
	return lp;
}


/*
 * Construct a list whose elements are integer quotients of the elements
 * of a specified list by a specified number.
 */
LIST *
listquo(LIST *oldlp, VALUE *v2, VALUE *v3)
{
	LIST *lp;
	LISTELEM *oldep, *ep, *eq;

	lp = listalloc();
	oldep = oldlp->l_first;
	lp->l_count = oldlp->l_count;
	if (oldep) {
		ep = elemalloc();
		lp->l_first = ep;
		for (;;) {
			quovalue(&oldep->e_value, v2, v3, &ep->e_value);
			oldep = oldep->e_next;
			if (!oldep)
				break;
			eq = elemalloc();
			ep->e_next = eq;
			eq->e_prev = ep;
			ep = eq;
		}
		lp->l_last = ep;
	}
	return lp;
}


/*
 * Construct a list whose elements are the remainders after integral
 * division of the elements of a specified list by a specified number.
 */
LIST *
listmod(LIST *oldlp, VALUE *v2, VALUE *v3)
{
	LIST *lp;
	LISTELEM *oldep, *ep, *eq;

	lp = listalloc();
	oldep = oldlp->l_first;
	lp->l_count = oldlp->l_count;
	if (oldep) {
		ep = elemalloc();
		lp->l_first = ep;
		for (;;) {
			modvalue(&oldep->e_value, v2, v3, &ep->e_value);
			oldep = oldep->e_next;
			if (!oldep)
				break;
			eq = elemalloc();
			ep->e_next = eq;
			eq->e_prev = ep;
			ep = eq;
		}
		lp->l_last = ep;
	}
	return lp;
}


void
listreverse(LIST *lp)
{
	LISTELEM *e1, *e2;
	VALUE tmp;
	long s;

	s = lp->l_count/2;
	e1 = lp->l_first;
	e2 = lp->l_last;
	lp->l_cache = NULL;
	while (s-- > 0) {
		tmp = e1->e_value;
		e1->e_value = e2->e_value;
		e2->e_value = tmp;
		e1 = e1->e_next;
		e2 = e2->e_prev;
	}
}


void
listsort(LIST *lp)
{
	LISTELEM *start;
	LISTELEM *last, *a, *a1, *b, *next;
	LISTELEM *S[LONG_BITS+1];
	long len[LONG_BITS+1];
	long i, j, k;

	if (lp->l_count < 2)
		return;
	lp->l_cache = NULL;
	start = elemalloc();
	next = lp->l_first;
	last = start;
	start->e_next = next;
	for (k = 0; next && k < LONG_BITS; k++) {
		next->e_prev = last;
		last = next;
		S[k] = next;
		next = next->e_next;
		len[k] = 1;
		while (k > 0 && (!next || len[k] >= len[k - 1])) {/* merging */
			j = len[k];
			b = S[k--];
			i = len[k];
			a = S[k];
			a1 = b->e_prev;
			len[k] = i + j;
			if (precvalue(&b->e_value, &a->e_value)) {
				S[k] = b;
				a->e_prev->e_next = b;
				b->e_prev = a->e_prev;
				j--;
				while (j > 0) {
					b = b->e_next;
					if (!precvalue(&b->e_value,
						       &a->e_value))
						break;
					j--;
				}
				if (j == 0) {
					b->e_next = a;
					a->e_prev = b;
					last = a1;
					continue;
				}
				b->e_prev->e_next = a;
				a->e_prev = b->e_prev;
			}

			do {
				i--;
				while (i > 0) {
					a = a->e_next;
					if (precvalue(&b->e_value,
						      &a->e_value))
						break;
					i--;
				}
				if (i == 0)
					break;
				a->e_prev->e_next = b;
				b->e_prev = a->e_prev;
				j--;
				while (j > 0) {
					b = b->e_next;
					if (!precvalue(&b->e_value,
						       &a->e_value))
						break;
					j--;
				}
				if (j != 0) {
					b->e_prev->e_next = a;
					a->e_prev = b->e_prev;
				}
			} while (j != 0);

			if (i == 0) {
				a->e_next = b;
				b->e_prev = a;
			} else if (j == 0) {
				b->e_next = a;
				a->e_prev = b;
				last = a1;
			}
		}
	}
	if (k >= LONG_BITS) {
		/* this should never happen */
		math_error("impossible k overflow in listsort!");
		/*NOTREACHED*/
	}
	lp->l_first = start->e_next;
	lp->l_first->e_prev = NULL;
	lp->l_last = last;
	lp->l_last->e_next = NULL;
	elemfree(start);
}

void
listrandperm(LIST *lp)
{
	LISTELEM *ep, *eq;
	long i, s;
	VALUE val;

	s = lp->l_count;
	for (ep = lp->l_last; s > 1; ep = ep->e_prev) {
		i = irand(s--);
		if (i < s) {
			eq = listelement(lp, i);
			val = eq->e_value;
			eq->e_value = ep->e_value;
			ep->e_value = val;
		}
	}
}



/*
 * Allocate an element for a list.
 */
S_FUNC LISTELEM *
elemalloc(void)
{
	LISTELEM *ep;

	ep = (LISTELEM *) malloc(sizeof(LISTELEM));
	if (ep == NULL) {
		math_error("Cannot allocate list element");
		/*NOTREACHED*/
	}
	ep->e_next = NULL;
	ep->e_prev = NULL;
	ep->e_value.v_type = V_NULL;
	ep->e_value.v_subtype = V_NOSUBTYPE;
	return ep;
}


/*
 * Free a list element, along with any contained value.
 */
S_FUNC void
elemfree(LISTELEM *ep)
{
	if (ep->e_value.v_type != V_NULL)
		freevalue(&ep->e_value);
	free(ep);
}


/*
 * Allocate a new list header.
 */
LIST *
listalloc(void)
{
	register LIST *lp;

	lp = (LIST *) malloc(sizeof(LIST));
	if (lp == NULL) {
		math_error("Cannot allocate list header");
		/*NOTREACHED*/
	}
	lp->l_first = NULL;
	lp->l_last = NULL;
	lp->l_cache = NULL;
	lp->l_cacheindex = 0;
	lp->l_count = 0;
	return lp;
}


/*
 * Free a list header, along with all of its list elements.
 */
void
listfree(LIST *lp)
{
	register LISTELEM *ep;

	while (lp->l_count-- > 0) {
		ep = lp->l_first;
		lp->l_first = ep->e_next;
		elemfree(ep);
	}
	free(lp);
}


/*
 * Print out a list along with the specified number of its elements.
 * The elements are printed out in shortened form.
 */
void
listprint(LIST *lp, long max_print)
{
	long count;
	long index;
	LISTELEM *ep;

	if (max_print > lp->l_count)
		max_print = lp->l_count;
	count = 0;
	ep = lp->l_first;
	index = lp->l_count;
	while (index-- > 0) {
		if ((ep->e_value.v_type != V_NUM) ||
			(!qiszero(ep->e_value.v_num)))
				count++;
		ep = ep->e_next;
	}
	if (max_print > 0)
		math_str("\n");
	math_fmt("list (%ld element%s, %ld nonzero)", lp->l_count,
		((lp->l_count == 1) ? "" : "s"), count);
	if (max_print <= 0)
		return;

	/*
	 * Walk through the first few list elements, printing their
	 * value in short and unambiguous format.
	 */
	math_str(":\n");
	ep = lp->l_first;
	for (index = 0; index < max_print; index++) {
		math_fmt("\t[[%ld]] = ", index);
		printvalue(&ep->e_value, PRINT_SHORT | PRINT_UNAMBIG);
		math_str("\n");
		ep = ep->e_next;
	}
	if (max_print < lp->l_count)
		math_str("  ...\n");
}
