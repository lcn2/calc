/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Size and sizeof functions are implemented here
 */


#include "value.h"
#include "zrand.h"
#include "zrandom.h"
#include "block.h"


/*
 * forward declarations
 */
static long zsize(ZVALUE);
static long qsize(NUMBER*);
static long csize(COMPLEX*);
static long memzsize(ZVALUE);
static long memqsize(NUMBER*);


/*
 * elm_count - return information about the number of elements
 *
 * Return information about the number of elements or part of
 * a value.  This is what the size() builtin returns with the
 * exception of the V_FILE type.  To get V_FILE size length,
 * the getsize(vp->v_file, &len) should be called directly.
 *
 * This is not the sizeof, see lsizeof() for that information.
 *
 * given:
 *	vp	pointer to a value
 *
 * return:
 *	number of elements
 */
long
elm_count(VALUE *vp)
{
	long result;

	/*
	 * return information about the number of elements
	 *
	 * This is not the sizeof, see lsizeof() for that information.
	 * This is does not include overhead, see memsize() for that info.
	 */

	switch (vp->v_type) {
	case V_NULL:
	case V_INT:
	case V_ADDR:
	case V_OCTET:
		result = 0;
		break;
	case V_MAT:
		result = vp->v_mat->m_size;
		break;
	case V_LIST:
		result = vp->v_list->l_count;
		break;
	case V_ASSOC:
		result = vp->v_assoc->a_count;
		break;
	case V_OBJ:
		result = vp->v_obj->o_actions->count;
		break;
	case V_STR:
		result = vp->v_str->s_len;
		break;
	case V_BLOCK:
		result = (long) vp->v_block->datalen;
		break;
	case V_NBLOCK:
		result = (long) vp->v_nblock->blk->datalen;
		break;
	/*
	 * V_NUM, V_COM, V_RAND, V_RANDOM, V_CONFIG, V_HASH
	 *
	 * V_FILE (use getsize(vp->v_file, &len) for file length)
	 */
	default:
		result = (vp->v_type > 0);
		break;
	}
	return result;
}


/*
 * zsize - calculate memory footprint of a ZVALUE (exlcuding overhead)
 *
 * The numeric -1, - and 1 storage values are ignored.
 *
 * given:
 *	z	ZVALUE to examine
 *
 * returns:
 *	value size
 */
static long
zsize(ZVALUE z)
{
	/* ignore the size of 0, 1 and -1 */
	if (z.v != _zeroval_ && z.v != _oneval_ && !zisunit(z) && !ziszero(z)) {
		return (long)z.len * (long)sizeof(HALF);
	} else {
		return (long)0;
	}
}


/*
 * qsize - calculate memory footprint of a NUMBER (exlcuding overhead)
 *
 * The numeric -1, - and 1 storage values are ignored.	Denominator
 * parts of integers are ignored.
 *
 * given:
 *	q	pointer to NUMBER to examine
 *
 * returns:
 *	value size
 */
static long
qsize(NUMBER *q)
{
	/* ingore denominator parts of integers */
	if (qisint(q)) {
		return zsize(q->num);
	} else {
		return (zsize(q->num) + zsize(q->den));
	}
}


/*
 * csize - calculate memory footprint of a COMPLEX (exlcuding overhead)
 *
 * The numeric -1, - and 1 storage values are ignored.	Denominator
 * parts of integers are ignored.  Imaginary parts of pure reals
 * are ignored.
 *
 * given:
 *	c	pointer to COMPLEX to examine
 *
 * returns:
 *	value size
 */
static long
csize(COMPLEX *c)
{
	/* ingore denominator parts of integers */
	if (cisreal(c)) {
		return qsize(c->real);
	} else {
		return (qsize(c->real) + qsize(c->imag));
	}
}


/*
 * memzsize - calculate memory footprint of a ZVALUE including overhead
 *
 * given:
 *	z	ZVALUE to examine
 *
 * returns:
 *	memory footprint
 */
static long
memzsize(ZVALUE z)
{
	return (long)sizeof(ZVALUE) + ((long)z.len * (long)sizeof(HALF));
}


/*
 * memqsize - calculate memory footprint of a NUMBER including overhead
 *
 * given:
 *	q	pointer of NUMBER to examine
 *
 * returns:
 *	memory footprint
 */
static long
memqsize(NUMBER *q)
{
	return (long)sizeof(NUMBER) + memzsize(q->num) + memzsize(q->den);
}


/*
 * lsizeof - calculate memory footprint of a VALUE (not counting overhead)
 *
 * given:
 *	vp	pointer of VALUE to examine
 *
 * returns:
 *	memory footprint
 */
