/*
 * value - generic value manipulation routines
 *
 * Copyright (C) 1999-2007,2014,2017,2021  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:25
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <sys/types.h>
#include "value.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"
#include "str.h"
#include "zrand.h"
#include "zrandom.h"
#include "cmath.h"
#include "nametype.h"
#include "file.h"
#include "config.h"


#include "banned.h"	/* include after system header <> includes */


#define LINELEN 80		/* length of a typical tty line */

/*
 * Free a value and set its type to undefined.
 *
 * given:
 *	vp		value to be freed
 */
void
freevalue(VALUE *vp)
{
	int type;		/* type of value being freed */

	type = vp->v_type;
	vp->v_type = V_NULL;
	vp->v_subtype = V_NOSUBTYPE;
	if (type <= 0)
		return;
	switch (type) {
	case V_ADDR:
	case V_OCTET:
	case V_NBLOCK:
	case V_FILE:
	case V_VPTR:
	case V_OPTR:
	case V_SPTR:
	case V_NPTR:
		/* nothing to free */
		break;
	case V_STR:
		sfree(vp->v_str);
		break;
	case V_NUM:
		qfree(vp->v_num);
		break;
	case V_COM:
		comfree(vp->v_com);
		break;
	case V_MAT:
		matfree(vp->v_mat);
		break;
	case V_LIST:
		listfree(vp->v_list);
		break;
	case V_ASSOC:
		assocfree(vp->v_assoc);
		break;
	case V_OBJ:
		objfree(vp->v_obj);
		break;
	case V_RAND:
		randfree(vp->v_rand);
		break;
	case V_RANDOM:
		randomfree(vp->v_random);
		break;
	case V_CONFIG:
		config_free(vp->v_config);
		break;
	case V_HASH:
		hash_free(vp->v_hash);
		break;
	case V_BLOCK:
		blk_free(vp->v_block);
		break;
	default:
		math_error("Freeing unknown value type");
		/*NOTREACHED*/
	}
}


/*
 * Set protection status for a value and all of its components
 */
void
protecttodepth(VALUE *vp, int sts, int depth)
{
	VALUE *vq;
	int i;
	LISTELEM *ep;
	ASSOC *ap;

	if (vp->v_type == V_NBLOCK) {
		if (sts > 0)
			vp->v_nblock->subtype |= sts;
		else if (sts < 0)
			vp->v_nblock->subtype &= ~(-sts);
		else vp->v_nblock->subtype = 0;
		return;
	}
	if (sts > 0)
		vp->v_subtype |= sts;
	else if (sts < 0)
		vp->v_subtype &= ~(-sts);
	else
		vp->v_subtype = 0;


	if (depth > 0) {
	    switch(vp->v_type) {
	    case V_MAT:
		    vq = vp->v_mat->m_table;
		    i = vp->v_mat->m_size;
		    while (i-- > 0)
			    protecttodepth(vq++, sts, depth - 1);
		    break;
	    case V_LIST:
		    for (ep = vp->v_list->l_first; ep; ep = ep->e_next)
			    protecttodepth(&ep->e_value, sts, depth - 1);
		    break;
	    case V_OBJ:
		    vq = vp->v_obj->o_table;
		    i = vp->v_obj->o_actions->oa_count;
		    while (i-- > 0)
			    protecttodepth(vq++, sts, depth - 1);
		    break;
	    case V_ASSOC:
		    ap = vp->v_assoc;
		    for (i = 0; i < ap->a_count; i++)
			    protecttodepth(assocfindex(ap, i), sts, depth - 1);
	    }
	}
}


/*
 * Copy a value from one location to another.
 * This overwrites the specified new value without checking it.
 *
 * given:
 *	oldvp		value to be copied from
 *	newvp		value to be copied into
 */
void
copyvalue(VALUE *oldvp, VALUE *newvp)
{
	/* firewall */
	if (oldvp == NULL)
		return;

	newvp->v_type = oldvp->v_type;
	if (oldvp->v_type >= 0) {
		switch (oldvp->v_type) {
		case V_NULL:
		case V_ADDR:
		case V_VPTR:
		case V_OPTR:
		case V_SPTR:
		case V_NPTR:
			*newvp = *oldvp;
			break;
		case V_FILE:
			newvp->v_file = oldvp->v_file;
			break;
		case V_NUM:
			newvp->v_num = qlink(oldvp->v_num);
			break;
		case V_COM:
			newvp->v_com = clink(oldvp->v_com);
			break;
		case V_STR:
			newvp->v_str = slink(oldvp->v_str);
			break;
		case V_MAT:
			newvp->v_mat = matcopy(oldvp->v_mat);
			break;
		case V_LIST:
			newvp->v_list = listcopy(oldvp->v_list);
			break;
		case V_ASSOC:
			newvp->v_assoc = assoccopy(oldvp->v_assoc);
			break;
		case V_OBJ:
			newvp->v_obj = objcopy(oldvp->v_obj);
			break;
		case V_RAND:
			newvp->v_rand = randcopy(oldvp->v_rand);
			break;
		case V_RANDOM:
			newvp->v_random = randomcopy(oldvp->v_random);
			break;
		case V_CONFIG:
			newvp->v_config = config_copy(oldvp->v_config);
			break;
		case V_HASH:
			newvp->v_hash = hash_copy(oldvp->v_hash);
			break;
		case V_BLOCK:
			newvp->v_block = blk_copy(oldvp->v_block);
			break;
		case V_OCTET:
			newvp->v_type = V_NUM;
			newvp->v_num = itoq((long) *oldvp->v_octet);
			break;
		case V_NBLOCK:
			newvp->v_nblock = oldvp->v_nblock;
			break;
		default:
			math_error("Copying unknown value type");
			/*NOTREACHED*/
		}
	}
	newvp->v_subtype = oldvp->v_subtype;
}


/*
 * copy the low order 8 bits of a value to an octet
 */
void
copy2octet(VALUE *vp, OCTET *op)
{
	USB8 oval;	/* low order 8 bits to store into OCTET */
	NUMBER *q;
	HALF h;

	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;

	oval = 0;

	/*
	 * we can (at the moment) only store certain types
	 * values into an OCTET, so get the low order 8 bits
	 * of these particular value types
	 */
	h = 0;
	switch(vp->v_type) {
	case V_NULL:
		/* nothing to store ... so do nothing */
		return;
	case V_INT:
		oval = (USB8)(vp->v_int & 0xff);
		break;
	case V_NUM:
		if (qisint(vp->v_num)) {
			/* use low order 8 bits of integer value */
			h = vp->v_num->num.v[0];
		} else {
			/* use low order 8 bits of int(value) */
			q = qint(vp->v_num);
			h = q->num.v[0];
			qfree(q);
		}
		if (qisneg(vp->v_num))
			h = -h;
		oval = (USB8) h;
		break;
	case V_COM:
		if (cisint(vp->v_com)) {
			/* use low order 8 bits of integer value */
			h = vp->v_com->real->num.v[0];
		} else {
			/* use low order 8 bits of int(value) */
			q = qint(vp->v_com->real);
			h = q->num.v[0];
			qfree(q);
		}
		if (qisneg(vp->v_com->real))
			h = -h;
		oval = (USB8) h;
		break;
	case V_STR:
		oval = (USB8) vp->v_str->s_str[0];
		break;
	case V_BLOCK:
		oval = (USB8) vp->v_block->data[0];
		break;
	case V_OCTET:
		oval = *vp->v_octet;
		break;
	case V_NBLOCK:
		if (vp->v_nblock->blk->data == NULL)
			return;
		oval = (USB8) vp->v_nblock->blk->data[0];
		break;
	default:
		math_error("invalid assignment into an OCTET");
		break;
	}
	*op = oval;
}


/*
 * Negate an arbitrary value.
 * Result is placed in the indicated location.
 */
void
negvalue(VALUE *vp, VALUE *vres)
{
	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qneg(vp->v_num);
		return;
	case V_COM:
		vres->v_com = c_neg(vp->v_com);
		return;
	case V_MAT:
		vres->v_mat = matneg(vp->v_mat);
		return;
	case V_STR:
		vres->v_str = stringneg(vp->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRNEG);
		return;
	case V_OCTET:
		vres->v_type = V_NUM;
		vres->v_subtype = V_NOSUBTYPE;
		vres->v_num = itoq(- (long) *vp->v_octet);
		return;

	case V_OBJ:
		*vres = objcall(OBJ_NEG, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type <= 0)
			return;
		*vres = error_value(E_NEG);
		return;
	}
}


/*
 * Add two arbitrary values together.
 * Result is placed in the indicated location.
 */
