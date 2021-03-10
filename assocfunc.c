/*
 * assocfunc - association table routines
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
 * Under source code control:	1993/07/20 23:04:27
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Association table routines.
 * An association table is a type of value which can be "indexed" by
 * one or more arbitrary values.  Each element in the table is thus an
 * association between a particular set of index values and a result value.
 * The elements in an association table are stored in a hash table for
 * quick access.
 */


#include "value.h"


#include "banned.h"	/* include after system header <> includes */


#define MINHASHSIZE	31	/* minimum size of hash tables */
#define GROWHASHSIZE	50	/* approximate growth for hash tables */
#define CHAINLENGTH	10	/* desired number of elements on a hash chain */
#define ELEMSIZE(n)	(sizeof(ASSOCELEM) + (sizeof(VALUE) * ((n) - 1)))


S_FUNC ASSOCELEM *elemindex(ASSOC *ap, long index);
S_FUNC BOOL compareindices(VALUE *v1, VALUE *v2, long dim);
S_FUNC void resize(ASSOC *ap, long newsize);
S_FUNC void assoc_elemfree(ASSOCELEM *ep);


/*
 * Return the address of the value specified by normal indexing of
 * an association.  The create flag is TRUE if a value is going to be
 * assigned into the specified indexing location.  If create is FALSE and
 * the index value doesn't exist, a pointer to a NULL value is returned.
 *
 * given:
 *	ap		association to index into
 *	create		whether to create the index value
 *	dim		dimension of the indexing
 *	indices		table of values being indexed by
 */
VALUE *
associndex(ASSOC *ap, BOOL create, long dim, VALUE *indices)
{
	ASSOCELEM **listhead;
	ASSOCELEM *ep;
	STATIC VALUE val;
	QCKHASH hash;
	int i;

	if (dim < 0) {
		math_error("Negative dimension for indexing association");
		/*NOTREACHED*/
	}

	/*
	 * Calculate the hash value to use for this set of indices
	 * so that we can first select the correct hash chain, and
	 * also so we can quickly compare each element for a match.
	 */
	hash = QUICKHASH_BASIS;
	for (i = 0; i < dim; i++)
		hash = hashvalue(&indices[i], hash);

	/*
	 * Search the correct hash chain for the specified set of indices.
	 * If found, return the address of the found element's value.
	 */
	listhead = &ap->a_table[hash % ap->a_size];
	for (ep = *listhead; ep; ep = ep->e_next) {
		if ((ep->e_hash != hash) || (ep->e_dim != dim))
			continue;
		if (compareindices(ep->e_indices, indices, dim))
			return &ep->e_value;
	}

	/*
	 * The set of indices was not found.
	 * Either return a pointer to a NULL value for a read reference,
	 * or allocate a new element in the list for a write reference.
	 */
	if (!create) {
		val.v_type = V_NULL;
		val.v_subtype = V_NOSUBTYPE;
		return &val;
	}

	ep = (ASSOCELEM *) malloc(ELEMSIZE(dim));
	if (ep == NULL) {
		math_error("Cannot allocate association element");
		/*NOTREACHED*/
	}
	ep->e_dim = dim;
	ep->e_hash = hash;
	ep->e_value.v_type = V_NULL;
	ep->e_value.v_subtype = V_NOSUBTYPE;
	for (i = 0; i < dim; i++)
		copyvalue(&indices[i], &ep->e_indices[i]);
	ep->e_next = *listhead;
	*listhead = ep;
	ap->a_count++;

	resize(ap, ap->a_count / CHAINLENGTH);

	return &ep->e_value;
}


/*
 * Search an association for the specified value starting at the
 * specified index.  Returns 0 and stores index if value found,
 * otherwise returns 1.
 */
int
assocsearch(ASSOC *ap, VALUE *vp, long i, long j, ZVALUE *index)
{
	ASSOCELEM *ep;

	if (i < 0 || j > ap->a_count) {
		math_error("This should not happen in assocsearch");
		/*NOTREACHED*/
	}
	while (i < j) {
		ep = elemindex(ap, i);
		if (ep == NULL) {
			math_error("This should not happen in assocsearch");
			/*NOTREACHED*/
		}
		if (acceptvalue(&ep->e_value, vp)) {
			utoz(i, index);
			return 0;
		}
		i++;
	}
	return 1;
}


/*
 * Search an association backwards for the specified value starting at the
 * specified index.  Returns 0 and stores the index if the value is
 * found; otherwise returns 1.
 */