long
lsizeof(VALUE *vp)
{
	VALUE *p;
	LISTELEM *ep;
	OBJECTACTIONS *oap;
	ASSOCELEM *aep;
	ASSOCELEM **ept;
	long s;
	long i;

	/*
	 * return information about memory footprint
	 *
	 * This is not the number of elements, see elm_count() for that info.
	 * This is does not include overhead, see memsize() for that info.
	 */
	i = 0;
	s = 0;
	if (vp->v_type > 0) {
		switch(vp->v_type) {
		case V_INT:
		case V_ADDR:
		case V_OCTET:
			break;
		case V_NUM:
			s = qsize(vp->v_num);
			break;
		case V_COM:
			s = csize(vp->v_com);
			break;
		case V_STR:
			s = vp->v_str->s_len + 1;
			break;
		case V_MAT:
			i = vp->v_mat->m_size;
			p = vp->v_mat->m_table;
			while (i-- > 0)
				s += lsizeof(p++);
			break;
		case V_LIST:
			for (ep = vp->v_list->l_first; ep; ep = ep->e_next) {
				s += lsizeof(&ep->e_value);
			}
			break;
		case V_ASSOC:
			i = vp->v_assoc->a_size;
			ept = vp->v_assoc->a_table;
			while (i-- > 0) {
				for (aep = ept[i]; aep; aep = aep->e_next) {
					s += lsizeof(&aep->e_value);
				}
			}
			break;
		case V_OBJ:
			oap = vp->v_obj->o_actions;
			i = oap->count;
			p = vp->v_obj->o_table;
			while (i-- > 0)
				s += lsizeof(p++);
			break;
		case V_FILE:
			s = sizeof(vp->v_file);
			break;
		case V_RAND:
			s = sizeof(RAND);
			break;
		case V_RANDOM:
			s = (long)sizeof(RANDOM) +
				zsize(vp->v_random->n) +
				zsize(vp->v_random->r);
			break;
		case V_CONFIG:
			s = (long)sizeof(CONFIG) +
				(long)strlen(vp->v_config->prompt1) +
				(long)strlen(vp->v_config->prompt2) + 2;
			break;
		case V_HASH:
			/* ignore the unused part of the union */
			s = (long)sizeof(HASH) +
				vp->v_hash->unionsize -
				(long)sizeof(vp->v_hash->h_union);
			break;
		case V_BLOCK:
			s = vp->v_block->maxsize;
			break;
		case V_NBLOCK:
			s = vp->v_nblock->blk->maxsize;
			break;
		default:
			math_error("sizeof not defined for value type");
			/*NOTREACHED*/
		}
	}
	return s;
}


/*
 * memsize - calculate memory footprint of a VALUE including overhead
 *
 * given:
 *	vp	pointer of VALUE to examine
 *
 * returns:
 *	memory footprint including overhead
 */
long
memsize(VALUE *vp)
{
	long s;
	long i, j;
	VALUE *p;
	LISTELEM *ep;
	OBJECTACTIONS *oap;
	ASSOCELEM *aep;
	ASSOCELEM **ept;

	/*
	 * return information about memory footprint
	 *
	 * This is not the sizeof, see memsize() for that information.
	 * This is not the number of elements, see elm_count() for that info.
	 */
	i = j = 0;
	s = (long) sizeof(VALUE);
	if (vp->v_type > 0) {
		switch(vp->v_type) {
		case V_INT:
		case V_ADDR:
		case V_OCTET:
			break;
		case V_NUM:
			s = memqsize(vp->v_num);
			break;
		case V_COM:
			s = (long)sizeof(COMPLEX) +
				memqsize(vp->v_com->real) +
				memqsize(vp->v_com->imag);
			break;
		case V_STR:
			s = (long)sizeof(STRING) + vp->v_str->s_len + 1;
			break;
		case V_MAT:
			s = sizeof(MATRIX);
			i = vp->v_mat->m_size;
			p = vp->v_mat->m_table;
			while (i-- > 0)
				s += memsize(p++);
			break;
		case V_LIST:
			s = sizeof(LIST);
			for (ep = vp->v_list->l_first; ep; ep = ep->e_next) {
				s += sizeof(LISTELEM) - sizeof(VALUE) +
					memsize(&ep->e_value);
			}
			break;
		case V_ASSOC:
			s = sizeof(ASSOC);
			i = vp->v_assoc->a_size;
			ept = vp->v_assoc->a_table;
			while (i-- > 0) {
			    s += sizeof(ASSOCELEM *);
			    for (aep = *ept++; aep; aep = aep->e_next) {
				s += sizeof(ASSOCELEM) - sizeof(VALUE) +
					memsize(&aep->e_value);
				j = aep->e_dim;
				p = aep->e_indices;
				while (j-- > 0)
					s += memsize(p++);
				}
			}
			break;
		case V_OBJ:
			s = sizeof(OBJECT);
			oap = vp->v_obj->o_actions;
			s += (long)strlen(oap->name) + 1;
			i = oap->count;
			s += (i + 2) * sizeof(int);
			p = vp->v_obj->o_table;
			while (i-- > 0)
				s += memsize(p++);
			break;
		case V_FILE:
			s = sizeof(vp->v_file);
			break;
		case V_RAND:
			s = sizeof(RAND);
			break;
		case V_RANDOM:
			s = (long)sizeof(RANDOM) +
				memzsize(vp->v_random->n) +
				memzsize(vp->v_random->r);
			break;
		case V_CONFIG:
			s = (long)sizeof(CONFIG) + 2 +
				(long)strlen(vp->v_config->prompt1) +
				(long)strlen(vp->v_config->prompt2);
			break;
		case V_HASH:
			s = sizeof(HASH);
			break;
		case V_BLOCK:
			s = (long)sizeof(BLOCK) + vp->v_block->maxsize;
			break;
		case V_NBLOCK:
			s = (long)sizeof(NBLOCK) + (long)sizeof(BLOCK) +
				vp->v_nblock->blk->maxsize +
				(long)strlen(vp->v_nblock->name) + 1;
			break;
		default:
			math_error("memsize not defined for value type");
			/*NOTREACHED*/
		}
	}
	return s;
}
