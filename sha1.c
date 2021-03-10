/*
 * sha1 - implements new NIST Secure Hash Standard-1 (SHA1)
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file has been extensively modified by:
 *
 *	Landon Curt Noll
 *	http://www.isthe.com/chongo/
 *
 *	chongo <was here> /\../\
 *
 * This code has been placed in the public domain.  Please do not
 * copyright this code.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH  REGARD  TO
 * THIS	 SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS.  IN NO EVENT SHALL	 LANDON	 CURT
 * NOLL	 BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM  LOSS  OF
 * USE,	 DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 */


#include <stdio.h>
#include "alloc.h"
#include "longbits.h"
#include "align32.h"
#include "endian_calc.h"
#include "value.h"
#include "hash.h"
#include "sha1.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * The SHA1 f()-functions.  The f1 and f3 functions can be optimized
 * to save one boolean operation each - thanks to Rich Schroeppel,
 * rcs@cs.arizona.edu for discovering this.
 *
 * f1: ((x&y) | (~x&z)) == (z ^ (x&(y^z)))
 * f3: ((x&y) | (x&z) | (y&z)) == ((x&y) | (z&(x|y)))
 */
#define f1(x,y,z)	(z ^ (x&(y^z)))		/* Rounds  0-19 */
#define f2(x,y,z)	(x^y^z)			/* Rounds 20-39 */
#define f3(x,y,z)	((x&y) | (z&(x|y)))	/* Rounds 40-59 */
#define f4(x,y,z)	(x^y^z)			/* Rounds 60-79 */

/* The SHA1 Mysterious Constants */
#define K1	0x5A827999L	/* Rounds  0-19 */
#define K2	0x6ED9EBA1L	/* Rounds 20-39 */
#define K3	0x8F1BBCDCL	/* Rounds 40-59 */
#define K4	0xCA62C1D6L	/* Rounds 60-79 */

/* SHA1 initial values */
#define h0init	0x67452301L
#define h1init	0xEFCDAB89L
#define h2init	0x98BADCFEL
#define h3init	0x10325476L
#define h4init	0xC3D2E1F0L

/* 32-bit rotate left - kludged with shifts */
#define LEFT_ROT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

/*
 *
 * The initial expanding function.  The hash function is defined over an
 * 80-word expanded input array W, where the first 16 are copies of the input
 * data, and the remaining 64 are defined by
 *
 *	W[i] = LEFT_ROT(W[i-16] ^ W[i-14] ^ W[i-8] ^ W[i-3], 1)
 *
 * NOTE: The expanding function used in rounds 16 to 79 was changed from the
 *	 original SHA (in FIPS Pub 180) to one that also left circular shifted
 *	 by one bit for Secure Hash Algorithm-1 (FIPS Pub 180-1).
 */
#define exor(W,i,t) \
    (t = (W[i&15] ^ W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15]), \
     W[i&15] = LEFT_ROT(t, 1))

/*
 * The prototype SHA1 sub-round.  The fundamental sub-round is:
 *
 *	a' = e + LEFT_ROT(a,5) + f(b,c,d) + k + data;
 *	b' = a;
 *	c' = LEFT_ROT(b,30);
 *	d' = c;
 *	e' = d;
 *
 * but this is implemented by unrolling the loop 5 times and renaming the
 * variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
 * This code is then replicated 20 times for each of the 4 functions, using
 * the next 20 values from the W[] array each time.
 */
#define subRound(a, b, c, d, e, f, k, data) \
    (e += LEFT_ROT(a,5) + f(b,c,d) + k + data, b = LEFT_ROT(b,30))

/* forward declarations */
S_FUNC void sha1Init(HASH*);
S_FUNC void sha1Transform(USB32*, USB32*);
S_FUNC void sha1Update(HASH*, USB8*, USB32);
S_FUNC void sha1Final(HASH*);
S_FUNC void sha1_chkpt(HASH*);
S_FUNC void sha1_note(int, HASH*);
S_FUNC void sha1_type(int, HASH*);
void sha1_init_state(HASH*);
S_FUNC ZVALUE sha1_final_state(HASH*);
S_FUNC int sha1_cmp(HASH*, HASH*);
S_FUNC void sha1_print(HASH*);


