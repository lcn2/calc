/*
 * md5 - RSA Data Security, Inc. MD5 Message-Digest Algorithm
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
 * @(#) $Revision: 29.3 $
 * @(#) $Id: md5.c,v 29.3 2004/08/03 12:28:29 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/md5.c,v $
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **								      **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.					      **
 **								      **
 ** License is also granted to make and use derivative works	      **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all	      **
 ** material mentioning or referencing the derived work.	      **
 **								      **
 ** RSA Data Security, Inc. makes no representations concerning	      **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.	      **
 **								      **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.				      **
 ***********************************************************************
 */

/*
 ***********************************************************************
 **  Message-digest routines:					      **
 **  To form the message digest for a message M			      **
 **    (1) Initialize a context buffer md5Ctx using MD5Init	      **
 **    (2) Call MD5Update on md5Ctx and M			      **
 **    (3) Call MD5Final on md5Ctx				      **
 **  The message digest is now in md5Ctx->digest[0...15]	      **
 ***********************************************************************
 */


#include <stdio.h>
#include "longbits.h"
#include "align32.h"
#include "endian_calc.h"
#include "value.h"
#include "hash.h"
#include "md5.h"


/*
 * The F, G, H and I are basic MD5 functions.  The following
 * identity saves one boolean operation.
 *
 * F: (((x) & (y)) | (~(x) & (z))) == ((z) ^ ((x) & ((y) ^ (z))))
 * G: (((x) & (z)) | ((y) & ~(z))) == ((y) ^ ((z) & ((x) ^ (y))))
 */
/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* rotate a 32 bit value */
#define ROT(X,n) (((X)<<(n)) | ((X)>>(32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define FF(a, b, c, d, x, s, ac) \
	{(a) += F((b), (c), (d)) + (x) + (USB32)(ac); \
	 (a) = ROT((a), (s)); \
	 (a) += (b); \
	}
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define GG(a, b, c, d, x, s, ac) \
	{(a) += G((b), (c), (d)) + (x) + (USB32)(ac); \
	 (a) = ROT((a), (s)); \
	 (a) += (b); \
	}
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define HH(a, b, c, d, x, s, ac) \
	{(a) += H((b), (c), (d)) + (x) + (USB32)(ac); \
	 (a) = ROT((a), (s)); \
	 (a) += (b); \
	}
#define S41 6
#define S42 10
#define S43 15
#define S44 21
#define II(a, b, c, d, x, s, ac) \
	{(a) += I((b), (c), (d)) + (x) + (USB32)(ac); \
	 (a) = ROT((a), (s)); \
	 (a) += (b); \
	}

/* forward declaration */
static void MD5Init(HASH *state);
static void MD5Update(HASH *state, USB8 *inBuf, USB32 count);
static void MD5Transform(USB32*, USB32*);
static void MD5Final(HASH *state);
/* static USB32 in[MD5_CHUNKWORDS]; */
static void MD5_chkpt(HASH *state);
static void MD5_note(int special, HASH *state);
static void MD5_type(int type, HASH *state);
static ZVALUE MD5_final_state(HASH *state);
static int MD5_cmp(HASH *a, HASH *b);
static void MD5_print(HASH *state);


/*
 * MD5Init - initialize the message-digest context
 *
 * The routine MD5Init initializes the message-digest context
 * md5Ctx. All fields are set to zero.
 */
static void
MD5Init(HASH *state)
{
	MD5_CTX *md5Ctx = &state->h_union.h_md5;	/* digest state */

	/* load magic initialization constants */
	md5Ctx->digest[0] = (USB32)0x67452301;
	md5Ctx->digest[1] = (USB32)0xefcdab89;
	md5Ctx->digest[2] = (USB32)0x98badcfe;
	md5Ctx->digest[3] = (USB32)0x10325476;

	/* Initialise bit count */
	md5Ctx->countLo = 0L;
	md5Ctx->countHi = 0L;
	md5Ctx->datalen = 0L;
}


/*
 * MD5Update - update message-digest context
 */