void
addvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	COMPLEX *c;
	VALUE tmp;
	NUMBER *q;
	long i;

	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type == V_LIST) {
		tmp.v_type = V_NULL;
		addlistitems(v1->v_list, &tmp);
		addvalue(&tmp, v2, vres);
		return;
	}
	if (v2->v_type == V_LIST) {
		copyvalue(v1, vres);
		addlistitems(v2->v_list, vres);
		return;
	}
	if (v1->v_type == V_NULL) {
		copyvalue(v2, vres);
		return;
	}
	if (v2->v_type == V_NULL) {
		copyvalue(v1, vres);
		return;
	}
	vres->v_type = v1->v_type;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qqadd(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_COM, V_NUM):
		vres->v_com = c_addq(v1->v_com, v2->v_num);
		return;
	case TWOVAL(V_NUM, V_COM):
		vres->v_com = c_addq(v2->v_com, v1->v_num);
		vres->v_type = V_COM;
		return;
	case TWOVAL(V_COM, V_COM):
		vres->v_com = c_add(v1->v_com, v2->v_com);
		c = vres->v_com;
		if (!cisreal(c))
			return;
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	case TWOVAL(V_MAT, V_MAT):
		vres->v_mat = matadd(v1->v_mat, v2->v_mat);
		return;
	case TWOVAL(V_STR, V_STR):
		vres->v_str = stringadd(v1->v_str, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRADD);
		return;
	case TWOVAL(V_VPTR, V_NUM):
		q = v2->v_num;
		if (qisfrac(q)) {
			math_error("Adding non-integer to address");
			/*NOTREACHED*/
		}
		i = qtoi(q);
		vres->v_addr = v1->v_addr + i;
		vres->v_type = V_VPTR;
		return;
	case TWOVAL(V_OPTR, V_NUM):
		q = v2->v_num;
		if (qisfrac(q)) {
			math_error("Adding non-integer to address");
			/*NOTREACHED*/
		}
		i = qtoi(q);
		vres->v_octet = v1->v_octet + i;
		vres->v_type = V_OPTR;
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			if (v1->v_type < 0)
				return;
			if (v2->v_type > 0)
				*vres = error_value(E_ADD);
			else
				vres->v_type = v2->v_type;
			return;
		}
		*vres = objcall(OBJ_ADD, v1, v2, NULL_VALUE);
		return;
	}
}


/*
 * Subtract one arbitrary value from another one.
 * Result is placed in the indicated location.
 */
void
subvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	COMPLEX *c;
	NUMBER *q;
	int i;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qsub(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_COM, V_NUM):
		vres->v_com = c_subq(v1->v_com, v2->v_num);
		return;
	case TWOVAL(V_NUM, V_COM):
		c = c_subq(v2->v_com, v1->v_num);
		vres->v_type = V_COM;
		vres->v_com = c_neg(c);
		comfree(c);
		return;
	case TWOVAL(V_COM, V_COM):
		vres->v_com = c_sub(v1->v_com, v2->v_com);
		c = vres->v_com;
		if (!cisreal(c))
			return;
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	case TWOVAL(V_MAT, V_MAT):
		vres->v_mat = matsub(v1->v_mat, v2->v_mat);
		return;
	case TWOVAL(V_STR, V_STR):
		vres->v_str = stringsub(v1->v_str, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRSUB);
		return;
	case TWOVAL(V_VPTR, V_NUM):
		q = v2->v_num;
		if (qisfrac(q)) {
			math_error("Subtracting non-integer from address");
			/*NOTREACHED*/
		}
		i = qtoi(q);
		vres->v_addr = v1->v_addr - i;
		vres->v_type = V_VPTR;
		return;
	case TWOVAL(V_OPTR, V_NUM):
		q = v2->v_num;
		if (qisfrac(q)) {
			math_error("Adding non-integer to address");
			/*NOTREACHED*/
		}
		i = qtoi(q);
		vres->v_octet = v1->v_octet - i;
		vres->v_type = V_OPTR;
		return;
	case TWOVAL(V_VPTR, V_VPTR):
		vres->v_type = V_NUM;
		vres->v_num = itoq(v1->v_addr - v2->v_addr);
		return;
	case TWOVAL(V_OPTR, V_OPTR):
		vres->v_type = V_NUM;
		vres->v_num = itoq(v1->v_octet - v2->v_octet);
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			if (v1->v_type <= 0)
				return;
			if (v2->v_type <= 0) {
				vres->v_type = v2->v_type;
				return;
			}
			*vres = error_value(E_SUB);
			return;
		}
		*vres = objcall(OBJ_SUB, v1, v2, NULL_VALUE);
		return;
	}
}


/*
 * Multiply two arbitrary values together.
 * Result is placed in the indicated location.
 */
void
mulvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	COMPLEX *c;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qmul(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_COM, V_NUM):
		vres->v_com = c_mulq(v1->v_com, v2->v_num);
		break;
	case TWOVAL(V_NUM, V_COM):
		vres->v_com = c_mulq(v2->v_com, v1->v_num);
		vres->v_type = V_COM;
		break;
	case TWOVAL(V_COM, V_COM):
		vres->v_com = c_mul(v1->v_com, v2->v_com);
		break;
	case TWOVAL(V_MAT, V_MAT):
		vres->v_mat = matmul(v1->v_mat, v2->v_mat);
		return;
	case TWOVAL(V_MAT, V_NUM):
	case TWOVAL(V_MAT, V_COM):
		vres->v_mat = matmulval(v1->v_mat, v2);
		return;
	case TWOVAL(V_NUM, V_MAT):
	case TWOVAL(V_COM, V_MAT):
		vres->v_mat = matmulval(v2->v_mat, v1);
		vres->v_type = V_MAT;
		return;
	case TWOVAL(V_NUM, V_STR):
		vres->v_type = V_STR;
		vres->v_str = stringmul(v1->v_num, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRMUL);
		return;
	case TWOVAL(V_STR, V_NUM):
		vres->v_str= stringmul(v2->v_num, v1->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRMUL);
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			if (v1->v_type <= 0)
				return;
			if (v2->v_type <= 0) {
				vres->v_type = v2->v_type;
				return;
			}
			*vres = error_value(E_MUL);
			return;
		}
		*vres = objcall(OBJ_MUL, v1, v2, NULL_VALUE);
		return;
	}
	c = vres->v_com;
	if (cisreal(c)) {
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
	}
}


/*
 * Square an arbitrary value.
 * Result is placed in the indicated location.
 */
void
squarevalue(VALUE *vp, VALUE *vres)
{
	COMPLEX *c;

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qsquare(vp->v_num);
		return;
	case V_COM:
		vres->v_com = c_square(vp->v_com);
		c = vres->v_com;
		if (!cisreal(c))
			return;
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	case V_MAT:
		vres->v_mat = matsquare(vp->v_mat);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_SQUARE, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type <= 0) {
			vres->v_type = vp->v_type;
			return;
		}
		*vres = error_value(E_SQUARE);
		return;
	}
}


/*
 * Invert an arbitrary value.
 * Result is placed in the indicated location.
 */
void
invertvalue(VALUE *vp, VALUE *vres)
{
	NUMBER *q1, *q2;

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		if (qiszero(vp->v_num))
			*vres = error_value(E_1OVER0);
		else
			vres->v_num = qinv(vp->v_num);
		return;
	case V_COM:
		vres->v_com = c_inv(vp->v_com);
		return;
	case V_MAT:
		vres->v_mat = matinv(vp->v_mat);
		return;
	case V_OCTET:
		if (*vp->v_octet == 0) {
			*vres = error_value(E_1OVER0);
			return;
		}
		q1 = itoq((long) *vp->v_octet);
		q2 = qinv(q1);
		qfree(q1);
		vres->v_num = q2;
		vres->v_type = V_NUM;
		return;
	case V_OBJ:
		*vres = objcall(OBJ_INV, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type == -E_1OVER0) {
			vres->v_type = V_NUM;
			vres->v_num = qlink(&_qzero_);
			return;
		}
		if (vp->v_type <= 0)
			return;
		*vres = error_value(E_INV);
		return;
	}
}



/*
 * "AND" two arbitrary values together.
 * Result is placed in the indicated location.
 */
void
andvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type == V_NULL) {
		copyvalue(v2, vres);
		return;
	}
	if (v2->v_type == V_NULL) {
		copyvalue(v1, vres);
		return;
	}
	vres->v_type = v1->v_type;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qand(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_STR, V_STR):
		vres->v_str = stringand(v1->v_str, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRAND);
		return;
	case TWOVAL(V_OCTET, V_OCTET):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet & *v2->v_octet);
		return;
	case TWOVAL(V_STR, V_OCTET):
		vres->v_str = charstring(*v1->v_str->s_str &
					*v2->v_octet);
		return;
	case TWOVAL(V_OCTET, V_STR):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet &
					*v2->v_str->s_str);
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			if (v1->v_type < 0)
				return;
			if (v2->v_type < 0) {
				vres->v_type = v2->v_type;
				return;
			}
			*vres = error_value(E_AND);
			return;
		}
		*vres = objcall(OBJ_AND, v1, v2, NULL_VALUE);
		return;
	}
}


/*
 * "OR" two arbitrary values together.
 * Result is placed in the indicated location.
 */
