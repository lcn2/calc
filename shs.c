/* XXX - this code is currently not really used, but it will be soon */
/*
 * shs - old Secure Hash Standard
 *
 **************************************************************************
 * This version implements the old Secure Hash Algorithm specified by     *
 * (FIPS Pub 180).  This version is kept for backward compatibility with  *
 * shs version 2.10.1.  See the shs utility for the new standard.	  *
 **************************************************************************
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file was Modified/Re-written by:
 *
 *	 Landon Curt Noll  (chongo@toad.com)	chongo <was here> /\../\
 *
 * This code has been placed in the public domain.  Please do not
 * copyright this code.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH  REGARD  TO
 * THIS  SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS.  IN NO EVENT SHALL  LANDON  CURT
 * NOLL  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM  LOSS  OF
 * USE,  DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Based on Version 2.11 (09 Mar 1995) from Landon Curt Noll's
 * (chongo@toad.com) shs hash program.
 *
 ****
 *
 * The SHS algorithm hashes 32 bit unsigned values, 16 at a time.
 * It further specifies that strings are to be converted into
 * 32 bit values in BIG ENDIAN order.  That is on little endian
 * machines, strings are byte swaped into BIG ENDIAN order before
 * they are taken 32 bit at a time.  Even so, when hashing 32 bit
 * numeric values the byte order DOES NOT MATTER because the
 * algorithm works off of their numeric value, not their byte order.
 *
 * In calc, we want to hash equal values to the same hash value.
 * For the most part, we will be hashing arrays of HALF's instead
 * of strings.  For this reason, the functions below do not byte
 * swap on little endian machines automatically.  Instead it is
 * the responsibility of the caller of the internal SHS function
 * to ensure that the values are already in the canonical 32 bit
 * numeric value form.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "calc.h"
#include "zrand.h"
#include "longbits.h"
#include "align32.h"
#include "endian_calc.h"
#include "shs.h"
#include "value.h"


/*
 * The SHS f()-functions.  The f1 and f3 functions can be optimized
 * to save one boolean operation each - thanks to Rich Schroeppel,
 * rcs@cs.arizona.edu for discovering this.
 *
 * f1: ((x&y) | (~x&z)) == (z ^ (x&(y^z)))
 * f3: ((x&y) | (x&z) | (y&z)) == ((x&y) | (z&(x|y)))
 */
#define f1(x,y,z)       (z ^ (x&(y^z)))		/* Rounds  0-19 */
#define f2(x,y,z)       (x^y^z)			/* Rounds 20-39 */
#define f3(x,y,z)       ((x&y) | (z&(x|y)))	/* Rounds 40-59 */
#define f4(x,y,z)       (x^y^z)			/* Rounds 60-79 */

/* The SHS Mysterious Constants */
#define K1      0x5A827999L	/* Rounds  0-19 */
#define K2      0x6ED9EBA1L	/* Rounds 20-39 */
#define K3      0x8F1BBCDCL	/* Rounds 40-59 */
#define K4      0xCA62C1D6L	/* Rounds 60-79 */

/* SHS initial values */
#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* 32-bit rotate left - kludged with shifts */
#define LEFT_ROT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

/*
 * The initial expanding function.  The hash function is defined over an
 * 80-word expanded input array W, where the first 16 are copies of the input
 * data, and the remaining 64 are defined by
 *
 *      W[i] = W[i-16] ^ W[i-14] ^ W[i-8] ^ W[i-3]
 *
 * This implementation generates these values on the fly in a circular
 * buffer - thanks to Colin Plumb (colin@nyx10.cs.du.edu) for this
 * optimization.
 */
#define exor(W,i) (W[i&15] ^= (W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15]))

/*
 * The prototype SHS sub-round.  The fundamental sub-round is:
 *
 *      a' = e + LEFT_ROT(a,5) + f(b,c,d) + k + data;
 *      b' = a;
 *      c' = LEFT_ROT(b,30);
 *      d' = c;
 *      e' = d;
 *
 * but this is implemented by unrolling the loop 5 times and renaming the
 * variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
 * This code is then replicated 20 times for each of the 4 functions, using
 * the next 20 values from the W[] array each time.
 */
