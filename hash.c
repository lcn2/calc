/*
 * hash - one-way hash routines
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll
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
 * Under source code control:	1995/11/23 05:13:11
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "calc.h"
#include "alloc.h"
#include "value.h"
#include "zrand.h"
#include "zrandom.h"
#include "hash.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * external hash_setup functions
 */
E_FUNC void shs_init_state(HASH*);
E_FUNC void sha1_init_state(HASH*);
E_FUNC void MD5_init_state(HASH*);


/*
 * hash_long can deal with BOOL's, int's, FLAGS's and LEN's
 */
#define hash_bool(type, val, state) (hash_long((type), (long)(val), (state)))
#define hash_int(type, val, state) (hash_long((type), (long)(val), (state)))
#define hash_flag(type, val, state) (hash_long((type), (long)(val), (state)))
#define hash_len(type, val, state) (hash_long((type), (long)(val), (state)))


/*
 * hash_setup - setup the hash state for a given hash
 */
STATIC struct hash_setup {
	int type;		/* hash type (see XYZ_HASH_TYPE below) */
	void (*init_state)(HASH*);	/* initialize a hash state */
} htbl[] = {
	{ SHA1_HASH_TYPE, sha1_init_state },	/* SHA-1 / SHA-1 */
	{ -1, NULL }		/* must be last */
};


/*
 * hash_init - initialize a hash state
 *
 * given:
 *	type	- hash type (see hash.h)
 *	state	- the state to initialize, or NULL to malloc it
 *
 * returns:
 *	initialized state
 */
HASH *
hash_init(int type, HASH *state)
{
	int i;

	/*
	 * malloc if needed
	 */
	if (state == NULL) {
		state = (HASH *)malloc(sizeof(HASH));
		if (state == NULL) {
			math_error("hash_init: cannot malloc HASH");
			/*NOTREACHED*/
		}
	}

	/*
	 * clear hash value
	 */
	memset((void*)state, 0, sizeof(HASH));
	state->bytes = TRUE;

	/*
	 * search for the hash_setup function
	 */
	for (i=0; htbl[i].init_state != NULL; ++i) {

		/* if we found the state that we were looking for */
		if (type == htbl[i].type) {

			/* initialize state and return */
			(htbl[i].init_state)(state);

			/* firewall - MAX_CHUNKSIZE must be >= chunksize */
			if (state->chunksize > MAX_CHUNKSIZE) {
				math_error(
				  "internal error: MAX_CHUNKSIZE is too small");
				/*NOTREACHED*/
			}
			return state;
		}
	}

	/*
	 * no such hash state
	 */
	math_error("internal error: hash type not found in htbl[]");
	return NULL;
}


/*
 * hash_free - free the hash state
 */
void
hash_free(HASH *state)
{
	/*
	 * do nothing if state is NULL
	 */
	if (state == NULL) {
		return;
	}

	/*
	 * free main state and return
	 */
	free(state);
	return;
}


/*
 * hash_copy - copy a hash state
 *
 * given:
 *	state	- the state to copy
 *
 * returns:
 *	pointer to copy of state
 */
HASH *
hash_copy(HASH *state)
{
	HASH *hnew;		/* copy of state */

	/*
	 * malloc new state
	 */
	hnew = (HASH *)malloc(sizeof(HASH));
	if (hnew == NULL) {
		math_error("hash_init: cannot malloc HASH");
		/*NOTREACHED*/
	}

	/*
	 * duplicate state
	 */
	memcpy((void *)hnew, (void *)state, sizeof(HASH));
	return hnew;
}


/*
 * hash_cmp - compare hash values
 *
 * given:
 *	a	first hash state
 *	b	second hash state
 *
 * returns:
 *	TRUE => hash states are different
 *	FALSE => hash states are the same
 */
