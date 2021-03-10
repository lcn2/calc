/*
 * matfunc - extended precision rational arithmetic matrix functions
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
 * Extended precision rational arithmetic matrix functions.
 * Matrices can contain arbitrary types of elements.
 */

#include "alloc.h"
#include "value.h"
#include "zrand.h"
#include "calcerr.h"

#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


E_FUNC long irand(long s);

S_FUNC void matswaprow(MATRIX *m, long r1, long r2);
S_FUNC void matsubrow(MATRIX *m, long oprow, long baserow, VALUE *mulval);
S_FUNC void matmulrow(MATRIX *m, long row, VALUE *mulval);
S_FUNC MATRIX *matident(MATRIX *m);



/*
 * Add two compatible matrices.
 */
MATRIX *
matadd(MATRIX *m1, MATRIX *m2)
{
	int dim;

	long min1, min2, max1, max2, index;
	VALUE *v1, *v2, *vres;
	MATRIX *res;
	MATRIX tmp;

	if (m1->m_dim != m2->m_dim) {
		math_error("Incompatible matrix dimensions for add");
		/*NOTREACHED*/
	}
	tmp.m_dim = m1->m_dim;
	tmp.m_size = m1->m_size;
	for (dim = 0; dim < m1->m_dim; dim++) {
		min1 = m1->m_min[dim];
		max1 = m1->m_max[dim];
		min2 = m2->m_min[dim];
		max2 = m2->m_max[dim];
		if ((min1 && min2 && (min1 != min2)) ||
		    ((max1-min1) != (max2-min2))) {
			math_error("Incompatible matrix bounds for add");
			/*NOTREACHED*/
		}
		tmp.m_min[dim] = (min1 ? min1 : min2);
		tmp.m_max[dim] = tmp.m_min[dim] + (max1 - min1);
	}
	res = matalloc(m1->m_size);
	*res = tmp;
	v1 = m1->m_table;
	v2 = m2->m_table;
	vres = res->m_table;
	for (index = m1->m_size; index > 0; index--)
		addvalue(v1++, v2++, vres++);
	return res;
}


/*
 * Subtract two compatible matrices.
 */
MATRIX *
matsub(MATRIX *m1, MATRIX *m2)
{
	int dim;
	long min1, min2, max1, max2, index;
	VALUE *v1, *v2, *vres;
	MATRIX *res;
	MATRIX tmp;

	if (m1->m_dim != m2->m_dim) {
		math_error("Incompatible matrix dimensions for sub");
		/*NOTREACHED*/
	}
	tmp.m_dim = m1->m_dim;
	tmp.m_size = m1->m_size;
	for (dim = 0; dim < m1->m_dim; dim++) {
		min1 = m1->m_min[dim];
		max1 = m1->m_max[dim];
		min2 = m2->m_min[dim];
		max2 = m2->m_max[dim];
		if ((min1 && min2 && (min1 != min2)) ||
		    ((max1-min1) != (max2-min2))) {
			math_error("Incompatible matrix bounds for sub");
			/*NOTREACHED*/
		}
		tmp.m_min[dim] = (min1 ? min1 : min2);
		tmp.m_max[dim] = tmp.m_min[dim] + (max1 - min1);
	}
	res = matalloc(m1->m_size);
	*res = tmp;
	v1 = m1->m_table;
	v2 = m2->m_table;
	vres = res->m_table;
	for (index = m1->m_size; index > 0; index--)
		subvalue(v1++, v2++, vres++);
	return res;
}


/*
 * Produce the negative of a matrix.
 */
MATRIX *
matneg(MATRIX *m)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		negvalue(val++, vres++);
	return res;
}


/*
 * Multiply two compatible matrices.
 */
