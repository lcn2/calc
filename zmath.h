/*
 * zmath - declarations for extended precision integer arithmetic
 *
 * Copyright (C) 1999-2007,2014,2021  David I. Bell
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
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1993/07/30 19:42:48
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Data structure declarations for extended precision integer arithmetic.
 * The assumption made is that a long is 32 bits and shorts are 16 bits,
 * and longs must be addressable on word boundaries.
 */


#if !defined(INCLUDE_ZMATH_H)
#define INCLUDE_ZMATH_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
# include "alloc.h"
# include "endian_calc.h"
# include "longbits.h"
# include "byteswap.h"
# include "have_stdlib.h"
#else
# include <calc/decl.h>
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
#define HALF_LEN (sizeof(HALF))			/* length of HALF in bites */
#define FULL_LEN (sizeof(FULL))			/* length of FULL in bites */

/*
 * ROUNDUP(value, mult) - round up value to the next multiple of mult
 *
 * NOTE: value and mult musty be of an integer type.
 *
 * NOTE: mult must != 0
 *
 * NOTE: If value is a multiple of mult, then ROUNDUP(value, mult)
 *	 will just return value.
 */
#define ROUNDUP(value, mult) ( ( ((value)+(mult)-1) / (mult) ) * (mult) )

#define TOPHALF ((FULL)1 << (BASEB-1))		/* highest bit in a HALF */
#define MAXHALF (TOPHALF - (FULL)1)		/* largest SHALF value */

#define TOPFULL ((FULL)1 << (FULL_BITS-1))	/* highest bit in FULL */
#define MAXFULL (TOPFULL - (FULL)1)		/* largest SFULL value */
#define MINSFULL ((SFULL)(TOPFULL))		/* most negative SFULL value */
#define MAXUFULL (MAXFULL | TOPFULL)		/* largest FULL value */

#define TOPLONG ((unsigned long)1 << (LONG_BITS-1))	/* top long bit */
#define MAXLONG ((long) (TOPLONG - (unsigned long)1))	/* largest long val */
#define MAXULONG (MAXLONG | TOPLONG)		/* largest unsigned long val */


/*
 * other misc typedefs
 */
typedef USB32 QCKHASH;			/* 32 bit hash value */
#if defined(HAVE_B64) && LONG_BITS == 32
typedef HALF PRINT;			/* cast for zio printing functions */
#define SWAP_B16_IN_PRINT(dest, src)	SWAP_B16_IN_HALF(dest, src)
#define SWAP_B8_IN_PRINT(dest, src)	SWAP_B8_IN_HALF(dest, src)
#else
typedef FULL PRINT;			/* cast for zio printing functions */
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
 * Quickhash basis
 *
 * We start the hash at a non-zero value at the beginning so that
 * hashing blocks of data with all 0 bits do not map onto the same
 * 0 hash value.  The virgin value that we use below is the 32-bit
 * FNV-0 hash value that we would get from following 32 ASCII characters:
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
#define QUICKHASH_BASIS ((QCKHASH)(0x811c9dc5))


/*
 * The largest power of 10 we will compute for our decimal conversion
 * internal constants is: 10^(2^TEN_MAX).
 */
#define TEN_MAX 31	/* 10^2^31 requires about 1.66 * 2^29 bytes */


/*
 * LEN storage size must be <= FULL storage size
 */
#define MAXLEN	((LEN) 0x7fffffff >> 3) /* longest value allowed */


#define MAXREDC 256			/* number of entries in REDC cache */
#define SQ_ALG2 28			/* size for alternative squaring */
#define MUL_ALG2 28			/* size for alternative multiply */
#define POW_ALG2 20			/* size for using REDC for powers */
/* old REDC_ALG2 was 5/4 of POW_ALG2, so we will keep the same ratio */
#define REDC_ALG2 25			/* size for using alternative REDC */


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
E_FUNC HALF * alloc(LEN len);
#ifdef	ALLOCTEST
E_FUNC void freeh(HALF *);
#endif


/*
 * Input, output, and conversion routines.
 */
E_FUNC void zcopy(ZVALUE z, ZVALUE *res);
E_FUNC void itoz(long i, ZVALUE *res);
E_FUNC void utoz(FULL i, ZVALUE *res);
E_FUNC void stoz(SFULL i, ZVALUE *res);
E_FUNC void str2z(char *s, ZVALUE *res);
E_FUNC long ztoi(ZVALUE z);
E_FUNC FULL ztou(ZVALUE z);
E_FUNC SFULL ztos(ZVALUE z);
E_FUNC void zprintval(ZVALUE z, long decimals, long width);
E_FUNC void zprintx(ZVALUE z, long width);
E_FUNC void zprintb(ZVALUE z, long width);
E_FUNC void zprinto(ZVALUE z, long width);
E_FUNC void fitzprint(ZVALUE, long, long);