int
hash_cmp(HASH *a, HASH *b)
{
	/*
	 * firewall and quick check
	 */
	if (a == b) {
		/* pointers to the same object */
		return FALSE;
	}
	if (a == NULL || b == NULL) {
		/* one pointer is NULL, so they differ */
		return TRUE;
	}
	if (a->cmp == NULL || b->cmp == NULL) {
		/* one cmp function is NULL, so they differ */
		return TRUE;
	}

	/*
	 * compare hash types
	 */
	if (a->hashtype != b->hashtype) {
		/* different hash types are different */
		return TRUE;
	}

	/*
	 * perform the hash specific comparison
	 */
	return ((a->cmp)(a,b));
}


/*
 * hash_print - print the name and value of a hash
 *
 * given:
 *	state	the hash state to print name and value of
 */
void
hash_print(HASH *state)
{
	/* print the hash */
	(state->print)(state);
	return;
}


/*
 * hash_final - finalize the state of a hash and return a ZVALUE
 *
 * given:
 *	state	the hash state to finalize
 *
 * returns:
 *	hash state as a ZVALUE
 */
ZVALUE
hash_final(HASH *state)
{
	/* return the finalized the hash value */
	return (state->final)(state);
}


/*
 * hash_long - note a long value
 *
 * given:
 *	type	- hash type (see hash.h)
 *	longval - a long value
 *	state	- the state to hash
 *
 * returns:
 *	the new state
 *
 * This function will hash a long value as if it were a 64 bit value.
 * The input is a long.	 If a long is smaller than 64 bits, we will
 * hash a final 32 bits of zeros.
 *
 * This function is OK to hash BOOL's, unsigned long's, unsigned int's
 * signed int's as well as FLAG's and LEN's.
 */
