/*
 * quickhash - quickly hash a calc value using a 32-bit FNV-0 hash
 *
 * WARNING: General use of FNV-0 hash is not recommended.  Calc uses FNV-0
 *	    for a specific You should use FNV-1a hash instead.  See:
 *
 *	http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll
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
 * Under source code control:	1995/03/04 11:34:23
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * NOTE: This file does not contain a hash interface.  It is used by
 *	 associative arrays and other internal processes.
 */


#include "value.h"
#include "zrand.h"
#include "zrandom.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * forward declarations
 */
S_FUNC QCKHASH assochash(ASSOC *ap, QCKHASH val);
S_FUNC QCKHASH listhash(LIST *lp, QCKHASH val);
S_FUNC QCKHASH mathash(MATRIX *m, QCKHASH val);
S_FUNC QCKHASH objhash(OBJECT *op, QCKHASH val);
S_FUNC QCKHASH randhash(RAND *r, QCKHASH val);
S_FUNC QCKHASH randomhash(RANDOM *state, QCKHASH val);
S_FUNC QCKHASH config_hash(CONFIG *cfg, QCKHASH val);
S_FUNC QCKHASH fnv_strhash(char *ch, QCKHASH val);
S_FUNC QCKHASH fnv_STRhash(STRING *str, QCKHASH val);
S_FUNC QCKHASH fnv_fullhash(FULL *v, LEN len, QCKHASH val);
S_FUNC QCKHASH fnv_zhash(ZVALUE z, QCKHASH val);
S_FUNC QCKHASH hash_hash(HASH *hash, QCKHASH val);
S_FUNC QCKHASH blk_hash(BLOCK *blk, QCKHASH val);


/*
 * quasi_fnv - quasi Fowler/Noll/Vo-0 32 bit hash
 *
 * NOTE: General use of FNV-0 hadh
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *	Phong Vo (http://www.research.att.com/info/kpv/)
 *	Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *	Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.	 Some people tried this hash
 * and found that it worked rather well.  In an Email message
 * to Landon, they named it ``Fowler/Noll/Vo'' or the FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *	http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 * given:
 *	x	the value to hash (must not be longer than 32 bits)
 *	val	previous QCKHASH value
 *
 * returns:
 *	the next 32 bit QCKHASH
 *
 * Example:
 * 	QCKHASH val;
 * 	int x;
 *
 * 	quasi_fnv(x, val);
 *
 * NOTE: The (x) argument may be an expression such as something with
 * 	 a ++ or --.  The macro must only use (x) once.
 *
 * NOTE: The (val) argument just be a lvalue / something to which
 * 	 a value can be assigned.
 *
 * The careful observer will note that (x) need not be a simple
 * octet.  This is not a bug, but a feature.  The FNV hash was
 * designed to operate on octets, not abstract objects such
 * as associations, file descriptors and PRNG states.
 * You will also notice that we sometimes add values directly
 * to the (val) hash state.  This is a simulation of hashing
 * a variable type.
 *
 * The 32-bit FNV-0 hash does a very good job in producing
 * a 32 bit hash arrays of octets in a short amount of time.
 * It is not bad for hashing calc data as well.	 So doing a
 * quick and dirty job of hashing on a part of a calc value
 * is all that calc really needs.
 *
 * The core of the of the FNV hash has been adopted as the calc
 * quick hash with the provision that it operates on 32 bit
 * objects instead of octets.  For calc's internal purposes,
 * this is sufficient.  For general FNV hashing, this is not
 * recommended.
 *
 * It has been observed that gcc, when using -O, -O2, -O3 or
 * better on an AMD or AMD-like processor (such as i686) will
 * produce slightly faster code when using the shift/add
 * expression than when performing a multiply with a constant.
 */
#if defined(NO_HASH_CPU_OPTIMIZATION)
#define quasi_fnv(x,val) \
    ((val) = (((QCKHASH)(val)*(QCKHASH)16777619) ^ ((QCKHASH)(x))))