int
assocrsearch(ASSOC *ap, VALUE *vp, long i, long j, ZVALUE *index)
{
	ASSOCELEM *ep;

	if (i < 0 || j > ap->a_count) {
		math_error("This should not happen in assocsearch");
		/*NOTREACHED*/
	}
	j--;
	while (j >= i) {
		ep = elemindex(ap, j);
		if (ep == NULL) {
			math_error("This should not happen in assocsearch");
			/*NOTREACHED*/
		}
		if (acceptvalue(&ep->e_value, vp)) {
			utoz(j, index);
			return 0;
		}
		j--;
	}
	return 1;
}


/*
 * Return the address of an element of an association indexed by the
 * double-bracket operation.
 *
 * given:
 *	ap		association to index into
 *	index		index of desired element
 */
S_FUNC ASSOCELEM *
elemindex(ASSOC *ap, long index)
{
	ASSOCELEM *ep;
	int i;

	if ((index < 0) || (index > ap->a_count))
		return NULL;

	/*
	 * This loop should be made more efficient by remembering
	 * previously requested locations within the association.
	 */
	for (i = 0; i < ap->a_size; i++) {
		for (ep = ap->a_table[i]; ep; ep = ep->e_next) {
			if (index-- == 0)
				return ep;
		}
	}
	return NULL;
}


/*
 * Return the address of the value specified by double-bracket indexing
 * of an association.  Returns NULL if there is no such element.
 *
 * given:
 *	ap		association to index into
 *	index		index of desired element
 */
VALUE *
assocfindex(ASSOC *ap, long index)
{
	ASSOCELEM *ep;

	ep = elemindex(ap, index);
	if (ep == NULL)
		return NULL;
	return &ep->e_value;
}


/*
 * Returns the list of indices for an association element with specified
 * double-bracket index.
 */
LIST *
associndices(ASSOC *ap, long index)
{
	ASSOCELEM *ep;
	LIST *lp;
	int i;

	ep = elemindex(ap, index);
	if (ep == NULL)
		return NULL;
	lp = listalloc();
	for (i = 0; i < ep->e_dim; i++)
		insertlistlast(lp, &ep->e_indices[i]);
	return lp;
}


/*
 * Compare two associations to see if they are identical.
 * Returns TRUE if they are different.
 */
BOOL
assoccmp(ASSOC *ap1, ASSOC *ap2)
{
	ASSOCELEM **table1;
	ASSOCELEM *ep1;
	ASSOCELEM *ep2;
	long size1;
	long size2;
	QCKHASH hash;
	long dim;

	if (ap1 == ap2)
		return FALSE;
	if (ap1->a_count != ap2->a_count)
		return TRUE;

	table1 = ap1->a_table;
	size1 = ap1->a_size;
	size2 = ap2->a_size;
	while (size1-- > 0) {
		for (ep1 = *table1++; ep1; ep1 = ep1->e_next) {
			hash = ep1->e_hash;
			dim = ep1->e_dim;
			for (ep2 = ap2->a_table[hash % size2]; ;
				ep2 = ep2->e_next) {
				if (ep2 == NULL)
					return TRUE;
				if (ep2->e_hash != hash)
					continue;
				if (ep2->e_dim != dim)
					continue;
				if (compareindices(ep1->e_indices,
					ep2->e_indices, dim))
						break;
			}
			if (comparevalue(&ep1->e_value, &ep2->e_value))
				return TRUE;
		}
	}
	return FALSE;
}


/*
 * Copy an association value.
 */
ASSOC *
assoccopy(ASSOC *oldap)
{
	ASSOC *ap;
	ASSOCELEM *oldep;
	ASSOCELEM *ep;
	ASSOCELEM **listhead;
	int oldhi;
	int i;

	ap = assocalloc(oldap->a_count / CHAINLENGTH);
	ap->a_count = oldap->a_count;

	for (oldhi = 0; oldhi < oldap->a_size; oldhi++) {
		for (oldep = oldap->a_table[oldhi]; oldep;
			oldep = oldep->e_next) {
			ep = (ASSOCELEM *) malloc(ELEMSIZE(oldep->e_dim));
			if (ep == NULL) {
				math_error("Cannot allocate "
					   "association element");
				/*NOTREACHED*/
			}
			ep->e_dim = oldep->e_dim;
			ep->e_hash = oldep->e_hash;
			ep->e_value.v_type = V_NULL;
			ep->e_value.v_subtype = V_NOSUBTYPE;
			for (i = 0; i < ep->e_dim; i++)
				copyvalue(&oldep->e_indices[i],
					  &ep->e_indices[i]);
			copyvalue(&oldep->e_value, &ep->e_value);
			listhead = &ap->a_table[ep->e_hash % ap->a_size];
			ep->e_next = *listhead;
			*listhead = ep;
		}
	}
	return ap;
}


/*
 * Resize the hash table for an association to be the specified size.
 * This is only actually done if the growth from the previous size is
 * enough to make this worthwhile.
 */