void
orvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	if (v1->v_type == V_NULL) {
		copyvalue(v2, vres);
		return;
	}
	if (v2->v_type == V_NULL) {
		copyvalue(v1, vres);
		return;
	}
	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qor(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_STR, V_STR):
		vres->v_str = stringor(v1->v_str, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STROR);
		return;
	case TWOVAL(V_OCTET, V_OCTET):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet | *v2->v_octet);
		return;
	case TWOVAL(V_STR, V_OCTET):
		vres->v_str = charstring(*v1->v_str->s_str |
				*v2->v_octet);
		return;
	case TWOVAL(V_OCTET, V_STR):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet |
				*v2->v_str->s_str);
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			if (v1->v_type < 0)
				return;
			if (v2->v_type < 0) {
				vres->v_type = v2->v_type;
				return;
			}
			*vres = error_value(E_OR);
			return;
		}
		*vres = objcall(OBJ_OR, v1, v2, NULL_VALUE);
		return;
	}
}


/*
 * "~" two values, returns the "symmetric difference" bitwise xor(v1, v2) for
 * strings, octets and real numbers, and a user-defined function if at least
 * one of v1 and v2 is an object.
 */
void
xorvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case (TWOVAL(V_NUM, V_NUM)):
		vres->v_num = qxor(v1->v_num, v2->v_num);
		return;
	case (TWOVAL(V_STR, V_STR)):
		vres->v_str = stringxor(v1->v_str, v2->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRDIFF);
		return;
	case (TWOVAL(V_STR, V_OCTET)):
		if (v1->v_str->s_len) {
			vres->v_str = stringcopy(v1->v_str);
			*vres->v_str->s_str ^= *v2->v_octet;
		} else {
			vres->v_str = charstring(*v2->v_octet);
		}
		return;
	case (TWOVAL(V_OCTET, V_STR)):
		if (v2->v_str->s_len) {
			vres->v_str = stringcopy(v2->v_str);
			*vres->v_str->s_str ^= *v1->v_octet;
		} else {
			vres->v_str = charstring(*v1->v_octet);
		}
		return;
	case (TWOVAL(V_OCTET, V_OCTET)):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet ^ *v2->v_octet);
		return;
	default:
		if (v1->v_type == V_OBJ || v2->v_type == V_OBJ)
			*vres = objcall(OBJ_XOR, v1, v2, NULL_VALUE);
		else
			*vres = error_value(E_XOR);
	}
}


/*
 * "#" two values - abs(v1-v2) for numbers, user-defined for objects
 */
void
hashopvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	NUMBER *q;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		q = qsub(v1->v_num, v2->v_num);
		vres->v_num = qqabs(q);
		qfree(q);
		return;
	default:
		if (v1->v_type == V_OBJ || v2->v_type == V_OBJ)
			*vres = objcall(OBJ_HASHOP, v1, v2, NULL_VALUE);
		else
			*vres = error_value(E_HASHOP);
	}
}


void
compvalue(VALUE *vp, VALUE *vres)
{

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qcomp(vp->v_num);
		return;
	case V_STR:
		vres->v_str = stringcomp(vp->v_str);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRCOMP);
		return;
	case V_OCTET:
		vres->v_type = V_STR;
		vres->v_str = charstring(~*vp->v_octet);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_COMP, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		*vres = error_value(E_COMP);
	}
}

/*
 * "\" a value, user-defined only
 */
void
backslashvalue(VALUE *vp, VALUE *vres)
{
	if (vp->v_type == V_OBJ)
		*vres = objcall(OBJ_BACKSLASH, vp, NULL_VALUE, NULL_VALUE);
	else
		*vres = error_value(E_BACKSLASH);
}


/*
 * "\" two values, for strings performs bitwise "AND-NOT" operation
 * User defined for objects
 */
void
setminusvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qandnot(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_STR, V_STR):
		vres->v_str = stringdiff(v1->v_str, v2->v_str);
		return;
	case TWOVAL(V_STR, V_OCTET):
		vres->v_str = charstring(*v1->v_str->s_str &
			~*v2->v_octet);
		return;
	case TWOVAL(V_OCTET, V_STR):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet &
			~*v2->v_str->s_str);
		return;
	case TWOVAL(V_OCTET, V_OCTET):
		vres->v_type = V_STR;
		vres->v_str = charstring(*v1->v_octet &
			~*v2->v_octet);
		return;
	default:
		if (v1->v_type == V_OBJ || v2->v_type == V_OBJ)
			*vres = objcall(OBJ_SETMINUS, v1, v2,
				NULL_VALUE);
		else
			*vres = error_value(E_SETMINUS);
	}
}


/*
 * "#" a value, for strings and octets returns the number of nonzero bits
 * in the value; user-defined for an object
 */
void
contentvalue(VALUE *vp, VALUE *vres)
{
	long count;
	unsigned char u;

	vres->v_type = V_NUM;
	vres->v_subtype = V_NOSUBTYPE;
	count = 0;
	switch (vp->v_type) {
	case V_STR:
		count = stringcontent(vp->v_str);
		break;
	case V_OCTET:
		for (u = *vp->v_octet; u; u >>= 1)
			count += (u & 1);
		break;
	case V_NUM:
		count = zpopcnt(vp->v_num->num, 1);
		break;
	case V_OBJ:
		*vres = objcall(OBJ_CONTENT, vp, NULL_VALUE,
			NULL_VALUE);
		return;
	default:
		*vres = error_value(E_CONTENT);
		return;
	}
	vres->v_num = itoq(count);
}


/*
 * Approximate numbers by multiples of v2 using rounding criterion v3.
 * Result is placed in the indicated location.
 */
void
apprvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *e;
	long R = 0;
	NUMBER *q1, *q2;
	COMPLEX *c;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0)
		return;

	e = NULL;
	switch(v2->v_type) {
	case V_NUM:	e = v2->v_num;
			break;
	case V_NULL:	e = conf->epsilon;
			break;
	default:
		*vres = error_value(E_APPR2);
		return;
	}
	switch(v3->v_type) {
	case V_NUM:	if (qisfrac(v3->v_num)) {
				*vres = error_value(E_APPR3);
				return;
			}
			R = qtoi(v3->v_num);
			break;
	case V_NULL:	R = conf->appr;
			break;
	default:
		*vres = error_value(E_APPR3);
		return;
	}

	if (qiszero(e)) {
		copyvalue(v1, vres);
		return;
	}
	switch (v1->v_type) {
	case V_NUM:
		vres->v_num = qmappr(v1->v_num, e, R);
		return;
	case V_MAT:
		vres->v_mat = matappr(v1->v_mat, v2, v3);
		return;
	case V_LIST:
		vres->v_list = listappr(v1->v_list, v2, v3);
		return;
	case V_COM:
		q1 = qmappr(v1->v_com->real, e, R);
		q2 = qmappr(v1->v_com->imag, e, R);
		if (qiszero(q2)) {
			vres->v_type = V_NUM;
			vres->v_num = q1;
			qfree(q2);
			return;
		}
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = q1;
		c->imag = q2;
		vres->v_com = c;
		return;
	default:
		*vres = error_value(E_APPR);
		return;
	}
}


/*
 * Round numbers to number of decimals specified by v2, type of rounding
 * specified by v3.  Result placed in location vres.
 */
void
roundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *q1, *q2;
	COMPLEX *c;
	long places, rnd;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type == V_MAT) {
		vres->v_mat = matround(v1->v_mat, v2, v3);
		return;
	}
	if (v1->v_type == V_LIST) {
		vres->v_list = listround(v1->v_list, v2, v3);
		return;
	}
	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_ROUND, v1, v2, v3);
		return;
	}
	places = 0;
	switch (v2->v_type) {
	case V_NUM:
		if (qisfrac(v2->v_num)) {
			*vres = error_value(E_ROUND2);
			return;
		}
		places = qtoi(v2->v_num);
		break;
	case V_NULL:
		break;
	default:
		*vres = error_value(E_ROUND2);
		return;
	}
	rnd = 0;
	switch (v3->v_type) {
	case V_NUM:
		if (qisfrac(v3->v_num)) {
			*vres = error_value(E_ROUND3);
			return;
		}
		rnd = qtoi(v3->v_num);
		break;
	case V_NULL:
		rnd = conf->round;
		break;
	default:
		*vres = error_value(E_ROUND3);
		return;
	}
	switch(v1->v_type) {
	case V_NUM:
		vres->v_num = qround(v1->v_num, places, rnd);
		return;
	case V_COM:
		q1 = qround(v1->v_com->real, places, rnd);
		q2 = qround(v1->v_com->imag, places, rnd);
		if (qiszero(q2)) {
			vres->v_type = V_NUM;
			vres->v_num = q1;
			qfree(q2);
			return;
		}
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = q1;
		c->imag = q2;
		vres->v_com = c;
		return;
	default:
		if (v1->v_type <= 0)
			return;
		*vres = error_value(E_ROUND);
		return;
	}
}



/*
 * Round numbers to number of binary digits specified by v2, type of rounding
 * specified by v3.  Result placed in location vres.
 */
