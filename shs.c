/*
 * shs - old Secure Hash Standard
 *
 **************************************************************************
 * This version implements the old Secure Hash Algorithm specified by	  *
 * (FIPS Pub 180).  This version is kept for backward compatibility with  *
 * shs version 2.10.1.	See the shs utility for the new standard.	  *
 **************************************************************************
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file was Modified/Re-written by:
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
 * Based on Version 2.11 (09 Mar 1995) from Landon Curt Noll's
 * (http://www.isthe.com/chongo/) shs hash program.
 *
 * @(#) $Revision: 29.3 $
 * @(#) $Id: shs.c,v 29.3 2004/02/23 08:14:15 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/shs.c,v $
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 *
 ****
 *
 * The SHS algorithm hashes 32 bit unsigned values, 16 at a time.
 * It further specifies that strings are to be converted into
 * 32 bit values in BIG ENDIAN order.  That is on little endian
 * machines, strings are byte swapped into BIG ENDIAN order before
 * they are taken 32 bit at a time.  Even so, when hashing 32 bit
 * numeric values the byte order DOES NOT MATTER because the
 * algorithm works off of their numeric value, not their byte order.
 *
 * In calc, we want to hash equal values to the same hash value.
 * For the most part, we will be hashing arrays of HALF's instead
 * of strings.	For this reason, the functions below do not byte
 * swap on little endian machines automatically.  Instead it is
 * the responsibility of the caller of the internal SHS function
 * to ensure that the values are already in the canonical 32 bit
 * numeric value form.
 */

#include <stdio.h>
#include "longbits.h"
#include "align32.h"
#include "endian_calc.h"
#include "value.h"
#include "hash.h"
#include "shs.h"


/*
 * The SHS f()-functions.  The f1 and f3 functions can be optimized
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

/* The SHS Mysterious Constants */
#define K1	0x5A827999L	/* Rounds  0-19 */
#define K2	0x6ED9EBA1L	/* Rounds 20-39 */
#define K3	0x8F1BBCDCL	/* Rounds 40-59 */
#define K4	0xCA62C1D6L	/* Rounds 60-79 */

/* SHS initial values */
#define h0init	0x67452301L
#define h1init	0xEFCDAB89L
#define h2init	0x98BADCFEL
#define h3init	0x10325476L
#define h4init	0xC3D2E1F0L

/* 32-bit rotate left - kludged with shifts */
#define LEFT_ROT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

/*
 * The initial expanding function.  The hash function is defined over an
 * 80-word expanded input array W, where the first 16 are copies of the input
 * data, and the remaining 64 are defined by
 *
 *	W[i] = W[i-16] ^ W[i-14] ^ W[i-8] ^ W[i-3]
 *
 * This implementation generates these values on the fly in a circular
 * buffer - thanks to Colin Plumb (colin@nyx10.cs.du.edu) for this
 * optimization.
 */
#define exor(W,i) (W[i&15] ^= (W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15]))

/*
 * The prototype SHS sub-round.	 The fundamental sub-round is:
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


/*
 * forward declarations
 */
static void shsInit(HASH*);
static void shsTransform(USB32*, USB32*);
static void shsUpdate(HASH*, USB8*, USB32);
static void shsFinal(HASH*);
static void shs_chkpt(HASH*);
static void shs_note(int, HASH*);
static void shs_type(int, HASH*);
void shs_init_state(HASH*);
static ZVALUE shs_final_state(HASH*);
static int shs_cmp(HASH*, HASH*);
static void shs_print(HASH*);


/*
 * shsInit - initialize the SHS state
 */
static void
shsInit(HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */

	/* Set the h-vars to their initial values */
	dig->digest[0] = h0init;
	dig->digest[1] = h1init;
	dig->digest[2] = h2init;
	dig->digest[3] = h3init;
	dig->digest[4] = h4init;

	/* Initialise bit count */
	dig->countLo = 0;
	dig->countHi = 0;
	dig->datalen = 0;
}


/*
 * shsTransform - perform the SHS transformatio
 *
 * Note that this code, like MD5, seems to break some optimizing compilers.
 * It may be necessary to split it into sections, eg based on the four
 * subrounds.  One may also want to roll each subround into a loop.
 */