/*
 * sha1Init - initialize the SHA1 state
 */
S_FUNC void
sha1Init(HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;  /* digest state */

	/* Set the h-vars to their initial values */
	dig->digest[0] = h0init;
	dig->digest[1] = h1init;
	dig->digest[2] = h2init;
	dig->digest[3] = h3init;
	dig->digest[4] = h4init;

	/* Initialize bit count */
	dig->countLo = 0;
	dig->countHi = 0;
	dig->datalen = 0;
}


/*
 * sha1Transform - perform the SHA1 transformation
 *
 * Note that this code, like MD5, seems to break some optimizing compilers.
 * It may be necessary to split it into sections, e.g., based on the four
 * subrounds.  One may also want to roll each subround into a loop.
 */
S_FUNC void
sha1Transform(USB32 *digest, USB32 *W)
{
	USB32 A, B, C, D, E;		/* Local vars */
	USB32 t;			/* temp storage for exor() */

	/* Set up first buffer and local data buffer */
	A = digest[0];
	B = digest[1];
	C = digest[2];
	D = digest[3];
	E = digest[4];

	/* Heavy mangling, in 4 sub-rounds of 20 iterations each. */
	subRound(A, B, C, D, E, f1, K1, W[ 0]);
	subRound(E, A, B, C, D, f1, K1, W[ 1]);
	subRound(D, E, A, B, C, f1, K1, W[ 2]);
	subRound(C, D, E, A, B, f1, K1, W[ 3]);
	subRound(B, C, D, E, A, f1, K1, W[ 4]);
	subRound(A, B, C, D, E, f1, K1, W[ 5]);
	subRound(E, A, B, C, D, f1, K1, W[ 6]);
	subRound(D, E, A, B, C, f1, K1, W[ 7]);
	subRound(C, D, E, A, B, f1, K1, W[ 8]);
	subRound(B, C, D, E, A, f1, K1, W[ 9]);
	subRound(A, B, C, D, E, f1, K1, W[10]);
	subRound(E, A, B, C, D, f1, K1, W[11]);
	subRound(D, E, A, B, C, f1, K1, W[12]);
	subRound(C, D, E, A, B, f1, K1, W[13]);
	subRound(B, C, D, E, A, f1, K1, W[14]);
	subRound(A, B, C, D, E, f1, K1, W[15]);
	subRound(E, A, B, C, D, f1, K1, exor(W,16,t));
	subRound(D, E, A, B, C, f1, K1, exor(W,17,t));
	subRound(C, D, E, A, B, f1, K1, exor(W,18,t));
	subRound(B, C, D, E, A, f1, K1, exor(W,19,t));

	subRound(A, B, C, D, E, f2, K2, exor(W,20,t));
	subRound(E, A, B, C, D, f2, K2, exor(W,21,t));
	subRound(D, E, A, B, C, f2, K2, exor(W,22,t));
	subRound(C, D, E, A, B, f2, K2, exor(W,23,t));
	subRound(B, C, D, E, A, f2, K2, exor(W,24,t));
	subRound(A, B, C, D, E, f2, K2, exor(W,25,t));
	subRound(E, A, B, C, D, f2, K2, exor(W,26,t));
	subRound(D, E, A, B, C, f2, K2, exor(W,27,t));
	subRound(C, D, E, A, B, f2, K2, exor(W,28,t));
	subRound(B, C, D, E, A, f2, K2, exor(W,29,t));
	subRound(A, B, C, D, E, f2, K2, exor(W,30,t));
	subRound(E, A, B, C, D, f2, K2, exor(W,31,t));
	subRound(D, E, A, B, C, f2, K2, exor(W,32,t));
	subRound(C, D, E, A, B, f2, K2, exor(W,33,t));
	subRound(B, C, D, E, A, f2, K2, exor(W,34,t));
	subRound(A, B, C, D, E, f2, K2, exor(W,35,t));
	subRound(E, A, B, C, D, f2, K2, exor(W,36,t));
	subRound(D, E, A, B, C, f2, K2, exor(W,37,t));
	subRound(C, D, E, A, B, f2, K2, exor(W,38,t));
	subRound(B, C, D, E, A, f2, K2, exor(W,39,t));

	subRound(A, B, C, D, E, f3, K3, exor(W,40,t));
	subRound(E, A, B, C, D, f3, K3, exor(W,41,t));
	subRound(D, E, A, B, C, f3, K3, exor(W,42,t));
	subRound(C, D, E, A, B, f3, K3, exor(W,43,t));
	subRound(B, C, D, E, A, f3, K3, exor(W,44,t));
	subRound(A, B, C, D, E, f3, K3, exor(W,45,t));
	subRound(E, A, B, C, D, f3, K3, exor(W,46,t));
	subRound(D, E, A, B, C, f3, K3, exor(W,47,t));
	subRound(C, D, E, A, B, f3, K3, exor(W,48,t));
	subRound(B, C, D, E, A, f3, K3, exor(W,49,t));
	subRound(A, B, C, D, E, f3, K3, exor(W,50,t));
	subRound(E, A, B, C, D, f3, K3, exor(W,51,t));
	subRound(D, E, A, B, C, f3, K3, exor(W,52,t));
	subRound(C, D, E, A, B, f3, K3, exor(W,53,t));
	subRound(B, C, D, E, A, f3, K3, exor(W,54,t));
	subRound(A, B, C, D, E, f3, K3, exor(W,55,t));
	subRound(E, A, B, C, D, f3, K3, exor(W,56,t));
	subRound(D, E, A, B, C, f3, K3, exor(W,57,t));
	subRound(C, D, E, A, B, f3, K3, exor(W,58,t));
	subRound(B, C, D, E, A, f3, K3, exor(W,59,t));

	subRound(A, B, C, D, E, f4, K4, exor(W,60,t));
	subRound(E, A, B, C, D, f4, K4, exor(W,61,t));
	subRound(D, E, A, B, C, f4, K4, exor(W,62,t));
	subRound(C, D, E, A, B, f4, K4, exor(W,63,t));
	subRound(B, C, D, E, A, f4, K4, exor(W,64,t));
	subRound(A, B, C, D, E, f4, K4, exor(W,65,t));
	subRound(E, A, B, C, D, f4, K4, exor(W,66,t));
	subRound(D, E, A, B, C, f4, K4, exor(W,67,t));
	subRound(C, D, E, A, B, f4, K4, exor(W,68,t));
	subRound(B, C, D, E, A, f4, K4, exor(W,69,t));
	subRound(A, B, C, D, E, f4, K4, exor(W,70,t));
	subRound(E, A, B, C, D, f4, K4, exor(W,71,t));
	subRound(D, E, A, B, C, f4, K4, exor(W,72,t));
	subRound(C, D, E, A, B, f4, K4, exor(W,73,t));
	subRound(B, C, D, E, A, f4, K4, exor(W,74,t));
	subRound(A, B, C, D, E, f4, K4, exor(W,75,t));
	subRound(E, A, B, C, D, f4, K4, exor(W,76,t));
	subRound(D, E, A, B, C, f4, K4, exor(W,77,t));
	subRound(C, D, E, A, B, f4, K4, exor(W,78,t));
	subRound(B, C, D, E, A, f4, K4, exor(W,79,t));

	/* Build message digest */
	digest[0] += A;
	digest[1] += B;
	digest[2] += C;
	digest[3] += D;
	digest[4] += E;
}