void
broundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *q1, *q2;
	COMPLEX *c;
	long places, rnd;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type == V_MAT) {
		vres->v_mat = matbround(v1->v_mat, v2, v3);
		return;
	}
	if (v1->v_type == V_LIST) {
		vres->v_list = listbround(v1->v_list, v2, v3);
		return;
	}
	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_BROUND, v1, v2, v3);
		return;
	}
	places = 0;
	switch (v2->v_type) {
	case V_NUM:
		if (qisfrac(v2->v_num)) {
			*vres = error_value(E_BROUND2);
			return;
		}
		places = qtoi(v2->v_num);
		break;
	case V_NULL:
		break;
	default:
		*vres = error_value(E_BROUND2);
		return;
	}
	rnd = 0;
	switch (v3->v_type) {
	case V_NUM:
		if (qisfrac(v3->v_num)) {
			*vres = error_value(E_BROUND3);
			return;
		}
		rnd = qtoi(v3->v_num);
		break;
	case V_NULL:
		rnd = conf->round;
		break;
	default:
		*vres = error_value(E_BROUND3);
		return;
	}
	switch(v1->v_type) {
	case V_NUM:
		vres->v_num = qbround(v1->v_num, places, rnd);
		return;
	case V_COM:
		q1 = qbround(v1->v_com->real, places, rnd);
		q2 = qbround(v1->v_com->imag, places, rnd);
		if (qiszero(q2)) {
			vres->v_type = V_NUM;
			vres->v_num = q1;
			qfree(q2);
			return;
		}
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = q1;
		c->imag = q2;
		vres->v_com = c;
		return;
	default:
		if (v1->v_type <= 0)
			return;
		*vres = error_value(E_BROUND);
		return;
	}
}

/*
 * Take the integer part of an arbitrary value.
 * Result is placed in the indicated location.
 */
void
intvalue(VALUE *vp, VALUE *vres)
{
	COMPLEX *c;

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		if (qisint(vp->v_num))
			vres->v_num = qlink(vp->v_num);
		else
			vres->v_num = qint(vp->v_num);
		return;
	case V_COM:
		if (cisint(vp->v_com)) {
			vres->v_com = clink(vp->v_com);
			return;
		}
		vres->v_com = c_int(vp->v_com);
		c = vres->v_com;
		if (cisreal(c)) {
			vres->v_num = qlink(c->real);
			vres->v_type = V_NUM;
			comfree(c);
		}
		return;
	case V_MAT:
		vres->v_mat = matint(vp->v_mat);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_INT, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type <= 0)
			return;
		*vres = error_value(E_INT);
		return;
	}
}


/*
 * Take the fractional part of an arbitrary value.
 * Result is placed in the indicated location.
 */
void
fracvalue(VALUE *vp, VALUE *vres)
{
	COMPLEX *c;

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		if (qisint(vp->v_num))
			vres->v_num = qlink(&_qzero_);
		else
			vres->v_num = qfrac(vp->v_num);
		return;
	case V_COM:
		if (cisint(vp->v_com)) {
			vres->v_num = clink(&_qzero_);
			vres->v_type = V_NUM;
			return;
		}
		vres->v_com = c_frac(vp->v_com);
		c = vres->v_com;
		if (cisreal(c)) {
			vres->v_num = qlink(c->real);
			vres->v_type = V_NUM;
			comfree(c);
		}
		return;
	case V_MAT:
		vres->v_mat = matfrac(vp->v_mat);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_FRAC, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type < 0)
			return;
		*vres = error_value(E_FRAC);
		return;
	}
}


/*
 * Increment an arbitrary value by one.
 * Result is placed in the indicated location.
 */
void
incvalue(VALUE *vp, VALUE *vres)
{
	vres->v_type = vp->v_type;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qinc(vp->v_num);
		break;
	case V_COM:
		vres->v_com = c_addq(vp->v_com, &_qone_);
		break;
	case V_OBJ:
		*vres = objcall(OBJ_INC, vp, NULL_VALUE, NULL_VALUE);
		break;
	case V_OCTET:
		*vres->v_octet = *vp->v_octet + 1;
		break;
	case V_OPTR:
		vres->v_octet = vp->v_octet + 1;
		break;
	case V_VPTR:
		vres->v_addr = vp->v_addr + 1;
		break;
	default:
		if (vp->v_type > 0)
			*vres = error_value(E_INCV);
		break;
	}
	vres->v_subtype = vp->v_subtype;
}


/*
 * Decrement an arbitrary value by one.
 * Result is placed in the indicated location.
 */
void
decvalue(VALUE *vp, VALUE *vres)
{
	vres->v_type = vp->v_type;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qdec(vp->v_num);
		break;
	case V_COM:
		vres->v_com = c_addq(vp->v_com, &_qnegone_);
		break;
	case V_OBJ:
		*vres = objcall(OBJ_DEC, vp, NULL_VALUE, NULL_VALUE);
		break;
	case V_OCTET:
		*vres->v_octet = *vp->v_octet - 1;
		break;
	case V_OPTR:
		vres->v_octet = vp->v_octet - 1;
		break;
	case V_VPTR:
		vres->v_addr = vp->v_addr - 1;
		break;
	default:
		if (vp->v_type >= 0)
			*vres = error_value(E_DECV);
		break;
	}
	vres->v_subtype = vp->v_subtype;
}


/*
 * Produce the 'conjugate' of an arbitrary value.
 * Result is placed in the indicated location.
 * (Example: complex conjugate.)
 */
void
conjvalue(VALUE *vp, VALUE *vres)
{
	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qlink(vp->v_num);
		return;
	case V_COM:
		vres->v_com = comalloc();
		qfree(vres->v_com->real);
		qfree(vres->v_com->imag)
		vres->v_com->real = qlink(vp->v_com->real);
		vres->v_com->imag = qneg(vp->v_com->imag);
		return;
	case V_MAT:
		vres->v_mat = matconj(vp->v_mat);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_CONJ, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type <= 0) {
			vres->v_type = vp->v_type;
			return;
		}
		*vres = error_value(E_CONJ);
		return;
	}
}


/*
 * Take the square root of an arbitrary value within the specified error.
 * Result is placed in the indicated location.
 */
void
sqrtvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *q, *tmp;
	COMPLEX *c;
	long R;

	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
			*vres = objcall(OBJ_SQRT, v1, v2, v3);
			return;
	}
	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	if (v2->v_type == V_NULL) {
		q = conf->epsilon;
	} else {
		if (v2->v_type != V_NUM || qiszero(v2->v_num)) {
			*vres = error_value(E_SQRT2);
			return;
		}
		q = v2->v_num;
	}
	if (v3->v_type == V_NULL) {
		R = conf->sqrt;
	} else {
		if (v3->v_type != V_NUM || qisfrac(v3->v_num)) {
			*vres = error_value(E_SQRT3);
			return;
		}
		R = qtoi(v3->v_num);
	}
	switch (v1->v_type) {
	case V_NUM:
		if (!qisneg(v1->v_num)) {
			vres->v_num = qsqrt(v1->v_num, q, R);
			return;
		}
		tmp = qneg(v1->v_num);
		c = comalloc();
		qfree(c->imag);
		c->imag = qsqrt(tmp, q, R);
		qfree(tmp);
		vres->v_com = c;
		vres->v_type = V_COM;
		break;
	case V_COM:
		vres->v_com = c_sqrt(v1->v_com, q, R);
		break;
	default:
		*vres = error_value(E_SQRT);
		return;
	}
	c = vres->v_com;
	if (cisreal(c)) {
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
	}
}


/*
 * Take the Nth root of an arbitrary value within the specified error.
 * Result is placed in the indicated location.
 *
 * given:
 *	v1		value to take root of
 *	v2		value specifying root to take
 *	v3		value specifying error
 *	vres		result
 */
void
rootvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *q2, *q3;
	COMPLEX ctmp;
	COMPLEX *c;

	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	if (v2->v_type != V_NUM) {
		*vres = error_value(E_ROOT2);
		return;
	}
	q2 = v2->v_num;
	if (qisneg(q2) || qiszero(q2) || qisfrac(q2)) {
		*vres = error_value(E_ROOT2);
		return;
	}
	if (v3->v_type != V_NUM || qiszero(v3->v_num)) {
		*vres = error_value(E_ROOT3);
		return;
	}
	q3 = v3->v_num;
	switch (v1->v_type) {
	case V_NUM:
		if (!qisneg(v1->v_num)) {
			vres->v_num = qroot(v1->v_num, q2, q3);
			if (vres->v_num == NULL)
				*vres = error_value(E_ROOT4);
			vres->v_type = V_NUM;
			return;
		}
		ctmp.real = v1->v_num;
		ctmp.imag = &_qzero_;
		ctmp.links = 1;
		c = c_root(&ctmp, q2, q3);
		break;
	case V_COM:
		c = c_root(v1->v_com, q2, q3);
		break;
	case V_OBJ:
		*vres = objcall(OBJ_ROOT, v1, v2, v3);
		return;
	default:
		*vres = error_value(E_ROOT);
		return;
	}
	if (c == NULL) {
		*vres = error_value(E_ROOT4);
		return;
	}
	vres->v_com = c;
	vres->v_type = V_COM;
	if (cisreal(c)) {
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
	}
}


/*
 * Take the absolute value of an arbitrary value within the specified error.
 * Result is placed in the indicated location.
 */