static void
shsTransform(USB32 *digest, USB32 *W)
{
	USB32 A, B, C, D, E;	/* Local vars */

	/* Set up first buffer and local data buffer */
	A = digest[0];
	B = digest[1];
	C = digest[2];
	D = digest[3];
	E = digest[4];

	/* Heavy mangling, in 4 sub-rounds of 20 interations each. */
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
	subRound(E, A, B, C, D, f1, K1, exor(W,16));
	subRound(D, E, A, B, C, f1, K1, exor(W,17));
	subRound(C, D, E, A, B, f1, K1, exor(W,18));
	subRound(B, C, D, E, A, f1, K1, exor(W,19));

	subRound(A, B, C, D, E, f2, K2, exor(W,20));
	subRound(E, A, B, C, D, f2, K2, exor(W,21));
	subRound(D, E, A, B, C, f2, K2, exor(W,22));
	subRound(C, D, E, A, B, f2, K2, exor(W,23));
	subRound(B, C, D, E, A, f2, K2, exor(W,24));
	subRound(A, B, C, D, E, f2, K2, exor(W,25));
	subRound(E, A, B, C, D, f2, K2, exor(W,26));
	subRound(D, E, A, B, C, f2, K2, exor(W,27));
	subRound(C, D, E, A, B, f2, K2, exor(W,28));
	subRound(B, C, D, E, A, f2, K2, exor(W,29));
	subRound(A, B, C, D, E, f2, K2, exor(W,30));
	subRound(E, A, B, C, D, f2, K2, exor(W,31));
	subRound(D, E, A, B, C, f2, K2, exor(W,32));
	subRound(C, D, E, A, B, f2, K2, exor(W,33));
	subRound(B, C, D, E, A, f2, K2, exor(W,34));
	subRound(A, B, C, D, E, f2, K2, exor(W,35));
	subRound(E, A, B, C, D, f2, K2, exor(W,36));
	subRound(D, E, A, B, C, f2, K2, exor(W,37));
	subRound(C, D, E, A, B, f2, K2, exor(W,38));
	subRound(B, C, D, E, A, f2, K2, exor(W,39));

	subRound(A, B, C, D, E, f3, K3, exor(W,40));
	subRound(E, A, B, C, D, f3, K3, exor(W,41));
	subRound(D, E, A, B, C, f3, K3, exor(W,42));
	subRound(C, D, E, A, B, f3, K3, exor(W,43));
	subRound(B, C, D, E, A, f3, K3, exor(W,44));
	subRound(A, B, C, D, E, f3, K3, exor(W,45));
	subRound(E, A, B, C, D, f3, K3, exor(W,46));
	subRound(D, E, A, B, C, f3, K3, exor(W,47));
	subRound(C, D, E, A, B, f3, K3, exor(W,48));
	subRound(B, C, D, E, A, f3, K3, exor(W,49));
	subRound(A, B, C, D, E, f3, K3, exor(W,50));
	subRound(E, A, B, C, D, f3, K3, exor(W,51));
	subRound(D, E, A, B, C, f3, K3, exor(W,52));
	subRound(C, D, E, A, B, f3, K3, exor(W,53));
	subRound(B, C, D, E, A, f3, K3, exor(W,54));
	subRound(A, B, C, D, E, f3, K3, exor(W,55));
	subRound(E, A, B, C, D, f3, K3, exor(W,56));
	subRound(D, E, A, B, C, f3, K3, exor(W,57));
	subRound(C, D, E, A, B, f3, K3, exor(W,58));
	subRound(B, C, D, E, A, f3, K3, exor(W,59));

	subRound(A, B, C, D, E, f4, K4, exor(W,60));
	subRound(E, A, B, C, D, f4, K4, exor(W,61));
	subRound(D, E, A, B, C, f4, K4, exor(W,62));
	subRound(C, D, E, A, B, f4, K4, exor(W,63));
	subRound(B, C, D, E, A, f4, K4, exor(W,64));
	subRound(A, B, C, D, E, f4, K4, exor(W,65));
	subRound(E, A, B, C, D, f4, K4, exor(W,66));
	subRound(D, E, A, B, C, f4, K4, exor(W,67));
	subRound(C, D, E, A, B, f4, K4, exor(W,68));
	subRound(B, C, D, E, A, f4, K4, exor(W,69));
	subRound(A, B, C, D, E, f4, K4, exor(W,70));
	subRound(E, A, B, C, D, f4, K4, exor(W,71));
	subRound(D, E, A, B, C, f4, K4, exor(W,72));
	subRound(C, D, E, A, B, f4, K4, exor(W,73));
	subRound(B, C, D, E, A, f4, K4, exor(W,74));
	subRound(A, B, C, D, E, f4, K4, exor(W,75));
	subRound(E, A, B, C, D, f4, K4, exor(W,76));
	subRound(D, E, A, B, C, f4, K4, exor(W,77));
	subRound(C, D, E, A, B, f4, K4, exor(W,78));
	subRound(B, C, D, E, A, f4, K4, exor(W,79));

	/* Build message digest */
	digest[0] += A;
	digest[1] += B;
	digest[2] += C;
	digest[3] += D;
	digest[4] += E;
}