static void
MD5Update(HASH *state, USB8 *inBuf, USB32 count)
{
	MD5_CTX *md5Ctx = &state->h_union.h_md5;	/* digest state */
	USB32 datalen = md5Ctx->datalen;
	USB32 cpylen;
#if CALC_BYTE_ORDER == BIG_ENDIAN
	int cnt;
#endif

	/*
	 * Update the full count, even if some of it is buffered for later
	 */
	MD5COUNT(md5Ctx, count);

	/* determine the size we need to copy */
	cpylen = MD5_CHUNKSIZE - datalen;

	/* case: new data will not fill the inBuf */
	if (cpylen > count) {
		memcpy((char *)md5Ctx->data + datalen,
		       (char *)inBuf, count);
		md5Ctx->datalen = datalen + count;
		return;
	}

	/* case: md5Ctx->in will be filled */
	memcpy((char *)md5Ctx->data + datalen, inBuf, cpylen);

	/*
	 * process data in MD5_CHUNKSIZE chunks
	 */
	for (;;) {
#if CALC_BYTE_ORDER == BIG_ENDIAN
		if (state->bytes) {
		/* byte swap data into little endian order */
			for (cnt=0; cnt < (int)MD5_CHUNKWORDS; ++cnt) {
				SWAP_B8_IN_B32(md5Ctx->data + cnt,
					md5Ctx->data + cnt);
			}
		}
#endif
		MD5Transform(md5Ctx->digest, md5Ctx->data);
		inBuf += cpylen;
		count -= cpylen;
		if (count < MD5_CHUNKSIZE)
			break;
		cpylen = MD5_CHUNKSIZE;
		memcpy(md5Ctx->data, inBuf, cpylen);
	}

	/*
	 * Handle any remaining bytes of data.
	 * This should only happen once on the final lot of data
	 */
	if (count > 0) {
		memcpy(md5Ctx->data, inBuf, count);
	}

	md5Ctx->datalen = count;
}


/*
 * MD5Final - terminate the message-digest computation
 *
 * The routine MD5Final terminates the message-digest computation and
 * ends with the desired message digest in md5Ctx->digest[0...15].
 */
static void
MD5Final(HASH *state)
{
	MD5_CTX *md5Ctx = &state->h_union.h_md5;	/* digest state */
	USB32 count = md5Ctx->datalen;
	USB32 lowBitcount = md5Ctx->countLo;
	USB32 highBitcount = md5Ctx->countHi;
	USB8 *data = (USB8 *)md5Ctx->data;
#if CALC_BYTE_ORDER == BIG_ENDIAN
	int i;
#endif

	/* Pad to end of chunk */

	memset(data + count, 0, MD5_CHUNKSIZE - count);

	/*
	 * If processing bytes, set the first byte of padding to 0x80.
	 * if processing words: on a big-endian machine set the first
	 * byte of padding to 0x80000000, on a little-endian machine set
	 * the first four bytes to 0x80.
	 *
	 * This is safe since there is always at least one byte or word free.
	 */

#if CALC_BYTE_ORDER == BIG_ENDIAN
	if (state->bytes) {
		data[count] = 0x80;
		for (i=0; i < (int)MD5_CHUNKWORDS; ++i) {
			SWAP_B8_IN_B32(md5Ctx->data + i,
				md5Ctx->data + i);
		}
	} else {
		if (count % 4) {
			math_error("This should not happen in MD5Final");
			/*NOTREACHED*/
		}
		data[count + 3] = 0x80;
	}
#else
	data[count] = 0x80;
#endif

	if (count >= MD5_CHUNKSIZE-8) {
		MD5Transform(md5Ctx->digest, md5Ctx->data);

		/* Now load another chunk with 56 bytes of padding */
		memset(data, 0, MD5_CHUNKSIZE-8);
	}

	/* append length in bits and transform */
	md5Ctx->data[MD5_LOW] = (lowBitcount << 3);
	md5Ctx->data[MD5_HIGH] = (highBitcount << 3) | (lowBitcount >> 29);

	MD5Transform(md5Ctx->digest, md5Ctx->data);
}


/*
 * Basic MD5 step. Transforms digest based on in.
 */