MATRIX *
matmul(MATRIX *m1, MATRIX *m2)
{
	register MATRIX *res;
	long i1, i2, max1, max2, index, maxindex;
	VALUE *v1, *v2, *vres;
	VALUE sum, tmp1, tmp2;

	if (m1->m_dim == 0) {
		i2 = m2->m_size;
		v2 = m2->m_table;
		res = matalloc(i2);
		*res = *m2;
		vres = res->m_table;
		while (i2-- > 0)
			mulvalue(m1->m_table, v2++, vres++);
		return res;
	}
	if (m2->m_dim == 0) {
		i1 = m1->m_size;
		v1 = m1->m_table;
		res = matalloc(i1);
		*res = *m1;
		vres = res->m_table;
		while (i1-- > 0)
			mulvalue(v1++, m2->m_table, vres++);
		return res;
	}
	if (m1->m_dim == 1 && m2->m_dim == 1) {
		if (m1->m_max[0]-m1->m_min[0] != m2->m_max[0]-m2->m_min[0]) {
			math_error("Incompatible bounds for 1D * 1D  matmul");
			/*NOTREACHED*/
		}
		res = matalloc(m1->m_size);
		*res = *m1;
		v1 = m1->m_table;
		v2 = m2->m_table;
		vres = res->m_table;
		for (index = m1->m_size; index > 0; index--)
			mulvalue(v1++, v2++, vres++);
		return res;
	}
	if (m1->m_dim == 1 && m2->m_dim == 2) {
		if (m1->m_max[0]-m1->m_min[0] != m2->m_max[0]-m2->m_min[0]) {
			math_error("Incompatible bounds for 1D * 2D matmul");
			/*NOTREACHED*/
		}
		res = matalloc(m2->m_size);
		*res = *m2;
		i1 = m1->m_max[0] - m1->m_min[0] + 1;
		max2 = m2->m_max[1] - m2->m_min[1] + 1;
		v1 = m1->m_table;
		v2 = m2->m_table;
		vres = res->m_table;
		while (i1-- > 0) {
			i2 = max2;
			while (i2-- > 0)
				mulvalue(v1, v2++, vres++);
			v1++;
		}
		return res;
	}
	if (m1->m_dim == 2 && m2->m_dim == 1) {
		if (m1->m_max[1]-m1->m_min[1] != m2->m_max[0]-m2->m_min[0]) {
			math_error("Incompatible bounds for 2D * 1D matmul");
			/*NOTREACHED*/
		}
		res = matalloc(m1->m_size);
		*res = *m1;
		i1 = m1->m_max[0] - m1->m_min[0] + 1;
		max1 = m1->m_max[1] - m1->m_min[1] + 1;
		v1 = m1->m_table;
		vres = res->m_table;
		while (i1-- > 0) {
			v2 = m2->m_table;
			i2 = max1;
			while (i2-- > 0)
				mulvalue(v1++, v2++, vres++);
		}
		return res;
	}

	if ((m1->m_dim != 2) || (m2->m_dim != 2)) {
		math_error("Matrix dimensions not compatible for mul");
		/*NOTREACHED*/
	}
	if ((m1->m_max[1]-m1->m_min[1]) != (m2->m_max[0]-m2->m_min[0])) {
		math_error("Incompatible bounds for 2D * 2D matrix mul");
		/*NOTREACHED*/
	}
	max1 = (m1->m_max[0] - m1->m_min[0] + 1);
	max2 = (m2->m_max[1] - m2->m_min[1] + 1);
	maxindex = (m1->m_max[1] - m1->m_min[1] + 1);
	res = matalloc(max1 * max2);
	res->m_dim = 2;
	res->m_min[0] = m1->m_min[0];
	res->m_max[0] = m1->m_max[0];
	res->m_min[1] = m2->m_min[1];
	res->m_max[1] = m2->m_max[1];
	for (i1 = 0; i1 < max1; i1++) {
		for (i2 = 0; i2 < max2; i2++) {
			sum.v_type = V_NULL;
			sum.v_subtype = V_NOSUBTYPE;
			v1 = &m1->m_table[i1 * maxindex];
			v2 = &m2->m_table[i2];
			for (index = 0; index < maxindex; index++) {
				mulvalue(v1, v2, &tmp1);
				addvalue(&sum, &tmp1, &tmp2);
				freevalue(&tmp1);
				freevalue(&sum);
				sum = tmp2;
				v1++;
				if (index+1 < maxindex)
					v2 += max2;
			}
			index = (i1 * max2) + i2;
			res->m_table[index] = sum;
		}
	}
	return res;
}


/*
 * Square a matrix.
 */
MATRIX *
matsquare(MATRIX *m)
{
	register MATRIX *res;
	long i1, i2, max, index;
	VALUE *v1, *v2;
	VALUE sum, tmp1, tmp2;

	if (m->m_dim < 2) {
		res = matalloc(m->m_size);
		*res = *m;
		v1 = m->m_table;
		v2 = res->m_table;
		for (index = m->m_size; index > 0; index--)
			squarevalue(v1++, v2++);
		return res;
	}
	if (m->m_dim != 2) {
		math_error("Matrix dimension exceeds two for square");
		/*NOTREACHED*/
	}
	if ((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1])) {
		math_error("Squaring non-square matrix");
		/*NOTREACHED*/
	}
	max = (m->m_max[0] - m->m_min[0] + 1);
	res = matalloc(max * max);
	res->m_dim = 2;
	res->m_min[0] = m->m_min[0];
	res->m_max[0] = m->m_max[0];
	res->m_min[1] = m->m_min[1];
	res->m_max[1] = m->m_max[1];
	for (i1 = 0; i1 < max; i1++) {
		for (i2 = 0; i2 < max; i2++) {
			sum.v_type = V_NULL;
			sum.v_subtype = V_NOSUBTYPE;
			v1 = &m->m_table[i1 * max];
			v2 = &m->m_table[i2];
			for (index = 0; index < max; index++) {
				mulvalue(v1, v2, &tmp1);
				addvalue(&sum, &tmp1, &tmp2);
				freevalue(&tmp1);
				freevalue(&sum);
				sum = tmp2;
				v1++;
				v2 += max;
			}
			index = (i1 * max) + i2;
			res->m_table[index] = sum;
		}
	}
	return res;
}


/*
 * Compute the result of raising a matrix to an integer power if
 * dimension <= 2 and for dimension == 2, the matrix is square.
 * Negative powers mean the positive power of the inverse.
 * Note: This calculation could someday be improved for large powers
 * by using the characteristic polynomial of the matrix.
 *
 * given:
 *	m		matrix to be raised
 *	q		power to raise it to
 */
MATRIX *
matpowi(MATRIX *m, NUMBER *q)
{
	MATRIX *res, *tmp;
	long power;		/* power to raise to */
	FULL bit;		/* current bit value */

	if (m->m_dim > 2) {
		math_error("Matrix dimension greater than 2 for power");
		/*NOTREACHED*/
	}
	if (m->m_dim == 2 && (m->m_max[0] - m->m_min[0] !=
			 m->m_max[1] - m->m_min[1])) {
		math_error("Raising non-square 2D matrix to a power");
		/*NOTREACHED*/
	}
	if (qisfrac(q)) {
		math_error("Raising matrix to non-integral power");
		/*NOTREACHED*/
	}
	if (zge31b(q->num)) {
		math_error("Raising matrix to very large power");
		/*NOTREACHED*/
	}
	power = ztolong(q->num);
	if (qisneg(q))
		power = -power;
	/*
	 * Handle some low powers specially
	 */
	if ((power <= 4) && (power >= -2)) {
		switch ((int) power) {
		case 0:
			return matident(m);
		case 1:
			return matcopy(m);
		case -1:
			return matinv(m);
		case 2:
			return matsquare(m);
		case -2:
			tmp = matinv(m);
			res = matsquare(tmp);
			matfree(tmp);
			return res;
		case 3:
			tmp = matsquare(m);
			res = matmul(m, tmp);
			matfree(tmp);
			return res;
		case 4:
			tmp = matsquare(m);
			res = matsquare(tmp);
			matfree(tmp);
			return res;
		}
	}
	if (power < 0) {
		m = matinv(m);
		power = -power;
	}
	/*
	 * Compute the power by squaring and multiplying.
	 * This uses the left to right method of power raising.
	 */
	bit = TOPFULL;
	while ((bit & power) == 0)
		bit >>= 1L;
	bit >>= 1L;
	res = matsquare(m);
	if (bit & power) {
		tmp = matmul(res, m);
		matfree(res);
		res = tmp;
	}
	bit >>= 1L;
	while (bit) {
		tmp = matsquare(res);
		matfree(res);
		res = tmp;
		if (bit & power) {
			tmp = matmul(res, m);
			matfree(res);
			res = tmp;
		}
		bit >>= 1L;
	}
	if (qisneg(q))
		matfree(m);
	return res;
}