/*
 * shsUpdate - update SHS with arbitrary length data
 */
static void
shsUpdate(HASH *state, USB8 *buffer, USB32 count)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */
	USB32 datalen = dig->datalen;
	USB32 cpylen;
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/*
	 * Update the full count, even if some of it is buffered for later
	 */
	SHSCOUNT(dig, count);


	/* determine the size we need to copy */
	cpylen = SHS_CHUNKSIZE - datalen;

	/* case: new data will not fill the buffer */
	if (cpylen > count) {
		memcpy((char *)dig->data + datalen,
		       (char *)buffer, count);
		dig->datalen = datalen+count;
		return;
	}

	/* case: buffer will be filled */
	memcpy((char *)dig->data + datalen, (char *)buffer, cpylen);

	/*
	 * process data in SHS_CHUNKSIZE chunks
	 */
	for (;;) {

#if CALC_BYTE_ORDER == LITTLE_ENDIAN
		if (state->bytes) {
			for (i=0; i < SHS_CHUNKWORDS; ++i) {
				SWAP_B8_IN_B32(dig->data+i, dig->data+i);
			}
		}
#endif
		shsTransform(dig->digest, dig->data);
		buffer += cpylen;
		count -= cpylen;
		if (count < SHS_CHUNKSIZE)
			break;
		cpylen = SHS_CHUNKSIZE;
		memcpy((char *) dig->data, (char *) buffer, cpylen);
	}

	/*
	 * Handle any remaining bytes of data.
	 */
	if (count > 0) {
		memcpy((char *)dig->data, (char *)buffer, count);
	}
	dig->datalen = count;
}


/*
 * shsFinal - perform final SHS transforms
 *
 * At this point we have less than a full chunk of data remaining
 * (and possibly no data) in the shs state data buffer.
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
static void
shsFinal(HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */
	long count = (long)(dig->datalen);
	USB32 lowBitcount;
	USB32 highBitcount;
	USB8 *data = (USB8 *) dig->data;
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/*
	 * If processing bytes, set the first byte of padding to 0x80.
	 * if processing words: on a big-endian machine set the first
	 * byte of padding to 0x80, on a little-endian machine set
	 * the first four bytes to 0x00000080
	 * This is safe since there is always at least one byte or word free
	 */

	/* Pad to end of chunk */

	memset(data + count, 0, SHS_CHUNKSIZE - count);

#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	if (state->bytes) {
		data[count] = 0x80;
		for (i=0; i < SHS_CHUNKWORDS; ++i) {
			SWAP_B8_IN_B32(dig->data+i, dig->data+i);
		}
	} else {
		if (count % 4) {
			math_error("This should not happen in shsFinal");
			/*NOTREACHED*/
		}
		data[count + 3] = 0x80;
	}
#else
	data[count] = 0x80;
#endif

	if (count >= SHS_CHUNKSIZE-8) {
		shsTransform(dig->digest, dig->data);

		/* Now fill another chunk with 56 bytes */
		memset(data, 0, SHS_CHUNKSIZE-8);
	}

	/*
	 * Append length in bits and transform
	 *
	 * We assume that bit count is a multiple of 8 because we have
	 * only processed full bytes.
	 */
	highBitcount = dig->countHi;
	lowBitcount = dig->countLo;
	dig->data[SHS_HIGH] = (highBitcount << 3) | (lowBitcount >> 29);
	dig->data[SHS_LOW] = (lowBitcount << 3);
	shsTransform(dig->digest, dig->data);
	dig->datalen = 0;
}


/*
 * shs_chkpt - checkpoint a SHS state
 *
 * given:
 *	state	the state to checkpoint
 *
 * This function will ensure that the the hash chunk buffer is empty.
 * Any partially hashed data will be padded out with 0's and hashed.
 */
static void
shs_chkpt(HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	unsigned int i;
#endif

	/*
	 * checkpoint if partial buffer exists
	 */
	if (dig->datalen > 0) {

		/* pad to the end of the chunk */
		memset((USB8 *)dig->data + dig->datalen, 0,
		       SHS_CHUNKSIZE-dig->datalen);
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
		if (state->bytes) {
			for (i=0; i < SHS_CHUNKWORDS; ++i) {
				SWAP_B8_IN_B32(dig->data+i, dig->data+i);
			}
		}
#endif

		/* transform padded chunk */
		shsTransform(dig->digest, dig->data);
		SHSCOUNT(dig, SHS_CHUNKSIZE-dig->datalen);

		/* empty buffer */
		dig->datalen = 0;
	}
	return;
}