HASH *
hash_long(int type, long longval, HASH *state)
{
	long lval[64/LONG_BITS];	/* 64 bits of longs */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the hash_long
	 */
	(state->chkpt)(state);
	state->bytes = FALSE;	/* data to be read as words */

	/*
	 * catch the zero numeric value special case
	 */
	if (longval == 0) {
		/* note a zero numeric value and return */
		(state->note)(HASH_ZERO(state->base), state);
		return state;
	}

	/*
	 * prep for a long value hash
	 */
	(state->note)(state->base, state);

	/*
	 * hash as if we have a 64 bit value
	 */
	memset((char *)lval, 0, sizeof(lval));
	lval[0] = longval;
	(state->update)(state, (USB8 *)lval, sizeof(lval));

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_zvalue - hash a ZVALUE
 *
 * given:
 *	type	- hash type (see hash.h)
 *	zval	- the ZVALUE
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_zvalue(int type, ZVALUE zval, HASH *state)
{

#if CALC_BYTE_ORDER == BIG_ENDIAN && BASEB == 16
	int full_lim;		/* HALFs in whole chunks in zval */
	int chunkhalf;		/* size of half buffer in HALFs */
	int i;
	int j;
#endif
#if BASEB == 16
	HALF half[MAX_CHUNKSIZE];	/* For endian reversal */
#endif

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the ZVALUE hash
	 */
	(state->chkpt)(state);
	state->bytes = FALSE;	/* data to be read as words */

	/*
	 * catch the zero numeric value special case
	 */
	if (ziszero(zval)) {
		/* note a zero numeric value and return */
		(state->note)(HASH_ZERO(state->base), state);
		return state;
	}

	/*
	 * prep for a ZVALUE hash
	 */
	(state->note)(HASH_ZVALUE(state->base), state);
	/* note if we have a negative value */
	if (zisneg(zval)) {
		(state->note)(HASH_NEG(state->base), state);
	}

#if CALC_BYTE_ORDER == BIG_ENDIAN && BASEB == 16

	/*
	 * hash full chunks
	 *
	 * We need to convert the array of HALFs into canonical architectural
	 * independent form -- 32 bit arrays.  Because we have 16 bit values
	 * in Big Endian form, we need to swap 16 bit values so that they
	 * appear as 32 bit Big Endian values.
	 */
	chunkhalf = state->chunksize/sizeof(HALF);
	full_lim = (zval.len / chunkhalf) * chunkhalf;
	for (i=0; i < full_lim; i += chunkhalf) {
		/* HALF swap copy a chunk into a data buffer */
		for (j=0; j < chunkhalf; j += 2) {
			half[j] = zval.v[i+j+1];
			half[j+1] = zval.v[i+j];
		}
		(state->update)(state, (USB8*) half, state->chunksize);
	}

	/*
	 * hash the final partial chunk (if any)
	 *
	 * We need to convert the array of HALFs into canonical architectural
	 * independent form -- 32 bit arrays.  Because we have 16 bit values
	 * in Big Endian form, we need to swap 16 bit values so that they
	 * appear as 32 bit Big Endian values.
	 */
	if (zval.len > full_lim) {
		for (j=0; j < zval.len-full_lim-1; j += 2) {
			half[j] = zval.v[full_lim+j+1];
			half[j+1] = zval.v[full_lim+j];
		}
		if (j < zval.len-full_lim) {
			half[j] = (HALF)0;
			half[j+1] = zval.v[zval.len-1];
			--full_lim;
		}
		(state->update)(state, (USB8 *) half,
			  (zval.len-full_lim)*sizeof(HALF));
	}

#else

	/*
	 * hash the array of HALFs
	 *
	 * The array of HALFs is equivalent to the canonical architectural
	 * independent form.  We either have 32 bit HALFs (in which case
	 * we do not case the byte order) or we have 16 bit HALFs in Little
	 * Endian order (which happens to be laid out in the same order as
	 * 32 bit values).
	 */
	(state->update)(state, (USB8 *)zval.v, zval.len*sizeof(HALF));

#if BASEB == 16
	if (zval.len & 1) {		/* padding to complete word */
		half[0] = 0;
		(state->update)(state, (USB8 *) half, 2);
	}
#endif

#endif

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_number - hash a NUMBER
 *
 * given:
 *	type	- hash type (see hash.h)
 *	n	- the NUMBER
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_number(int type, void *n, HASH *state)
{
	NUMBER *number = (NUMBER *)n;	/* n as a NUMBER pointer */
	BOOL sign;			/* sign of the denominator */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the NUMBER hash
	 */
	(state->chkpt)(state);
	state->bytes = FALSE;

	/*
	 * process the numerator
	 */
	state = hash_zvalue(type, number->num, state);

	/*
	 * if the NUMBER is not an integer, process the denominator
	 */
	if (qisfrac(number)) {

		/* note the division */
		(state->note)(HASH_DIV(state->base), state);

		/* hash denominator as positive -- just in case */
		sign = number->den.sign;
		number->den.sign = 0;

		/* hash the denominator */
		state = hash_zvalue(type, number->den, state);

		/* restore the sign */
		number->den.sign = sign;
	}

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_complex - hash a COMPLEX
 *
 * given:
 *	type	- hash type (see hash.h)
 *	c	- the COMPLEX
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_complex(int type, void *c, HASH *state)
{
	COMPLEX *complex = (COMPLEX *)c;	/* c as a COMPLEX pointer */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the COMPLEX hash
	 */
	(state->chkpt)(state);
	state->bytes = FALSE;

	/*
	 * catch the zero special case
	 */
	if (ciszero(complex)) {
		/* note a zero numeric value and return */
		(state->note)(HASH_ZERO(state->base), state);
		return state;
	}

	/*
	 * process the real value if not pure imaginary
	 *
	 * We will ignore the real part if the value is of the form 0+xi.
	 */
	if (!qiszero(complex->real)) {
		state = hash_number(type, complex->real, state);
	}

	/*
	 * if the NUMBER is not real, process the imaginary value
	 *
	 * We will ignore the imaginary part of the value is of the form x+0i.
	 */
	if (!cisreal(complex)) {

		/* note the sqrt(-1) */
		(state->note)(HASH_COMPLEX(state->base), state);

		/* hash the imaginary value */
		state = hash_number(type, complex->imag, state);
	}

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_str - hash a null-terminated string
 *
 * given:
 *	type	- hash type (see hash.h)
 *	str	- the string
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_str(int type, char *str, HASH *state)
{
	size_t len;		/* string length */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the string hash
	 */
	if (!state->bytes) {
		(state->chkpt)(state);
		state->bytes = TRUE;
	}

	len = strlen(str);

	/*
	 * hash the string
	 */
	(state->update)(state, (USB8*)str, len);

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_STR - hash a STRING
 *
 * given:
 *	type	- hash type (see hash.h)
 *	str	- the STRING
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_STR(int type, STRING *str, HASH *state)
{
	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the string hash
	 */
	if (!state->bytes) {
		(state->chkpt)(state);
		state->bytes = TRUE;
	}

	/*
	 * hash the string
	 */
	(state->update)(state, (USB8*) str->s_str, (USB32) str->s_len);

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_usb8 - hash an array of USB8s
 *
 * given:
 *	type	- hash type (see hash.h)
 *	byte	- pointer to an array of USB8s
 *	len	- number of USB8s to hash
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_usb8(int type, USB8 *byte, int len, HASH *state)
{
	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * setup for the string hash
	 */
	if (!state->bytes) {
		(state->chkpt)(state);
		state->bytes = TRUE;
	}

	/*
	 * hash the array of octets
	 */
	(state->update)(state, byte, (USB32)len);

	/*
	 * all done
	 */
	return state;
}


/*
 * hash_value - hash a value
 *
 * given:
 *	type	- hash type (see hash.h)
 *	v	- the value
 *	state	- the state to hash or NULL
 *
 * returns:
 *	the new state
 */
HASH *
hash_value(int type, void *v, HASH *state)
{
	LISTELEM *ep;		/* list element pointer */
	ASSOCELEM **assochead;	/* association chain head */
	ASSOCELEM *aep;		/* current association value */
	ASSOCELEM *nextaep;	/* next association value */
	VALUE *value = (VALUE *)v;	/* v cast to a VALUE */
	VALUE *vp;		/* pointer to next OBJ table value */
	ZVALUE fileval;		/* size, position, dev, inode of a file */
	int i;

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = hash_init(type, NULL);
	}

	/*
	 * process the value type
	 */
	switch (value->v_type) {
	case V_NULL:
		(state->chkpt)(state);
		state->bytes = TRUE;
		break;

	case V_INT:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash as if we have a 64 bit value */
		state = hash_int(type, value->v_int, state);
		break;

	case V_NUM:
		/* hash this type */
		state = hash_number(type, value->v_num, state);
		break;

	case V_COM:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash this type */
		state = hash_complex(type, value->v_com, state);
		break;

	case V_ADDR:
		/* there is nothing to setup, simply hash what we point at */
		state = hash_value(type, value->v_addr, state);
		break;

	case V_STR:
		/* strings have no setup */

		/* hash this type */
		state = hash_STR(type, value->v_str, state);
		break;

	case V_MAT:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);
		state->bytes = TRUE;

		/* hash all the elements of the matrix */
		for (i=0; i < value->v_mat->m_size; ++i) {

			/* hash the next matrix value */
			state = hash_value(type,
					value->v_mat->m_table+i, state);
			state->bytes = FALSE;	/* as if reading words */
		}
		break;

	case V_LIST:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash all the elements of the list */
		for (i=0, ep = value->v_list->l_first;
		     ep != NULL && i < value->v_list->l_count;
		     ++i, ep = ep->e_next) {

			/* hash the next list value */
			state = hash_value(type, &ep->e_value, state);
			state->bytes = FALSE;	/* as if reading words */
		}
		break;

	case V_ASSOC:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);
		state->bytes = TRUE;

		/* hash the association */
		assochead = value->v_assoc->a_table;
		for (i = 0; i < value->v_assoc->a_size; i++) {
			nextaep = *assochead;
			while (nextaep) {
				aep = nextaep;
				nextaep = aep->e_next;

				/* hash the next association value */
				state = hash_value(type, &aep->e_value, state);
				state->bytes = FALSE; /* as if reading words */
			}
			assochead++;
		}
		break;

	case V_OBJ:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);
		state->bytes = TRUE;	/* reading bytes */

		/* hash the object name and then the element values */

		state = hash_str(type, objtypename(
			value->v_obj->o_actions->oa_index), state);
		(state->chkpt)(state);

		for (i=value->v_obj->o_actions->oa_count,
		     vp=value->v_obj->o_table;
		     i-- > 0;
		     vp++) {

			/* hash the next object value */
			state = hash_value(type, vp, state);
			state->bytes = FALSE;	/* as if reading words */
		}
		break;

	case V_FILE:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash file length if possible */
		if (getsize(value->v_file, &fileval) == 0) {
			state = hash_zvalue(type, fileval, state);
			zfree(fileval);
		} else {
			/* hash -1 for invalid length */
			state = hash_long(type, (long)-1, state);
		}
		/* hash the file position if possible */
		if (getloc(value->v_file, &fileval) == 0) {
			state = hash_zvalue(type, fileval, state);
			zfree(fileval);
		} else {
			/* hash -1 for invalid location */
			state = hash_long(type, (long)-1, state);
		}
		/* hash the file device if possible */
		if (get_device(value->v_file, &fileval) == 0) {
			state = hash_zvalue(type, fileval, state);
			zfree(fileval);
		} else {
			/* hash -1 for invalid device */
			state = hash_long(type, (long)-1, state);
		}
		/* hash the file inode if possible */
		if (get_inode(value->v_file, &fileval) == 0) {
			state = hash_zvalue(type, fileval, state);
			zfree(fileval);
		} else {
			/* hash -1 for invalid inode */
			state = hash_long(type, (long)-1, state);
		}
		break;

	case V_RAND:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash the RAND state */
		state = hash_int(type, value->v_rand->seeded, state);
		state = hash_int(type, value->v_rand->bits, state);
		(state->update)(state,
			(USB8 *)value->v_rand->buffer, SLEN*FULL_BITS/8);
		state = hash_int(type, value->v_rand->j, state);
		state = hash_int(type, value->v_rand->k, state);
		state = hash_int(type, value->v_rand->need_to_skip, state);
		(state->update)(state,
			(USB8 *)value->v_rand->slot, SCNT*FULL_BITS/8);
		(state->update)(state,
			(USB8*)value->v_rand->shuf, SHUFLEN*FULL_BITS/8);
		state->bytes = FALSE;	/* as if reading words */
		break;

	case V_RANDOM:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash the RANDOM state */
		state = hash_int(type, value->v_random->seeded, state);
		state = hash_int(type, value->v_random->bits, state);
		(state->update)(state,
			(USB8 *)&(value->v_random->buffer), BASEB/8);
		state = hash_zvalue(type, value->v_random->r, state);
		state = hash_zvalue(type, value->v_random->n, state);
		state->bytes = FALSE;	/* as if reading words */
		break;

	case V_CONFIG:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash the CONFIG state */
		state = hash_int(type, value->v_config->outmode, state);
		state = hash_int(type, value->v_config->outmode2, state);
		state = hash_long(type,(long)value->v_config->outdigits, state);
		state = hash_number(type, value->v_config->epsilon, state);
		state = hash_long(type,
			(long)value->v_config->epsilonprec, state);
		state = hash_flag(type, value->v_config->traceflags, state);
		state = hash_long(type, (long)value->v_config->maxprint, state);
		state = hash_len(type, value->v_config->mul2, state);
		state = hash_len(type, value->v_config->sq2, state);
		state = hash_len(type, value->v_config->pow2, state);
		state = hash_len(type, value->v_config->redc2, state);
		state = hash_bool(type, value->v_config->tilde_ok, state);
		state = hash_bool(type, value->v_config->tab_ok, state);
		state = hash_long(type, (long)value->v_config->quomod, state);
		state = hash_long(type, (long)value->v_config->quo, state);
		state = hash_long(type, (long)value->v_config->mod, state);
		state = hash_long(type, (long)value->v_config->sqrt, state);
		state = hash_long(type, (long)value->v_config->appr, state);
		state = hash_long(type, (long)value->v_config->cfappr, state);
		state = hash_long(type, (long)value->v_config->cfsim, state);
		state = hash_long(type, (long)value->v_config->outround, state);
		state = hash_long(type, (long)value->v_config->round, state);
		state = hash_bool(type, value->v_config->leadzero, state);
		state = hash_bool(type, value->v_config->fullzero, state);
		state = hash_long(type,
			(long)value->v_config->maxscancount, state);
		state = hash_str(type, value->v_config->prompt1, state);
		state->bytes = FALSE;	/* as if just read words */
		state = hash_str(type, value->v_config->prompt2, state);
		state->bytes = FALSE;	/* as if just read words */
		state = hash_int(type, value->v_config->blkmaxprint, state);
		state = hash_bool(type, value->v_config->blkverbose, state);
		state = hash_int(type, value->v_config->blkbase, state);
		state = hash_int(type, value->v_config->blkfmt, state);
		state = hash_long(type,
			(long)value->v_config->resource_debug, state);
		state = hash_long(type,
			(long)value->v_config->calc_debug, state);
		state = hash_long(type,
			(long)value->v_config->user_debug, state);
		state = hash_bool(type, value->v_config->verbose_quit, state);
		state = hash_int(type, value->v_config->ctrl_d, state);
		state = hash_str(type, value->v_config->program, state);
		state = hash_str(type, value->v_config->base_name, state);
		state = hash_bool(type, value->v_config->windows, state);
		state = hash_bool(type, value->v_config->cygwin, state);
		state = hash_bool(type, value->v_config->compile_custom, state);
		if (value->v_config->allow_custom != NULL &&
		    *(value->v_config->allow_custom)) {
			state = hash_bool(type, TRUE, state);
		} else {
			state = hash_bool(type, FALSE, state);
		}
		state = hash_str(type, value->v_config->version, state);
		state = hash_int(type, value->v_config->baseb, state);
		state = hash_bool(type, value->v_config->redecl_warn, state);
		state = hash_bool(type, value->v_config->dupvar_warn, state);
		break;

	case V_HASH:
		/* setup for the this value type */
		(state->chkpt)(state);
		(state->type)(value->v_type, state);

		/* hash the HASH state */
		state = hash_int(type, value->v_hash->type, state);
		state = hash_bool(type, value->v_hash->bytes,state);
		state = hash_int(type, value->v_hash->base, state);
		state = hash_int(type, value->v_hash->chunksize, state);
		state = hash_int(type, value->v_hash->unionsize, state);
		(state->update)(state,
		    value->v_hash->h_union.data, state->unionsize);
		state->bytes = FALSE;	/* as if reading words */
		break;

	case V_BLOCK:
		/* there is no setup for a BLOCK */

		/* hash the octets in the BLOCK */
		if (value->v_block->datalen > 0) {
			state = hash_usb8(type, value->v_block->data,
					   value->v_block->datalen, state);
		}
		break;

	case V_OCTET:
		/* there is no setup for an OCTET */

		/* hash the OCTET */
		state = hash_usb8(type, value->v_octet, 1, state);
		break;

	case V_NBLOCK:
		/* there is no setup for a NBLOCK */

		/* hash the octets in the NBLOCK */
		if (value->v_nblock->blk->datalen > 0) {
			state = hash_usb8(type, value->v_nblock->blk->data,
					   value->v_nblock->blk->datalen,
					   state);
		}
		break;

	default:
		math_error("hashing an unknown value");
		/*NOTREACHED*/
	}
	return state;
}