/*
 * Calculate the cross product of two one dimensional matrices each
 * with three components.
 *	m3 = matcross(m1, m2);
 */
MATRIX *
matcross(MATRIX *m1, MATRIX *m2)
{
	MATRIX *res;
	VALUE *v1, *v2, *vr;
	VALUE tmp1, tmp2;

	res = matalloc(3L);
	res->m_dim = 1;
	res->m_min[0] = 0;
	res->m_max[0] = 2;
	v1 = m1->m_table;
	v2 = m2->m_table;
	vr = res->m_table;
	mulvalue(v1 + 1, v2 + 2, &tmp1);
	mulvalue(v1 + 2, v2 + 1, &tmp2);
	subvalue(&tmp1, &tmp2, vr + 0);
	freevalue(&tmp1);
	freevalue(&tmp2);
	mulvalue(v1 + 2, v2 + 0, &tmp1);
	mulvalue(v1 + 0, v2 + 2, &tmp2);
	subvalue(&tmp1, &tmp2, vr + 1);
	freevalue(&tmp1);
	freevalue(&tmp2);
	mulvalue(v1 + 0, v2 + 1, &tmp1);
	mulvalue(v1 + 1, v2 + 0, &tmp2);
	subvalue(&tmp1, &tmp2, vr + 2);
	freevalue(&tmp1);
	freevalue(&tmp2);
	return res;
}


/*
 * Return the dot product of two matrices.
 *	result = matdot(m1, m2);
 */
VALUE
matdot(MATRIX *m1, MATRIX *m2)
{
	VALUE *v1, *v2;
	VALUE result, tmp1, tmp2;
	long len;

	v1 = m1->m_table;
	v2 = m2->m_table;
	mulvalue(v1, v2, &result);
	len = m1->m_size;
	while (--len > 0) {
		mulvalue(++v1, ++v2, &tmp1);
		addvalue(&result, &tmp1, &tmp2);
		freevalue(&tmp1);
		freevalue(&result);
		result = tmp2;
	}
	return result;
}


/*
 * Scale the elements of a matrix by a specified power of two.
 *
 * given:
 *	m		matrix to be scaled
 *	n		scale factor
 */
MATRIX *
matscale(MATRIX *m, long n)
{
	register VALUE *val, *vres;
	VALUE temp;
	long index;
	MATRIX *res;		/* resulting matrix */

	if (n == 0)
		return matcopy(m);
	temp.v_type = V_NUM;
	temp.v_subtype = V_NOSUBTYPE;
	temp.v_num = itoq(n);
	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		scalevalue(val++, &temp, vres++);
	qfree(temp.v_num);
	return res;
}


/*
 * Shift the elements of a matrix by the specified number of bits.
 * Positive shift means leftwards, negative shift rightwards.
 *
 * given:
 *	m		matrix to be shifted
 *	n		shift count
 */
MATRIX *
matshift(MATRIX *m, long n)
{
	register VALUE *val, *vres;
	VALUE temp;
	long index;
	MATRIX *res;		/* resulting matrix */

	if (n == 0)
		return matcopy(m);
	temp.v_type = V_NUM;
	temp.v_subtype = V_NOSUBTYPE;
	temp.v_num = itoq(n);
	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		shiftvalue(val++, &temp, FALSE, vres++);
	qfree(temp.v_num);
	return res;
}


/*
 * Multiply the elements of a matrix by a specified value.
 *
 * given:
 *	m		matrix to be multiplied
 *	vp		value to multiply by
 */
MATRIX *
matmulval(MATRIX *m, VALUE *vp)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		mulvalue(val++, vp, vres++);
	return res;
}


/*
 * Divide the elements of a matrix by a specified value, keeping
 * only the integer quotient.
 *
 * given:
 *	m		matrix to be divided
 *	vp		value to divide by
 *	v3		rounding type parameter
 */
MATRIX *
matquoval(MATRIX *m, VALUE *vp, VALUE *v3)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	if ((vp->v_type == V_NUM) && qiszero(vp->v_num)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		quovalue(val++, vp, v3, vres++);
	return res;
}


/*
 * Divide the elements of a matrix by a specified value, keeping
 * only the remainder of the division.
 *
 * given:
 *	m		matrix to be divided
 *	vp		value to divide by
 *	v3		rounding type parameter
 */
MATRIX *
matmodval(MATRIX *m, VALUE *vp, VALUE *v3)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	if ((vp->v_type == V_NUM) && qiszero(vp->v_num)) {
		math_error("Division by zero");
		/*NOTREACHED*/
	}
	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		modvalue(val++, vp, v3, vres++);
	return res;
}