void
absvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	STATIC NUMBER *q;

	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_ABS, v1, v2, NULL_VALUE);
		return;
	}
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	switch (v1->v_type) {
	case V_NUM:
		if (qisneg(v1->v_num))
			q = qneg(v1->v_num);
		else
			q = qlink(v1->v_num);
		break;
	case V_COM:
		if (v2->v_type != V_NUM || qiszero(v2->v_num)) {
			*vres = error_value(E_ABS2);
			return;
		}
		q = qhypot(v1->v_com->real, v1->v_com->imag, v2->v_num);
		break;
	default:
		*vres = error_value(E_ABS);
		return;
	}
	vres->v_num = q;
	vres->v_type = V_NUM;
}


/*
 * Calculate the norm of an arbitrary value.
 * Result is placed in the indicated location.
 * The norm is the square of the absolute value.
 */
void
normvalue(VALUE *vp, VALUE *vres)
{
	NUMBER *q1, *q2;

	vres->v_type = vp->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (vp->v_type <= 0) {
		vres->v_type = vp->v_type;
		return;
	}
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qsquare(vp->v_num);
		return;
	case V_COM:
		q1 = qsquare(vp->v_com->real);
		q2 = qsquare(vp->v_com->imag);
		vres->v_num = qqadd(q1, q2);
		vres->v_type = V_NUM;
		qfree(q1);
		qfree(q2);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_NORM, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		*vres = error_value(E_NORM);
		return;
	}
}


/*
 * Shift a value left or right by the specified number of bits.
 * Negative shift value means shift the direction opposite the selected dir.
 * Right shifts are defined to lose bits off the low end of the number.
 * Result is placed in the indicated location.
 *
 * given:
 *	v1		value to shift
 *	v2		shift amount
 *	rightshift	TRUE if shift right instead of left
 *	vres		result
 */
void
shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres)
{
	COMPLEX *c;
	long n = 0;
	unsigned int ch;
	VALUE tmp;

	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	if ((v2->v_type != V_NUM) || (qisfrac(v2->v_num))) {
		*vres = error_value(E_SHIFT2);
		return;
	}
	if (v1->v_type != V_OBJ) {
		if (zge31b(v2->v_num->num)) {
			*vres = error_value(E_SHIFT2);
			return;
		}
		n = qtoi(v2->v_num);
	}
	if (rightshift)
		n = -n;
	vres->v_type = v1->v_type;
	switch (v1->v_type) {
	case V_NUM:
		if (qisfrac(v1->v_num)) {
			*vres = error_value(E_SHIFT);
			return;
		}
		vres->v_num = qshift(v1->v_num, n);
		return;
	case V_COM:
		if (qisfrac(v1->v_com->real) ||
				qisfrac(v1->v_com->imag)) {
			*vres = error_value(E_SHIFT);
			return;
		}
		c = c_shift(v1->v_com, n);
		if (!cisreal(c)) {
			vres->v_com = c;
			return;
		}
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	case V_MAT:
		vres->v_mat = matshift(v1->v_mat, n);
		return;
	case V_STR:
		vres->v_str = stringshift(v1->v_str, n);
		if (vres->v_str == NULL)
			*vres = error_value(E_STRSHIFT);
		return;
	case V_OCTET:
		vres->v_type = V_STR;
		if (n >= 8 || n <= -8)
			ch = 0;
		else if (n >= 0)
			ch = (unsigned int) *v1->v_octet << n;
		else
			ch = (unsigned int) *v1->v_octet >> -n;
		vres->v_str = charstring(ch);
		return;
	case V_OBJ:
		if (!rightshift) {
			*vres = objcall(OBJ_SHIFT, v1, v2, NULL_VALUE);
			return;
		}
		tmp.v_num = qneg(v2->v_num);
		tmp.v_type = V_NUM;
		*vres = objcall(OBJ_SHIFT, v1, &tmp, NULL_VALUE);
		qfree(tmp.v_num);
		return;
	default:
		*vres = error_value(E_SHIFT);
		return;
	}
}


/*
 * Scale a value by a power of two.
 * Result is placed in the indicated location.
 */
void
scalevalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	long n = 0;

	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	if ((v2->v_type != V_NUM) || qisfrac(v2->v_num)) {
		*vres = error_value(E_SCALE2);
		return;
	}
	if (v1->v_type != V_OBJ) {
		if (zge31b(v2->v_num->num)) {
			*vres = error_value(E_SCALE2);
			return;
		}
		n = qtoi(v2->v_num);
	}
	vres->v_type = v1->v_type;
	switch (v1->v_type) {
	case V_NUM:
		vres->v_num = qscale(v1->v_num, n);
		return;
	case V_COM:
		vres->v_com = c_scale(v1->v_com, n);
		return;
	case V_MAT:
		vres->v_mat = matscale(v1->v_mat, n);
		return;
	case V_OBJ:
		*vres = objcall(OBJ_SCALE, v1, v2, NULL_VALUE);
		return;
	default:
		*vres = error_value(E_SCALE);
		return;
	}
}


/*
 * Raise a value to an power.
 * Result is placed in the indicated location.
 */
void
powvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	NUMBER *real_v2;	/* real part of v2 */
	COMPLEX *c;

	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_POW, v1, v2, NULL_VALUE);
		return;
	}
	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0 && v1->v_type != -E_1OVER0)
		return;
	if (v2->v_type <= 0) {
		vres->v_type = v2->v_type;
		return;
	}
	real_v2 = v2->v_num;

	/* case: raising to a real power */
	switch (v2->v_type) {
	case V_NUM:

		/* deal with the division by 0 value */
		if (v1->v_type == -E_1OVER0) {
			if (qisneg(real_v2)) {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
			} else {
				vres->v_type = -E_1OVER0;
			}
			break;
		}

		/* raise something with a real exponent */
		switch (v1->v_type) {
		case V_NUM:
			if (qiszero(v1->v_num)) {
				if (qisneg(real_v2)) {
					*vres = error_value(E_1OVER0);
					break;
				}
				/* 0 ^ non-neg is 1, including 0^0 */
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qone_);
			} else if (qisint(real_v2)) {
				vres->v_num = qpowi(v1->v_num, real_v2);
			} else {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
				powervalue(v1, v2, NULL, vres);
			}
			break;
		case V_COM:
			if (qisint(real_v2)) {
				vres->v_com = c_powi(v1->v_com, real_v2);
			} else {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
				powervalue(v1, v2, NULL, vres);
			}
			if (vres->v_type == V_COM) {
				c = vres->v_com;
				if (!cisreal(c))
					break;
				vres->v_num = qlink(c->real);
				vres->v_type = V_NUM;
				comfree(c);
			}
			break;
		case V_MAT:
			vres->v_mat = matpowi(v1->v_mat, real_v2);
			break;
		default:
			*vres = error_value(E_POWI);
			break;
		}
		break;

	case V_COM:

		/* deal with the division by 0 value */
		if (v1->v_type == -E_1OVER0) {
			if (cisreal(v2->v_com) && qisneg(real_v2)) {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
			} else {
				vres->v_type = -E_1OVER0;
			}
			break;
		}

		/* raise something with a real exponent */
		switch (v1->v_type) {
		case V_NUM:
			if (qiszero(v1->v_num)) {
				if (cisreal(v2->v_com) && qisneg(real_v2)) {
					*vres = error_value(E_1OVER0);
					break;
				}
				/*
				 * 0 ^ real non-neg is zero
				 * 0 ^ complex is zero
				 */
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
			}
			if (cisreal(v2->v_com) && qisint(real_v2)) {
				vres->v_num = qpowi(v1->v_num, real_v2);
			} else {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
				powervalue(v1, v2, NULL, vres);
			}
			if (vres->v_type == V_COM) {
				c = vres->v_com;
				if (!cisreal(c))
					break;
				vres->v_num = qlink(c->real);
				vres->v_type = V_NUM;
				comfree(c);
			}
			break;
		case V_COM:
			if (cisreal(v2->v_com) && qisint(real_v2)) {
				vres->v_com = c_powi(v1->v_com, real_v2);
			} else {
				vres->v_type = V_NUM;
				vres->v_num = qlink(&_qzero_);
				powervalue(v1, v2, NULL, vres);
			}
			if (vres->v_type == V_COM) {
				c = vres->v_com;
				if (!cisreal(c))
					break;
				vres->v_num = qlink(c->real);
				vres->v_type = V_NUM;
				comfree(c);
			}
			break;
		default:
			*vres = error_value(E_POWI);
			break;
		}
		break;

	/* unsupported exponent type */
	default:
		*vres = error_value(E_POWI2);
		break;
	}
	return;
}


/*
 * Raise one value to another value's power, within the specified error.
 * Result is placed in the indicated location.  If v3 is NULL, the
 * value conf->epsilon is used.
 */