#else
#define quasi_fnv(x,val) ( \
    ((val) += (((QCKHASH)(val)<<1) + ((QCKHASH)(val)<<4) + \
	       ((QCKHASH)(val)<<7) + ((QCKHASH)(val)<<8) + \
	       ((QCKHASH)(val)<<24))), \
    ((val) ^= (QCKHASH)(x)) \
)
#endif


/*
 * fnv_qhash - compute the next 32-bit FNV-0 hash given a NUMBER
 *
 * given:
 *	q	pointer to a NUMBER
 *	val	previous QCKHASH value
 *
 * returns:
 *	the next 32 bit QCKHASH
 */
#define fnv_qhash(q,val) \
  (qisint(q) ? fnv_zhash((q)->num, (val)) : \
   fnv_zhash((q)->num, fnv_zhash((q)->den, (val))))


/*
 * fnv_chash - compute the next 32-bit FNV-0 hash given a COMPLEX
 *
 * given:
 *	c	pointer to a COMPLEX
 *	val	previous QCKHASH value
 *
 * returns:
 *	the next 32 bit QCKHASH
 */
#define fnv_chash(c,val) \
  (cisreal(c) ? fnv_qhash((c)->real, (val)) : \
   fnv_qhash((c)->real, fnv_qhash((c)->imag, (val))))


/*
 * hashvalue - calculate a hash value for a value.
 *
 * The hash does not have to be a perfect one, it is only used for
 * making associations faster.
 *
 * given:
 *	vp	pointer to a VALUE
 *	val	previous QCKHASH value
 *
 * returns:
 *	next QCKHASH value
 */
QCKHASH
hashvalue(VALUE *vp, QCKHASH val)
{
	switch (vp->v_type) {
	case V_INT:
		val += V_NUM;
		return quasi_fnv(vp->v_int, val);
	case V_NUM:
		return fnv_qhash(vp->v_num, val);
	case V_COM:
		return fnv_chash(vp->v_com, val);
	case V_STR:
		return fnv_STRhash(vp->v_str, val);
	case V_NULL:
		return val;
	case V_OBJ:
		return objhash(vp->v_obj, val);
	case V_LIST:
		return listhash(vp->v_list, val);
	case V_ASSOC:
		return assochash(vp->v_assoc, val);
	case V_MAT:
		return mathash(vp->v_mat, val);
	case V_FILE:
		val += V_FILE;
		return quasi_fnv(vp->v_file, val);
	case V_RAND:
		return randhash(vp->v_rand, val);
	case V_RANDOM:
		return randomhash(vp->v_random, val);
	case V_CONFIG:
		return config_hash(vp->v_config, val);
	case V_HASH:
		return hash_hash(vp->v_hash, val);
	case V_BLOCK:
		return blk_hash(vp->v_block, val);
	case V_OCTET:
		val += V_OCTET;
		return quasi_fnv((int)*vp->v_octet, val);
	case V_NBLOCK:
		return blk_hash(vp->v_nblock->blk, val);
	default:
		math_error("Hashing unknown value");
		/*NOTREACHED*/
	}
	return (QCKHASH)0;
}


/*
 * Return a trivial hash value for an association.
 */
S_FUNC QCKHASH
assochash(ASSOC *ap, QCKHASH val)
{
	/*
	 * XXX - maybe we should hash the first few and last few values???
	 *	 Perhaps we should hash associations in a different but
	 *	 fast way?
	 */
        val += V_ASSOC;
	return quasi_fnv(ap->a_count, val);
}


/*
 * Return a trivial hash value for a list.
 */
S_FUNC QCKHASH
listhash(LIST *lp, QCKHASH val)
{
	/*
	 * hash small lists
	 */
	switch (lp->l_count) {
	case 0:
		/* empty list hashes to just V_LIST */
		return V_LIST+val;
	case 1:
		/* single element list hashes just that element */
		return hashvalue(&lp->l_first->e_value, V_LIST+val);
	}

	/*
	 * multi element list hashes the first and last elements
	 */
	return hashvalue(&lp->l_first->e_value,
			 hashvalue(&lp->l_last->e_value, V_LIST+val));
}


/*
 * Return a trivial hash value for a matrix.
 */