VALUE
mattrace(MATRIX *m)
{
	VALUE *vp;
	VALUE sum;
	VALUE tmp;
	long i, j;

	if (m->m_dim < 2) {
		matsum(m, &sum);
		return sum;
	}
	if (m->m_dim != 2)
		return error_value(E_MATTRACE2);
	i = (m->m_max[0] - m->m_min[0] + 1);
	j = (m->m_max[1] - m->m_min[1] + 1);
	if (i != j)
		return error_value(E_MATTRACE3);
	vp = m->m_table;
	copyvalue(vp, &sum);
	j++;
	while (--i > 0) {
		vp += j;
		addvalue(&sum, vp, &tmp);
		freevalue(&sum);
		sum = tmp;
	}
	return sum;
}


/*
 * Transpose a 2-dimensional matrix
 */
MATRIX *
mattrans(MATRIX *m)
{
	register VALUE *v1, *v2;	/* current values */
	long rows, cols;		/* rows and columns in new matrix */
	long row, col;			/* current row and column */
	MATRIX *res;

	if (m->m_dim < 2)
		return matcopy(m);
	res = matalloc(m->m_size);
	res->m_dim = 2;
	res->m_min[0] = m->m_min[1];
	res->m_max[0] = m->m_max[1];
	res->m_min[1] = m->m_min[0];
	res->m_max[1] = m->m_max[0];
	rows = (m->m_max[1] - m->m_min[1] + 1);
	cols = (m->m_max[0] - m->m_min[0] + 1);
	v1 = res->m_table;
	for (row = 0; row < rows; row++) {
		v2 = &m->m_table[row];
		for (col = 0; col < cols; col++) {
			copyvalue(v2, v1);
			v1++;
			v2 += rows;
		}
	}
	return res;
}


/*
 * Produce a matrix with values all of which are conjugated.
 */
MATRIX *
matconj(MATRIX *m)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		conjvalue(val++, vres++);
	return res;
}


/*
 * Round elements of a matrix to specified number of decimal digits
 */
MATRIX *
matround(MATRIX *m, VALUE *v2, VALUE *v3)
{
	VALUE *p, *q;
	long s;
	MATRIX *res;

	s = m->m_size;
	res = matalloc(s);
	*res = *m;
	p = m->m_table;
	q = res->m_table;
	while (s-- > 0)
		roundvalue(p++, v2, v3, q++);
	return res;
}


/*
 * Round elements of a matrix to specified number of binary digits
 */
MATRIX *
matbround(MATRIX *m, VALUE *v2, VALUE *v3)
{
	VALUE *p, *q;
	long s;
	MATRIX *res;

	s = m->m_size;
	res = matalloc(s);
	*res = *m;
	p = m->m_table;
	q = res->m_table;
	while (s-- > 0)
		broundvalue(p++, v2, v3, q++);
	return res;
}

/*
 * Approximate a matrix by approximating elements to be multiples of
 * v2, rounding type determined by v3.
 */
MATRIX *
matappr(MATRIX *m, VALUE *v2, VALUE *v3)
{
	VALUE *p, *q;
	long s;
	MATRIX *res;

	s = m->m_size;
	res = matalloc(s);
	*res = *m;
	p = m->m_table;
	q = res->m_table;
	while (s-- > 0)
		apprvalue(p++, v2, v3, q++);
	return res;
}




/*
 * Produce a matrix with values all of which have been truncated to integers.
 */
MATRIX *
matint(MATRIX *m)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		intvalue(val++, vres++);
	return res;
}


/*
 * Produce a matrix with values all of which have only the fraction part left.
 */
MATRIX *
matfrac(MATRIX *m)
{
	register VALUE *val, *vres;
	long index;
	MATRIX *res;

	res = matalloc(m->m_size);
	*res = *m;
	val = m->m_table;
	vres = res->m_table;
	for (index = m->m_size; index > 0; index--)
		fracvalue(val++, vres++);
	return res;
}


/*
 * Index a matrix normally by the specified set of index values.
 * Returns the address of the matrix element if it is valid, or generates
 * an error if the index values are out of range.  The create flag is TRUE
 * if the element is to be written, but this is ignored here.
 *
 * given:
 *	mp		matrix to operate on
 *	create		TRUE => create if element does not exist
 *	dim		dimension of the indexing
 *	indices		table of values being indexed by
 */
/*ARGSUSED*/
VALUE *
matindex(MATRIX *mp, BOOL UNUSED(create), long dim, VALUE *indices)
{
	NUMBER *q;		/* index value */
	VALUE *vp;
	long index;		/* index value as an integer */
	long offset;		/* current offset into array */
	int i;			/* loop counter */

	if (dim < 0) {
		math_error("Negative dimension %ld for matrix", dim);
		/*NOTREACHED*/
	}
	for (;;) {
		if (dim <  mp->m_dim) {
			math_error(
			    "Indexing a %ldd matrix as a %ldd matrix",
			    mp->m_dim, dim);
			/*NOTREACHED*/
		}
		offset = 0;
		for (i = 0; i < mp->m_dim; i++) {
			if (indices->v_type != V_NUM) {
				math_error("Non-numeric index for matrix");
				/*NOTREACHED*/
			}
			q = indices->v_num;
			if (qisfrac(q)) {
				math_error("Non-integral index for matrix");
				/*NOTREACHED*/
			}
			index = qtoi(q);
			if (zge31b(q->num) || (index < mp->m_min[i]) ||
			    (index > mp->m_max[i])) {
				math_error("Index out of bounds for matrix");
				/*NOTREACHED*/
			}
			offset *= (mp->m_max[i] - mp->m_min[i] + 1);
			offset += (index - mp->m_min[i]);
			indices++;
		}
		vp = mp->m_table + offset;
		dim -= mp->m_dim;
		if (dim == 0)
			break;
		if (vp->v_type != V_MAT) {
			math_error("Non-matrix argument for matindex");
			/*NOTREACHED*/
		}
		mp = vp->v_mat;
	}
	return vp;
}


/*
 * Returns the list of indices for a matrix element with specified
 * double-bracket index.
 */
