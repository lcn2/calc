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
 * This file was Modified by:
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
 * @(#) $Revision: 29.2 $
 * @(#) $Id: shs.h,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/shs.h,v $
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 */


#if !defined(__SHS_H__)
#define __SHS_H__


/* SHS_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define SHS_CHUNKSIZE (1<<6)
#define SHS_CHUNKWORDS (SHS_CHUNKSIZE/sizeof(USB32))

/* SHS_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define SHS_DIGESTSIZE (20)
#define SHS_DIGESTWORDS (SHS_DIGESTSIZE/sizeof(USB32))

/* SHS_LOW - where low 32 bits of 64 bit count is stored during final */
#define SHS_LOW 15

/* SHS_HIGH - where high 32 bits of 64 bit count is stored during final */
#define SHS_HIGH 14

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
	USB32 data[SHS_CHUNKWORDS];	/* SHS chunk buffer */
} SHS_INFO;

/*
 * SHSCOUNT(SHS_INFO*, USB32) - update the 64 bit count in an SHS_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.  This assumes that the count is < 2^32.
 */
#define SHSCOUNT(shsinfo, count) {					\
	USB32 tmp_countLo;						\
	tmp_countLo = (shsinfo)->countLo;				\
	if (((shsinfo)->countLo += (count)) < tmp_countLo) {		\
		(shsinfo)->countHi++;					\
	}								\
}


#endif /* !__SHS_H__ */