/*
 * Basic numeric routines.
 */
E_FUNC void zmuli(ZVALUE z, long n, ZVALUE *res);
E_FUNC long zdivi(ZVALUE z, long n, ZVALUE *res);
E_FUNC long zmodi(ZVALUE z, long n);
E_FUNC void zadd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zsub(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zmul(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC long zdiv(ZVALUE z1, ZVALUE z2, ZVALUE *res, ZVALUE *rem, long R);
E_FUNC long zquo(ZVALUE z1, ZVALUE z2, ZVALUE *res, long R);
E_FUNC long zmod(ZVALUE z1, ZVALUE z2, ZVALUE *rem, long R);
E_FUNC void zequo(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC BOOL zdivides(ZVALUE z1, ZVALUE z2);
E_FUNC void zor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zand(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zxor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zandnot(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC long zpopcnt(ZVALUE z, int bitval);
E_FUNC void zshift(ZVALUE z, long n, ZVALUE *res);
E_FUNC void zsquare(ZVALUE z, ZVALUE *res);
E_FUNC long zlowbit(ZVALUE z);
E_FUNC LEN zhighbit(ZVALUE z);
E_FUNC void zbitvalue(long n, ZVALUE *res);
E_FUNC BOOL zisset(ZVALUE z, long n);
E_FUNC BOOL zisonebit(ZVALUE z);
E_FUNC BOOL zisallbits(ZVALUE z);
E_FUNC FLAG ztest(ZVALUE z);
E_FUNC FLAG zrel(ZVALUE z1, ZVALUE z2);
E_FUNC FLAG zabsrel(ZVALUE z1, ZVALUE z2);
E_FUNC BOOL zcmp(ZVALUE z1, ZVALUE z2);


/*
 * More complicated numeric functions.
 */
E_FUNC FULL uugcd(FULL i1, FULL i2);
E_FUNC long iigcd(long i1, long i2);
E_FUNC void zgcd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zlcm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zreduce(ZVALUE z1, ZVALUE z2, ZVALUE *z1res, ZVALUE *z2res);
E_FUNC void zfact(ZVALUE z, ZVALUE *dest);
E_FUNC void zperm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC int zcomb(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC FLAG zjacobi(ZVALUE z1, ZVALUE z2);
E_FUNC void zfib(ZVALUE z, ZVALUE *res);
E_FUNC void zpowi(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void ztenpow(long power, ZVALUE *res);
E_FUNC void zpowermod(ZVALUE z1, ZVALUE z2, ZVALUE z3, ZVALUE *res);
E_FUNC BOOL zmodinv(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC BOOL zrelprime(ZVALUE z1, ZVALUE z2);
E_FUNC long zlog(ZVALUE z1, ZVALUE z2);
E_FUNC long zlog10(ZVALUE z, BOOL *was_10_power);
E_FUNC long zdivcount(ZVALUE z1, ZVALUE z2);
E_FUNC long zfacrem(ZVALUE z1, ZVALUE z2, ZVALUE *rem);
E_FUNC long zgcdrem(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC long zdigits(ZVALUE z1);
E_FUNC long zdigit(ZVALUE z1, long n);
E_FUNC FLAG zsqrt(ZVALUE z1, ZVALUE *dest, long R);
E_FUNC void zroot(ZVALUE z1, ZVALUE z2, ZVALUE *dest);
E_FUNC BOOL zissquare(ZVALUE z);
E_FUNC void zhnrmod(ZVALUE v, ZVALUE h, ZVALUE zn, ZVALUE zr, ZVALUE *res);


/*
 * Prime related functions.
 */
E_FUNC FLAG zisprime(ZVALUE z);
E_FUNC FULL znprime(ZVALUE z);
E_FUNC FULL next_prime(FULL v);
E_FUNC FULL zpprime(ZVALUE z);
E_FUNC void zpfact(ZVALUE z, ZVALUE *dest);
E_FUNC BOOL zprimetest(ZVALUE z, long count, ZVALUE skip);
E_FUNC BOOL zredcprimetest(ZVALUE z, long count, ZVALUE skip);
E_FUNC BOOL znextcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res,
		      ZVALUE mod, ZVALUE *cand);
E_FUNC BOOL zprevcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res,
		      ZVALUE mod, ZVALUE *cand);
E_FUNC FULL zlowfactor(ZVALUE z, long count);
E_FUNC FLAG zfactor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC long zpix(ZVALUE z1);
E_FUNC void zlcmfact(ZVALUE z, ZVALUE *dest);


/*
 * Misc misc functions. :-)
 */
E_FUNC void zsquaremod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zminmod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC BOOL zcmpmod(ZVALUE z1, ZVALUE z2, ZVALUE z3);
E_FUNC void zio_init(void);


/*
 * These functions are for internal use only.
 */
E_FUNC void ztrim(ZVALUE *z);
E_FUNC void zshiftr(ZVALUE z, long n);
E_FUNC void zshiftl(ZVALUE z, long n);
E_FUNC HALF *zalloctemp(LEN len);


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

E_FUNC REDC *zredcalloc(ZVALUE z1);
E_FUNC void zredcfree(REDC *rp);
E_FUNC void zredcencode(REDC *rp, ZVALUE z1, ZVALUE *res);
E_FUNC void zredcdecode(REDC *rp, ZVALUE z1, ZVALUE *res);
E_FUNC void zredcmul(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);
E_FUNC void zredcsquare(REDC *rp, ZVALUE z1, ZVALUE *res);
E_FUNC void zredcpower(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);


/*
 * macro expansions to speed this thing up
 */
#define ziseven(z)	(!(*(z).v & 0x1))
#define zisodd(z)	(*(z).v & 0x1)
#define ziszero(z)	((*(z).v == 0) && ((z).len == 1))
#define zisneg(z)	((z).sign)
#define zispos(z)	(((z).sign == 0) && (*(z).v || ((z).len > 1)))
#define zisunit(z)	((*(z).v == 1) && ((z).len == 1))
#define zisone(z)	((*(z).v == 1) && ((z).len == 1) && !(z).sign)
#define zisnegone(z)	((*(z).v == 1) && ((z).len == 1) && (z).sign)
#define zltnegone(z)	(zisneg(z) && (((z).len > 1) || *(z).v > 1))
#define zistwo(z)	((*(z).v == 2) && ((z).len == 1) && !(z).sign)
#define zisabstwo(z)	((*(z).v == 2) && ((z).len == 1))
#define zisabsleone(z)	((*(z).v <= 1) && ((z).len == 1))
#define zislezero(z)	(zisneg(z) || ziszero(z))
#define zisleone(z)	(zisneg(z) || zisabsleone(z))
#define zistiny(z)	((z).len == 1)

/*
 * zgtmaxfull(z)	TRUE if abs(z) > MAXFULL
 */
#define zgtmaxfull(z)	(((z).len > 2) || (((z).len == 2) && \
			 (((SHALF)(z).v[1]) < 0)))

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
#define zgtmaxlong(z)	(((z).len > 1) || (((z).len == 1) && \
			 (((SHALF)(z).v[0]) < 0)))
#else
#define zgtmaxlong(z)	zgtmaxfull(z)
#endif

/*
 * Some algorithms testing for values of a certain length.  Macros such as
 * zistiny() do this well.  In other cases algorithms require tests for values
 * in comparison to a given power of 2.	 In the later case, zistiny() compares
 * against a different power of 2 on a 64 bit machine.	The macros below
 * provide a tests against powers of 2 that are independent of the work size.
 *
 *	zge16b(z)	TRUE if abs(z) >= 2^16
 *	zge24b(z)	TRUE if abs(z) >= 2^24
 *	zge31b(z)	TRUE if abs(z) >= 2^31
 *	zge32b(z)	TRUE if abs(z) >= 2^32
 *	zge64b(z)	TRUE if abs(z) >= 2^64
 *	zge128b(z)	TRUE if abs(z) >= 2^128
 *	zge256b(z)	TRUE if abs(z) >= 2^256
 *	zge512b(z)	TRUE if abs(z) >= 2^512
 *	zge1024b(z)	TRUE if abs(z) >= 2^1024
 *	zge2048b(z)	TRUE if abs(z) >= 2^2048
 *	zge4096b(z)	TRUE if abs(z) >= 2^4096
 *	zge8192b(z)	TRUE if abs(z) >= 2^8192
 */
#if BASEB == 32

#define zge16b(z)	(!zistiny(z) || ((z).v[0] >= (HALF)0x10000))
#define zge24b(z)	(!zistiny(z) || ((z).v[0] >= (HALF)0x1000000))
#define zge31b(z)	(!zistiny(z) || (((SHALF)(z).v[0]) < 0))
#define zge32b(z)	(!zistiny(z))
#define zge64b(z)	((z).len > 2)
#define zge128b(z)	((z).len > 4)
#define zge256b(z)	((z).len > 8)
#define zge512b(z)	((z).len > 16)
#define zge1024b(z)	((z).len > 32)
#define zge2048b(z)	((z).len > 64)
#define zge4096b(z)	((z).len > 128)
#define zge8192b(z)	((z).len > 256)

#else

#define zge16b(z)	(!zistiny(z))
#define zge24b(z)	(((z).len > 2) || (((z).len == 2) && \
			 ((z).v[1] >= (HALF)0x100)))
#define zge31b(z)	(((z).len > 2) || (((z).len == 2) && \
			 (((SHALF)(z).v[1]) < 0)))