void
powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *epsilon;
	COMPLEX *c, ctmp1, ctmp2;

	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0) {
		vres->v_type = v1->v_type;
		return;
	}
	if (v1->v_type != V_NUM && v1->v_type != V_COM) {
		*vres = error_value(E_POWER);
		return;
	}
	if (v2->v_type != V_NUM && v2->v_type != V_COM) {
		*vres = error_value(E_POWER2);
		return;
	}

	/* NULL epsilon means use built-in epsilon value */
	if (v3 == NULL) {
		epsilon = conf->epsilon;
	} else {
		if (v3->v_type != V_NUM || qiszero(v3->v_num)) {
			*vres = error_value(E_POWER3);
			return;
		}
		epsilon = v3->v_num;
	}
	if (qiszero(epsilon)) {
		*vres = error_value(E_POWER3);
		return;
	}

	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		if (qisneg(v1->v_num)) {
			ctmp1.real = v1->v_num;
			ctmp1.imag = &_qzero_;
			ctmp1.links = 1;
			ctmp2.real = v2->v_num;
			ctmp2.imag = &_qzero_;
			ctmp2.links = 1;
			c = c_power(&ctmp1, &ctmp2, epsilon);
			break;
		}
		vres->v_num = qpower(v1->v_num, v2->v_num, epsilon);
		vres->v_type = V_NUM;
		if (vres->v_num == NULL)
			*vres = error_value(E_POWER4);
		return;
	case TWOVAL(V_NUM, V_COM):
		ctmp1.real = v1->v_num;
		ctmp1.imag = &_qzero_;
		ctmp1.links = 1;
		c = c_power(&ctmp1, v2->v_com, epsilon);
		break;
	case TWOVAL(V_COM, V_NUM):
		ctmp2.real = v2->v_num;
		ctmp2.imag = &_qzero_;
		ctmp2.links = 1;
		c = c_power(v1->v_com, &ctmp2, epsilon);
		break;
	case TWOVAL(V_COM, V_COM):
		c = c_power(v1->v_com, v2->v_com, epsilon);
		break;
	default:
		*vres = error_value(E_POWER);
		return;
	}
	/*
	 * Here for any complex result.
	 */
	vres->v_type = V_COM;
	vres->v_com = c;
	if (cisreal(c)) {
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
	}
}


/*
 * Divide one arbitrary value by another one.
 * Result is placed in the indicated location.
 */
void
divvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	COMPLEX *c;
	COMPLEX ctmp;
	NUMBER *q;
	VALUE tmpval;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0)
		return;
	if (v2->v_type <= 0) {
		if (testvalue(v1) && v2->v_type == -E_1OVER0) {
			vres->v_type = V_NUM;
			vres->v_num = qlink(&_qzero_);
		}
		else
			vres->v_type = v2->v_type;
		return;
	}
	if (!testvalue(v2)) {
		if (testvalue(v1))
			*vres = error_value(E_1OVER0);
		else
			*vres = error_value(E_0OVER0);
		return;
	}
	vres->v_type = v1->v_type;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qqdiv(v1->v_num, v2->v_num);
		return;
	case TWOVAL(V_COM, V_NUM):
		vres->v_com = c_divq(v1->v_com, v2->v_num);
		return;
	case TWOVAL(V_NUM, V_COM):
		if (qiszero(v1->v_num)) {
			vres->v_num = qlink(&_qzero_);
			return;
		}
		ctmp.real = v1->v_num;
		ctmp.imag = &_qzero_;
		ctmp.links = 1;
		vres->v_com = c_div(&ctmp, v2->v_com);
		vres->v_type = V_COM;
		return;
	case TWOVAL(V_COM, V_COM):
		vres->v_com = c_div(v1->v_com, v2->v_com);
		c = vres->v_com;
		if (cisreal(c)) {
			vres->v_num = qlink(c->real);
			vres->v_type = V_NUM;
			comfree(c);
		}
		return;
	case TWOVAL(V_MAT, V_NUM):
	case TWOVAL(V_MAT, V_COM):
		invertvalue(v2, &tmpval);
		vres->v_mat = matmulval(v1->v_mat, &tmpval);
		freevalue(&tmpval);
		return;
	case TWOVAL(V_STR, V_NUM):
		q = qinv(v2->v_num);
		vres->v_str = stringmul(q, v1->v_str);
		qfree(q);
		if (vres->v_str == NULL)
			*vres = error_value(E_DIV);
		return;
	default:
		if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
			*vres = error_value(E_DIV);
			return;
		}
		*vres = objcall(OBJ_DIV, v1, v2, NULL_VALUE);
		return;
	}
}


/*
 * Divide one arbitrary value by another one keeping only the integer part.
 * Result is placed in the indicated location.
 */
void
quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	COMPLEX *c;
	NUMBER *q1, *q2;
	long rnd;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0)
		return;

	if (v1->v_type == V_MAT) {
		vres->v_mat = matquoval(v1->v_mat, v2, v3);
		return;
	}
	if (v1->v_type == V_LIST) {
		vres->v_list = listquo(v1->v_list, v2, v3);
		return;
	}
	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_QUO, v1, v2, v3);
		return;
	}
	if (v2->v_type <= 0) {
		vres->v_type = v2->v_type;
		return;
	}
	if (v2->v_type != V_NUM) {
		*vres = error_value(E_QUO2);
		return;
	}
	rnd = 0;
	switch (v3->v_type) {
	case V_NUM:
		if (qisfrac(v3->v_num)) {
			*vres = error_value(E_QUO3);
			return;
		}
		rnd = qtoi(v3->v_num);
		break;
	case V_NULL:
		rnd = conf->quo;
		break;
	default:
		*vres = error_value(E_QUO3);
		return;
	}
	switch (v1->v_type) {
	case V_NUM:
		vres->v_num = qquo(v1->v_num, v2->v_num, rnd);
		return;
	case V_COM:
		q1 = qquo(v1->v_com->real, v2->v_num, rnd);
		q2 = qquo(v1->v_com->imag, v2->v_num, rnd);
		if (qiszero(q2)) {
			qfree(q2);
			vres->v_type = V_NUM;
			vres->v_num = q1;
			return;
		}
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = q1;
		c->imag = q2;
		vres->v_com = c;
		return;
	default:
		*vres = error_value(E_QUO);
		return;
	}
}


/*
 * Divide one arbitrary value by another one keeping only the remainder.
 * Result is placed in the indicated location.
 */
void
modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	COMPLEX *c;
	NUMBER *q1, *q2;
	long rnd;

	vres->v_type = v1->v_type;
	vres->v_subtype = V_NOSUBTYPE;
	if (v1->v_type <= 0)
		return;

	if (v1->v_type == V_MAT) {
		vres->v_mat = matmodval(v1->v_mat, v2, v3);
		return;
	}
	if (v1->v_type == V_LIST) {
		vres->v_list = listmod(v1->v_list, v2, v3);
		return;
	}
	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_MOD, v1, v2, v3);
		return;
	}
	if (v2->v_type <= 0) {
		vres->v_type = v2->v_type;
		return;
	}
	if (v2->v_type != V_NUM) {
		*vres = error_value(E_MOD2);
		return;
	}
	rnd = 0;
	switch (v3->v_type) {
	case V_NUM:
		if (qisfrac(v3->v_num)) {
			*vres = error_value(E_MOD3);
			return;
		}
		rnd = qtoi(v3->v_num);
		break;
	case V_NULL:
		rnd = conf->mod;
		break;
	default:
		*vres = error_value(E_MOD3);
		return;
	}
	switch (v1->v_type) {
	case V_NUM:
		vres->v_num = qmod(v1->v_num, v2->v_num, rnd);
		return;
	case V_COM:
		q1 = qmod(v1->v_com->real, v2->v_num, rnd);
		q2 = qmod(v1->v_com->imag, v2->v_num, rnd);
		if (qiszero(q2)) {
			qfree(q2);
			vres->v_type = V_NUM;
			vres->v_num = q1;
			return;
		}
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = q1;
		c->imag = q2;
		vres->v_com = c;
		return;
	default:
		*vres = error_value(E_MOD);
		return;
	}
}


/*
 * Test an arbitrary value to see if it is equal to "zero".
 * The definition of zero varies depending on the value type.  For example,
 * the null string is "zero", and a matrix with zero values is "zero".
 * Returns TRUE if value is not equal to zero.
 */
BOOL
testvalue(VALUE *vp)
{
	VALUE val;
	LISTELEM *ep;
	int i;

	switch (vp->v_type) {
	case V_NUM:
		return !qiszero(vp->v_num);
	case V_COM:
		return !ciszero(vp->v_com);
	case V_STR:
		return stringtest(vp->v_str);
	case V_MAT:
		return mattest(vp->v_mat);
	case V_LIST:
		for (ep = vp->v_list->l_first; ep; ep = ep->e_next) {
			if (testvalue(&ep->e_value))
				return TRUE;
		}
		return FALSE;
	case V_ASSOC:
		return (vp->v_assoc->a_count != 0);
	case V_FILE:
		return validid(vp->v_file);
	case V_NULL:
		break;	/* hack to get gcc on SunOS to be quiet */
	case V_OBJ:
		val = objcall(OBJ_TEST, vp, NULL_VALUE, NULL_VALUE);
		return (val.v_int != 0);
	case V_BLOCK:
		for (i=0; i < vp->v_block->datalen; ++i) {
			if (vp->v_block->data[i]) {
				return TRUE;
			}
		}
		return FALSE;
	case V_OCTET:
		return (*vp->v_octet != 0);
	case V_NBLOCK:
		if (vp->v_nblock->blk->data == NULL)
			return FALSE;
		for (i=0; i < vp->v_nblock->blk->datalen; ++i) {
			if (vp->v_nblock->blk->data[i]) {
				return TRUE;
			}
		}
		return FALSE;
	default:
		return TRUE;
	}
	/* hack to get gcc on SunOS to be quiet */
	return FALSE;
}