LIST *
matindices(MATRIX *mp, long index)
{
	LIST *lp;
	int j;
	long d;
	VALUE val;

	if (index < 0 || index >= mp->m_size)
		return NULL;

	lp = listalloc();
	val.v_type = V_NUM;
	j = mp->m_dim;

	while (--j >= 0) {
		d = mp->m_max[j] - mp->m_min[j] + 1;
		val.v_num = itoq(index % d + mp->m_min[j]);
		insertlistfirst(lp, &val);
		qfree(val.v_num);
		index /= d;
	}
	return lp;
}


/*
 * Search a matrix for the specified value, starting with the specified index.
 * Returns 0 and stores index if value found; otherwise returns 1.
 */
int
matsearch(MATRIX *m, VALUE *vp, long i, long j, ZVALUE *index)
{
	register VALUE *val;

	val = &m->m_table[i];
	if (i < 0 || j > m->m_size) {
		math_error("This should not happen in call to matsearch");
		/*NOTREACHED*/
	}
	while (i < j) {
		if (acceptvalue(val++, vp)) {
			utoz(i, index);
			return 0;
		}
		i++;
	}
	return 1;
}


/*
 * Search a matrix backwards for the specified value, starting with the
 * specified index.  Returns 0 and stores index if value found; otherwise
 * returns 1.
 */
int
matrsearch(MATRIX *m, VALUE *vp, long i, long j, ZVALUE *index)
{
	register VALUE *val;

	if (i < 0 || j > m->m_size) {
		math_error("This should not happen in call to matrsearch");
		/*NOTREACHED*/
	}
	val = &m->m_table[--j];
	while (j >= i) {
		if (acceptvalue(val--, vp)) {
			utoz(j, index);
			return 0;
		}
		j--;
	}
	return 1;
}

/*
 * Fill all of the elements of a matrix with one of two specified values.
 * All entries are filled with the first specified value, except that if
 * the matrix is w-dimensional and the second value pointer is non-NULL, then
 * all diagonal entries are filled with the second value.  This routine
 * affects the supplied matrix directly, and doesn't return a copy.
 *
 * given:
 *	m		matrix to be filled
 *	v1		value to fill most of matrix with
 *	v2		value for diagonal entries or null
 */
void
matfill(MATRIX *m, VALUE *v1, VALUE *v2)
{
	register VALUE *val;
	VALUE temp1, temp2;
	long rows, cols;
	long i, j;

	copyvalue(v1, &temp1);

	val = m->m_table;
	if (m->m_dim != 2 || v2 == NULL) {
		for (i = m->m_size; i > 0; i--) {
			freevalue(val);
			copyvalue(&temp1, val++);
		}
		freevalue(&temp1);
		return;
	}

	copyvalue(v2, &temp2);
	rows = m->m_max[0] - m->m_min[0] + 1;
	cols = m->m_max[1] - m->m_min[1] + 1;

	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			freevalue(val);
			if (i == j)
				copyvalue(&temp2, val++);
			else
				copyvalue(&temp1, val++);
		}
	}
	freevalue(&temp1);
	freevalue(&temp2);
}



/*
 * Set a copy of a square matrix to the identity matrix.
 */
S_FUNC MATRIX *
matident(MATRIX *m)
{
	register VALUE *val;	/* current value */
	long row, col;		/* current row and column */
	long rows;		/* number of rows */
	MATRIX *res;		/* resulting matrix */

	if (m->m_dim != 2) {
		math_error(
		    "Matrix dimension must be two for setting to identity");
		/*NOTREACHED*/
	}
	if ((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1])) {
		math_error("Matrix must be square for setting to identity");
		/*NOTREACHED*/
	}
	res = matalloc(m->m_size);
	*res = *m;
	val = res->m_table;
	rows = (res->m_max[0] - res->m_min[0] + 1);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < rows; col++) {
			val->v_type = V_NUM;
			val->v_num = ((row == col) ? qlink(&_qone_) :
						     qlink(&_qzero_));
			val++;
		}
	}
	return res;
}


/*
 * Calculate the inverse of a matrix if it exists.
 * This is done by using transformations on the supplied matrix to convert
 * it to the identity matrix, and simultaneously applying the same set of
 * transformations to the identity matrix.
 */
