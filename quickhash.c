/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *      Landon Curt Noll
 *      http://reality.sgi.com/chongo
 *
 *      chongo <was here> /\../\
 */

/*
 * quickhash - quickly hash a calc value using a partial Fowler/Noll/Vo hash
 *
 * NOTE: This file does not contain a hash interface.  It is used by
 *	 associative arrays and other internal processes.
 *
 * We will compute a hash value for any type of calc value
 * for use in associative arrays and the hash() builtin.
 * Hash speed is of primary importance to make associative
 * arrays work at a reasonable speed.  For this reason, we
 * cut corners by hashing only a small part of a calc value.
 *
 * The Fowler/Noll/Vo hash does a very good job in producing
 * a 32 bit hash from ASCII strings in a short amount of time.
 * It is not bad for hashing calc data as well.  So doing a
 * quick and dirty job of hashing on a part of a calc value,
 * combined with using a reasonable hash function will result
 * acceptable associative array performance.
 *
 * See:
 *	http://reality.sgi.com/chongo/src/fnv/fnv_hash.tar.gz
 *	http://reality.sgi.com/chongo/src/fnv/h32.c
 *	http://reality.sgi.com/chongo/src/fnv/h64.c
 *
 * for information on 32bit and 64bit Fowler/Noll/Vo hashs.
 */

#include "value.h"
#include "zrand.h"
#include "zrandom.h"

/*
 * forward declarations
 */
static QCKHASH assochash(ASSOC *ap, QCKHASH val);
static QCKHASH listhash(LIST *lp, QCKHASH val);
static QCKHASH mathash(MATRIX *m, QCKHASH val);
static QCKHASH objhash(OBJECT *op, QCKHASH val);
static QCKHASH randhash(RAND *r, QCKHASH val);
static QCKHASH randomhash(RANDOM *state, QCKHASH val);
static QCKHASH config_hash(CONFIG *cfg, QCKHASH val);
static QCKHASH fnv_strhash(char *ch, QCKHASH val);
static QCKHASH fnv_STRhash(STRING *str, QCKHASH val);
static QCKHASH fnv_fullhash(FULL *v, LEN len, QCKHASH val);
static QCKHASH fnv_zhash(ZVALUE z, QCKHASH val);
static QCKHASH hash_hash(HASH *hash, QCKHASH val);
static QCKHASH blk_hash(BLOCK *blk, QCKHASH val);


/*
 * FNV-0 - Fowler/Noll/Vo-0 32 bit hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://reality.sgi.com/chongo)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it ``Fowler/Noll/Vo'' or the FNV hash.
 *
 * FNV hashes are architected to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://reality.sgi.com/chongo/tech/comp/fnv/
 *
 * for more details as well as other forms of the FNV hash.
 *
 * given:
 *	x	the value to hash (must not be longer than 32 bits)
 *	val	previous QCKHASH value
 *
 * returns:
 *	the next 32 bit QCKHASH
 */
#define fnv(x,val) (((QCKHASH)(val)*(QCKHASH)16777619) ^ ((QCKHASH)(x)))


/*
 * fnv_qhash - compute the next Fowler/Noll/Vo hash given a NUMBER
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
 * fnv_chash - compute the next Fowler/Noll/Vo hash given a COMPLEX
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
			return fnv(vp->v_int, V_NUM+val);
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
			return fnv(vp->v_file, V_FILE+val);
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
			return fnv((int)*vp->v_octet, V_OCTET+val);
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
static QCKHASH
assochash(ASSOC *ap, QCKHASH val)
{
	/* XXX - hash the first and last values??? */
	return fnv(ap->a_count, V_ASSOC+val);
}


/*
 * Return a trivial hash value for a list.
 */
static QCKHASH
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
static QCKHASH
mathash(MATRIX *m, QCKHASH val)
{
	long skip;
	long i;
	VALUE *vp;

	/*
	 * hash size parts of the matrix
	 */
	val = fnv(m->m_dim, V_MAT+val);
	val = fnv(m->m_size, val);

	/*
	 * hash the matrix index bounds
	 */
	for (i = m->m_dim - 1; i >= 0; i--) {
		val = fnv(m->m_min[i], val);
		val = fnv(m->m_max[i], val);
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
	vp = &m->m_table[16];
	skip = (m->m_size / 11) + 1;
	while (i < m->m_size) {
		val = hashvalue(vp, val);
		i += skip;
		vp += skip;
	}
	return val;
}


/*
 * Return a trivial hash value for an object.
 */
static QCKHASH
objhash(OBJECT *op, QCKHASH val)
{
	int i;

	i = op->o_actions->count;
	while (--i >= 0)
		val = hashvalue(&op->o_table[i], val);
	return val;
}


/*
 * randhash - return a trivial hash for an a55 state
 *
 * given:
 *	state - state to hash
 *
 * returns:
 *	trivial hash integer
 */
static QCKHASH
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
		val = fnv(r->j, V_RAND+val);
		val = fnv(r->k, val);
		val = fnv(r->bits, val);

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
static QCKHASH
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
	val = fnv(state->buffer+state->bits, V_RANDOM+val);
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
static QCKHASH
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
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->outdigits);
	/* epsilon is handeled out of order */
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
	/* prompt1 is handeled out of order */
	/* prompt2 is handeled out of order */
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkmaxprint);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkverbose);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkbase);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->blkfmt);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->lib_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->calc_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->user_debug);
	value = (((value>>5) | (value<<27)) ^ (USB32)cfg->verbose_quit);

	/*
	 * hash the built up scalar
	 */
	val = fnv(value, V_CONFIG+val);

	/*
	 * hash the strings if possible
	 */
	if (cfg->prompt1) {
		val = fnv_strhash(cfg->prompt1, val);
	}
	if (cfg->prompt2) {
		val = fnv_strhash(cfg->prompt2, val);
	}

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
static QCKHASH
fnv_strhash(char *ch, QCKHASH val)
{
	/*
	 * hash each character in the string
	 */
	while (*ch) {
		val = fnv(*ch++, val);
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
static QCKHASH
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
		val = fnv(*ch++, val);
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
static QCKHASH
fnv_fullhash(FULL *v, LEN len, QCKHASH val)
{
	/*
	 * hash each character in the string
	 */
	while (len-- > 0) {
		val = fnv(*v++, val);
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
static QCKHASH
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
	val = fnv(z.sign, val + V_NUM);

	n = z.len;
	hp = z.v;

#if BASEB == 16
	while (n > 1) {
		f = (FULL) *hp++;
		f |= (FULL) *hp++ << BASEB;
		val = fnv(f, val);
		n -= 2;
	}
	if (n) {
		val = fnv(*hp, val);
	}
#else
	while (n--  > 0) {
		val = fnv(*hp, val);
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
static QCKHASH
hash_hash(HASH *hash, QCKHASH val)
{
	int i;

	/*
	 * hash each USB8 in the BLOCK
	 */
	for (i=0; i < hash->unionsize; ++i) {
		val = fnv(hash->h_union.data[i], val);
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
static QCKHASH
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
			val = fnv(blk->data[i], val);
		}
	}
	return val;
}