#define zge32b(z)	((z).len > 2)
#define zge64b(z)	((z).len > 4)
#define zge128b(z)	((z).len > 8)
#define zge256b(z)	((z).len > 16)
#define zge512b(z)	((z).len > 32)
#define zge1024b(z)	((z).len > 64)
#define zge2048b(z)	((z).len > 128)
#define zge4096b(z)	((z).len > 256)
#define zge8192b(z)	((z).len > 512)

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
#define MODE_REAL_AUTO	8
#define MODE_MAX	8
#define MODE2_OFF	(MODE_MAX+1)

/* XXX - perhaps we need the MODE_REAL_AUTO vs MODE_REAL as a config mode? */
#if 0 	/* XXX - can we safely set MODE_INITIAL to MODE_REAL_AUTO ?? */
#define MODE_INITIAL	MODE_REAL_AUTO
#else
#define MODE_INITIAL	MODE_REAL
#endif
#define MODE2_INITIAL	MODE2_OFF


/*
 * Output routines for either FILE handles or strings.
 */
E_FUNC void math_chr(int ch);
E_FUNC void math_str(char *str);
E_FUNC void math_fill(char *str, long width);
E_FUNC void math_flush(void);
E_FUNC void math_divertio(void);
E_FUNC void math_cleardiversions(void);
E_FUNC char *math_getdivertedio(void);
E_FUNC int math_setmode(int mode);
E_FUNC int math_setmode2(int mode);
E_FUNC LEN math_setdigits(LEN digits);
E_FUNC void math_fmt(char *, ...) PRINTF_FORMAT(1, 2);