MATRIX *
matinv(MATRIX *m)
{
	MATRIX *res;		/* matrix to become the inverse */
	long rows;		/* number of rows */
	long cur;		/* current row being worked on */
	long row, col;		/* temp row and column values */
	VALUE *val;		/* current value in matrix*/
	VALUE *vres;		/* current value in result for dim < 2 */
	VALUE mulval;		/* value to multiply rows by */
	VALUE tmpval;		/* temporary value */

	if (m->m_dim < 2) {
		res = matalloc(m->m_size);
		*res = *m;
		val = m->m_table;
		vres = res->m_table;
		for (cur = m->m_size; cur > 0; cur--)
			invertvalue(val++, vres++);
		return res;
	}
	if (m->m_dim != 2) {
		math_error("Matrix dimension exceeds two for inverse");
		/*NOTREACHED*/
	}
	if ((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1])) {
		math_error("Inverting non-square matrix");
		/*NOTREACHED*/
	}
	/*
	 * Begin by creating the identity matrix with the same attributes.
	 */
	res = matalloc(m->m_size);
	*res = *m;
	rows = (m->m_max[0] - m->m_min[0] + 1);
	val = res->m_table;
	for (row = 0; row < rows; row++) {
		for (col = 0; col < rows; col++) {
			if (row == col)
				val->v_num = qlink(&_qone_);
			else
				val->v_num = qlink(&_qzero_);
			val->v_type = V_NUM;
			val++;
		}
	}
	/*
	 * Now loop over each row, and eliminate all entries in the
	 * corresponding column by using row operations.  Do the same
	 * operations on the resulting matrix.	Copy the original matrix
	 * so that we don't destroy it.
	 */
	m = matcopy(m);
	for (cur = 0; cur < rows; cur++) {
		/*
		 * Find the first nonzero value in the rest of the column
		 * downwards from [cur,cur].  If there is no such value, then
		 * the matrix is not invertible.  If the first nonzero entry
		 * is not the current row, then swap the two rows to make the
		 * current one nonzero.
		 */
		row = cur;
		val = &m->m_table[(row * rows) + row];
		while (testvalue(val) == 0) {
			if (++row >= rows) {
				matfree(m);
				matfree(res);
				math_error("Matrix is not invertible");
				/*NOTREACHED*/
			}
			val += rows;
		}
		invertvalue(val, &mulval);
		if (row != cur) {
			matswaprow(m, row, cur);
			matswaprow(res, row, cur);
		}
		/*
		 * Now for every other nonzero entry in the current column,
		 * subtract the appropriate multiple of the current row to
		 * force that entry to become zero.
		 */
		val = &m->m_table[cur];
		for (row = 0; row < rows; row++) {
			if ((row == cur) || (testvalue(val) == 0)) {
				if (row+1 < rows)
					val += rows;
				continue;
			}
			mulvalue(val, &mulval, &tmpval);
			matsubrow(m, row, cur, &tmpval);
			matsubrow(res, row, cur, &tmpval);
			freevalue(&tmpval);
			if (row+1 < rows)
				val += rows;
		}
		freevalue(&mulval);
	}
	/*
	 * Now the original matrix has nonzero entries only on its main
	 * diagonal.  Scale the rows of the result matrix by the inverse
	 * of those entries.
	 */
	val = m->m_table;
	for (row = 0; row < rows; row++) {
		if ((val->v_type != V_NUM) || !qisone(val->v_num)) {
			invertvalue(val, &mulval);
			matmulrow(res, row, &mulval);
			freevalue(&mulval);
		}
		if (row+1 < rows)
			val += (rows + 1);
	}
	matfree(m);
	return res;
}


/*
 * Calculate the determinant of a square matrix.
 * This uses the fraction-free Gauss-Bareiss algorithm.
 */
VALUE
matdet(MATRIX *m)
{
	long n;			/* original matrix is n x n */
	long k;			/* working submatrix is k x k */
	long i, j;
	VALUE *pivot, *div, *val;
	VALUE *vp, *vv;
	VALUE tmp1, tmp2, tmp3;
	BOOL neg;		/* whether to negate determinant */

	if (m->m_dim < 2) {
		vp = m->m_table;
		i = m->m_size;
		copyvalue(vp, &tmp1);

		while (--i > 0) {
			mulvalue(&tmp1, ++vp, &tmp2);
			freevalue(&tmp1);
			tmp1 = tmp2;
		}
		return tmp1;
	}

	if (m->m_dim != 2)
		return error_value(E_DET2);
	if ((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1]))
		return error_value(E_DET3);

	/*
	 * Loop over each row, and eliminate all lower entries in the
	 * corresponding column by using row operations.  Copy the original
	 * matrix so that we don't destroy it.
	 */
	neg = FALSE;
	m = matcopy(m);
	n = (m->m_max[0] - m->m_min[0] + 1);
	pivot = div = m->m_table;
	for (k = n; k > 0; k--) {
		/*
		 * Find the first nonzero value in the rest of the column
		 * downwards from pivot.  If there is no such value, then
		 * the determinant is zero.  If the first nonzero entry is not
		 * the pivot, then swap rows in the k * k submatrix, and
		 * remember that the determinant changes sign.
		 */
		val = pivot;
		i = k;
		while (!testvalue(val)) {
			if (--i <= 0) {
				tmp1.v_type = V_NUM;
				tmp1.v_subtype = V_NOSUBTYPE;
				tmp1.v_num = qlink(&_qzero_);
				matfree(m);
				return tmp1;
			}
			val += n;
		}
		if (i < k) {
			vp = pivot;
			vv = val;
			j = k;
			while (j-- > 0) {
				tmp1 = *vp;
				*vp++ = *vv;
				*vv++ = tmp1;
			}
			neg = !neg;
		}
		/*
		 * Now for every val below the pivot, for each entry to
		 * the right of val, calculate the 2 x 2 determinant
		 * with corners at the pivot and the entry.  If
		 * k < n, divide by div (the previous pivot value).
		 */
		val = pivot;
		i = k;
		while (--i > 0) {
			val += n;
			vp = pivot;
			vv = val;
			j = k;
			while (--j > 0) {
				mulvalue(pivot, ++vv, &tmp1);
				mulvalue(val, ++vp, &tmp2);
				subvalue(&tmp1, &tmp2, &tmp3);
				freevalue(&tmp1);
				freevalue(&tmp2);
				freevalue(vv);
				if (k < n) {
					divvalue(&tmp3, div, vv);
					freevalue(&tmp3);
				}
				else
					*vv = tmp3;
			}
		}
		div = pivot;
		if (k > 0)
			pivot += n + 1;
	}
	if (neg)
		negvalue(div, &tmp1);
	else
		copyvalue(div, &tmp1);
	matfree(m);
	return tmp1;
}


/*
 * Local utility routine to swap two rows of a square matrix.
 * No checks are made to verify the legality of the arguments.
 */
S_FUNC void
matswaprow(MATRIX *m, long r1, long r2)
{
	register VALUE *v1, *v2;
	register long rows;
	VALUE tmp;

	if (r1 == r2)
		return;
	rows = (m->m_max[0] - m->m_min[0] + 1);
	v1 = &m->m_table[r1 * rows];
	v2 = &m->m_table[r2 * rows];
	while (rows-- > 0) {
		tmp = *v1;
		*v1 = *v2;
		*v2 = tmp;
		v1++;
		v2++;
	}
}


/*
 * Local utility routine to subtract a multiple of one row to another one.
 * The row to be changed is oprow, the row to be subtracted is baserow.
 * No checks are made to verify the legality of the arguments.
 */