/*
 * shs_note - note a special value
 *
 * given:
 *	state		the state to hash
 *	special		a special value (SHS_HASH_XYZ) to note
 *
 * This function will note that a special value is about to be hashed.
 * Types include negative values, complex values, division, zero numeric
 * and array of HALFs.
 */
static void
shs_note(int special, HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */
	unsigned int i;

	/*
	 * change state to reflect a special value
	 */
	dig->digest[0] ^= special;
	for (i=1; i < SHS_DIGESTWORDS; ++i) {
		dig->digest[i] ^= (special + dig->digest[i-1] + i);
	}
	return;
}


/*
 * shs_type - note a VALUE type
 *
 * given:
 *	state		the state to hash
 *	type		the VALUE type to note
 *
 * This function will note that a type of value is about to be hashed.
 * The type of a VALUE will be noted.  For purposes of hash comparison,
 * we will do nothing with V_NUM and V_COM so that the other functions
 * can hash to the same value regardless of if shs_value() is called
 * or not.  We also do nothing with V_STR so that a hash of a string
 * will produce the same value as the standard hash function.
 */
static void
shs_type(int type, HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;	/* digest state */
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
	for (i=1; i < SHS_DIGESTWORDS; ++i) {
		dig->digest[i] += ((type+i) ^ dig->digest[i-1]);
	}
	return;
}


/*
 * shs_init_state - initialize a hash state structure for this hash
 *
 * given:
 *	state	- pointer to the hfunction element to initialize
 */
void
shs_init_state(HASH *state)
{
	/*
	 * initalize state
	 */
	state->hashtype = SHS_HASH_TYPE;
	state->bytes = TRUE;
	state->update = shsUpdate;
	state->chkpt = shs_chkpt;
	state->note = shs_note;
	state->type = shs_type;
	state->final = shs_final_state;
	state->cmp = shs_cmp;
	state->print = shs_print;
	state->base = SHS_BASE;
	state->chunksize = SHS_CHUNKSIZE;
	state->unionsize = sizeof(SHS_INFO);

	/*
	 * perform the internal init function
	 */
	memset((void *)&(state->h_union.h_shs), 0, sizeof(SHS_INFO));
	shsInit(state);
	return;
}


/*
 * shs_final_state - complete hash state and return a ZVALUE
 *
 * given:
 *	state	the state to complete and convert
 *
 * returns:
 *	a ZVALUE representing the state
 */
static ZVALUE
shs_final_state(HASH *state)
{
	SHS_INFO *dig = &state->h_union.h_shs;		/* digest state */
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
		shs_init_state(state);
	}

	/*
	 * complete the hash state
	 */
	shsFinal(state);

	/*
	 * allocate storage for ZVALUE
	 */
	ret.len = SHS_DIGESTSIZE/sizeof(HALF);
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
 * shs_cmp - compare two hash states
 *
 * given:
 *	a	first hash state
 *	b	second hash state
 *
 * returns:
 *	TRUE => hash states are different
 *	FALSE => hash states are the same
 */
static int
shs_cmp(HASH *a, HASH *b)
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
	if (a->h_union.h_shs.countLo != b->h_union.h_shs.countLo ||
	    a->h_union.h_shs.countHi != b->h_union.h_shs.countHi) {
		/* counts differ */
		return TRUE;
	}

	/*
	 * compare pending buffers
	 */
	if (a->h_union.h_shs.datalen != b->h_union.h_shs.datalen) {
		/* buffer lengths differ */
		return TRUE;
	}
	if (memcmp((USB8*)a->h_union.h_shs.data,
		   (USB8*)b->h_union.h_shs.data,
		   a->h_union.h_shs.datalen) != 0) {
		/* buffer contents differ */
		return TRUE;
	}

	/*
	 * compare digest
	 */
	return (memcmp((USB8*)(a->h_union.h_shs.digest),
		       (USB8*)(b->h_union.h_shs.digest),
		       SHS_DIGESTSIZE) != 0);
}


/*
 * shs_print - print a hash state
 *
 * given:
 *	state	the hash state to print
 */
static void
shs_print(HASH *state)
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
		sprintf(buf,
			"sha: 0x%08x%08x%08x%08x%08x data: %d octets",
			(int)state->h_union.h_shs.digest[0],
			(int)state->h_union.h_shs.digest[1],
			(int)state->h_union.h_shs.digest[2],
			(int)state->h_union.h_shs.digest[3],
			(int)state->h_union.h_shs.digest[4],
			(int)state->h_union.h_shs.datalen);
		math_str(buf);
	} else {
		math_str("sha hash state");
	}
	return;
}