/*
 * sha1Update - update SHA1 with arbitrary length data
 */
void
sha1Update(HASH *state, USB8 *buffer, USB32 count)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;  /* digest state */
	USB32 datalen = dig->datalen;
	USB32 cpylen;
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/*
	 * Update the full count, even if some of it is buffered for later
	 */
	SHA1COUNT(dig, count);


	/* determine the size we need to copy */
	cpylen = SHA1_CHUNKSIZE - datalen;

	/* case: new data will not fill the buffer */
	if (cpylen > count) {
		memcpy((char *)dig->data+datalen,
			(char *)buffer, count);
		dig->datalen = datalen+count;
		return;
	}

	/* case: buffer will be filled */
	memcpy((char *)dig->data + datalen, (char *)buffer, cpylen);

	/*
	 * Process data in SHA1_CHUNKSIZE chunks
	 */
	for (;;) {
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
		if (state->bytes) {
			for (i=0; i < SHA1_CHUNKWORDS; ++i) {
				SWAP_B8_IN_B32(dig->data+i, dig->data+i);
			}
		}
#endif
		sha1Transform(dig->digest, dig->data);
		buffer += cpylen;
		count -= cpylen;
		if (count < SHA1_CHUNKSIZE)
			break;
		cpylen = SHA1_CHUNKSIZE;
		memcpy(dig->data, buffer, cpylen);
	}

	/*
	 * Handle any remaining bytes of data.
	 * This should only happen once on the final lot of data
	 */
	if (count > 0) {
		memcpy((char *)dig->data, (char *)buffer, count);
	}
	dig->datalen = count;
}


