/*
 * Copyright (c) 1996 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Generic value manipulation routines.
 */

#include "value.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"
#include "string.h"
#include "zrand.h"
#include "cmath.h"


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
	if (type < 0)
		return;
	switch (type) {
		case V_NULL:
		case V_ADDR:
		case V_FILE:
			break;
		case V_STR:
			if (vp->v_subtype == V_STRALLOC)
				free(vp->v_str);
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
#if 0 /* XXX - write */
		case V_HASH:
			hash_free(vp->v_hash);
			break;
#endif
		default:
			math_error("Freeing unknown value type");
			/*NOTREACHED*/
	}
	vp->v_subtype = V_NOSUBTYPE;
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
	if (oldvp->v_type < 0) {
		newvp->v_type = oldvp->v_type;
		return;
	}
	newvp->v_type = V_NULL;
	switch (oldvp->v_type) {
		case V_NULL:
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
			newvp->v_str = oldvp->v_str;
			if (oldvp->v_subtype == V_STRALLOC) {
				newvp->v_str = (char *)malloc(strlen(oldvp->v_str) + 1);
				if (newvp->v_str == NULL) {
					math_error("Cannot get memory for string copy");
					/*NOTREACHED*/
				}
				strcpy(newvp->v_str, oldvp->v_str);
			}
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
		case V_ADDR:
			newvp->v_addr = oldvp->v_addr;
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
#if 0 /* XXX - write */
		case V_HASH:
			newvp->v_hash = hash_copy(oldvp->v_hash);
			break;
#endif
		default:
			math_error("Copying unknown value type");
			/*NOTREACHED*/
	}
	if (oldvp->v_type == V_STR) {
		newvp->v_subtype = oldvp->v_subtype;
	} else {
		newvp->v_subtype = V_NOSUBTYPE;
	}
	newvp->v_type = oldvp->v_type;

}


/*
 * Negate an arbitrary value.
 * Result is placed in the indicated location.
 */
void
negvalue(VALUE *vp, VALUE *vres)
{
	vres->v_type = vp->v_type;
	switch (vp->v_type) {
		case V_NUM:
			vres->v_num = qneg(vp->v_num);
			return;
		case V_COM:
			vres->v_com = cneg(vp->v_com);
			return;
		case V_MAT:
			vres->v_mat = matneg(vp->v_mat);
			return;
		case V_OBJ:
			*vres = objcall(OBJ_NEG, vp, NULL_VALUE, NULL_VALUE);
			return;
		default:
			if (vp->v_type < 0)
				return;
			*vres = error_value(E_NEG);
			return;
	}
}


/*
 * addnumeric - add two numeric values togethter
 *
 * If either value is not real or complex, it is assumed to have
 * a value of 0.
 *
 * Result is placed in the indicated location.
 */