/*
 * Compare two values for equality.
 * Returns TRUE if the two values differ.
 */
BOOL
comparevalue(VALUE *v1, VALUE *v2)
{
	int r = FALSE;
	VALUE val;

	if ((v1->v_type == V_OBJ) || (v2->v_type == V_OBJ)) {
		val = objcall(OBJ_CMP, v1, v2, NULL_VALUE);
		return (val.v_int != 0);
	}
	if (v1 == v2)
		return FALSE;
	if (v1->v_type ==  V_OCTET) {
		if (v2->v_type == V_OCTET)
			return (*v1->v_octet != *v2->v_octet);
		if (v2->v_type == V_STR)
			return (*v1->v_octet != (OCTET) *v2->v_str->s_str)
				|| (v2->v_str->s_len != 1);
		if (v2->v_type != V_NUM || qisfrac(v2->v_num) ||
			qisneg(v2->v_num) || v2->v_num->num.len > 1)
				return TRUE;
		return (*v2->v_num->num.v != *v1->v_octet);
	}
	if (v2->v_type == V_OCTET)
		return comparevalue(v2, v1);
	if (v1->v_type != v2->v_type)
		return TRUE;
	if (v1->v_type <= 0)
		return FALSE;
	switch (v1->v_type) {
	case V_NUM:
		r = qcmp(v1->v_num, v2->v_num);
		break;
	case V_COM:
		r = c_cmp(v1->v_com, v2->v_com);
		break;
	case V_STR:
		r = stringcmp(v1->v_str, v2->v_str);
		break;
	case V_MAT:
		r = matcmp(v1->v_mat, v2->v_mat);
		break;
	case V_LIST:
		r = listcmp(v1->v_list, v2->v_list);
		break;
	case V_ASSOC:
		r = assoccmp(v1->v_assoc, v2->v_assoc);
		break;
	case V_FILE:
		r = (v1->v_file != v2->v_file);
		break;
	case V_RAND:
		r = randcmp(v1->v_rand, v2->v_rand);
		break;
	case V_RANDOM:
		r = randomcmp(v1->v_random, v2->v_random);
		break;
	case V_CONFIG:
		r = config_cmp(v1->v_config, v2->v_config);
		break;
	case V_HASH:
		r = hash_cmp(v1->v_hash, v2->v_hash);
		break;
	case V_BLOCK:
		r = blk_cmp(v1->v_block, v2->v_block);
		break;
	case V_OCTET:
		r = (v1->v_octet != v2->v_octet);
		break;
	case V_NBLOCK:
		return (v1->v_nblock != v2->v_nblock);
	case V_VPTR:
		return (v1->v_addr != v2->v_addr);
	case V_OPTR:
		return (v1->v_octet != v2->v_octet);
	case V_SPTR:
		return (v1->v_str != v2->v_str);
	case V_NPTR:
		return (v1->v_num != v2->v_num);
	default:
		math_error("Illegal values for comparevalue");
		/*NOTREACHED*/
	}
	return (r != 0);
}

BOOL
acceptvalue(VALUE *v1, VALUE *v2)
{
	long index;
	FUNC *fp;
	BOOL ret;

	index = adduserfunc("accept");
	fp = findfunc(index);
	if (fp) {
		++stack;
		stack->v_type = V_ADDR;
		stack->v_subtype = V_NOSUBTYPE;
		stack->v_addr = v1;
		++stack;
		stack->v_type = V_ADDR;
		stack->v_subtype = V_NOSUBTYPE;
		stack->v_addr = v2;
		calculate(fp, 2);
		ret = testvalue(stack);
		freevalue(stack--);
		return ret;
	}
	return (!comparevalue(v1, v2));
}


BOOL
precvalue(VALUE *v1, VALUE *v2)
{
	VALUE val;
	long index;
	int r = 0;
	FUNC *fp;
	BOOL ret;

	index = adduserfunc("precedes");
	fp = findfunc(index);
	if (fp) {
		++stack;
		stack->v_type = V_ADDR;
		stack->v_subtype = V_NOSUBTYPE;
		stack->v_addr = v1;
		++stack;
		stack->v_type = V_ADDR;
		stack->v_subtype = V_NOSUBTYPE;
		stack->v_addr = v2;
		calculate(fp, 2);
		ret = testvalue(stack);
		freevalue(stack--);
		return ret;
	}
	relvalue(v1, v2, &val);
	if ((val.v_type == V_NUM && qisneg(val.v_num)) ||
		(val.v_type == V_COM && qisneg(val.v_com->imag)))
		r = 1;
	if (val.v_type == V_NULL)
		r = (v1->v_type < v2->v_type);
	freevalue(&val);
	return r;
}


VALUE
signval(int r)
{
	VALUE val;

	val.v_type = V_NUM;
	val.v_subtype = V_NOSUBTYPE;
	if (r > 0)
		val.v_num = qlink(&_qone_);
	else if (r < 0)
		val.v_num = qlink(&_qnegone_);
	else
		val.v_num = qlink(&_qzero_);
	return val;
}


/*
 * Compare two values for their relative values.
 * Result is placed in the indicated location.
 */
void
relvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	int r = 0;
	int i = 0;
	NUMBER *q;
	COMPLEX *c;

	vres->v_subtype = V_NOSUBTYPE;
	vres->v_type = V_NULL;
	if ((v1->v_type == V_OBJ) || (v2->v_type == V_OBJ)) {
		*vres = objcall(OBJ_REL, v1, v2, NULL_VALUE);
		return;
	}
	switch(v1->v_type) {
	case V_NUM:
		switch(v2->v_type) {
		case V_NUM:
			r = qrel(v1->v_num, v2->v_num);
			break;
		case V_OCTET:
			q = itoq((long) *v2->v_octet);
			r = qrel(v1->v_num, q);
			qfree(q);
			break;
		case V_COM:
			r = qrel(v1->v_num, v2->v_com->real);
			i = qrel(&_qzero_, v2->v_com->imag);
			break;
		default:
			return;
		}
		break;
	case V_COM:
		switch(v2->v_type) {
		case V_NUM:
			r = qrel(v1->v_com->real, v2->v_num);
			i = qrel(v1->v_com->imag, &_qzero_);
			break;
		case V_COM:
			r = qrel(v1->v_com->real, v2->v_com->real);
			i = qrel(v1->v_com->imag, v2->v_com->imag);
			break;
		case V_OCTET:
			q = itoq((long) *v2->v_octet);
			r = qrel(v1->v_com->real, q);
			qfree(q);
			i = qrel(v1->v_com->imag, &_qzero_);
			break;
		default:
			return;
		}
		break;
	case V_STR:
		switch(v2->v_type) {
		case V_STR:
			r = stringrel(v1->v_str, v2->v_str);
			break;
		case V_OCTET:
			r = (unsigned char) *v1->v_str->s_str
				- *v2->v_octet;
			if (r == 0) {
				if (v1->v_str->s_len == 0)
					r = -1;
				else
					r = (v1->v_str->s_len > 1);
			}
			break;
		default:
			return;
		}
		break;
	case V_OCTET:
		switch(v2->v_type) {
		case V_NUM:
			q = itoq((long) *v1->v_octet);
			r = qrel(q, v2->v_num);
			qfree(q);
			break;
		case V_COM:
			q = itoq((long) *v1->v_octet);
			r = qrel(q, v2->v_com->real);
			qfree(q);
			i = qrel(&_qzero_, v2->v_com->imag);
			break;
		case V_OCTET:
			r = *v1->v_octet - *v2->v_octet;
			break;
		case V_STR:
			r = *v1->v_octet -
				(unsigned char) *v2->v_str->s_str;
			if (r == 0) {
				if (v2->v_str->s_len == 0)
					r = 1;
				else
					r = -(v2->v_str->s_len > 1);
			}
			break;
		default:
			return;
		}
		break;
	case V_VPTR:
		if (v2->v_type != V_VPTR)
			return;
		r = (v1->v_addr - v2->v_addr);
		break;
	case V_OPTR:
		if (v2->v_type != V_OPTR)
			return;
		r = (v1->v_octet - v2->v_octet);
		break;
	default:
		return;
	}
	vres->v_type = V_NUM;
	*vres = signval(r);
	if (i == 0)
		return;
	c = comalloc();
	qfree(c->real);
	c->real = vres->v_num;
	*vres = signval(i);
	qfree(c->imag);
	c->imag = vres->v_num;
	vres->v_type = V_COM;
	vres->v_com = c;
	return;
}


/*
 * Find a value representing sign or signs in a value
 * Result is placed in the indicated location.
 */