#define subRound(a, b, c, d, e, f, k, data) \
    (e += LEFT_ROT(a,5) + f(b,c,d) + k + data, b = LEFT_ROT(b,30))


/* forward declarations */
#if defined(MUST_ALIGN32)
static USB32 in[SHS_CHUNKWORDS];
#endif
static void shsInit(SHS_INFO*);
static void shsTransform(USB32*, USB32*);
static void shsUpdate(SHS_INFO*, USB8*, USB32);
static void shsfullUpdate(SHS_INFO*, USB8*, USB32);
static void shsFinal(SHS_INFO*);
static void shs_chkpt(HASH*);
static void shs_note(HASH*, int);
static void shs_type(HASH*, int);
static HASH *shs_init(HASH*);
static HASH *shs_long(HASH*, long);
static HASH *shs_zvalue(HASH*, ZVALUE);
static HASH *shs_number(HASH*, NUMBER*);
static HASH *shs_complex(HASH*, COMPLEX*);
static HASH *shs_str(HASH*, char*);
static HASH *shs_value(HASH*, VALUE*);
static ZVALUE shs_final(HASH*);


/*
 * shsInit - initialize the SHS state
 */
static void
shsInit(SHS_INFO *dig)
{
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
 *
 * This code does not assume that the buffer size is a multiple of
 * SHS_CHUNKSIZE bytes long.  This code handles partial chunk between
 * calls to shsUpdate().
 */
static void
shsUpdate(SHS_INFO *dig, USB8 *buffer, USB32 count)
{
	USB32 datalen = dig->datalen;

	/*
	 * Catch the case of a non-empty data buffer
	 */
	if (datalen > 0) {

		/* determine the size we need to copy */
		USB32 cpylen = SHS_CHUNKSIZE - datalen;

		/* case: new data will not fill the buffer */
		if (cpylen > count) {
			memcpy((char *)dig->data+datalen,
			       (char *)buffer, count);
			dig->datalen = datalen+count;
			return;

		/* case: buffer will be filled */
		} else {
			memcpy((char *)dig->data+datalen,
			       (char *)buffer, cpylen);
			shsTransform(dig->digest, dig->data);
			buffer += cpylen;
			count -= cpylen;
			dig->datalen = 0;
		}
	}

	/*
	 * Process data in SHS_CHUNKSIZE chunks
	 */
	if (count >= SHS_CHUNKSIZE) {
		shsfullUpdate(dig, buffer, count);
		buffer += (count/SHS_CHUNKSIZE)*SHS_CHUNKSIZE;
		count %= SHS_CHUNKSIZE;
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
 * shsfullUpdate - update SHS with chunk multiple length data
 *
 * This function assumes that count is a multiple of SHS_CHUNKSIZE and that
 * no partial chunk is left over from a previous call.
 */
static void
shsfullUpdate(SHS_INFO *dig, USB8 *buffer, USB32 count)
{
	/*
	 * Process data in SHS_CHUNKSIZE chunks
	 */
	while (count >= SHS_CHUNKSIZE) {
#if defined(MUST_ALIGN32)
		if ((long)buffer & (sizeof(USB32)-1)) {
			memcpy((char *)in, (char *)buffer, SHS_CHUNKSIZE);
			shsTransform(dig->digest, in);
		} else {
			shsTransform(dig->digest, (USB32 *)buffer);
		}
#else
		shsTransform(dig->digest, (USB32 *)buffer);
#endif
		buffer += SHS_CHUNKSIZE;
		count -= SHS_CHUNKSIZE;
	}
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
 * chunk byte.  Regardless, we wind up with 56 bytes data.
 *
 * Finally we append the 64 bit length on to the 56 bytes of data
 * remaining.  This final chunk is transformed.
 */
static void
shsFinal(SHS_INFO *dig)
{
	long count = (long)(dig->datalen);
	USB32 lowBitcount = dig->countLo;
	USB32 highBitcount = dig->countHi;
#if BYTE_ORDER == LITTLE_ENDIAN
	int i;
#endif

	/*
	 * Set the first char of padding to 0x80.
	 * This is safe since there is always at least one byte free
	 */
	((USB8 *)dig->data)[count++] = 0x80;

	/* Pad out to 56 mod SHS_CHUNKSIZE */
	if (count > SHS_CHUNKSIZE-8) {
		/* Pad the first chunk to SHS_CHUNKSIZE bytes */
		memset((USB8 *)dig->data + count, 0, SHS_CHUNKSIZE - count);
		shsTransform(dig->digest, dig->data);

		/* Now fill the next chunk with 56 bytes */
		memset(dig->data, 0, SHS_CHUNKSIZE-8);
	} else {
		/* Pad chunk to 56 bytes */
		memset((USB8 *)dig->data + count, 0, SHS_CHUNKSIZE-8 - count);
	}
#if BYTE_ORDER == LITTLE_ENDIAN
	for (i=0; i < SHS_CHUNKWORDS; ++i) {
		SWAP_B8_IN_B32(dig->data+i, dig->data+i);
	}
#endif

	/*
	 * Append length in bits and transform
	 *
	 * We assume that bit count is a multiple of 8 because we have
	 * only processed full bytes.
	 */
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
	SHS_INFO *dig = &state->h_shs;	/* digest state */

	/*
	 * checkpoint if partial buffer exists
	 */
	if (dig->datalen > 0) {

		/* pad to the end of the chunk */
		memset((USB8 *)dig->data + dig->datalen, 0,
		       SHS_CHUNKSIZE-dig->datalen);

		/* transform padded chunk */
		shsTransform(dig->digest, dig->data);
		SHSCOUNT(dig, SHS_CHUNKSIZE-dig->datalen);

		/* empty buffer */
		dig->datalen = 0;

		/* previous value is now not a string */
		state->prevstr = FALSE;
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
shs_note(HASH *state, int special)
{
	SHS_INFO *dig = &state->h_shs;	/* digest state */
	int i;

	/*
	 * change state to reflect a special value
	 */
	dig->digest[0] ^= special;
	for (i=1; i < SHS_DIGESTWORDS; ++i) {
		dig->digest[i] ^= (special + dig->digest[i-1] + i);
	}
	state->prevstr = FALSE;	/* it is as we just hashed a non-string */
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
 * can hash to the same value reguardless of if shs_value() is called
 * or not.  We also do nothing with V_STR so that a hash of a string
 * will produce the same value as the standard hash function.
 */
static void
shs_type(HASH *state, int type)
{
	SHS_INFO *dig = &state->h_shs;	/* digest state */
	int i;

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
	state->prevstr = FALSE;	/* it is as if we just hashed a non-string */
	return;
}


/*
 * shs_init - initialize SHS hash state
 *
 * given:
 *	state	the state to initialize, or NULL to malloc it
 *
 * returns:
 *	initialized state
 */
static HASH *
shs_init(HASH *state)
{
	/*
	 * malloc if needed
	 */
	if (state == NULL) {
		state = (HASH *)malloc(sizeof(HASH));
		if (state == NULL) {
			math_error("cannot malloc HASH");
			/*NOTREACHED*/
		}
	}

	/*
	 * initialize
	 */
	shsInit((SHS_INFO *)state);
	state->prevstr = FALSE;

	/*
	 * return state
	 */
	return (HASH *)state;
}


/*
 * shs_long - note a long value
 *
 * given:
 *	state		the state to hash
 *	longval		a long value
 *
 * returns:
 *      the new state
 *
 * This function will hash a long value as if it were a 64 bit value.
 * The input is a long.  If a long is smaller than 64 bits, we will
 * hash a final 32 bits of zeros.
 */
static HASH *
shs_long(HASH *state, long longval)
{
	SHS_INFO *dig;			/* digest state */
	long lval[64/LONG_BITS];	/* 64 bits of longs */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the long value hash
	 */
	shs_chkpt(state);

	/*
	 * catch the zero numeric value special case
	 */
	if (longval == 0) {
		/* note a zero numeric value and return */
		shs_note(state, SHS_HASH_ZERO);
		state->prevstr = FALSE;	/* we just hashed a non-string */
		return state;
	}

	/*
	 * prep for a long value hash
	 */
	shs_note(state, SHS_BASE);
	dig = &state->h_shs;

	/*
	 * hash as if we have a 64 bit value
	 */
	memset((char *)lval, 0, sizeof(lval));
	lval[0] = longval;
	shsUpdate(dig, (USB8 *)lval, sizeof(lval));
	SHSCOUNT(dig, 64/8);

	/*
	 * all done
	 */
	state->prevstr = FALSE;	/* we just hashed a non-string */
	return state;
}


/*
 * shs_zvalue - hash a ZVALUE
 *
 * given:
 *	state	the state to hash or NULL
 *	zval	the ZVALUE
 *
 * returns:
 *	the new state
 */
static HASH *
shs_zvalue(HASH *state, ZVALUE zval)
{
	SHS_INFO *dig;			/* digest state */
#if BYTE_ORDER == BIG_ENDIAN && BASEB == 16
	HALF half[SHS_CHUNKHALFS];	/* SHS chunk buffer as HALFs */
	int full_lim;			/* HALFs in whole chunks in zval */
	int i;
	int j;
#endif

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the ZVALUE hash
	 */
	shs_chkpt(state);

	/*
	 * catch the zero numeric value special case
	 */
	if (ziszero(zval)) {
		/* note a zero numeric value and return */
		shs_note(state, SHS_HASH_ZERO);
		state->prevstr = FALSE;	/* we just hashed a non-string */
		return state;
	}

	/*
	 * prep for a ZVALUE hash
	 */
	shs_note(state, SHS_HASH_ZVALUE);
	/* note if we have a negative value */
	if (zisneg(zval)) {
		shs_note(state, SHS_HASH_NEG);
	}
	dig = &state->h_shs;

#if BYTE_ORDER == BIG_ENDIAN && BASEB == 16

	/*
	 * hash full chunks
	 *
	 * We need to convert the array of HALFs into canonical architectural
	 * independent form -- 32 bit arrays.  Because we have 16 bit values
	 * in Big Endian form, we need to swap 16 bit values so that they
	 * appear as 32 bit Big Endian values.
	 */
	full_lim = (zval.len / SHS_CHUNKHALFS) * SHS_CHUNKHALFS;
	for (i=0; i < full_lim; i += SHS_CHUNKHALFS) {
		/* HALF swap copy a chunk into a data buffer */
		for (j=0; j < SHS_CHUNKHALFS; j += 2) {
			half[j] = zval.v[i+j+1];
			half[j+1] = zval.v[i+j];
		}
		shsfullUpdate(dig, (USB8 *)half, SHS_CHUNKSIZE);
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
			half[j] = zval.v[full_lim+i+1];
			half[j+1] = zval.v[full_lim+i];
		}
		if (j < zval.len-full_lim) {
			half[j] = (HALF)0;
			half[j+1] = zval.v[zval.len-1];
			--full_lim;
			SHSCOUNT(dig, sizeof(HALF));
		}
		shsUpdate(dig, (USB8 *)half,
			  (zval.len-full_lim)*sizeof(HALF));
	}
	SHSCOUNT(dig, zval.len*sizeof(HALF));

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
	shsUpdate(dig, (USB8 *)zval.v, zval.len*sizeof(HALF));
	SHSCOUNT(dig, zval.len*sizeof(HALF));

#endif

	/*
	 * all done
	 */
	state->prevstr = FALSE;	/* we just hashed a non-string */
	return state;
}


/*
 * shs_number - hash a NUMBER
 *
 * given:
 *	state	the state to hash or NULL
 *	number	the NUMBER
 *
 * returns:
 *	the new state
 */
static HASH *
shs_number(HASH *state, NUMBER *number)
{
	BOOL sign;			/* sign of the denominator */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the ZVALUE hash
	 */
	shs_chkpt(state);

	/*
	 * process the numerator
	 */
	state = shs_zvalue(state, number->num);

	/*
	 * if the NUMBER is not an integer, process the denominator
	 */
	if (qisfrac(number)) {

		/* note the division */
		shs_note(state, SHS_HASH_DIV);

		/* hash denominator as positive -- just in case */
		sign = number->den.sign;
		number->den.sign = 0;

		/* hash the denominator */
		state = shs_zvalue(state, number->den);

		/* restore the sign */
		number->den.sign = sign;
	}

	/*
	 * all done
	 */
	state->prevstr = FALSE;	/* we just hashed a non-string */
	return state;
}


/*
 * shs_complex - hash a COMPLEX
 *
 * given:
 *	state	the state to hash or NULL
 *	complex	the COMPLEX
 *
 * returns:
 *	the new state
 */
static HASH *
shs_complex(HASH *state, COMPLEX *complex)
{
	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the COMPLEX hash
	 */
	shs_chkpt(state);

	/*
	 * catch the zero special case
	 */
	if (ciszero(complex)) {
		/* note a zero numeric value and return */
		shs_note(state, SHS_HASH_ZERO);
		state->prevstr = FALSE;	/* we just hashed a non-string */
		return state;
	}

	/*
	 * process the real value if not pure imaginary
	 *
	 * We will ignore the real part if the value is of the form 0+xi.
	 */
	if (!qiszero(complex->real)) {
		state = shs_number(state, complex->real);
	}

	/*
	 * if the NUMBER is not real, process the imaginary value
	 *
	 * We will ignore the imaginary part of the value is of the form x+0i.
	 */
	if (!cisreal(complex)) {

		/* note the sqrt(-1) */
		shs_note(state, SHS_HASH_COMPLEX);

		/* hash the imaginary value */
		state = shs_number(state, complex->imag);
	}

	/*
	 * all done
	 */
	state->prevstr = FALSE;	/* we just hashed a non-string */
	return state;
}


/*
 * shs_str - hash a string
 *
 * given:
 *	state	the state to hash or NULL
 *	str	the string
 *
 * returns:
 *	the new state
 */
static HASH *
shs_str(HASH *state, char *str)
{
	SHS_INFO *dig;			/* digest state */
#if BYTE_ORDER == LITTLE_ENDIAN
	char *newstr;		/* Big Endian version of str */
	USB32 newlen;		/* newstr string length */
	int i;
#endif
	USB32 len;			/* string length */

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the string hash
	 */
	if (!state->prevstr) {
		shs_chkpt(state);
	}
	len = strlen(str);
	dig = &state->h_shs;

#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * shs hashes in Big Endian form directly
	 */
	shsUpdate(dig, (USB8*)str, len);
#else
	/*
	 * we must convert from Little Endian string to Big Endian string
	 */
	newlen = ((len+3)/4)*4;
	newstr = (char *)malloc(newlen+1);
	if (newstr) {
		math_error("hash of string malloc failed");
		/*NOTREACHED*/
	}
	strcpy(newstr, str);
	newstr[len+1] = 0;
	newstr[len+2] = 0;
	newstr[len+3] = 0;
	for (i=0; i < newlen; i += 4) {
		SWAP_B8_IN_B32(newstr+i, newstr+i);
	}
	shsUpdate(dig, (USB8*)newstr, newlen);
#endif
	SHSCOUNT((SHS_INFO *)dig, len);

	/*
	 * all done
	 */
	state->prevstr = TRUE;	/* we just hashed a string */
	return state;
}


/*
 * shs_value - hash a value
 *
 * given:
 *	state	the state to hash or NULL
 *	value	the value
 *
 * returns:
 *	the new state
 */
static HASH *
shs_value(HASH *state, VALUE *value)
{
	SHS_INFO *dig;		/* digest state */
	LISTELEM *ep;		/* list element pointer */
	ASSOCELEM **assochead;	/* association chain head */
	ASSOCELEM *aep;		/* current association value */
	ASSOCELEM *nextaep;	/* next association value */
	VALUE *vp;		/* pointer to next OBJ table value */
	ZVALUE fileval;		/* size, position, dev, inode of a file */
	int i;

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * setup for the next type of value
	 */
	shs_chkpt(state);
	shs_type(state, value->v_type);
	dig = &state->h_shs;

	/*
	 * process the value type
	 */
	switch (value->v_type) {
	case V_INT:
		/* hash as if we have a 64 bit value */
		state = shs_long(state, (long)value->v_int);
		break;
	case V_NUM:
		state = shs_number(state, value->v_num);
		break;
	case V_COM:
		state = shs_complex(state, value->v_com);
		break;
	case V_ADDR:
		state = shs_value(state, value->v_addr);
		break;
	case V_STR:
		state = shs_str(state, value->v_str);
		break;
	case V_MAT:
		/* hash all the elements of the matrix */
		for (i=0; i < value->v_mat->m_size; ++i) {
			/* force strings to not be concatinated */
			state->prevstr = FALSE;
			/* hash the next matrix value */
			state = shs_value(state, value->v_mat->m_table+i);
		}
		/* don't allow the next string to concatinate to the matrix */
		state->prevstr = FALSE;
		break;
	case V_LIST:
		/* hash all the elements of the list */
		for (i=0, ep = value->v_list->l_first;
		     ep != NULL && i < value->v_list->l_count;
		     ++i, ep = ep->e_next) {
			/* force strings to not be concatinated */
			state->prevstr = FALSE;
			/* hash the next list value */
			state = shs_value(state, &ep->e_value);
		}
		/* don't allow the next string to concatinate to the list */
		state->prevstr = FALSE;
		break;
	case V_ASSOC:
		assochead = value->v_assoc->a_table;
		for (i = 0; i < value->v_assoc->a_size; i++) {
			nextaep = *assochead;
			while (nextaep) {
				aep = nextaep;
				nextaep = aep->e_next;
				/* force strings to not be concatinated */
				state->prevstr = FALSE;
				/* hash the next association value */
				state = shs_value(state, &aep->e_value);
			}
			assochead++;
		}
		/* don't allow the next string to concatinate to the assoc */
		state->prevstr = FALSE;
		break;
	case V_OBJ:
		for (i=value->v_obj->o_actions->count, vp=value->v_obj->o_table;
		     i-- > 0;
		     vp++) {
			/* force strings to not be concatinated */
			state->prevstr = FALSE;
			/* hash the next object value */
			shs_value(state, vp);
		}
		/* don't allow the next string to concatinate to the object */
		state->prevstr = FALSE;
		break;
	case V_FILE:
		/* hash file length if possible */
		if (getsize(value->v_file, &fileval) == 0) {
			state = shs_zvalue(state, fileval);
			zfree(fileval);
		} else {
			/* hash -1 for invalid length */
			state = shs_long(state, (long)-1);
		}
		/* hash the file position if possible */
		if (getloc(value->v_file, &fileval) == 0) {
			state = shs_zvalue(state, fileval);
			zfree(fileval);
		} else {
			/* hash -1 for invalid location */
			state = shs_long(state, (long)-1);
		}
		/* hash the file device if possible */
		if (get_device(value->v_file, &fileval) == 0) {
			state = shs_zvalue(state, fileval);
			zfree(fileval);
		} else {
			/* hash -1 for invalid device */
			state = shs_long(state, (long)-1);
		}
		/* hash the file inode if possible */
		if (get_inode(value->v_file, &fileval) == 0) {
			state = shs_zvalue(state, fileval);
			zfree(fileval);
		} else {
			/* hash -1 for invalid inode */
			state = shs_long(state, (long)-1);
		}
		break;
	case V_RAND:
		state = shs_long(state, (long)value->v_rand->seeded);
		state = shs_long(state, (long)value->v_rand->bits);
		shsUpdate(dig, (USB8 *)value->v_rand->buffer, SLEN*FULL_BITS/8);
		SHSCOUNT(dig, SLEN*FULL_BITS/8);
		state = shs_long(state, (long)value->v_rand->j);
		state = shs_long(state, (long)value->v_rand->k);
		shsUpdate(dig, (USB8 *)value->v_rand->slot, SCNT*FULL_BITS/8);
		SHSCOUNT(dig, SCNT*FULL_BITS/8);
		shsUpdate(dig, (USB8*)value->v_rand->shuf, SHUFLEN*FULL_BITS/8);
		SHSCOUNT(dig, SHUFLEN*FULL_BITS/8);
		/* don't allow the next string to concatinate to the list */
		state->prevstr = FALSE;
		break;
	case V_RANDOM:
		state = shs_long(state, (long)value->v_random->seeded);
		state = shs_long(state, (long)value->v_random->bits);
		shsUpdate(dig, (USB8 *)&(value->v_random->buffer), BASEB/8);
		SHSCOUNT(dig, SLEN*FULL_BITS/8);
		state = shs_zvalue(state, *(value->v_random->r));
		state = shs_zvalue(state, *(value->v_random->n));
		/* don't allow the next string to concatinate to the list */
		state->prevstr = FALSE;
		break;
	case V_CONFIG:
		state = shs_long(state, (long)value->v_config->outmode);
		state = shs_long(state, (long)value->v_config->outdigits);
		state = shs_number(state, value->v_config->epsilon);
		state = shs_long(state, (long)value->v_config->epsilonprec);
		state = shs_long(state, (long)value->v_config->traceflags);
		state = shs_long(state, (long)value->v_config->maxprint);
		state = shs_long(state, (long)value->v_config->mul2);
		state = shs_long(state, (long)value->v_config->sq2);
		state = shs_long(state, (long)value->v_config->pow2);
		state = shs_long(state, (long)value->v_config->redc2);
		state = shs_long(state, (long)value->v_config->tilde_ok);
		state = shs_long(state, (long)value->v_config->tab_ok);
		state = shs_long(state, (long)value->v_config->quomod);
		state = shs_long(state, (long)value->v_config->quo);
		state = shs_long(state, (long)value->v_config->mod);
		state = shs_long(state, (long)value->v_config->sqrt);
		state = shs_long(state, (long)value->v_config->appr);
		state = shs_long(state, (long)value->v_config->cfappr);
		state = shs_long(state, (long)value->v_config->cfsim);
		state = shs_long(state, (long)value->v_config->outround);
		state = shs_long(state, (long)value->v_config->round);
		state = shs_long(state, (long)value->v_config->leadzero);
		state = shs_long(state, (long)value->v_config->fullzero);
		state = shs_long(state, (long)value->v_config->maxerrorcount);
		state = shs_str(state, value->v_config->prompt1);
		state = shs_str(state, value->v_config->prompt2);
		/* don't allow the next string to concatinate to the list */
		state->prevstr = FALSE;
		break;
	case V_HASH:
		if (value->v_hash->type == SHS_HASH_TYPE) {
			shsUpdate(dig, (USB8 *)&value->v_hash->h_shs,
				  sizeof(SHS_INFO));
			SHSCOUNT(dig, sizeof(SHS_INFO));
		} else {
			math_error("SHS hashing a non-SHS hash state");
			/*NOTREACHED*/
		}
		/* don't allow the next string to concatinate to the list */
		state->prevstr = FALSE;
		break;
	default:
		math_error("hashing an unknown value");
		/*NOTREACHED*/
	}
	return state;
}


/*
 * shs_final - complete hash state and return a ZVALUE
 *
 * given:
 *	state	the state to complete and convert
 *
 * returns:
 *	a ZVALUE representing the state
 */
static ZVALUE
shs_final(HASH *state)
{
	SHS_INFO *dig;		/* digest state */
	ZVALUE ret;		/* return ZVALUE of completed hash state */
#if BTYE_ORDER == BIG_ENDIAN && BASEB == 16
	int i;
#endif

	/*
	 * initialize if state is NULL
	 */
	if (state == NULL) {
		state = shs_init(state);
	}

	/*
	 * complete the hash state
	 */
	dig = &state->h_shs;
	shsFinal(dig);

	/*
	 * allocate storage for ZVALUE
	 */
	ret.len = SHS_DIGESTSIZE/sizeof(HALF);
	ret.sign = 0;
	ret.v = alloc(ret.len);

	/*
	 * load ZVALUE
	 */
#if BTYE_ORDER == BIG_ENDIAN && BASEB == 16
	for (i=0; i < ret.len; i+=2) {
		rev.v[i+1] = ((HALF*)dig->digest)[i];
		rev.v[i] = ((HALF*)dig->digest)[i+1];
	}
#else
	memcpy(ret.v, dig->digest, SHS_DIGESTSIZE);
#endif

	/*
	 * return ZVALUE
	 */
	return ret;
}


/*
 * shs_hashfunc - initialize a hashfunc for an interface for this hash
 *
 * given:
 *	hfunc	- pointer to the hfunction element to initialize
 */
void
shs_hashfunc(HASHFUNC *hfunc)
{
	/*
	 * initalize
	 */
	hfunc->type = SHS_HASH_TYPE;
	hfunc->init = shs_init;
	hfunc->longval = shs_long;
	hfunc->str = shs_str;
	hfunc->value = shs_value;
	hfunc->complex = shs_complex;
	hfunc->number = shs_number;
	hfunc->zvalue = shs_zvalue;
	hfunc->final = shs_final;
	return;
}