/*
 * sha1Final - perform final SHA1 transforms
 *
 * At this point we have less than a full chunk of data remaining
 * (and possibly no data) in the sha1 state data buffer.
 *
 * First we append a final 0x80 byte.
 *
 * Next if we have more than 56 bytes, we will zero fill the remainder
 * of the chunk, transform and then zero fill the first 56 bytes.
 * If we have 56 or fewer bytes, we will zero fill out to the 56th
 * chunk byte.	Regardless, we wind up with 56 bytes data.
 *
 * Finally we append the 64 bit length on to the 56 bytes of data
 * remaining.  This final chunk is transformed.
 */

void
sha1Final(HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;	/* digest state */
	long count = (long)(dig->datalen);
	USB32 lowBitcount;
	USB32 highBitcount;
	USB8 *data = (USB8 *) dig->data;
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/* Pad to end of chunk */

	memset(data + count, 0, SHA1_CHUNKSIZE - count);

	/*
	 * If processing bytes, set the first byte of padding to 0x80.
	 * if processing words: on a big-endian machine set the first
	 * byte of padding to 0x80, on a little-endian machine set
	 * the first four bytes to 0x00000080
	 * This is safe since there is always at least one byte or word free
	 */

	memset(data + count, 0, SHA1_CHUNKSIZE - count);

#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	if (state->bytes) {
		data[count] = 0x80;
		for (i=0; i < SHA1_CHUNKWORDS; ++i) {
			SWAP_B8_IN_B32(dig->data+i, dig->data+i);
		}
	} else {
		if (count % 4) {
			math_error("This should not happen in sha1Final");
			/*NOTREACHED*/
		}
		data[count + 3] = 0x80;
	}
#else
	data[count] = 0x80;
#endif

	if (count >= SHA1_CHUNKSIZE-8) {
		sha1Transform(dig->digest, dig->data);

		/* Now load another chunk with 56 bytes of padding */
		memset(data, 0, SHA1_CHUNKSIZE-8);
	}

	/*
	 * Append length in bits and transform
	 *
	 * We assume that bit count is a multiple of 8 because we have
	 * only processed full bytes.
	 */
	highBitcount = dig->countHi;
	lowBitcount = dig->countLo;
	dig->data[SHA1_HIGH] = (highBitcount << 3) | (lowBitcount >> 29);
	dig->data[SHA1_LOW] = (lowBitcount << 3);
	sha1Transform(dig->digest, dig->data);
	dig->datalen = 0;
}


