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
 * This file was Modified by:
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
 * See shsdrvr.c for version and modification history.
 */

#if !defined(SHS_H)
#define SHS_H

#include <sys/types.h>
#include <sys/stat.h>

/* SHS_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define SHS_CHUNKSIZE (1<<6)
#define SHS_CHUNKWORDS (SHS_CHUNKSIZE/sizeof(USB32))
#define SHS_CHUNKHALFS (SHS_CHUNKSIZE/sizeof(HALF))

/* SHS_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define SHS_DIGESTSIZE (20)
#define SHS_DIGESTWORDS (SHS_DIGESTSIZE/sizeof(USB32))

/* SHS_LOW - where low 32 bits of 64 bit count is stored during final */
#define SHS_LOW 15

/* SHS_HIGH - where high 32 bits of 64 bit count is stored during final */
#define SHS_HIGH 14

/* what to xor to digest value when hashing special values */
#define SHS_BASE 0x1234face		/* base special hash value */
#define SHS_HASH_NEG (1+SHS_BASE)	/* note a negative value */
#define SHS_HASH_COMPLEX (2+SHS_BASE)	/* note a complex value */
#define SHS_HASH_DIV (4+SHS_BASE)	/* note a division by a value */
#define SHS_HASH_ZERO (8+SHS_BASE)	/* note a zero numeric value */
#define SHS_HASH_ZVALUE (16+SHS_BASE)	/* note a ZVALUE */

/*
 * The structure for storing SHS info
 *
 * We will assume that bit count is a multiple of 8.
 */
typedef struct {
    USB32 digest[SHS_DIGESTWORDS];	/* message digest */
    USB32 countLo;			/* 64 bit count: bits 3-34 */
    USB32 countHi;			/* 64 bit count: bits 35-63 */
    USB32 datalen;			/* length of data in data */
    USB32 data[SHS_CHUNKWORDS];		/* SHS chunk buffer */
} SHS_INFO;

/*
 * SHSCOUNT(SHS_INFO*, USB32) - update the 64 bit count in an SHS_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.  This assumes that the count is < 2^32.
 */
#define SHSCOUNT(shsinfo, count) {				\
    USB32 tmp_countLo;						\
    tmp_countLo = (shsinfo)->countLo;				\
    if (((shsinfo)->countLo += (count)) < tmp_countLo) {	\
	(shsinfo)->countHi++;					\
    }								\
}

#endif