S_FUNC QCKHASH
mathash(MATRIX *m, QCKHASH val)
{
	long skip;
	long i;
	VALUE *vp;

	/*
	 * hash size parts of the matrix
	 */
	val += V_MAT;
	quasi_fnv(m->m_dim, val);
	quasi_fnv(m->m_size, val);

	/*
	 * hash the matrix index bounds
	 */
	for (i = m->m_dim - 1; i >= 0; i--) {
		quasi_fnv(m->m_min[i], val);
		quasi_fnv(m->m_max[i], val);
	}

	/*
	 * hash the first 16 elements
	 */
	vp = m->m_table;
	for (i = 0; ((i < m->m_size) && (i < 16)); i++) {
		val = hashvalue(vp++, val);
	}

	/*
	 * hash 10 more elements if they exist
	 */
	i = 16;
	if (i < m->m_size) {
		vp = (VALUE *)&m->m_table[i];
		skip = (m->m_size / 11) + 1;
		while (i < m->m_size) {
			val = hashvalue(vp, val);
			i += skip;
			vp += skip;
		}
	}
	return val;
}


/*
 * Return a trivial hash value for an object.
 */
S_FUNC QCKHASH
objhash(OBJECT *op, QCKHASH val)
{
	int i;

	quasi_fnv(op->o_actions->oa_index, val);

	i = op->o_actions->oa_count;
	while (--i >= 0)
		val = hashvalue(&op->o_table[i], val);
	return val;
}


/*
 * randhash - return a trivial hash for an s100 state
 *
 * given:
 *	state - state to hash
 *
 * returns:
 *	trivial hash integer
 */
S_FUNC QCKHASH
randhash(RAND *r, QCKHASH val)
{
	/*
	 * hash the RAND state
	 */
	if (!r->seeded) {
		/* unseeded state hashes to V_RAND */
		return V_RAND+val;
	} else {
		/* hash control values */
	    	val += V_RAND;
		quasi_fnv(r->j, val);
		quasi_fnv(r->k, val);
		quasi_fnv(r->bits, val);
		quasi_fnv(r->need_to_skip, val);

		/* hash the state arrays */
		return fnv_fullhash(&r->buffer[0], SLEN+SCNT+SHUFLEN, val);
	}
}


/*
 * randomhash - return a trivial hash for a Blum state
 *
 * given:
 *	state - state to hash
 *
 * returns:
 *	trivial hash integer
 */
S_FUNC QCKHASH
randomhash(RANDOM *state, QCKHASH val)
{
	/*
	 * unseeded RANDOM state hashes to V_RANDOM
	 */
	if (!state->seeded) {
		return V_RANDOM+val;
	}

	/*
	 * hash a seeded RANDOM state
	 */
	val += V_RANDOM;
	quasi_fnv(state->buffer+state->bits, val);
	if (state->r.v != NULL) {
		val = fnv_zhash(state->r, val);
	}
	if (state->n.v != NULL) {
		val = fnv_zhash(state->n, val);
	}
	return val;
}


/*
 * config_hash - return a trivial hash for a configuration state
 */
S_FUNC QCKHASH
config_hash(CONFIG *cfg, QCKHASH val)
{
	USB32 value;		/* value to hash from hash elements */

	/*
	 * build up a scalar value
	 *
	 * We will rotate a value left 5 bits and xor in each scalar element
	 */
	value = cfg->outmode;
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->outmode);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->outmode2);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->outdigits);
	/* epsilon is handled out of order */
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->epsilonprec);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->traceflags);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->maxprint);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->mul2);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->sq2);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->pow2);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->redc2);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->tilde_ok);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->tab_ok);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->quomod);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->quo);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->mod);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->sqrt);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->appr);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->cfappr);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->cfsim);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->outround);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->round);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->leadzero);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->fullzero);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->maxscancount);
	/* prompt1 is handled out of order */
	/* prompt2 is handled out of order */
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkmaxprint);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkverbose);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkbase);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkfmt);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->calc_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->resource_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->user_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->verbose_quit);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->ctrl_d);
	/* program is handled out of order */
	/* basename is handled out of order */
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->windows);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->cygwin);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->compile_custom);
	if (cfg->allow_custom != NULL && *(cfg->allow_custom)) {
		value = (((value>>5) | (value<<27)) ^ (USB32)TRUE);
	} else {
		value = (((value>>5) | (value<<27)) ^ (USB32)FALSE);
	}
	/* version is handled out of order */

	/*
	 * hash the built up scalar
	 */
	val += V_CONFIG;
	quasi_fnv(value, val);

	/*
	 * hash the strings and pointers if possible
	 */
	if (cfg->prompt1) {
		val = fnv_strhash(cfg->prompt1, val);
	}
	if (cfg->prompt2) {
		val = fnv_strhash(cfg->prompt2, val);
	}
	if (cfg->program) {
		val = fnv_strhash(cfg->program, val);
	}
	if (cfg->base_name) {
		val = fnv_strhash(cfg->base_name, val);
	}
	if (cfg->version) {
		val = fnv_strhash(cfg->version, val);
	}
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->baseb);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->redecl_warn);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->dupvar_warn);

	/*
	 * hash the epsilon if possible
	 */
	if (cfg->epsilon) {
		val = fnv_qhash(cfg->epsilon, val);
	}
	return val;
}


