/*
 * Copyright (c) 1996 by Landon Curt Noll.  All Rights Reserved.
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
 * chongo was here	/\../\
 */

/*
 * quickhash - qickly hash a calc value using a partial Fowler/Noll/Vo hash
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
 */

#include "value.h"
#include "zrand.h"

#define ZMOST 2		/* most significant HALFs to hash */
#define ZLEAST 2	/* least significant HALFs to hash */
#define ZMIDDLE 4	/* HALFs in the middle to hash */


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
static QCKHASH fnv_strhash(char *str, QCKHASH val);
static QCKHASH fnv_fullhash(FULL *v, LEN len, QCKHASH val);
static QCKHASH fnv_zhash(ZVALUE z, QCKHASH val);


/*
 * fnv - compute the next Fowler/Noll/Vo hash given a variable
 *
 * The basis of the hash algorithm was taken from an idea
 * sent by Email to the IEEE Posix P1003.2 mailing list from
 * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
 * Landon Curt Noll (chongo@toad.com) later improved on there
 * algorithm to come up with Fowler/Noll/Vo hash.
 *
 * The magic lies in the constant 16777619, which for 32 bit hashing
 * is able to process 234936 words from the web2 dictionary without
 * any collisions.
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
			return fnv_strhash(vp->v_str, val);
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
	if (state->r != NULL && state->r->v != NULL) {
		val = fnv_zhash(*(state->r), val);
	}
	if (state->n != NULL && state->n->v != NULL) {
		val = fnv_zhash(*(state->n), val);
	}
	return val;
}


/*
 * config_hash - return a trivial hash for a configuration state
 */
static QCKHASH
config_hash(CONFIG *cfg, QCKHASH val)
{
	/*
	 * hash scalar values
	 */
	val = fnv(cfg->traceflags + cfg->outdigits + cfg->outmode +
		  cfg->epsilonprec + cfg->maxprint + cfg->mul2 +
		  cfg->sq2 + cfg->pow2 + cfg->redc2 + cfg->tilde_ok +
	          cfg->tab_ok + cfg->quomod + cfg->quo + cfg->mod +
	          cfg->sqrt + cfg->appr + cfg->cfappr + cfg->cfsim +
	          cfg->outround + cfg->round + cfg->leadzero +
	          cfg->fullzero + cfg->maxerrorcount, V_CONFIG+val);

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
 * fnv_strhash - Fowler/Noll/Vo 32 bit hash of a string
 *
 * given:
 *	str	the string to hash
 *	val	initial hash value
 *
 * returns:
 *	a 32 bit QCKHASH value
 */
static QCKHASH
fnv_strhash(char *str, QCKHASH val)
{
	/*
	 * hash each character in the string
	 */
	while (*str) {
	    val = fnv(*str++, val);
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
	int skip;		/* HALFs to skip in the middle */
	int i;

	/*
	 * hash the sign and length
	 */
	if (zisneg(z)) {
		val = fnv(-(z.len), val+V_NUM);
	} else {
		val = fnv(z.len, val+V_NUM);
	}

	/*
	 * if a ZVALUE is short enough, hash it all
	 */
	if (z.len <= ZMOST+ZLEAST+ZMIDDLE) {
		/* hash all HALFs of a short ZVALUE */
		for (i=0; i < z.len; ++i) {
			val = fnv(z.v[i], val);
		}

	/*
	 * otherwise hash the ZLEAST significant HALFs followed by
	 * ZMIDDLE HALFs followed by the ZMOST significant HALFs.
	 */
	} else {
		/* hash the ZLEAST significant HALFs */
		for (i=0; i < ZLEAST; ++i) {
			val = fnv(z.v[i], val);
		}

		/* hash ZMIDDLE HALFs in the middle */
		skip = (z.len-ZLEAST-ZMOST)/(ZMIDDLE + 1);
		for (i=ZLEAST-1+skip; i < ZLEAST-1+skip*(ZMIDDLE+1); i+=skip) {
			val = fnv(z.v[i], val);
		}

		/* hash the ZMOST significant HALFs */
		for (i=z.len-1-ZMOST; i < z.len; ++i) {
			val = fnv(z.v[i], val);
		}
	}
	return val;
}