void
sgnvalue(VALUE *vp, VALUE *vres)
{
	COMPLEX *c;

	vres->v_type = vp->v_type;
	switch (vp->v_type) {
	case V_NUM:
		vres->v_num = qsign(vp->v_num);
		vres->v_subtype = vp->v_subtype;
		return;
	case V_COM:
		c = comalloc();
		qfree(c->real);
		qfree(c->imag);
		c->real = qsign(vp->v_com->real);
		c->imag = qsign(vp->v_com->imag);
		vres->v_com = c;
		vres->v_type = V_COM;
		vres->v_subtype = V_NOSUBTYPE;
		return;
	case V_OCTET:
		vres->v_type = V_NUM;
		vres->v_subtype = V_NOSUBTYPE;
		vres->v_num = itoq((long) (*vp->v_octet != 0));
		return;
	case V_OBJ:
		*vres = objcall(OBJ_SGN, vp, NULL_VALUE, NULL_VALUE);
		return;
	default:
		if (vp->v_type > 0)
			*vres = error_value(E_SGN);
		return;
	}
}


int
userfunc(char *fname, VALUE *vp)
{
	FUNC *fp;

	fp = findfunc(adduserfunc(fname));
	if (fp == NULL)
		return 0;
	++stack;
	stack->v_addr = vp;
	stack->v_type = V_ADDR;
	stack->v_subtype = V_NOSUBTYPE;
	calculate(fp, 1);
	freevalue(stack--);
	return 1;
}


/*
 * Print the value of a descriptor in one of several formats.
 * If flags contains PRINT_SHORT, then elements of arrays and lists
 * will not be printed.	 If flags contains PRINT_UNAMBIG, then quotes
 * are placed around strings and the null value is explicitly printed.
 */
void
printvalue(VALUE *vp, int flags)
{
	NUMBER *qtemp;
	int type;

	type = vp->v_type;
	if (type < 0) {
		if (userfunc("error_print", vp))
			return;
		if (-type >= E__BASE)
			math_fmt("Error %d", -type);
		else
			math_fmt("System error %d", -type);
		return;
	}
	switch (type) {
	case V_NUM:
		qprintnum(vp->v_num, MODE_DEFAULT, conf->outdigits);
		if (conf->traceflags & TRACE_LINKS)
			math_fmt("#%ld", vp->v_num->links);
		break;
	case V_COM:
		comprint(vp->v_com);
		if (conf->traceflags & TRACE_LINKS)
			math_fmt("##%ld", vp->v_com->links);
		break;
	case V_STR:
		if (flags & PRINT_UNAMBIG)
			math_chr('\"');
		math_str(vp->v_str->s_str);
		if (flags & PRINT_UNAMBIG)
			math_chr('\"');
		break;
	case V_NULL:
		if (flags & PRINT_UNAMBIG)
			math_str("NULL");
		break;
	case V_OBJ:
		(void) objcall(OBJ_PRINT, vp, NULL_VALUE, NULL_VALUE);
		break;
	case V_LIST:
		if (!userfunc("list_print", vp))
			listprint(vp->v_list,
			((flags & PRINT_SHORT) ? 0L : conf->maxprint));
		break;
	case V_ASSOC:
		assocprint(vp->v_assoc,
			((flags & PRINT_SHORT) ? 0L : conf->maxprint));
		break;
	case V_MAT:
		if (!userfunc("mat_print", vp))
			matprint(vp->v_mat,
			((flags & PRINT_SHORT) ? 0L : conf->maxprint));
		break;
	case V_FILE:
		if (!userfunc("file_print", vp))
			printid(vp->v_file, flags);
		break;
	case V_RAND:
		randprint(vp->v_rand, flags);
		break;
	case V_RANDOM:
		randomprint(vp->v_random, flags);
		break;
	case V_CONFIG:
		config_print(vp->v_config);
		break;
	case V_HASH:
		hash_print(vp->v_hash);
		break;
	case V_BLOCK:
		if (!userfunc("blk_print", vp))
			blk_print(vp->v_block);
		break;
	case V_OCTET:
		if (userfunc("octet_print", vp))
			break;
		qtemp = itoq((long) *vp->v_octet);
		qprintnum(qtemp, MODE_DEFAULT, conf->outdigits);
		qfree(qtemp);
		break;
	case V_OPTR:
		math_fmt("o-ptr: %p", (void *)vp->v_octet);
		break;
	case V_VPTR:
		math_fmt("v-ptr: %p", (void *)vp->v_addr);
		break;
	case V_SPTR:
		math_fmt("s_ptr: %p", (void *)vp->v_str);
		break;
	case V_NPTR:
		math_fmt("n_ptr: %p", (void *)vp->v_num);
		break;
	case V_NBLOCK:
		if (!userfunc("nblk_print", vp))
			nblock_print(vp->v_nblock);
		break;
	default:
		math_error("Printing unrecognized type of value");
		/*NOTREACHED*/
	}
}

/*
 * Print an exact text representation of a value
 */
void
printestr(VALUE *vp)
{
	LISTELEM *ep;
	MATRIX *mp;
	OBJECT *op;
	BLOCK *bp;
	int mode;
	long i, min, max;
	USB8 *cp;

	if (vp->v_type < 0) {
		math_fmt("error(%d)", -vp->v_type);
		return;
	}
	switch(vp->v_type) {
		case V_NULL:
			math_str("\"\"");
			return;
		case V_STR:
			math_chr('"');
			strprint(vp->v_str);
			math_chr('"');
			return;
		case V_NUM:
			qprintnum(vp->v_num, MODE_FRAC, conf->outdigits);
			return;
		case V_COM:
			mode = math_setmode(MODE_FRAC);
			comprint(vp->v_com);
			math_setmode(mode);
			return;
		case V_LIST:
			math_str("list(");
			ep = vp->v_list->l_first;
			if (ep) {
				printestr(&ep->e_value);
				while ((ep = ep->e_next)) {
					math_chr(',');
					printestr(&ep->e_value);
				}
			}
			math_chr(')');
			return;
		case V_MAT:
			mp = vp->v_mat;
			if (mp->m_dim == 0)
				math_str("(mat[])");
			else {
				math_str("mat[");
				for (i = 0; i < mp->m_dim; i++) {
					min = mp->m_min[i];
					max = mp->m_max[i];
					if (i > 0)
						math_chr(',');
					if (min)
						math_fmt("%ld:%ld", min, max);
					else
						math_fmt("%ld", max + 1);
				}
				math_chr(']');
			}
			i = mp->m_size;
			vp = mp->m_table;
			break;
		case V_OBJ:
			op = vp->v_obj;
			math_fmt("obj %s",objtypename(op->o_actions->oa_index));
			i = op->o_actions->oa_count;
			vp = op->o_table;
			break;
		case V_BLOCK:
		case V_NBLOCK:
			math_str("blk(");
			if (vp->v_type == V_BLOCK)
				bp = vp->v_block;
			else {
				math_fmt("\"%s\",", vp->v_nblock->name);
				bp = vp->v_nblock->blk;
			}
			i = bp->datalen;
			math_fmt("%ld,%d)", i, (int) bp->blkchunk);
			cp = bp->data;
			if (i > 0) {
				math_str("={");
				math_fmt("%d", *cp);
				while (--i > 0) {
					math_chr(',');
					math_fmt("%d", *++cp);
				}
				math_chr('}');
			}
			return;

		default:
			math_str("\"???\"");
			return;
	}
	if (i > 0) {
		math_str("={");
		printestr(vp);
		while (--i > 0) {
			math_chr(',');
			printestr(++vp);
		}
		math_chr('}');
	}
}


/*
 * config_print - print a configuration value
 *
 * given:
 *	cfg		what to print
 */
void
config_print(CONFIG *cfg)
{
	NAMETYPE *cp;
	VALUE tmp;
	int tab_over;		/* TRUE => ok move over one tab stop */
	size_t len;

	/*
	 * firewall
	 */
	if (cfg == NULL || cfg->epsilon == NULL || cfg->prompt1 == NULL ||
	    cfg->prompt2 == NULL) {
		math_error("CONFIG value is invalid");
		/*NOTREACHED*/
	}

	/*
	 * print each element
	 */
	tab_over = FALSE;
	for (cp = configs; cp->name; cp++) {

		/* skip if special all or duplicate maxerr value */
		if (cp->type == CONFIG_ALL || strcmp(cp->name, "maxerr") == 0 ||
		    strcmp(cp->name, "ctrl-d") == 0)
			continue;

		/* print tab if allowed */
		if (tab_over) {
			math_str("\t");
		} else if (conf->tab_ok) {
			tab_over = TRUE;	/* tab next time */
		}

		/* print name and spaces */
		math_fmt("%s", cp->name);
		len = 16 - strlen(cp->name);
		while (len-- > 0)
			math_str(" ");

		/* print value */
		config_value(cfg, cp->type, &tmp);
		printvalue(&tmp, PRINT_SHORT | PRINT_UNAMBIG);
		freevalue(&tmp);
		if ((cp+1)->name)
			math_str("\n");
	}
}