/*
 * The error routine.
 */
E_FUNC void math_error(char *, ...) PRINTF_FORMAT(1, 2);


/*
 * external swap functions
 */
E_FUNC HALF *swap_b8_in_HALFs(HALF *dest, HALF *src, LEN len);
E_FUNC ZVALUE *swap_b8_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);
E_FUNC HALF *swap_b16_in_HALFs(HALF *dest, HALF *src, LEN len);
E_FUNC ZVALUE *swap_b16_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);
E_FUNC ZVALUE *swap_HALF_in_ZVALUE(ZVALUE *dest, ZVALUE *src, BOOL all);


/*
 * constants used often by the arithmetic routines
 */
EXTERN HALF _zeroval_[], _oneval_[], _twoval_[], _threeval_[], _fourval_[];
EXTERN HALF _fiveval_[], _sixval_[], _sevenval_[], _eightval_[], _nineval_[];
EXTERN HALF _tenval_[], _elevenval_[], _twelveval_[], _thirteenval_[];
EXTERN HALF _fourteenval_[], _fifteenval_[];
EXTERN HALF _sqbaseval_[];
EXTERN HALF _fourthbaseval_[];

EXTERN ZVALUE zconst[];		/* ZVALUE integers from 0 thru 15 */

EXTERN ZVALUE _zero_, _one_, _two_, _ten_, _neg_one_;
EXTERN ZVALUE _sqbase_, _pow4base_, _pow8base_;

EXTERN ZVALUE _b32_, _b64_;

EXTERN BOOL _math_abort_;	/* nonzero to abort calculations */
EXTERN ZVALUE _tenpowers_[];	/* table of 10^2^n */

/*
 * Bit fiddling functions and types
 */
EXTERN HALF bitmask[];		/* bit rotation, norm 0 */
EXTERN HALF lowhalf[];		/* bit masks from low end of HALF */
EXTERN HALF rlowhalf[];		/* reversed bit masks from low end of HALF */
EXTERN HALF highhalf[];		/* bit masks from high end of HALF */
EXTERN HALF rhighhalf[];	/* reversed bit masks from high end of HALF */
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


#endif /* !INCLUDE_ZMATH_H*/