S_FUNC void
matsubrow(MATRIX *m, long oprow, long baserow, VALUE *mulval)
{
	register VALUE *vop, *vbase;
	register long entries;
	VALUE tmp1, tmp2;

	entries = (m->m_max[0] - m->m_min[0] + 1);
	vop = &m->m_table[oprow * entries];
	vbase = &m->m_table[baserow * entries];
	while (entries-- > 0) {
		mulvalue(vbase, mulval, &tmp1);
		subvalue(vop, &tmp1, &tmp2);
		freevalue(&tmp1);
		freevalue(vop);
		*vop = tmp2;
		vop++;
		vbase++;
	}
}


/*
 * Local utility routine to multiply a row by a specified number.
 * No checks are made to verify the legality of the arguments.
 */
S_FUNC void
matmulrow(MATRIX *m, long row, VALUE *mulval)
{
	register VALUE *val;
	register long rows;
	VALUE tmp;

	rows = (m->m_max[0] - m->m_min[0] + 1);
	val = &m->m_table[row * rows];
	while (rows-- > 0) {
		mulvalue(val, mulval, &tmp);
		freevalue(val);
		*val = tmp;
		val++;
	}
}


/*
 * Make a full copy of a matrix.
 */
MATRIX *
matcopy(MATRIX *m)
{
	MATRIX *res;
	register VALUE *v1, *v2;
	register long i;

	res = matalloc(m->m_size);
	*res = *m;
	v1 = m->m_table;
	v2 = res->m_table;
	i = m->m_size;
	while (i-- > 0) {
		copyvalue(v1, v2);
		v1++;
		v2++;
	}
	return res;
}


/*
 * Make a matrix the same size as another and filled with a fixed value.
 *
 * given:
 *	m		matrix to initialize
 *	v1		value to fill most of matrix with
 *	v2		value for diagonal entries (or NULL)
 */
MATRIX *
matinit(MATRIX *m, VALUE *v1, VALUE *v2)
{
	MATRIX *res;
	register VALUE *v;
	register long i;
	long row;
	long rows;

	/*
	 * clone matrix size
	 */
	res = matalloc(m->m_size);
	*res = *m;

	/*
	 * firewall
	 */
	if (v2 && ((res->m_dim != 2) ||
		((res->m_max[0] - res->m_min[0]) !=
		 (res->m_max[1] - res->m_min[1])))) {
			math_error("Filling diagonals of non-square matrix");
			/*NOTREACHED*/
	}

	/*
	 * fill the bulk of the matrix
	 */
	v = res->m_table;
	if (v2 == NULL) {
		i = m->m_size;
		while (i-- > 0) {
			copyvalue(v1, v++);
		}
		return res;
	}

	/*
	 * fill the diagonal of a square matrix if requested
	 */
	rows = res->m_max[0] - res->m_min[0] + 1;
	v = res->m_table;
	for (row = 0; row < rows; row++) {
		copyvalue(v2, v+row);
		v += rows;
	}
	return res;
}


/*
 * Allocate a matrix with the specified number of elements.
 */
MATRIX *
matalloc(long size)
{
	MATRIX *m;
	long i;
	VALUE *vp;

	m = (MATRIX *) malloc(matsize(size));
	if (m == NULL) {
		math_error("Cannot get memory to allocate matrix of size %ld",
			   size);
		/*NOTREACHED*/
	}
	m->m_size = size;
	for (i = size, vp = m->m_table; i > 0; i--, vp++)
		vp->v_subtype = V_NOSUBTYPE;
	return m;
}


/*
 * Free a matrix, along with all of its element values.
 */
void
matfree(MATRIX *m)
{
	register VALUE *vp;
	register long i;

	vp = m->m_table;
	i = m->m_size;
	while (i-- > 0)
		freevalue(vp++);
	free(m);
}


/*
 * Test whether a matrix has any "nonzero" values.
 * Returns TRUE if so.
 */
BOOL
mattest(MATRIX *m)
{
	register VALUE *vp;
	register long i;

	vp = m->m_table;
	i = m->m_size;
	while (i-- > 0) {
		if (testvalue(vp++))
			return TRUE;
	}
	return FALSE;
}

/*
 * Sum the elements in a matrix.
 */
void
matsum(MATRIX *m, VALUE *vres)
{
	VALUE *vp;
	VALUE tmp;		/* first sum value */
	VALUE sum;		/* final sum value */
	long i;

	vp = m->m_table;
	i = m->m_size;
	copyvalue(vp, &sum);

	while (--i > 0) {
		addvalue(&sum, ++vp, &tmp);
		freevalue(&sum);
		sum = tmp;
	}
	*vres = sum;
}


/*
 * Test whether or not two matrices are equal.
 * Equality is determined by the shape and values of the matrices,
 * but not by their index bounds.  Returns TRUE if they differ.
 */
BOOL
matcmp(MATRIX *m1, MATRIX *m2)
{
	VALUE *v1, *v2;
	long i;

	if (m1 == m2)
		return FALSE;
	if ((m1->m_dim != m2->m_dim) || (m1->m_size != m2->m_size))
		return TRUE;
	for (i = 0; i < m1->m_dim; i++) {
		if ((m1->m_max[i] - m1->m_min[i]) !=
		    (m2->m_max[i] - m2->m_min[i]))
		return TRUE;
	}
	v1 = m1->m_table;
	v2 = m2->m_table;
	i = m1->m_size;
	while (i-- > 0) {
		if (comparevalue(v1++, v2++))
			return TRUE;
	}
	return FALSE;
}


void
matreverse(MATRIX *m)
{
	VALUE *p, *q;
	VALUE tmp;

	p = m->m_table;
	q = m->m_table + m->m_size - 1;
	while (q > p) {
		tmp = *p;
		*p++ = *q;
		*q-- = tmp;
	}
}


