/*
 * sha1 - new NIST Secure Hash Standard-1 (SHA1)
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file and been extensively modified by:
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


#if !defined(INCLUDE_SHA1_H)
#define INCLUDE_SHA1_H


/* SHA1_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define SHA1_CHUNKSIZE (1<<6)
#define SHA1_CHUNKWORDS (SHA1_CHUNKSIZE/sizeof(USB32))

/* SHA1_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define SHA1_DIGESTSIZE (20)
#define SHA1_DIGESTWORDS (SHA1_DIGESTSIZE/sizeof(USB32))

/* SHA1_LOW - where low 32 bits of 64 bit count is stored during final */
#define SHA1_LOW 15

/* SHA1_HIGH - where high 32 bits of 64 bit count is stored during final */
#define SHA1_HIGH 14

/*
 * The structure for storing SHA1 info
 *
 * We will assume that bit count is a multiple of 8.
 */
typedef struct {
	USB32 digest[SHA1_DIGESTWORDS]; /* message digest */
	USB32 countLo;			/* 64 bit count: bits 3-34 */
	USB32 countHi;			/* 64 bit count: bits 35-63 */
	USB32 datalen;			/* length of data in data */
	USB32 data[SHA1_CHUNKWORDS];	/* SHA1 chunk buffer */
} SHA1_INFO;

/*
 * SHA1COUNT(SHA1_INFO*, USB32) - update the 64 bit count in an SHA1_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#define SHA1COUNT(sha1info, count) {					\
	USB32 tmp_countLo;						\
	tmp_countLo = (sha1info)->countLo;				\
	if (((sha1info)->countLo += (count)) < tmp_countLo) {		\
		(sha1info)->countHi++;					\
	}								\
}


#endif /* !INCLUDE_SHA1_H */