static void
MD5Transform(USB32 *digest, USB32 *in)
{
	USB32 a = digest[0], b = digest[1], c = digest[2], d = digest[3];

	/* Round 1 */
	FF( a, b, c, d, in[ 0], S11, 3614090360UL); /* 1 */
	FF( d, a, b, c, in[ 1], S12, 3905402710UL); /* 2 */
	FF( c, d, a, b, in[ 2], S13,  606105819UL); /* 3 */
	FF( b, c, d, a, in[ 3], S14, 3250441966UL); /* 4 */
	FF( a, b, c, d, in[ 4], S11, 4118548399UL); /* 5 */
	FF( d, a, b, c, in[ 5], S12, 1200080426UL); /* 6 */
	FF( c, d, a, b, in[ 6], S13, 2821735955UL); /* 7 */
	FF( b, c, d, a, in[ 7], S14, 4249261313UL); /* 8 */
	FF( a, b, c, d, in[ 8], S11, 1770035416UL); /* 9 */
	FF( d, a, b, c, in[ 9], S12, 2336552879UL); /* 10 */
	FF( c, d, a, b, in[10], S13, 4294925233UL); /* 11 */
	FF( b, c, d, a, in[11], S14, 2304563134UL); /* 12 */
	FF( a, b, c, d, in[12], S11, 1804603682UL); /* 13 */
	FF( d, a, b, c, in[13], S12, 4254626195UL); /* 14 */
	FF( c, d, a, b, in[14], S13, 2792965006UL); /* 15 */
	FF( b, c, d, a, in[15], S14, 1236535329UL); /* 16 */

	/* Round 2 */
	GG( a, b, c, d, in[ 1], S21, 4129170786UL); /* 17 */
	GG( d, a, b, c, in[ 6], S22, 3225465664UL); /* 18 */
	GG( c, d, a, b, in[11], S23,  643717713UL); /* 19 */
	GG( b, c, d, a, in[ 0], S24, 3921069994UL); /* 20 */
	GG( a, b, c, d, in[ 5], S21, 3593408605UL); /* 21 */
	GG( d, a, b, c, in[10], S22,   38016083UL); /* 22 */
	GG( c, d, a, b, in[15], S23, 3634488961UL); /* 23 */
	GG( b, c, d, a, in[ 4], S24, 3889429448UL); /* 24 */
	GG( a, b, c, d, in[ 9], S21,  568446438UL); /* 25 */
	GG( d, a, b, c, in[14], S22, 3275163606UL); /* 26 */
	GG( c, d, a, b, in[ 3], S23, 4107603335UL); /* 27 */
	GG( b, c, d, a, in[ 8], S24, 1163531501UL); /* 28 */
	GG( a, b, c, d, in[13], S21, 2850285829UL); /* 29 */
	GG( d, a, b, c, in[ 2], S22, 4243563512UL); /* 30 */
	GG( c, d, a, b, in[ 7], S23, 1735328473UL); /* 31 */
	GG( b, c, d, a, in[12], S24, 2368359562UL); /* 32 */

	/* Round 3 */
	HH( a, b, c, d, in[ 5], S31, 4294588738UL); /* 33 */
	HH( d, a, b, c, in[ 8], S32, 2272392833UL); /* 34 */
	HH( c, d, a, b, in[11], S33, 1839030562UL); /* 35 */
	HH( b, c, d, a, in[14], S34, 4259657740UL); /* 36 */
	HH( a, b, c, d, in[ 1], S31, 2763975236UL); /* 37 */
	HH( d, a, b, c, in[ 4], S32, 1272893353UL); /* 38 */
	HH( c, d, a, b, in[ 7], S33, 4139469664UL); /* 39 */
	HH( b, c, d, a, in[10], S34, 3200236656UL); /* 40 */
	HH( a, b, c, d, in[13], S31,  681279174UL); /* 41 */
	HH( d, a, b, c, in[ 0], S32, 3936430074UL); /* 42 */
	HH( c, d, a, b, in[ 3], S33, 3572445317UL); /* 43 */
	HH( b, c, d, a, in[ 6], S34,   76029189UL); /* 44 */
	HH( a, b, c, d, in[ 9], S31, 3654602809UL); /* 45 */
	HH( d, a, b, c, in[12], S32, 3873151461UL); /* 46 */
	HH( c, d, a, b, in[15], S33,  530742520UL); /* 47 */
	HH( b, c, d, a, in[ 2], S34, 3299628645UL); /* 48 */

	/* Round 4 */
	II( a, b, c, d, in[ 0], S41, 4096336452UL); /* 49 */
	II( d, a, b, c, in[ 7], S42, 1126891415UL); /* 50 */
	II( c, d, a, b, in[14], S43, 2878612391UL); /* 51 */
	II( b, c, d, a, in[ 5], S44, 4237533241UL); /* 52 */
	II( a, b, c, d, in[12], S41, 1700485571UL); /* 53 */
	II( d, a, b, c, in[ 3], S42, 2399980690UL); /* 54 */
	II( c, d, a, b, in[10], S43, 4293915773UL); /* 55 */
	II( b, c, d, a, in[ 1], S44, 2240044497UL); /* 56 */
	II( a, b, c, d, in[ 8], S41, 1873313359UL); /* 57 */
	II( d, a, b, c, in[15], S42, 4264355552UL); /* 58 */
	II( c, d, a, b, in[ 6], S43, 2734768916UL); /* 59 */
	II( b, c, d, a, in[13], S44, 1309151649UL); /* 60 */
	II( a, b, c, d, in[ 4], S41, 4149444226UL); /* 61 */
	II( d, a, b, c, in[11], S42, 3174756917UL); /* 62 */
	II( c, d, a, b, in[ 2], S43,  718787259UL); /* 63 */
	II( b, c, d, a, in[ 9], S44, 3951481745UL); /* 64 */

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
}