void
matsort(MATRIX *m)
{
	VALUE *a, *b, *next, *end;
	VALUE *buf, *p;
	VALUE *S[LONG_BITS];
	long len[LONG_BITS];
	long i, j, k;

	buf = (VALUE *) malloc(m->m_size * sizeof(VALUE));
	if (buf == NULL) {
		math_error("Not enough memory for matsort");
		/*NOTREACHED*/
	}
	next = m->m_table;
	end = next + m->m_size;
	for (k = 0; next && k < LONG_BITS; k++) {
		S[k] = next++;			/* S[k] is start of a run */
		len[k] = 1;
		if (next == end)
			next = NULL;
		while (k > 0 && (!next || len[k] >= len[k - 1])) {/* merging */
			j = len[k];
			b = S[k--];
			i = len[k];
			a = S[k];
			len[k] += j;
			p = buf;
			if (precvalue(b, a)) {
				do {
					*p++ = *b++;
					j--;
				} while (j > 0 && precvalue(b,a));
				if (j == 0) {
					memcpy(p, a, i * sizeof(VALUE));
					memcpy(S[k], buf,
					       len[k] * sizeof(VALUE));
					continue;
				}
			}

			do {
				do {
					*p++ = *a++;
					i--;
				} while (i > 0 && !precvalue(b,a));
				if (i == 0) {
					break;
				}
				do {
					*p++ = *b++;
					j--;
				} while (j > 0 && precvalue(b,a));
			} while (j != 0);

			if (i == 0) {
				memcpy(S[k], buf, (p - buf) * sizeof(VALUE));
			} else if (j == 0) {
				memcpy(p, a, i * sizeof(VALUE));
				memcpy(S[k], buf, len[k] * sizeof(VALUE));
			}
		}
	}
	free(buf);
	if (k >= LONG_BITS) {
		/* this should never happen */
		math_error("impossible k overflow in matsort!");
		/*NOTREACHED*/
	}
}

void
matrandperm(MATRIX *m)
{
	VALUE *vp;
	long s, i;
	VALUE val;

	s = m->m_size;
	for (vp = m->m_table; s > 1; vp++, s--) {
		i = irand(s);
		if (i > 0) {
			val = *vp;
			*vp = vp[i];
			vp[i] = val;
		}
	}
}


/*
 * Test whether or not a matrix is the identity matrix.
 * Returns TRUE if so.
 */
BOOL
matisident(MATRIX *m)
{
	register VALUE *val;	/* current value */
	long row, col;		/* row and column numbers */

	val = m->m_table;
	if (m->m_dim == 0) {
		return (val->v_type == V_NUM && qisone(val->v_num));
	}
	if (m->m_dim == 1) {
		for (row = m->m_min[0]; row <= m->m_max[0]; row++, val++) {
			if (val->v_type != V_NUM || !qisone(val->v_num))
				return FALSE;
		}
		return TRUE;
	}
	if ((m->m_dim != 2) ||
		((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1])))
			return FALSE;
	for (row = m->m_min[0]; row <= m->m_max[0]; row++) {
		/*
		 * We could use col = m->m_min[1]; col < m->m_max[1]
		 * but if m->m_min[0] != m->m_min[1] this won't work.
		 * We know that we have a square 2-dimensional matrix
		 * so we will pretend that m->m_min[0] == m->m_min[1].
		 */
		for (col = m->m_min[0]; col <= m->m_max[0]; col++) {
			if (val->v_type != V_NUM)
				return FALSE;
			if (row == col) {
				if (!qisone(val->v_num))
					return FALSE;
			} else {
				if (!qiszero(val->v_num))
					return FALSE;
			}
			val++;
		}
	}
	return TRUE;
}


/*
 * Print a matrix and possibly few of its elements.
 * The argument supplied specifies how many elements to allow printing.
 * If any elements are printed, they are printed in short form.
 */
void
matprint(MATRIX *m, long max_print)
{
	VALUE *vp;
	long fullsize, count, index;
	long dim, i, j;
	char *msg;
	long sizes[MAXDIM];

	dim = m->m_dim;
	fullsize = 1;
	for (i = dim - 1; i >= 0; i--) {
		sizes[i] = fullsize;
		fullsize *= (m->m_max[i] - m->m_min[i] + 1);
	}
	msg = ((max_print > 0) ? "\nmat [" : "mat [");
	if (dim) {
		for (i = 0; i < dim; i++) {
			if (m->m_min[i]) {
				math_fmt("%s%ld:%ld", msg,
					 m->m_min[i], m->m_max[i]);
			} else {
				math_fmt("%s%ld", msg, m->m_max[i] + 1);
			}
			msg = ",";
		}
	} else {
		math_str("mat [");
	}
	if (max_print > fullsize) {
		max_print = fullsize;
	}
	vp = m->m_table;
	count = 0;
	for (index = 0; index < fullsize; index++) {
		if ((vp->v_type != V_NUM) || !qiszero(vp->v_num))
			count++;
		vp++;
	}
	math_fmt("] (%ld element%s, %ld nonzero)",
		fullsize, (fullsize == 1) ? "" : "s", count);
	if (max_print <= 0)
		return;

	/*
	 * Now print the first few elements of the matrix in short
	 * and unambiguous format.
	 */
	math_str(":\n");
	vp = m->m_table;
	for (index = 0; index < max_print; index++) {
		msg = "	 [";
		j = index;
		if (dim) {
			for (i = 0; i < dim; i++) {
				math_fmt("%s%ld", msg,
					 m->m_min[i] + (j / sizes[i]));
				j %= sizes[i];
				msg = ",";
			}
		} else {
			math_str(msg);
		}
		math_str("] = ");
		printvalue(vp++, PRINT_SHORT | PRINT_UNAMBIG);
		math_str("\n");
	}
	if (max_print < fullsize)
		math_str("  ...\n");
}
