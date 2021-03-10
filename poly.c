/*
 * poly - polynomial functions
 *
 * Copyright (C) 1999,2021  Ernest Bowen and Landon Curt Noll
 *
 * Primary author:  Ernest Bowen
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
 * Under source code control:	1995/12/02 03:10:28
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include "value.h"


#include "banned.h"	/* include after system header <> includes */


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
				} else {
					s = TRUE;
					*vres = tmp1;
				}
			}
		} else {
			if (s) {
				addvalue(&v, vres, &tmp1);
				freevalue(vres);
				*vres = tmp1;
			} else {
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
		if (ep->e_value.v_type == V_LIST) {
			addlistinv(ep->e_value.v_list, vres);
		} else {
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
