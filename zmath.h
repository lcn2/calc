/*
 * zmath - declarations for extended precision integer arithmetic
 *
 * Copyright (C) 1999,2002,2004  David I. Bell
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.12 $
 * @(#) $Id: zmath.h,v 29.12 2004/03/31 04:58:40 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/zmath.h,v $
 *
 * Under source code control:	1993/07/30 19:42:48
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Data structure declarations for extended precision integer arithmetic.
 * The assumption made is that a long is 32 bits and shorts are 16 bits,
 * and longs must be addressible on word boundaries.
 */


#if !defined(__ZMATH_H__)
#define __ZMATH_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "win32dll.h"
# include "alloc.h"
# include "endian_calc.h"
# include "longbits.h"
# include "byteswap.h"
# include "have_stdlib.h"
#else
# include <calc/win32dll.h>
# include <calc/alloc.h>
# include <calc/endian_calc.h>
# include <calc/longbits.h>
# include <calc/byteswap.h>
# include <calc/have_stdlib.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif


#ifndef ALLOCTEST
# define freeh(p) { if (((void *)p != (void *)_zeroval_) &&		\
			 ((void *)p != (void *)_oneval_)) free((void *)p); }
#endif


#if !defined(TRUE)
#define TRUE	((BOOL) 1)			/* booleans */
#endif
#if !defined(FALSE)
#define FALSE	((BOOL) 0)
#endif


/*
 * NOTE: FULL must be twice the storage size of a HALF
 *	 HALF must be BASEB bits long
 */

#if defined(HAVE_B64)

#define BASEB	32			/* use base 2^32 */
typedef USB32 HALF;			/* unit of number storage */
typedef SB32 SHALF;			/* signed HALF */
typedef USB64 FULL;			/* double unit of number storage */
typedef SB64 SFULL;			/* signed FULL */

#define SWAP_HALF_IN_B64(dest, src)	SWAP_B32_IN_B64(dest, src)
#define SWAP_HALF_IN_B32(dest, src)	(*(dest) = *(src))
#define SWAP_HALF_IN_FULL(dest, src)	SWAP_B32_IN_B64(dest, src)
#define SWAP_HALF_IN_HASH(dest, src)	SWAP_B16_IN_HASH(dest, src)
#define SWAP_HALF_IN_FLAG(dest, src)	SWAP_B16_IN_FLAG(dest, src)
#define SWAP_HALF_IN_BOOL(dest, src)	SWAP_B16_IN_BOOL(dest, src)
#define SWAP_HALF_IN_LEN(dest, src)	SWAP_B16_IN_LEN(dest, src)
#define SWAP_B32_IN_FULL(dest, src)	SWAP_B32_IN_B64(dest, src)
#define SWAP_B16_IN_FULL(dest, src)	SWAP_B16_IN_B64(dest, src)
#define SWAP_B16_IN_HALF(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_FULL(dest, src)	SWAP_B8_IN_B64(dest, src)
#define SWAP_B8_IN_HALF(dest, src)	SWAP_B8_IN_B32(dest, src)

#else

#define BASEB	16			/* use base 2^16 */
typedef USB16 HALF;			/* unit of number storage */
typedef SB16 SHALF;			/* signed HALF */
typedef USB32 FULL;			/* double unit of number storage */
typedef SB32 SFULL;			/* signed FULL */

#define SWAP_HALF_IN_B64(dest, src)	SWAP_B16_IN_B64(dest, src)
#define SWAP_HALF_IN_B32(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_HALF_IN_FULL(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_HALF_IN_HASH(dest, src)	SWAP_B16_IN_HASH(dest, src)
#define SWAP_HALF_IN_FLAG(dest, src)	SWAP_B16_IN_FLAG(dest, src)
#define SWAP_HALF_IN_BOOL(dest, src)	SWAP_B16_IN_BOOL(dest, src)
#define SWAP_HALF_IN_LEN(dest, src)	SWAP_B16_IN_LEN(dest, src)
#define SWAP_B32_IN_FULL(dest, src)	(*(dest) = *(src))
#define SWAP_B16_IN_FULL(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B16_IN_HALF(dest, src)	(*(dest) = *(src))
#define SWAP_B8_IN_FULL(dest, src)	SWAP_B8_IN_B32(dest, src)
#define SWAP_B8_IN_HALF(dest, src)	SWAP_B8_IN_B16(dest, src)

