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
 * @(#) $Revision: 29.1 $
 * @(#) $Id: md5.h,v 29.1 1999/12/14 09:16:12 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/md5.h,v $
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

#if !defined(__MD5_H__)
#define __MD5_H__

/* MD5_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define MD5_CHUNKSIZE	(1<<6)
#define MD5_CHUNKWORDS (MD5_CHUNKSIZE/sizeof(USB32))

/* MD5_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define MD5_DIGESTSIZE	(16)
#define MD5_DIGESTWORDS (MD5_DIGESTSIZE/sizeof(USB32))

/* MD5_LOW - where low 32 bits of 64 bit count is stored during final */
#define MD5_LOW 14

/* MD5_HIGH - where high 32 bits of 64 bit count is stored during final */
#define MD5_HIGH 15

/*
 * MD5COUNT(MD5_CTX*, USB32) - update the 64 bit count in an MD5_CTX
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#define MD5COUNT(md5info, count) {					\
	USB32 tmp_countLo;						\
	tmp_countLo = (md5info)->countLo;				\
	if (((md5info)->countLo += (count)) < tmp_countLo) {		\
		(md5info)->countHi++;					\
	}								\
}

/*
 * Data structure for MD5 (Message-Digest) computation
 */
typedef struct {
	USB32 digest[MD5_DIGESTWORDS];		/* message digest */
	USB32 countLo;		/* 64 bit count: bits 3-34 */
	USB32 countHi;		/* 64 bit count: bits 35-63 (64-66 ignored) */
	USB32 datalen;		/* length of data in inp.inp_USB8 */
	USB32 data[MD5_CHUNKWORDS];  /* USB32 chunk buffer */
} MD5_CTX;

#endif /* __MD5_H__ */