void
addnumeric(VALUE *v1, VALUE *v2, VALUE *vres)
{
	COMPLEX *c;

	/*
	 * add numeric values
	 */
	vres->v_subtype = V_NOSUBTYPE;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
	case TWOVAL(V_NUM, V_NUM):
		vres->v_num = qqadd(v1->v_num, v2->v_num);
		vres->v_type = V_NUM;
		return;
	case TWOVAL(V_COM, V_NUM):
		vres->v_com = caddq(v1->v_com, v2->v_num);
		vres->v_type = V_COM;
		return;
	case TWOVAL(V_NUM, V_COM):
		vres->v_com = caddq(v2->v_com, v1->v_num);
		vres->v_type = V_COM;
		return;
	case TWOVAL(V_COM, V_COM):
		vres->v_com = cadd(v1->v_com, v2->v_com);
		vres->v_type = V_COM;
		c = vres->v_com;
		if (!cisreal(c))
			return;
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	}

	/*
	 * assume zero if a value is not numeric
	 */
	if (v1->v_type == V_NUM) {
		/* v1 + 0 == v1 */
		vres->v_type = v1->v_type;
		vres->v_num = qlink(v1->v_num);
	} else if (v1->v_type == V_COM) {
		/* v1 + 0 == v1 */
		vres->v_type = v1->v_type;
		vres->v_com = clink(v1->v_com);
	} else if (v2->v_type == V_NUM) {
		/* v2 + 0 == v2 */
		vres->v_type = v2->v_type;
		vres->v_num = qlink(v2->v_num);
	} else if (v2->v_type == V_COM) {
		/* v2 + 0 == v2 */
		vres->v_type = v2->v_type;
		vres->v_com = clink(v2->v_com);
	} else {
		/* 0 + 0 = 0 */
		vres->v_type = V_NUM;
		vres->v_num = qlink(&_qzero_);
	}
	return;
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
			vres->v_com = caddq(v1->v_com, v2->v_num);
			return;
		case TWOVAL(V_NUM, V_COM):
			vres->v_com = caddq(v2->v_com, v1->v_num);
			vres->v_type = V_COM;
			return;
		case TWOVAL(V_COM, V_COM):
			vres->v_com = cadd(v1->v_com, v2->v_com);
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
		default:
			if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
				if (v1->v_type < 0) {
					copyvalue(v1, vres);
					return;
				}
				if (v2->v_type < 0) {
					copyvalue(v2, vres);
					return;
				}
				*vres = error_value(E_ADD);
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

	vres->v_type = v1->v_type;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
		case TWOVAL(V_NUM, V_NUM):
			vres->v_num = qsub(v1->v_num, v2->v_num);
			return;
		case TWOVAL(V_COM, V_NUM):
			vres->v_com = csubq(v1->v_com, v2->v_num);
			return;
		case TWOVAL(V_NUM, V_COM):
			c = csubq(v2->v_com, v1->v_num);
			vres->v_type = V_COM;
			vres->v_com = cneg(c);
			comfree(c);
			return;
		case TWOVAL(V_COM, V_COM):
			vres->v_com = csub(v1->v_com, v2->v_com);
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
		default:
			if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
				if (v1->v_type < 0) {
					copyvalue(v1, vres);
					return;
				}
				if (v2->v_type < 0) {
					copyvalue(v2, vres);
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
	switch (TWOVAL(v1->v_type, v2->v_type)) {
		case TWOVAL(V_NUM, V_NUM):
			vres->v_num = qmul(v1->v_num, v2->v_num);
			return;
		case TWOVAL(V_COM, V_NUM):
			vres->v_com = cmulq(v1->v_com, v2->v_num);
			break;
		case TWOVAL(V_NUM, V_COM):
			vres->v_com = cmulq(v2->v_com, v1->v_num);
			vres->v_type = V_COM;
			break;
		case TWOVAL(V_COM, V_COM):
			vres->v_com = cmul(v1->v_com, v2->v_com);
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
		default:
			if ((v1->v_type != V_OBJ) && (v2->v_type != V_OBJ)) {
				if (v1->v_type < 0) {
					copyvalue(v1, vres);
					return;
				}
				if (v2->v_type < 0) {
					copyvalue(v2, vres);
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
	switch (vp->v_type) {
		case V_NUM:
			vres->v_num = qsquare(vp->v_num);
			return;
		case V_COM:
			vres->v_com = csquare(vp->v_com);
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
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
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
	vres->v_type = vp->v_type;
	switch (vp->v_type) {
		case V_NUM:
			vres->v_num = qinv(vp->v_num);
			return;
		case V_COM:
			vres->v_com = cinv(vp->v_com);
			return;
		case V_MAT:
			vres->v_mat = matinv(vp->v_mat);
			return;
		case V_OBJ:
			*vres = objcall(OBJ_INV, vp, NULL_VALUE, NULL_VALUE);
			return;
		default:
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
			*vres = error_value(E_INV);
			return;
	}
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
	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
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
			c->real = q1;
			c->imag = q2;
			vres->v_com = c;
			return;
		default:
			if (v1->v_type < 0) {
				copyvalue(v1, vres);
				return;
			}
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
			c->real = q1;
			c->imag = q2;
			vres->v_com = c;
			return;
		default:
			if (v1->v_type < 0) {
				copyvalue(v1, vres);
				return;
			}
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
			vres->v_com = cint(vp->v_com);
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
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
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
			vres->v_com = cfrac(vp->v_com);
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
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
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
			return;
		case V_COM:
			vres->v_com = caddq(vp->v_com, &_qone_);
			return;
		case V_OBJ:
			*vres = objcall(OBJ_INC, vp, NULL_VALUE, NULL_VALUE);
			return;
		default:
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
			*vres = error_value(E_INCV);
			return;
	}
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
			return;
		case V_COM:
			vres->v_com = caddq(vp->v_com, &_qnegone_);
			return;
		case V_OBJ:
			*vres = objcall(OBJ_DEC, vp, NULL_VALUE, NULL_VALUE);
			return;
		default:
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
			*vres = error_value(E_DECV);
			return;
	}
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
	switch (vp->v_type) {
		case V_NUM:
			vres->v_num = qlink(vp->v_num);
			return;
		case V_COM:
			vres->v_com = comalloc();
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
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
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
	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
	if (v2->v_type == V_NULL)
		q = conf->epsilon;
	else {
		if (v2->v_type != V_NUM || qiszero(v2->v_num)) {
			*vres = error_value(E_SQRT2);
			return;
		}
		q = v2->v_num;
	}
	if (v3->v_type == V_NULL)
		R = conf->sqrt;
	else {
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
			c->imag = qsqrt(tmp, q, R);
			qfree(tmp);
			vres->v_com = c;
			vres->v_type = V_COM;
			break;
		case V_COM:
			vres->v_com = csqrt(v1->v_com, q, R);
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
	NUMBER *q1, *q2;
	COMPLEX ctmp;
	COMPLEX *c;

	vres->v_type = v1->v_type;
	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
	if (v2->v_type != V_NUM) {
		*vres = error_value(E_ROOT2);
		return;
	}
	q1 = v2->v_num;
	if (qisneg(q1) || qiszero(q1) || qisfrac(q1)) {
		*vres = error_value(E_ROOT2);
		return;
	}
	if (v3->v_type != V_NUM || qiszero(v3->v_num)) {
		*vres = error_value(E_ROOT3);
		return;
	}
	q2 = v3->v_num;
	switch (v1->v_type) {
		case V_NUM:
			if (!qisneg(v1->v_num) || zisodd(q1->num)) {
				vres->v_num = qroot(v1->v_num, q1, q2);
				return;
			}
			ctmp.real = v1->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			vres->v_com = croot(&ctmp, q1, q2);
			vres->v_type = V_COM;
			break;
		case V_COM:
			vres->v_com = croot(v1->v_com, q1, q2);
			break;
		case V_OBJ:
			*vres = objcall(OBJ_ROOT, v1, v2, v3);
			return;
		default:
			*vres = error_value(E_ROOT);
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
 * Take the absolute value of an arbitrary value within the specified error.
 * Result is placed in the indicated location.
 */
void
absvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	static NUMBER *q;

	if (v1->v_type == V_OBJ || v2->v_type == V_OBJ) {
		*vres = objcall(OBJ_ABS, v1, v2, NULL_VALUE);
		return;
	}
	if (v1->v_type < 0) {
		copyvalue(v1, vres);
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
	if (vp->v_type < 0) {
		copyvalue(vp, vres);
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
 *	v2		shirt amount
 *	rightshift	TRUE if shift right instead of left
 *	vres		result
 */
void
shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres)
{
	COMPLEX *c;
	long n = 0;
	VALUE tmp;

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
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
			c = cshift(v1->v_com, n);
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

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
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
			vres->v_com = cscale(v1->v_com, n);
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
 * Raise a value to an integral power.
 * Result is placed in the indicated location.
 */
void
powivalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	NUMBER *q;
	COMPLEX *c;

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
	if (v2->v_type < 0) {
		copyvalue(v2, vres);
		return;
	}
	if (v2->v_type != V_NUM || qisfrac(v2->v_num)) {
		*vres = error_value(E_POWI2);
		return;
	}
	q = v2->v_num;
	vres->v_type = v1->v_type;
	switch (v1->v_type) {
		case V_NUM:
			vres->v_num = qpowi(v1->v_num, q);
			return;
		case V_COM:
			vres->v_com = cpowi(v1->v_com, q);
			c = vres->v_com;
			if (!cisreal(c))
				return;
			vres->v_num = qlink(c->real);
			vres->v_type = V_NUM;
			comfree(c);
			return;
		case V_MAT:
			vres->v_mat = matpowi(v1->v_mat, q);
			return;
		case V_OBJ:
			*vres = objcall(OBJ_POW, v1, v2, NULL_VALUE);
			return;
		default:
			*vres = error_value(E_POWI);
			return;
	}
}


/*
 * Raise one value to another value's power, within the specified error.
 * Result is placed in the indicated location.
 */
void
powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres)
{
	NUMBER *epsilon;
	COMPLEX *c, ctmp;

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
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

	if (v3->v_type != V_NUM || qiszero(v3->v_num)) {
		*vres = error_value(E_POWER3);
		return;
	}
	epsilon = v3->v_num;
	vres->v_type = v1->v_type;
	switch (TWOVAL(v1->v_type, v2->v_type)) {
		case TWOVAL(V_NUM, V_NUM):
			vres->v_num = qpower(v1->v_num, v2->v_num, epsilon);
			return;
		case TWOVAL(V_NUM, V_COM):
			ctmp.real = v1->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			vres->v_com = cpower(&ctmp, v2->v_com, epsilon);
			break;
		case TWOVAL(V_COM, V_NUM):
			ctmp.real = v2->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			vres->v_com = cpower(v1->v_com, &ctmp, epsilon);
			break;
		case TWOVAL(V_COM, V_COM):
			vres->v_com = cpower(v1->v_com, v2->v_com, epsilon);
			break;
		default:
			*vres = error_value(E_POWER);
			return;
	}
	/*
	 * Here for any complex result.
	 */
	vres->v_type = V_COM;
	c = vres->v_com;
	if (!cisreal(c))
		return;
	vres->v_num = qlink(c->real);
	vres->v_type = V_NUM;
	comfree(c);
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
	VALUE tmpval;

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
	if (v2->v_type < 0) {
		copyvalue(v2, vres);
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
			vres->v_num = qdiv(v1->v_num, v2->v_num);
			return;
		case TWOVAL(V_COM, V_NUM):
			vres->v_com = cdivq(v1->v_com, v2->v_num);
			return;
		case TWOVAL(V_NUM, V_COM):
			if (qiszero(v1->v_num)) {
				vres->v_num = qlink(&_qzero_);
				return;
			}
			ctmp.real = v1->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			vres->v_com = cdiv(&ctmp, v2->v_com);
			vres->v_type = V_COM;
			return;
		case TWOVAL(V_COM, V_COM):
			vres->v_com = cdiv(v1->v_com, v2->v_com);
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
	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
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
	if (v2->v_type < 0) {
		copyvalue(v2, vres);
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

	if (v1->v_type < 0) {
		copyvalue(v1, vres);
		return;
	}
	vres->v_type = v1->v_type;
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
	if (v2->v_type < 0) {
		copyvalue(v2, vres);
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

	switch (vp->v_type) {
		case V_NUM:
			return !qiszero(vp->v_num);
		case V_COM:
			return !ciszero(vp->v_com);
		case V_STR:
			return (vp->v_str[0] != '\0');
		case V_MAT:
			return mattest(vp->v_mat);
		case V_LIST:
			return (vp->v_list->l_count != 0);
		case V_ASSOC:
			return (vp->v_assoc->a_count != 0);
		case V_FILE:
			return validid(vp->v_file);
		case V_NULL:
			break;	/* hack to get gcc on SunOS to be quiet */
		case V_OBJ:
			val = objcall(OBJ_TEST, vp, NULL_VALUE, NULL_VALUE);
			return (val.v_int != 0);
		default:
			math_error("Testing improper type");
			/*NOTREACHED*/
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
	if (v1->v_type != v2->v_type)
		return TRUE;
	if (v1->v_type < 0)
		return FALSE;
	switch (v1->v_type) {
		case V_NUM:
			r = qcmp(v1->v_num, v2->v_num);
			break;
		case V_COM:
			r = ccmp(v1->v_com, v2->v_com);
			break;
		case V_STR:
			r = ((v1->v_str != v2->v_str) &&
				((v1->v_str[0] - v2->v_str[0]) ||
				strcmp(v1->v_str, v2->v_str)));
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
		case V_NULL:
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
#if 0 /* XXX - write */
		case V_HASH:
			r = hash_cmp(v1->v_hash, v2->v_hash);
			break;
#endif
		default:
			math_error("Illegal values for comparevalue");
			/*NOTREACHED*/
	}
	return (r != 0);
}


BOOL
precvalue(VALUE *v1, VALUE *v2)
{
	VALUE val;
	long index;
	int r = 0;
	FUNC *fp;

	index = adduserfunc("precedes");
	fp = findfunc(index);
	if (fp) {
		++stack;
		stack->v_type = V_ADDR;
		stack->v_addr = v1;
		++stack;
		stack->v_type = V_ADDR;
		stack->v_addr = v2;
		calculate(fp, 2);
		val = *stack--;
		if (val.v_type != V_NUM) {
			math_error("Non-numeric value for precvalue()");
			/*NOTREACHED*/
		}
		return (qtoi(val.v_num) ? TRUE : FALSE);
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


/*
 * Compare two values for their relative values.
 * Result is placed in the indicated location.
 */
void
relvalue(VALUE *v1, VALUE *v2, VALUE *vres)
{
	int r = 0;
	COMPLEX ctmp, *c;

	if ((v1->v_type == V_OBJ) || (v2->v_type == V_OBJ)) {
		*vres = objcall(OBJ_REL, v1, v2, NULL_VALUE);
		return;
	}
	switch (TWOVAL(v1->v_type, v2->v_type)) {
		case TWOVAL(V_NUM, V_NUM):
			r = qrel(v1->v_num, v2->v_num);
			vres->v_type = V_NUM;
			vres->v_num = itoq((long) r);
			return;
		case TWOVAL(V_STR, V_STR):
			r = strcmp(v1->v_str, v2->v_str);
			vres->v_type = V_NUM;
			if (r < 0) {
				vres->v_num = itoq((long) -1);
			} else if (r > 0) {
				vres->v_num = itoq((long) 1);
			} else {
				vres->v_num = itoq((long) 0);
			}
			return;
		case TWOVAL(V_COM, V_COM):
			c = crel(v1->v_com, v2->v_com);
			break;
		case TWOVAL(V_COM, V_NUM):
			ctmp.real = v2->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			c = crel(v1->v_com, &ctmp);
			break;
		case TWOVAL(V_NUM, V_COM):
			ctmp.real = v1->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			c = crel(&ctmp, v2->v_com);
			break;
		default:
			vres->v_type = V_NULL;
			return;
	}
	if (cisreal(c)) {
		vres->v_num = qlink(c->real);
		vres->v_type = V_NUM;
		comfree(c);
		return;
	}
	vres->v_com = c;
	vres->v_type = V_COM;
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
			return;
		case V_COM:
			c = comalloc();
			c->real = qsign(vp->v_com->real);
			c->imag = qsign(vp->v_com->imag);
			vres->v_com = c;
			vres->v_type = V_COM;
			return;
		case V_OBJ:
			*vres = objcall(OBJ_SGN, vp, NULL_VALUE, NULL_VALUE);
			return;
		default:
			if (vp->v_type < 0) {
				copyvalue(vp, vres);
				return;
			}
			*vres = error_value(E_SGN);
			return;
	}
}


/*
 * Print the value of a descriptor in one of several formats.
 * If flags contains PRINT_SHORT, then elements of arrays and lists
 * will not be printed.  If flags contains PRINT_UNAMBIG, then quotes
 * are placed around strings and the null value is explicitly printed.
 */
void
printvalue(VALUE *vp, int flags)
{
	int type;

	type = vp->v_type;
	if (type < 0) {
		if (-type > E__BASE)
			printf("Error %d", -type);
		else
			printf("System error %d", -type);
		return;
	}
	switch (type) {
		case V_NUM:
			qprintnum(vp->v_num, MODE_DEFAULT);
			if (conf->traceflags & TRACE_LINKS)
				printf("#%ld", vp->v_num->links);
			break;
		case V_COM:
			comprint(vp->v_com);
			if (conf->traceflags & TRACE_LINKS)
				printf("##%ld", vp->v_com->links);
			break;
		case V_STR:
			if (flags & PRINT_UNAMBIG)
				math_chr('\"');
			math_str(vp->v_str);
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
			listprint(vp->v_list,
				((flags & PRINT_SHORT) ? 0L : conf->maxprint));
			break;
		case V_ASSOC:
			assocprint(vp->v_assoc,
				((flags & PRINT_SHORT) ? 0L : conf->maxprint));
			break;
		case V_MAT:
			matprint(vp->v_mat,
				((flags & PRINT_SHORT) ? 0L : conf->maxprint));
			break;
		case V_FILE:
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
#if 0 /* XXX - write */
		case V_HASH:
			hash_print(vp->v_hash);
			break;
#endif
		default:
			math_error("Printing unknown value");
			/*NOTREACHED*/
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
	int i;

	/*
	 * firewall
	 */
	if (cfg == NULL || cfg->epsilon == NULL || cfg->prompt1 == NULL ||
	    cfg->prompt2 == NULL) {
		math_error("CONFIG value is invaid");
		/*NOTREACHED*/
	}

	/*
	 * print each element
	 */
	tab_over = FALSE;
	for (cp = configs; cp->name; cp++) {

		/* skip if special all value */
		if (cp->type == CONFIG_ALL)
			continue;

		/* print tab if allowed */
		if (tab_over) {
			printf("\t");
		} else if (conf->tab_ok) {
			tab_over = TRUE;	/* tab next time */
		}

		/* print name and spaces */
		printf("%s", cp->name);
		i = 16 - (int)strlen(cp->name);
		while (i-- > 0)
			printf(" ");

		/* print value */
		config_value(cfg, cp->type, &tmp);
		printvalue(&tmp, PRINT_SHORT | PRINT_UNAMBIG);
		freevalue(&tmp);
		if ((cp+1)->name)
			printf("\n");
	}
}

/* END CODE */
