/*
 * Copyright (c) 1995 Ernest Bowen and Landon Curt Noll
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * By: Ernest Bowen and Landon Curt Noll
 *     ernie@neumann.une.edu.au and chongo@toad.com
 */

#include "value.h"


BOOL
evp(LISTELEM *cp, LISTELEM *x, VALUE *vres)
{
	VALUE v, tmp1, tmp2;
	BOOL s;

	s = FALSE;
	while (cp) {
		if (s) {
			mulvalue(vres, &x->e_value, &tmp1);
			freevalue(vres);
			*vres = tmp1;
		}
		v = cp->e_value;
		if (v.v_type == V_LIST) {
			if (evalpoly(v.v_list, x->e_next, &tmp1)) {
				if (s) {
					addvalue(&tmp1, vres, &tmp2);
					freevalue(&tmp1);
					freevalue(vres);
					*vres = tmp2;
				}
				else {
					s = TRUE;
					*vres = tmp1;
				}
			}
		}
		else {
			if (s) {
				addvalue(&v, vres, &tmp1);
				freevalue(vres);
				*vres = tmp1;
			}
			else {
				s = TRUE;
				copyvalue(&v, vres);
			}
		}
		cp = cp->e_prev;
	}
	return s;
}


BOOL
evalpoly(LIST *clist, LISTELEM *x, VALUE *vres)
{
	LISTELEM *cp;
	VALUE v;

	cp = clist->l_first;
	if (cp == NULL)
		return FALSE;
	if (x == NULL) {
		v = cp->e_value;
		if (v.v_type == V_LIST)
			return evalpoly(v.v_list, x->e_next, vres);
		copyvalue(&v, vres);
		return TRUE;
	}
	return evp(clist->l_last, x, vres);
}

void
insertitems(LIST *lp1, LIST *lp2)
{
	LISTELEM *ep;

	for (ep = lp2->l_first; ep; ep = ep->e_next) {
		if (ep->e_value.v_type == V_LIST)
			insertitems(lp1, ep->e_value.v_list);
		else
			insertlistlast(lp1, &ep->e_value);
	}
}


long
countlistitems(LIST *lp)
{
	LISTELEM *ep;

	long n = 0;
	for (ep = lp->l_first; ep; ep = ep->e_next) {
		if (ep->e_value.v_type == V_LIST)
			n += countlistitems(ep->e_value.v_list);
		else
			n++;
	}
	return n;
}


void
addlistitems(LIST *lp, VALUE *vres)
{
	LISTELEM *ep;
	VALUE tmp;

	for (ep = lp->l_first; ep; ep = ep->e_next) {
		addvalue(vres, &ep->e_value, &tmp);
		freevalue(vres);
		*vres = tmp;
		if (vres->v_type < 0)
			return;
	}
}

void
addlistinv(LIST *lp, VALUE *vres)
{
	LISTELEM *ep;
	VALUE tmp1, tmp2;

	for (ep = lp->l_first; ep; ep = ep->e_next) {
		if (ep->e_value.v_type == V_LIST)
			addlistinv(ep->e_value.v_list, vres);
		else {
			invertvalue(&ep->e_value, &tmp1);
			addvalue(vres, &tmp1, &tmp2);
			freevalue(&tmp1);
			freevalue(vres);
			*vres = tmp2;
		}
		if (vres->v_type < 0)
			return;
	}
}


/* END CODE */