S_FUNC void
resize(ASSOC *ap, long newsize)
{
	ASSOCELEM **oldtable;
	ASSOCELEM **newtable;
	ASSOCELEM **oldlist;
	ASSOCELEM **newlist;
	ASSOCELEM *ep;
	int i;

	if (newsize < ap->a_size + GROWHASHSIZE)
		return;

	newsize = (long) next_prime((FULL)newsize);
	newtable = (ASSOCELEM **) malloc(sizeof(ASSOCELEM *) * newsize);
	if (newtable == NULL) {
		math_error("No memory to grow association");
		/*NOTREACHED*/
	}
	for (i = 0; i < newsize; i++)
		newtable[i] = NULL;

	oldtable = ap->a_table;
	oldlist = oldtable;
	for (i = 0; i < ap->a_size; i++) {
		while (*oldlist) {
			ep = *oldlist;
			*oldlist = ep->e_next;
			newlist = &newtable[ep->e_hash % newsize];
			ep->e_next = *newlist;
			*newlist = ep;
		}
		oldlist++;
	}

	ap->a_table = newtable;
	ap->a_size = newsize;
	free((char *) oldtable);
}


/*
 * Free an association element, along with any contained values.
 */
S_FUNC void
assoc_elemfree(ASSOCELEM *ep)
{
	int i;

	for (i = 0; i < ep->e_dim; i++)
		freevalue(&ep->e_indices[i]);
	freevalue(&ep->e_value);
	ep->e_dim = 0;
	ep->e_next = NULL;
	free((char *) ep);
}


/*
 * Allocate a new association value with an initial hash table.
 * The hash table size is set at specified (but at least a minimum size).
 */
ASSOC *
assocalloc(long initsize)
{
	register ASSOC *ap;
	int i;

	if (initsize < MINHASHSIZE)
		initsize = MINHASHSIZE;
	ap = (ASSOC *) malloc(sizeof(ASSOC));
	if (ap == NULL) {
		math_error("No memory for association");
		/*NOTREACHED*/
	}
	ap->a_count = 0;
	ap->a_size = initsize;
	ap->a_table = (ASSOCELEM **) malloc(sizeof(ASSOCELEM *) * initsize);
	if (ap->a_table == NULL) {
		free((char *) ap);
		math_error("No memory for association");
		/*NOTREACHED*/
	}
	for (i = 0; i < initsize; i++)
		ap->a_table[i] = NULL;
	return ap;
}


/*
 * Free an association value, along with all of its elements.
 */
void
assocfree(ASSOC *ap)
{
	ASSOCELEM **listhead;
	ASSOCELEM *ep;
	ASSOCELEM *nextep;
	int i;

	listhead = ap->a_table;
	for (i = 0; i < ap->a_size; i++) {
		nextep = *listhead;
		*listhead = NULL;
		while (nextep) {
			ep = nextep;
			nextep = ep->e_next;
			assoc_elemfree(ep);
		}
		listhead++;
	}
	free((char *) ap->a_table);
	ap->a_table = NULL;
	free((char *) ap);
}


/*
 * Print out an association along with the specified number of
 * its elements.  The elements are printed out in shortened form.
 */
void
assocprint(ASSOC *ap, long max_print)
{
	ASSOCELEM *ep;
	long index;
	long i;
	int savemode;

	if (max_print <= 0) {
		math_fmt("assoc (%ld element%s)", ap->a_count,
			((ap->a_count == 1) ? "" : "s"));
		return;
	}
	math_fmt("\n  assoc (%ld element%s):\n", ap->a_count,
		((ap->a_count == 1) ? "" : "s"));

	for (index = 0; ((index < max_print) && (index < ap->a_count));
		index++) {
		ep = elemindex(ap, index);
		if (ep == NULL)
			continue;
		math_str("  [");
		for (i = 0; i < ep->e_dim; i++) {
			if (i)
				math_chr(',');
			savemode = math_setmode(MODE_FRAC);
			printvalue(&ep->e_indices[i],
				(PRINT_SHORT | PRINT_UNAMBIG));
			math_setmode(savemode);
		}
		math_str("] = ");
		printvalue(&ep->e_value, PRINT_SHORT | PRINT_UNAMBIG);
		math_chr('\n');
	}
	if (max_print < ap->a_count)
		math_str("  ...\n");
}


/*
 * Compare two lists of index values to see if they are identical.
 * Returns TRUE if they are the same.
 */
S_FUNC BOOL
compareindices(VALUE *v1, VALUE *v2, long dim)
{
	int i;

	for (i = 0; i < dim; i++)
		if (v1[i].v_type != v2[i].v_type)
			return FALSE;

	while (dim-- > 0)
		if (comparevalue(v1++, v2++))
			return FALSE;

	return TRUE;
}