/*
 * sha1_chkpt - checkpoint a SHA1 state
 *
 * given:
 *	state	the state to checkpoint
 *
 * This function will ensure that the hash chunk buffer is empty.
 * Any partially hashed data will be padded out with 0's and hashed.
 */
S_FUNC void
sha1_chkpt(HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;	/* digest state */
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/*
	 * checkpoint if partial buffer exists
	 */
	if (dig->datalen > 0) {

		/* pad to the end of the chunk */
		memset((USB8 *)dig->data + dig->datalen, 0,
		       SHA1_CHUNKSIZE-dig->datalen);
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
		if (state->bytes) {
			for (i=0; i < SHA1_CHUNKWORDS; ++i) {
				SWAP_B8_IN_B32(dig->data+i, dig->data+i);
			}
		}
#endif
		/* transform padded chunk */
		sha1Transform(dig->digest, dig->data);
		SHA1COUNT(dig, SHA1_CHUNKSIZE-dig->datalen);

		/* empty buffer */
		dig->datalen = 0;
	}
	return;
}


/*
 * sha1_note - note a special value
 *
 * given:
 *	state		the state to hash
 *	special		a special value (SHA1_HASH_XYZ) to note
 *
 * This function will note that a special value is about to be hashed.
 * Types include negative values, complex values, division, zero numeric
 * and array of HALFs.
 */
S_FUNC void
sha1_note(int special, HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;	/* digest state */
	unsigned int i;

	/*
	 * change state to reflect a special value
	 */
	dig->digest[0] ^= special;
	for (i=1; i < SHA1_DIGESTWORDS; ++i) {
		dig->digest[i] ^= (special + dig->digest[i-1] + i);
	}
	return;
}


/*
 * sha1_type - note a VALUE type
 *
 * given:
 *	state		the state to hash
 *	type		the VALUE type to note
 *
 * This function will note that a type of value is about to be hashed.
 * The type of a VALUE will be noted.  For purposes of hash comparison,
 * we will do nothing with V_NUM and V_COM so that the other functions
 * can hash to the same value regardless of if sha1_value() is called
 * or not.  We also do nothing with V_STR so that a hash of a string
 * will produce the same value as the standard hash function.
 */
S_FUNC void
sha1_type(int type, HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;	/* digest state */
	unsigned int i;

	/*
	 * ignore NUMBER and COMPLEX
	 */
	if (type == V_NUM || type == V_COM || type == V_STR) {
		return;
	}

	/*
	 * change state to reflect a VALUE type
	 */
	dig->digest[0] += type;
	for (i=1; i < SHA1_DIGESTWORDS; ++i) {
		dig->digest[i] += ((type+i) ^ dig->digest[i-1]);
	}
	return;
}


/*
 * sha1_init_state - initialize a hash state structure for this hash
 *
 * given:
 *	state	- pointer to the hfunction element to initialize
 */
void
sha1_init_state(HASH *state)
{
	/*
	 * initialize state
	 */
	state->hashtype = SHA1_HASH_TYPE;
	state->bytes = TRUE;
	state->update = sha1Update;
	state->chkpt = sha1_chkpt;
	state->note = sha1_note;
	state->type = sha1_type;
	state->final = sha1_final_state;
	state->cmp = sha1_cmp;
	state->print = sha1_print;
	state->base = SHA1_BASE;
	state->chunksize = SHA1_CHUNKSIZE;
	state->unionsize = sizeof(SHA1_INFO);

	/*
	 * perform the internal init function
	 */
	memset((void *)&(state->h_union.h_sha1), 0, sizeof(SHA1_INFO));
	sha1Init(state);
	return;
}


/*
 * sha1_final_state - complete hash state and return a ZVALUE
 *
 * given:
 *	state	the state to complete and convert
 *
 * returns:
 *	a ZVALUE representing the state
 */