/*
 * MD5_chkpt - checkpoint a MD5 state
 *
 * given:
 *	state	the state to checkpoint
 *
 * This function will ensure that the the hash chunk buffer is empty.
 * Any partially hashed data will be padded out with 0's and hashed.
 */
static void
MD5_chkpt(HASH *state)
{
	MD5_CTX *dig = &state->h_union.h_md5;	/* digest state */
#if CALC_BYTE_ORDER == BIG_ENDIAN
	int cnt;
#endif

	/*
	 * checkpoint if partial buffer exists
	 */
	if (dig->datalen > 0) {

		/* pad to the end of the chunk */
		memset((USB8 *)dig->data + dig->datalen, 0,
		       MD5_CHUNKSIZE-dig->datalen);
#if CALC_BYTE_ORDER == BIG_ENDIAN
		if (state->bytes) {
			/* byte swap data into little endian order */
			for (cnt=0; cnt < (int)MD5_CHUNKWORDS; ++cnt) {
				SWAP_B8_IN_B32(dig->data + cnt,
					dig->data + cnt);
			}
		}
#endif

		/* transform padded chunk */
		MD5Transform((USB32*)dig->digest, dig->data);
		MD5COUNT(dig, MD5_CHUNKSIZE-dig->datalen);

		/* empty buffer */
		dig->datalen = 0;
	}
}


/*
 * MD5_note - note a special value
 *
 * given:
 *	state		the state to hash
 *	special		a special value (MD5_HASH_XYZ) to note
 *
 * This function will note that a special value is about to be hashed.
 * Types include negative values, complex values, division, zero numeric
 * and array of HALFs.
 */
static void
MD5_note(int special, HASH *state)
{
	MD5_CTX *dig = &state->h_union.h_md5;	/* digest state */
	unsigned int i;

	/*
	 * change state to reflect a special value
	 */
	dig->digest[0] ^= special;
	for (i=1; i < MD5_DIGESTWORDS; ++i) {
		dig->digest[i] ^= (special + dig->digest[i-1] + i);
	}
	return;
}


/*
 * MD5_type - note a VALUE type
 *
 * given:
 *	state		the state to hash
 *	type		the VALUE type to note
 *
 * This function will note that a type of value is about to be hashed.
 * The type of a VALUE will be noted.  For purposes of hash comparison,
 * we will do nothing with V_NUM and V_COM so that the other functions
 * can hash to the same value regardless of if MD5_value() is called
 * or not.  We also do nothing with V_STR so that a hash of a string
 * will produce the same value as the standard hash function.
 */
static void
MD5_type(int type, HASH *state)
{
	MD5_CTX *dig = &state->h_union.h_md5;	/* digest state */
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
	for (i=1; i < MD5_DIGESTWORDS; ++i) {
		dig->digest[i] += ((type+i) ^ dig->digest[i-1]);
	}
	return;
}


/*
 * MD5_init_state - initialize a hash state structure for this hash
 *
 * given:
 *	state	- pointer to the hfunction element to initialize
 */