/*
 * fnv_strhash - Fowler/Noll/Vo 32 bit hash of a null-terminated string
 *
 * given:
 *	ch	the start of the string to hash
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
fnv_strhash(char *ch, QCKHASH val)
{
	/*
	 * hash each character in the string
	 */
	while (*ch) {
		quasi_fnv(*ch++, val);
	}
	return val;
}

/*
 * fnv_STRhash - Fowler/Noll/Vo 32 bit hash of a STRING
 *
 * given:
 *	str	the string to hash
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
fnv_STRhash(STRING *str, QCKHASH val)
{
	char *ch;
	long n;

	ch = str->s_str;
	n = str->s_len;

	/*
	 * hash each character in the string
	 */
	while (n-- > 0) {
		quasi_fnv(*ch++, val);
	}
	return val;
}


/*
 * fnv_fullhash - Fowler/Noll/Vo 32 bit hash of an array of HALFs
 *
 * given:
 *	v	an array of FULLs
 *	len	length of buffer FULLs
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
fnv_fullhash(FULL *v, LEN len, QCKHASH val)
{
	/*
	 * hash each character in the string
	 */
	while (len-- > 0) {
		quasi_fnv(*v++, val);
	}
	return val;
}


/*
 * fnv_zhash - Fowler/Noll/Vo 32 bit hash of ZVALUE
 *
 * given:
 *	z	a ZVALUE
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
fnv_zhash(ZVALUE z, QCKHASH val)
{
	LEN n;
	HALF *hp;
#if BASEB == 16
	FULL f;
#endif

	/*
	 * hash the sign
	 */
	val += V_NUM;
	quasi_fnv(z.sign, val);

	n = z.len;
	hp = z.v;

#if BASEB == 16
	while (n > 1) {
		f = (FULL) *hp++;
		f |= (FULL) *hp++ << BASEB;
		quasi_fnv(f, val);
		n -= 2;
	}
	if (n) {
		quasi_fnv(*hp, val);
	}
#else
	while (n--  > 0) {
		quasi_fnv(*hp, val);
		++hp;
	}
#endif
	return val;
}


/*
 * hash_hash - Fowler/Noll/Vo 32 bit hash of a block
 *
 * given:
 *	hash	the HASH to quickhash
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
hash_hash(HASH *hash, QCKHASH val)
{
	int i;

	/*
	 * hash each USB8 in the BLOCK
	 */
	for (i=0; i < hash->unionsize; ++i) {
		quasi_fnv(hash->h_union.data[i], val);
	}
	return val;
}


/*
 * blk_hash - Fowler/Noll/Vo 32 bit hash of a block
 *
 * given:
 *	blk	the BLOCK to hash
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
S_FUNC QCKHASH
blk_hash(BLOCK *blk, QCKHASH val)
{
	int i;

	if (blk == NULL)		/* block has no data */
		return val;

	/*
	 * hash each USB8 in the BLOCK
	 */
	if (blk->datalen > 0) {
		for (i=0; i < blk->datalen; ++i) {
			quasi_fnv(blk->data[i], val);
		}
	}
	return val;
}
