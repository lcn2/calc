/*
 * shs1 - new NIST Secure Hash Standard-1 (SHS1)
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
 * @(#) $Revision: 29.2 $
 * @(#) $Id: shs1.h,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/shs1.h,v $
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 */


#if !defined(__SHS1_H__)
#define __SHS1_H__


/* SHS1_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define SHS1_CHUNKSIZE (1<<6)
#define SHS1_CHUNKWORDS (SHS1_CHUNKSIZE/sizeof(USB32))

/* SHS1_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define SHS1_DIGESTSIZE (20)
#define SHS1_DIGESTWORDS (SHS1_DIGESTSIZE/sizeof(USB32))

/* SHS1_LOW - where low 32 bits of 64 bit count is stored during final */
#define SHS1_LOW 15

/* SHS1_HIGH - where high 32 bits of 64 bit count is stored during final */
#define SHS1_HIGH 14

/*
 * The structure for storing SHS1 info
 *
 * We will assume that bit count is a multiple of 8.
 */
typedef struct {
	USB32 digest[SHS1_DIGESTWORDS]; /* message digest */
	USB32 countLo;			/* 64 bit count: bits 3-34 */
	USB32 countHi;			/* 64 bit count: bits 35-63 */
	USB32 datalen;			/* length of data in data */
	USB32 data[SHS1_CHUNKWORDS];	/* SHS1 chunk buffer */
} SHS1_INFO;

/*
 * SHS1COUNT(SHS1_INFO*, USB32) - update the 64 bit count in an SHS1_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#define SHS1COUNT(shs1info, count) {					\
	USB32 tmp_countLo;						\
	tmp_countLo = (shs1info)->countLo;				\
	if (((shs1info)->countLo += (count)) < tmp_countLo) {		\
		(shs1info)->countHi++;					\
	}								\
}


#endif /* !__SHS1_H__ */