#endif

#define BASE	((FULL)1<<BASEB)		/* base for calculations */
#define BASE1	(BASE - (FULL)1)		/* one less than base */
#define BASEDIG ((BASEB/16)*5)			/* number of digits in base */
#define FULL_BITS (2*BASEB)			/* bits in a FULL */

#define TOPHALF ((FULL)1 << (BASEB-1))		/* highest bit in a HALF */
#define MAXHALF (TOPHALF - (FULL)1)		/* largest SHALF value */

#define TOPFULL ((FULL)1 << (FULL_BITS-1))	/* highest bit in FULL */
#define MAXFULL (TOPFULL - (FULL)1)		/* largest SFULL value */
#define MAXUFULL (MAXFULL | TOPFULL)		/* largest FULL value */

#define TOPLONG ((unsigned long)1 << (LONG_BITS-1))	/* top long bit */
#define MAXLONG ((long) (TOPLONG - (unsigned long)1))	/* largest long val */
#define MAXULONG (MAXLONG | TOPLONG)		/* largest unsigned long val */


/*
 * other misc typedefs
 */
typedef USB32 QCKHASH;			/* 32 bit hash value */
#if defined(HAVE_B64) && LONG_BITS == 32
typedef HALF PRINT;			/* cast for zio printing fucctions */
#define SWAP_B16_IN_PRINT(dest, src)	SWAP_B16_IN_HALF(dest, src)
#define SWAP_B8_IN_PRINT(dest, src)	SWAP_B8_IN_HALF(dest, src)
#else
typedef FULL PRINT;			/* cast for zio printing fucctions */
#define SWAP_B16_IN_PRINT(dest, src)	SWAP_B16_IN_FULL(dest, src)
#define SWAP_B8_IN_PRINT(dest, src)	SWAP_B8_IN_FULL(dest, src)
#endif
typedef SB32 FLAG;			/* small value (e.g. comparison) */
typedef SB32 BOOL;			/* TRUE or FALSE value */
typedef SB32 LEN;			/* unit of length storage */

#define SWAP_B32_IN_HASH(dest, src)	(*(dest) = *(src))
#define SWAP_B16_IN_HASH(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_HASH(dest, src)	SWAP_B8_IN_B32(dest, src)

#define SWAP_B32_IN_FLAG(dest, src)	(*(dest) = *(src))
#define SWAP_B16_IN_FLAG(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_FLAG(dest, src)	SWAP_B8_IN_B32(dest, src)

#define SWAP_B32_IN_BOOL(dest, src)	(*(dest) = *(src))
#define SWAP_B16_IN_BOOL(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_BOOL(dest, src)	SWAP_B8_IN_B32(dest, src)

#define SWAP_B32_IN_LEN(dest, src)	(*(dest) = *(src))
#define SWAP_B16_IN_LEN(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_LEN(dest, src)	SWAP_B8_IN_B32(dest, src)

#if LONG_BITS == 64
#define SWAP_HALF_IN_LONG(dest, src)	SWAP_HALF_IN_B64(dest, src)
#else /* LONG_BITS == 64 */
#define SWAP_HALF_IN_LONG(dest, src)	SWAP_HALF_IN_B32(dest, src)
#endif /* LONG_BITS == 64 */


/*
 * FNV-1 basis
 *
 * We start the hash at a non-zero value at the beginning so that
 * hashing blocks of data with all 0 bits do not map onto the same
 * 0 hash value.  The virgin value that we use below is the hash value
 * that we would get from following 32 ASCII characters:
 *
 *		chongo <Landon Curt Noll> /\../\
 *
 * Note that the \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 *
 * The effect of this virgin initial value is the same as starting
 * with 0 and pre-pending those 32 characters onto the data being
 * hashed.
 *
 * Yes, even with this non-zero virgin value there is a set of data
 * that will result in a zero hash value.  Worse, appending any
 * about of zero bytes will continue to produce a zero hash value.
 * But that would happen with any initial value so long as the
 * hash of the initial was the `inverse' of the virgin prefix string.
 *
 * But then again for any hash function, there exists sets of data
 * which that the hash of every member is the same value.  That is
 * life with many to few mapping functions.  All we do here is to
 * prevent sets whose members consist of 0 or more bytes of 0's from
 * being such an awkward set.
 *
 * And yes, someone can figure out what the magic 'inverse' of the
 * 32 ASCII character are ... but this hash function is NOT intended
 * to be a cryptographic hash function, just a fast and reasonably
 * good hash function.
 */