S_FUNC ZVALUE
sha1_final_state(HASH *state)
{
	SHA1_INFO *dig = &state->h_union.h_sha1;	/* digest state */
	ZVALUE ret;		/* return ZVALUE of completed hash state */
	int i;

	/*
	 * malloc and initialize if state is NULL
	 */
	if (state == NULL) {
		state = (HASH *)malloc(sizeof(HASH));
		if (state == NULL) {
			math_error("cannot malloc HASH");
			/*NOTREACHED*/
		}
		sha1_init_state(state);
	}

	/*
	 * complete the hash state
	 */
	sha1Final(state);

	/*
	 * allocate storage for ZVALUE
	 */
	ret.len = SHA1_DIGESTSIZE/sizeof(HALF);
	ret.sign = 0;
	ret.v = alloc(ret.len);

	/*
	 * load ZVALUE
	 */
#if BASEB == 16 && CALC_BYTE_ORDER == LITTLE_ENDIAN
	for (i=0; i < ret.len; i+=2) {
		ret.v[ret.len-i-1] = ((HALF*)dig->digest)[i+1];
		ret.v[ret.len-i-2] = ((HALF*)dig->digest)[i];
	}
#else
	for (i=0; i < ret.len; ++i) {
		ret.v[ret.len-i-1] = ((HALF*)dig->digest)[i];
	}
#endif
	ztrim(&ret);

	/*
	 * return ZVALUE
	 */
	return ret;
}


/*
 * sha1_cmp - compare two hash states
 *
 * given:
 *	a	first hash state
 *	b	second hash state
 *
 * returns:
 *	TRUE => hash states are different
 *	FALSE => hash states are the same
 */
S_FUNC int
sha1_cmp(HASH *a, HASH *b)
{
	/*
	 * firewall and quick check
	 */
	if (a == b) {
		/* pointers to the same object */
		return FALSE;
	}
	if (a == NULL || b == NULL) {
		/* one is NULL, so they differ */
		return TRUE;
	}

	/*
	 * compare data-reading modes
	 */
	if (a->bytes != b->bytes)
		return TRUE;

	/*
	 * compare bit counts
	 */
	if (a->h_union.h_sha1.countLo != b->h_union.h_sha1.countLo ||
	    a->h_union.h_sha1.countHi != b->h_union.h_sha1.countHi) {
		/* counts differ */
		return TRUE;
	}

	/*
	 * compare pending buffers
	 */
	if (a->h_union.h_sha1.datalen != b->h_union.h_sha1.datalen) {
		/* buffer lengths differ */
		return TRUE;
	}
	if (memcmp((USB8*)a->h_union.h_sha1.data,
		   (USB8*)b->h_union.h_sha1.data,
		   a->h_union.h_sha1.datalen) != 0) {
		/* buffer contents differ */
		return TRUE;
	}

	/*
	 * compare digest
	 */
	return (memcmp((USB8*)(a->h_union.h_sha1.digest),
		       (USB8*)(b->h_union.h_sha1.digest),
		       SHA1_DIGESTSIZE) != 0);
}


/*
 * sha1_print - print a hash state
 *
 * given:
 *	state	the hash state to print
 */
S_FUNC void
sha1_print(HASH *state)
{
	/*
	 * form the hash value
	 */
	if (conf->calc_debug & CALCDBG_HASH_STATE) {
		char buf[DEBUG_SIZE+1]; /* hash value buffer */

		/*
		 * print numeric debug value
		 *
		 * NOTE: This value represents only the hash value as of
		 *	 the last full update or finalization.	Thus it
		 *	 may NOT be the actual hash value.
		 */
		snprintf(buf, DEBUG_SIZE,
			"sha1: 0x%08x%08x%08x%08x%08x data: %d octets",
			(int)state->h_union.h_sha1.digest[0],
			(int)state->h_union.h_sha1.digest[1],
			(int)state->h_union.h_sha1.digest[2],
			(int)state->h_union.h_sha1.digest[3],
			(int)state->h_union.h_sha1.digest[4],
			(int)state->h_union.h_sha1.datalen);
		buf[DEBUG_SIZE] = '\0';	/* paranoia */
		math_str(buf);
	} else {
		math_str("sha1 hash state");
	}
	return;
}