void
MD5_init_state(HASH *state)
{
	/*
	 * initalize state
	 */
	state->hashtype = MD5_HASH_TYPE;
	state->bytes = TRUE;
	state->update = MD5Update;
	state->chkpt = MD5_chkpt;
	state->note = MD5_note;
	state->type = MD5_type;
	state->final = MD5_final_state;
	state->cmp = MD5_cmp;
	state->print = MD5_print;
	state->base = MD5_BASE;
	state->chunksize = MD5_CHUNKSIZE;
	state->unionsize = sizeof(MD5_CTX);

	/*
	 * perform the internal init function
	 */
	memset((void *)&(state->h_union.h_md5), 0, sizeof(MD5_CTX));
	MD5Init(state);
	return;
}


/*
 * MD5_final_state - complete hash state and return a ZVALUE
 *
 * given:
 *	state	the state to complete and convert
 *
 * returns:
 *	a ZVALUE representing the state
 */
static ZVALUE
MD5_final_state(HASH *state)
{
	MD5_CTX *dig = &state->h_union.h_md5;		/* digest state */
	ZVALUE ret;		/* return ZVALUE of completed hash state */
	unsigned int i;

	/*
	 * malloc and initialize if state is NULL
	 */
	if (state == NULL) {
		state = (HASH *)malloc(sizeof(HASH));
		if (state == NULL) {
			math_error("cannot malloc HASH");
			/*NOTREACHED*/
		}
		MD5_init_state(state);
	}

	/*
	 * complete the hash state
	 */
	MD5Final(state);

	/*
	 * allocate storage for ZVALUE
	 */
	ret.len = MD5_DIGESTSIZE/sizeof(HALF);
	ret.sign = 0;
	ret.v = alloc(ret.len);

	/*
	 * load ZVALUE
	 */
#if CALC_BYTE_ORDER == LITTLE_ENDIAN && BASEB == 16
	for (i = 0; i < MD5_DIGESTSIZE; i += 2) {
		SWAP_B8_IN_B16(((USB8 *)dig->digest) + i,
			((USB8 *) dig->digest) + i);
	}
#else
	for (i = 0; i < MD5_DIGESTWORDS; ++i) {
		SWAP_B8_IN_B32(dig->digest + i, dig->digest + i);
	}
#endif

	for (i=0; i < (unsigned int)ret.len; ++i) {
		ret.v[ret.len-i-1] = ((HALF*)dig->digest)[i];
	}

	ztrim(&ret);

	/*
	 * return ZVALUE
	 */
	return ret;
}


/*
 * MD5_cmp - compare two hash states
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
MD5_cmp(HASH *a, HASH *b)
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
	 * compare concat states
	 */
	if (a->bytes != b->bytes)
		return TRUE;

	/*
	 * compare bit counts
	 */
	if (a->h_union.h_md5.countLo != b->h_union.h_md5.countLo ||
	    a->h_union.h_md5.countHi != b->h_union.h_md5.countHi) {
		/* counts differ */
		return TRUE;
	}

	/*
	 * compare pending buffers
	 */
	if (a->h_union.h_md5.datalen != b->h_union.h_md5.datalen) {
		/* buffer lengths differ */
		return TRUE;
	}
	if (memcmp((char*)a->h_union.h_md5.data,
		   (char*)b->h_union.h_md5.data,
		   a->h_union.h_md5.datalen) != 0) {
		/* buffer contents differ */
		return TRUE;
	}

	/*
	 * compare digest
	 */
	return (memcmp((char*)(a->h_union.h_md5.digest),
		       (char*)(b->h_union.h_md5.digest),
		       MD5_DIGESTSIZE) != 0);
}


/*
 * MD5_print - print a hash state
 *
 * given:
 *	state	the hash state to print
 */
static void
MD5_print(HASH *state)
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
			"md5: 0x%08x%08x%08x%08x data: %d octets",
			(int)state->h_union.h_md5.digest[0],
			(int)state->h_union.h_md5.digest[1],
			(int)state->h_union.h_md5.digest[2],
			(int)state->h_union.h_md5.digest[3],
			(int)state->h_union.h_md5.datalen);
		math_str(buf);
	} else {
		math_str("md5 hash state");
	}
	return;
}