#define FNV1_32_BASIS ((QCKHASH)(0x811c9dc5))


/*
 * The largest power of 10 we will compute for our decimal conversion
 * internal constants is: 10^(2^TEN_MAX).
 */
#define TEN_MAX 31	/* 10^2^31 requires about 1.66 * 2^29 bytes */


/*
 * LEN storage size must be <= FULL storage size
 */
#define MAXLEN	((LEN) 0x7fffffff >> 3) /* longest value allowed */


#define MAXREDC 5			/* number of entries in REDC cache */
#define SQ_ALG2 20			/* size for alternative squaring */
#define MUL_ALG2 20			/* size for alternative multiply */
#define POW_ALG2 40			/* size for using REDC for powers */
#define REDC_ALG2 50			/* size for using alternative REDC */


typedef union {
	FULL	ivalue;
	struct {
		HALF Svalue1;
		HALF Svalue2;
	} sis;
} SIUNION;


#if !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN	1234	/* Least Significant Byte first */
#endif
#if !defined(BIG_ENDIAN)
#define BIG_ENDIAN	4321	/* Most Significant Byte first */
#endif
/* PDP_ENDIAN - LSB in word, MSW in long is not supported */

#if CALC_BYTE_ORDER == LITTLE_ENDIAN
# define silow	sis.Svalue1	/* low order half of full value */
# define sihigh sis.Svalue2	/* high order half of full value */
#else
# if CALC_BYTE_ORDER == BIG_ENDIAN
#  define silow sis.Svalue2	/* low order half of full value */
#  define sihigh sis.Svalue1	/* high order half of full value */
# else
   /\oo/\    CALC_BYTE_ORDER must be BIG_ENDIAN or LITTLE_ENDIAN    /\oo/\  !!!
# endif
#endif


typedef struct {
	HALF	*v;		/* pointer to array of values */
	LEN	len;		/* number of values in array */
	BOOL	sign;		/* sign, nonzero is negative */
} ZVALUE;



/*
 * Function prototypes for integer math routines.
 */
extern DLL HALF * alloc(LEN len);
#ifdef	ALLOCTEST
extern DLL void freeh(HALF *);
#endif


/*
 * Input, output, and conversion routines.
 */
extern DLL void zcopy(ZVALUE z, ZVALUE *res);
extern DLL void itoz(long i, ZVALUE *res);
extern DLL void utoz(FULL i, ZVALUE *res);
extern DLL void str2z(char *s, ZVALUE *res);
extern DLL long ztoi(ZVALUE z);
extern DLL FULL ztou(ZVALUE z);
extern DLL void zprintval(ZVALUE z, long decimals, long width);
extern DLL void zprintx(ZVALUE z, long width);
extern DLL void zprintb(ZVALUE z, long width);
extern DLL void zprinto(ZVALUE z, long width);
extern DLL void fitzprint(ZVALUE, long, long);


/*
 * Basic numeric routines.
 */
extern DLL void zmuli(ZVALUE z, long n, ZVALUE *res);
extern DLL long zdivi(ZVALUE z, long n, ZVALUE *res);
extern DLL long zmodi(ZVALUE z, long n);
extern DLL void zadd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zsub(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zmul(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL long zdiv(ZVALUE z1, ZVALUE z2, ZVALUE *res, ZVALUE *rem, long R);
extern DLL long zquo(ZVALUE z1, ZVALUE z2, ZVALUE *res, long R);
extern DLL long zmod(ZVALUE z1, ZVALUE z2, ZVALUE *rem, long R);
extern DLL void zequo(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL BOOL zdivides(ZVALUE z1, ZVALUE z2);
extern DLL void zor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zand(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zxor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zandnot(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL long zpopcnt(ZVALUE z, int bitval);
extern DLL void zshift(ZVALUE z, long n, ZVALUE *res);
extern DLL void zsquare(ZVALUE z, ZVALUE *res);
extern DLL long zlowbit(ZVALUE z);
extern DLL LEN zhighbit(ZVALUE z);
extern DLL void zbitvalue(long n, ZVALUE *res);
extern DLL BOOL zisset(ZVALUE z, long n);
extern DLL BOOL zisonebit(ZVALUE z);
extern DLL BOOL zisallbits(ZVALUE z);
extern DLL FLAG ztest(ZVALUE z);
extern DLL FLAG zrel(ZVALUE z1, ZVALUE z2);
extern DLL FLAG zabsrel(ZVALUE z1, ZVALUE z2);
extern DLL BOOL zcmp(ZVALUE z1, ZVALUE z2);


/*
 * More complicated numeric functions.
 */
extern DLL FULL uugcd(FULL i1, FULL i2);
extern DLL long iigcd(long i1, long i2);
extern DLL void zgcd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zlcm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zreduce(ZVALUE z1, ZVALUE z2, ZVALUE *z1res, ZVALUE *z2res);
extern DLL void zfact(ZVALUE z, ZVALUE *dest);
extern DLL void zperm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL int zcomb(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL FLAG zjacobi(ZVALUE z1, ZVALUE z2);
extern DLL void zfib(ZVALUE z, ZVALUE *res);
extern DLL void zpowi(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void ztenpow(long power, ZVALUE *res);
extern DLL void zpowermod(ZVALUE z1, ZVALUE z2, ZVALUE z3, ZVALUE *res);
extern DLL BOOL zmodinv(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL BOOL zrelprime(ZVALUE z1, ZVALUE z2);
extern DLL long zlog(ZVALUE z1, ZVALUE z2);
extern DLL long zlog10(ZVALUE z);
extern DLL long zdivcount(ZVALUE z1, ZVALUE z2);
extern DLL long zfacrem(ZVALUE z1, ZVALUE z2, ZVALUE *rem);
extern DLL long zgcdrem(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL long zdigits(ZVALUE z1);
extern DLL long zdigit(ZVALUE z1, long n);
extern DLL FLAG zsqrt(ZVALUE z1, ZVALUE *dest, long R);
extern DLL void zroot(ZVALUE z1, ZVALUE z2, ZVALUE *dest);
extern DLL BOOL zissquare(ZVALUE z);
extern DLL void zhnrmod(ZVALUE v, ZVALUE h, ZVALUE zn, ZVALUE zr, ZVALUE *res);


/*
 * Prime related functions.
 */
extern DLL FLAG zisprime(ZVALUE z);
extern DLL FULL znprime(ZVALUE z);
extern DLL FULL next_prime(FULL v);
extern DLL FULL zpprime(ZVALUE z);
extern DLL void zpfact(ZVALUE z, ZVALUE *dest);
extern DLL BOOL zprimetest(ZVALUE z, long count, ZVALUE skip);
extern DLL BOOL zredcprimetest(ZVALUE z, long count, ZVALUE skip);
extern DLL BOOL znextcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res, ZVALUE mod, ZVALUE *cand);
extern DLL BOOL zprevcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res, ZVALUE mod, ZVALUE *cand);
extern DLL FULL zlowfactor(ZVALUE z, long count);
extern DLL FLAG zfactor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL long zpix(ZVALUE z1);
extern DLL void zlcmfact(ZVALUE z, ZVALUE *dest);


/*
 * Misc misc functions. :-)
 */
extern DLL void zsquaremod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zminmod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL BOOL zcmpmod(ZVALUE z1, ZVALUE z2, ZVALUE z3);
extern DLL void zio_init(void);


/*
 * These functions are for internal use only.
 */
extern DLL void ztrim(ZVALUE *z);
extern DLL void zshiftr(ZVALUE z, long n);
extern DLL void zshiftl(ZVALUE z, long n);
extern DLL HALF *zalloctemp(LEN len);


/*
 * Modulo arithmetic definitions.
 * Structure holding state of REDC initialization.
 * Multiple instances of this structure can be used allowing
 * calculations with more than one modulus at the same time.
 * Len of zero means the structure is not initialized.
 */
typedef struct {
	LEN len;		/* number of words in binary modulus */
	ZVALUE mod;		/* modulus REDC is computing with */
	ZVALUE inv;		/* inverse of modulus in binary modulus */
	ZVALUE one;		/* REDC format for the number 1 */
} REDC;

extern DLL REDC *zredcalloc(ZVALUE z1);
extern DLL void zredcfree(REDC *rp);
extern DLL void zredcencode(REDC *rp, ZVALUE z1, ZVALUE *res);
extern DLL void zredcdecode(REDC *rp, ZVALUE z1, ZVALUE *res);
extern DLL void zredcmul(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern DLL void zredcsquare(REDC *rp, ZVALUE z1, ZVALUE *res);
extern DLL void zredcpower(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);


/*
 * macro expansions to speed this thing up
 */
#define ziseven(z)	(!(*(z).v & 01))
#define zisodd(z)	(*(z).v & 01)
#define ziszero(z)	((*(z).v == 0) && ((z).len == 1))
#define zisneg(z)	((z).sign)
#define zispos(z)	(((z).sign == 0) && (*(z).v || ((z).len > 1)))
#define zisunit(z)	((*(z).v == 1) && ((z).len == 1))
#define zisone(z)	((*(z).v == 1) && ((z).len == 1) && !(z).sign)
#define zisnegone(z)	((*(z).v == 1) && ((z).len == 1) && (z).sign)
#define zistwo(z)	((*(z).v == 2) && ((z).len == 1) && !(z).sign)
#define zisabstwo(z)	((*(z).v == 2) && ((z).len == 1))
#define zisabsleone(z)	((*(z).v <= 1) && ((z).len == 1))
#define zislezero(z)	(zisneg(z) || ziszero(z))
#define zisleone(z)	(zisneg(z) || zisabsleone(z))
#define zistiny(z)	((z).len == 1)

/*
 * zgtmaxfull(z)	TRUE if abs(z) > MAXFULL
 */
#define zgtmaxfull(z)	(((z).len > 2) || (((z).len == 2) && (((SHALF)(z).v[1]) < 0)))

/*
 * zgtmaxufull(z)	TRUE if abs(z) will not fit into a FULL (> MAXUFULL)
 */
#define zgtmaxufull(z)	((z).len > 2)

/*
 * zgtmaxulong(z)	TRUE if abs(z) > MAXULONG
 */
#if BASEB >= LONG_BITS
#define zgtmaxulong(z)	((z).len > 1)
#else
#define zgtmaxulong(z)	zgtmaxufull(z)
#endif

/*
 * zgtmaxlong(z)	TRUE if abs(z) > MAXLONG
 */
#if BASEB >= LONG_BITS
#define zgtmaxlong(z)	(((z).len > 1) || (((z).len == 1) && (((SHALF)(z).v[0]) < 0)))
#else
#define zgtmaxlong(z)	zgtmaxfull(z)
#endif

/*
 * Some algorithms testing for values of a certain length.  Macros such as
 * zistiny() do this well.  In other cases algorthms require tests for values
 * in comparison to a given power of 2.	 In the later case, zistiny() compares
 * against a different power of 2 on a 64 bit machine.	The macros below
 * provide a tests against powers of 2 that are independent of the work size.
 *
 *	zge16b(z)	TRUE if abs(z) >= 2^16
 *	zge24b(z)	TRUE if abs(z) >= 2^24
 *	zge31b(z)	TRUE if abs(z) >= 2^31
 *	zge32b(z)	TRUE if abs(z) >= 2^32
 *	zge64b(z)	TRUE if abs(z) >= 2^64
 */
#if BASEB == 32

#define zge16b(z)	(!zistiny(z) || ((z).v[0] >= (HALF)0x10000))
#define zge24b(z)	(!zistiny(z) || ((z).v[0] >= (HALF)0x1000000))
#define zge31b(z)	(!zistiny(z) || (((SHALF)(z).v[0]) < 0))
#define zge32b(z)	(!zistiny(z))
#define zge64b(z)	((z).len > 2)

#else

#define zge16b(z)	(!zistiny(z))
#define zge24b(z)	(((z).len > 2) || (((z).len == 2) && ((z).v[1] >= (HALF)0x100)))
#define zge31b(z)	(((z).len > 2) || (((z).len == 2) && (((SHALF)(z).v[1]) < 0)))
#define zge32b(z)	((z).len > 2)
#define zge64b(z)	((z).len > 4)

#endif


/*
 * ztofull - convert an absolute value of a ZVALUE to a FULL if possible
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a FULL will be used.
 */
#define ztofull(z)	(zistiny(z) ? ((FULL)((z).v[0])) :		\
				      ((FULL)((z).v[0]) +		\
				       ((FULL)((z).v[1]) << BASEB)))

#define z1tol(z)	((long)((z).v[0]))
#define z2tol(z)	((long)(((z).v[0]) + \
				(((z).v[1] & MAXHALF) << BASEB)))

/*
 * ztoulong - convert an absolute value of a ZVALUE to an unsigned long
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a long will be used.
 */
#if BASEB >= LONG_BITS
# define ztoulong(z)	((unsigned long)z1tol(z))
#else
# define ztoulong(z)	((unsigned long)ztofull(z))
#endif

/*
 * ztolong - convert an absolute value of a ZVALUE to a long
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a long will be used.
 */
#define ztolong(z)	((long)(ztoulong(z) & MAXLONG))

#define zclearval(z)	memset((z).v, 0, (z).len * sizeof(HALF))
#define zcopyval(z1,z2) memcpy((z2).v, (z1).v, (z1).len * sizeof(HALF))
#define zquicktrim(z)	{if (((z).len > 1) && ((z).v[(z).len-1] == 0)) \
				(z).len--;}
#define zfree(z)	freeh((z).v)


/*
 * Output modes for numeric displays.
 */
#define MODE_DEFAULT	0
#define MODE_FRAC	1
#define MODE_INT	2
#define MODE_REAL	3
#define MODE_EXP	4
#define MODE_HEX	5
#define MODE_OCTAL	6
#define MODE_BINARY	7
#define MODE_MAX	7
#define MODE2_OFF	(MODE_MAX+1)

#define MODE_INITIAL	MODE_REAL
#define MODE2_INITIAL	MODE2_OFF


/*
 * Output routines for either FILE handles or strings.
 */
extern DLL void math_chr(int ch);
extern DLL void math_str(char *str);
extern DLL void math_fill(char *str, long width);
extern DLL void math_flush(void);
extern DLL void math_divertio(void);
extern DLL void math_cleardiversions(void);
extern DLL char *math_getdivertedio(void);
extern DLL int math_setmode(int mode);
extern DLL int math_setmode2(int mode);
extern DLL LEN math_setdigits(LEN digits);
extern DLL void math_fmt(char *, ...);


/*
 * The error routine.
 */
extern DLL void math_error(char *, ...);


/*
 * external swap functions
 */
extern DLL HALF *swap_b8_in_HALFs(HALF *dest, HALF *src, LEN len);
extern DLL ZVALUE *swap_b8_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);
extern DLL HALF *swap_b16_in_HALFs(HALF *dest, HALF *src, LEN len);
extern DLL ZVALUE *swap_b16_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);
extern DLL ZVALUE *swap_HALF_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);


/*
 * constants used often by the arithmetic routines
 */
extern HALF _zeroval_[], _oneval_[], _twoval_[], _threeval_[], _fourval_[];
extern HALF _fiveval_[], _sixval_[], _sevenval_[], _eightval_[], _nineval_[];
extern HALF _tenval_[], _elevenval_[], _twelveval_[], _thirteenval_[];
extern HALF _fourteenval_[], _fifteenval_[];
extern HALF _sqbaseval_[];
extern HALF _fourthbaseval_[];

extern ZVALUE zconst[];		/* ZVALUE integers from 0 thru 15 */

extern ZVALUE _zero_, _one_, _two_, _ten_, _neg_one_;
extern ZVALUE _sqbase_, _pow4base_, _pow8base_;

extern ZVALUE _b32_, _b64_;

extern DLL BOOL _math_abort_;	/* nonzero to abort calculations */
extern ZVALUE _tenpowers_[];	/* table of 10^2^n */

/*
 * Bit fiddeling functions and types
 */
extern HALF bitmask[];		/* bit rotation, norm 0 */
extern HALF lowhalf[];		/* bit masks from low end of HALF */
extern HALF rlowhalf[];		/* reversed bit masks from low end of HALF */
extern HALF highhalf[];		/* bit masks from high end of HALF */
extern HALF rhighhalf[];	/* reversed bit masks from high end of HALF */
#define HAVE_REVERSED_MASKS	/* allows old code to know libcalc.a has them */


/*
 * BITSTR - string of bits within an array of HALFs
 *
 * This typedef records a location of a bit in an array of HALFs.
 * Bit 0 in a HALF is assumed to be the least significant bit in that HALF.
 *
 * The most significant bit is found at (loc,bit).  Bits of lesser
 * significance may be found in previous bits and HALFs.
 */
typedef struct {
	HALF *loc;	/* half address of most significant bit */
	int bit;	/* bit position within half of most significant bit */
	int len;	/* length of string in bits */
} BITSTR;


#endif /* !__ZMATH_H__*/
