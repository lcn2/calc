/*
 * zmath - declarations for extended precision integer arithmetic
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  David I. Bell and Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1993/07/30 19:42:48
 * File existed as early as:    1993
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Data structure declarations for extended precision integer arithmetic.
 * The assumption made is that a long is 32 bits and shorts are 16 bits,
 * and longs must be addressable on word boundaries.
 */

#if !defined(INCLUDE_ZMATH_H)
#  define INCLUDE_ZMATH_H

#  if defined(CALC_SRC) /* if we are building from the calc source tree */
#    include "version.h"
#    include "int.h"
#    include "len_bits.h"
#    include "endian_calc.h"
#    include "byteswap.h"
#  else
#    include <calc/version.h>
#    include <calc/int.h>
#    include <calc/len_bits.h>
#    include <calc/endian_calc.h>
#    include <calc/byteswap.h>
#  endif

#  ifndef ALLOCTEST
#    define freeh(p)               \
        {                          \
            if (!is_const(p)) {    \
                free((void *)(p)); \
            }                      \
        }
#  endif

/*
 * length of a pointer
 */
#if !defined(INTPTR_LEN)
#  define INTPTR_LEN (INTPTR_WIDTH / CHAR_BIT)
#endif

/*
 * NOTE: FULL must be twice the storage size of a HALF
 *       HALF must be BASEB bits long
 */

/* BTW: BASEB is effectively HALF_BITS */

/* XXX - add code to allow for base 2^64, if available/possible, and perhaps base 2^128, if available/possible XXX */

#    define BASEB_LOG2 (5)          /* use base 2^32 */
#    define BASEB (1 << BASEB_LOG2) /* use base 2^32 */
typedef uint32_t HALF;  /* unit of number storage */
typedef int32_t SHALF;  /* signed HALF */
typedef uint64_t FULL;  /* double unit of number storage */
typedef int64_t SFULL;  /* signed FULL */

#  define SWAP_HALF_IN_B64(dest, src) SWAP_B32_IN_B64(dest, src)
#  define SWAP_HALF_IN_B32(dest, src) (*((HALF *)(dest)) = *((HALF *)(src)))
#  define SWAP_HALF_IN_FULL(dest, src) SWAP_B32_IN_B64(dest, src)
#  define SWAP_HALF_IN_HASH(dest, src) SWAP_B16_IN_HASH(dest, src)
#  define SWAP_HALF_IN_FLAG(dest, src) SWAP_B16_IN_FLAG(dest, src)
#  define SWAP_HALF_IN_BOOL(dest, src) SWAP_B16_IN_bool(dest, src)
#  define SWAP_HALF_IN_LEN(dest, src) SWAP_B16_IN_LEN(dest, src)
#  define SWAP_B32_IN_FULL(dest, src) SWAP_B32_IN_B64(dest, src)
#  define SWAP_B16_IN_FULL(dest, src) SWAP_B16_IN_B64(dest, src)
#  define SWAP_B16_IN_HALF(dest, src) SWAP_B16_IN_B32(dest, src)
#  define SWAP_B8_IN_FULL(dest, src) SWAP_B8_IN_B64(dest, src)
#  define SWAP_B8_IN_HALF(dest, src) SWAP_B8_IN_B32(dest, src)

#  define BASE ((FULL)1 << BASEB)    /* base for calculations */
#  define BASE1 (BASE - (FULL)1)     /* one less than base */
#  define BASEDIG ((BASEB / 16) * 5) /* number of digits in base */
#  define HALF_BITS (BASEB)          /* alias for BASEB - bits in a HALF */
#  define FULL_BITS (2 * BASEB)      /* bits in a FULL */
#  define HALF_LEN (sizeof(HALF))    /* length of HALF in bytes */
#  define FULL_LEN (sizeof(FULL))    /* length of FULL in bytes */

/*
 * ROUNDUP(value, mult) - round up value to the next multiple of mult
 *
 * NOTE: value and mult musty be of an integer type.
 *
 * NOTE: mult must != 0
 *
 * NOTE: If value is a multiple of mult, then ROUNDUP(value, mult)
 *       will just return value.
 */
#  define ROUNDUP(value, mult) ((((value) + (mult) - 1) / (mult)) * (mult))

/*
 * minimum and maximum HALF and FULL values
 */
#  define TOPHALF ((FULL)1 << (BASEB - 1)) /* highest bit in a HALF */
#  define MAXHALF (TOPHALF - (FULL)1)      /* largest SHALF value */

#  define TOPFULL ((FULL)1 << (FULL_BITS - 1)) /* highest bit in FULL */
#  define MAXFULL (TOPFULL - (FULL)1)          /* largest SFULL value */
#  define MINSFULL ((SFULL)(TOPFULL))          /* most negative SFULL value */
#  define MAXUFULL (MAXFULL | TOPFULL)         /* largest FULL value */

/*
 * minimum and maximum long values
 */
#  define TOPLONG ((unsigned long)1 << (LONG_BITS - 1)) /* top long bit */
#  define MAXLONG ((long)(TOPLONG - (unsigned long)1))  /* largest long val */
#  define MAXULONG (MAXLONG | TOPLONG)                  /* largest unsigned long val */

/*
 * How to print C integer types
 *
 * This impacts zio.c: specifically the zprintx() and zprinto() functions.
 */
#  if LONG_BITS == 64

    /*
     * 64-bit processors - how to print C integer variables
     *
     * FULL and PRINT are uint64_t
     *
     * When using printf formats:
     *
     *   Variables cast as (PRINT) are effectively long unsigned int
     *
     *   %lx  is OK
     *   %llx causes warnings
     *     format '%llx' expects argument of type 'long long unsigned int', but ... has type 'long unsigned int'
     */
#    if BASEB == 32
      typedef FULL PRINT; /* cast for zio.c printing in zprintx() and zprinto() */
#   elif BASEB == 16
      typedef FULL PRINT; /* cast for zio.c printing in zprintx() and zprinto() */
#   else
#     error "BASEB is not 32 nor 16, do not yet know how to typedef a PRINT value"
#   endif
#   define PRI_DEC PRId64
#   define PRI_OCT PRIo64
#   define PRI_HEX PRIx64

#  elif LONG_BITS == 32

    /*
     * 32-bit processors - how to print C integer variables
     *
     * FULL and PRINT are uint32_t
     *
     * When using printf formats:
     *
     *   Variables cast as (PRINT) are effectively unsigned int
     *
     *   %lx causes warnings
     *     format '%lx' expects argument of type 'long unsigned int', but ... has type 'unsigned int'
     *   %llx  is OK
     */
#    if BASEB == 32
      /* Yes, the larger base needs a smaller type! */
      typedef HALF PRINT; /* cast for zio.c printing in zprintx() and zprinto() */
#   elif BASEB == 16
      typedef FULL PRINT; /* cast for zio.c printing in zprintx() and zprinto() */
#   else
#     error "BASEB is not 32 nor 16, do not yet know how to typedef a PRINT value"
#   endif
#   define PRI_DEC PRId32
#   define PRI_OCT PRIo32
#   define PRI_HEX PRIx32

#  else

#   error "LONG_BITS is not 64 nor 32, do not yet know how to typedef PRINT"

#  endif
#  if BASEB == 32

#    define PRI_DEC_LEN "10"    /* base 2^32 requires values fully print with a width of 10 decimal digits */
#    define PRI_HEX_LEN "8"     /* base 2^32 requires values fully print with a width of 8 hex digits */
#    define PRI_OCT_LEN "11"    /* base 2^32 requires values fully print with a width of 11 octal digits */

#  elif BASEB == 16

#    define PRI_DEC_LEN "5"     /* base 2^16 requires values fully print with a width of 5 decimal digits */
#    define PRI_HEX_LEN "4"     /* base 2^16 requires values fully print with a width of 4 hex digits */
#    define PRI_OCT_LEN "6"     /* base 2^16 requires values fully print with a width of 6 octal digits */

#  else

#   error "BASEB is not 32 nor 16, do not yet know how to printf size a PRINT value"

#  endif

/*
 * other miscellaneous typedefs
 */
typedef uint32_t QCKHASH; /* 32 bit hash value */
typedef int32_t FLAG; /* small value (e.g., comparison) */

/*
 * length of internal integer values in units of HALF
 */
#  if MAJOR_VER >= 3
typedef intptr_t LEN; /* unit of length storage */
#  else
typedef int32_t LEN; /* calc v2 compatible unit of length storage */
#  endif

#  define SWAP_B32_IN_bool(dest, src) (*((bool *)(dest)) = *((bool *)(src)))
#  define SWAP_B16_IN_bool(dest, src) SWAP_B16_IN_B32(dest, src)
#  define SWAP_B8_IN_bool(dest, src) SWAP_B8_IN_B32(dest, src)

#  define SWAP_B32_IN_LEN(dest, src) (*((LEN *)(dest)) = *((LEN *)(src)))
#  define SWAP_B16_IN_LEN(dest, src) SWAP_B16_IN_B32(dest, src)
#  define SWAP_B8_IN_LEN(dest, src) SWAP_B8_IN_B32(dest, src)

#  if LONG_BITS == 64

#    define SWAP_HALF_IN_LONG(dest, src) SWAP_HALF_IN_B64(dest, src)

#  else

#    define SWAP_HALF_IN_LONG(dest, src) SWAP_HALF_IN_B32(dest, src)

#  endif

/*
 * Quickhash basis
 *
 * We start the hash at a non-zero value at the beginning so that
 * hashing blocks of data with all 0 bits do not map onto the same
 * 0 hash value.  The virgin value that we use below is the 32-bit
 * FNV-0 hash value that we would get from following 32 ASCII characters:
 *
 *              chongo <Landon Curt Noll> /\../\
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
#  define QUICKHASH_BASIS ((QCKHASH)(0x811c9dc5))

/*
 * MAXDATA - largest data object in bytes we will use
 *
 * We start with MAXDATA as 1 less than 1/16 of the maximum address space.
 * We limit to 1/16 because for a maximum complex value we
 * will need 4 huge integers, plus other data, code and stack space.
 * We often perform an operation such as 'a = b + c', so need 3 such
 * complex values.  So '3*4/16 == 3/4' of the address space, with spare
 * space for code, op stack and other data.
 *
 * NOTE: MAXDATA must be 1 less than a power of 2.
 * NOTE: MAXDATA_LOG2 is the log base 2 of MAXDATA+1.
 */
#  define ADDR_REDUCT 4                                      /* power of 2 address space reduction 1/(2^4) == 1/16 */
#  if MAJOR_VER >= 3
#    define MAXDATA_LOG2 ((int)(INTPTR_WIDTH - ADDR_REDUCT)) /* 1/16 of address space */
#    define MAXDATA (((LEN)1 << MAXDATA_LOG2) - 1)
#  else
#    define MAXDATA_LOG2 (32 - ADDR_REDUCT)     /* 1/16 of 32-bit address space as defined by calc v2 */
#    define MAXDATA ((1 << MAXDATA_LOG2) - 1)   /* calc v2 compatible supported address space */
#  endif

/*
 * MAXLEN - maximum length of internal integer values in units of HALF
 *
 * We limit MAXLEN based on 1 less than the number of HALFs that
 * will fit into MAXDATA bytes.
 *
 * NOTE: MAXLEN must be 1 less than a power of 2.
 * NOTE: MAXLEN_LOG2 is the log base 2 of MAXLEN+1.
 */
#  define MAXLEN_LOG2 (MAXDATA_LOG2 - BASEB_LOG2)
#  define MAXLEN (((LEN)1 << MAXLEN_LOG2) - 1)

/*
 * The number of decimal digits in 10^power_of_10, where power_of_10 is an integer >= 0:
 *
 *      power_of_10 + 1
 *
 * The number of bytes needed to hold 10^power_of_10:
 *
 *      ceil( (power_of_10 + 1) / log(256) )
 *
 *      NOTE: log() is a log base 10
 *
 * Let us convert that using log base 2 because that will be easier to work with later on.
 * The number of bytes needed to hold 10^power_of_10:
 *
 *      ceil( log2(10) * (power_of_10 + 1) / log2(256) )
 *
 *      NOTE: log2() is a log base 2
 *
 * NOTE: log2(256) == 8, so the number of bytes needed to hold 10^power_of_10:
 *
 *      ceil( log2(10) * (power_of_10 + 1) / 8 )
 *
 * The ztenpow() function (in zfunc.c) computes powers of 10^(2^x), where x is in an integer >= 0.
 * So the number of bytes required to hold 10^(2^x), where x is in an integer >= 0:
 *
 *      ceil( log2(10) * (2^x + 1) / 8 )
 *
 * TEN_MAX, is the limit of those powers of 10^(2^x) computed by the ztenpow(), so 0 <= x <= TEN_MAX.
 * So what is a reasonable value of TEN_MAX?  Read on.  :-)
 *
 * The bits are required to hold the length in bytes of 10^(2^x) is
 * the log base 2 of the number of bytes required to hold 10^(2^x).  So:
 *
 *      ceil( log2( ceil( log2(10) * (2^x + 1) / 8 ) ) )
 *
 * For x == 31, and thus for calculating up to 10^(2^31), we need about 850.5 MBytes.
 * For x == 49, and thus for calculating up to 10^(2^49), we need about 212.6 TBytes.
 */

/*
 * The largest power of 10 we will compute for our decimal conversion internal constants is: 10^(2^TEN_MAX)
 */
#  if MAJOR_VER >= 3
#    define TEN_MAX 49  /* calc v3+: 10^2^49 requires about 1.66 * 2^47 bytes */
#  else
#    define TEN_MAX 31  /* calc v2: 10^2^31 requires about 1.66 * 2^29 bytes */
#  endif

#  define MAXREDC 256 /* number of entries in REDC cache */
#  define SQ_ALG2 28  /* size for alternative squaring */
#  define MUL_ALG2 28 /* size for alternative multiply */
#  define POW_ALG2 20 /* size for using REDC for powers */
/* old REDC_ALG2 was 5/4 of POW_ALG2, so we will keep the same ratio */
#  define REDC_ALG2 25 /* size for using alternative REDC */

typedef union {
    FULL ivalue;
    struct {
        HALF Svalue1;
        HALF Svalue2;
    } sis;
} SIUNION;

#  if CALC_BYTE_ORDER == CALC_LITTLE_ENDIAN
#    define silow sis.Svalue1  /* low order half of full value */
#    define sihigh sis.Svalue2 /* high order half of full value */
#  elif CALC_BYTE_ORDER == CALC_BIG_ENDIAN
#    define silow sis.Svalue2  /* low order half of full value */
#    define sihigh sis.Svalue1 /* high order half of full value */
#  elif CALC_BYTE_ORDER == CALC_PDP_ENDIAN
#    error "calc does NOT (yet?) support pdp Endian"
#  else
#    error "CALC_BYTE_ORDER was defined but was not CALC_LITTLE_ENDIAN, nor CALC_BIG_ENDIAN, nor CALC_PDP_ENDIAN"
#  endif

#  if MAJOR_VER >= 3
typedef bool SIGN; /* sign of a multi-precision integer */
#  else
typedef int32_t SIGN; /* sign of a multi-precision integer */
#  endif

/*
 * ZVALUE - multi-precision integer
 */
typedef struct {
    HALF *v;   /* pointer to array of values */
    LEN len;   /* number of values in array */
    SIGN sign; /* sign, zero/true ==> positive, nonzero/false ==> negative */
} ZVALUE;

/*
 * Function prototypes for integer math routines.
 */
extern HALF *alloc(LEN len);
extern int is_const(HALF *h);
#  ifdef ALLOCTEST
extern void freeh(HALF *);
#  endif

/*
 * Input, output, and conversion routines.
 */
extern void zcopy(ZVALUE z, ZVALUE *res);
extern void itoz(long i, ZVALUE *res);
extern void utoz(FULL i, ZVALUE *res);
extern void stoz(SFULL i, ZVALUE *res);
extern void str2z(char *s, ZVALUE *res);
extern long ztoi(ZVALUE z);
extern FULL ztou(ZVALUE z);
extern SFULL ztos(ZVALUE z);
extern void zprintval(ZVALUE z, long decimals, long width);
extern void zprintx(ZVALUE z, long width);
extern void zprintb(ZVALUE z, long width);
extern void zprinto(ZVALUE z, long width);
extern void fitzprint(ZVALUE, long, long);

/*
 * Basic numeric routines.
 */
extern void zmuli(ZVALUE z, long n, ZVALUE *res);
extern long zdivi(ZVALUE z, long n, ZVALUE *res);
extern long zmodi(ZVALUE z, long n);
extern void zadd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zsub(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zmul(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern long zdiv(ZVALUE z1, ZVALUE z2, ZVALUE *res, ZVALUE *rem, long R);
extern long zquo(ZVALUE z1, ZVALUE z2, ZVALUE *res, long R);
extern long zmod(ZVALUE z1, ZVALUE z2, ZVALUE *rem, long R);
extern void zequo(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern bool zdivides(ZVALUE z1, ZVALUE z2);
extern void zor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zand(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zxor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zandnot(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern long zpopcnt(ZVALUE z, int bitval);
extern void zshift(ZVALUE z, long n, ZVALUE *res);
extern void zsquare(ZVALUE z, ZVALUE *res);
extern long zlowbit(ZVALUE z);
extern LEN zhighbit(ZVALUE z);
extern void zbitvalue(long n, ZVALUE *res);
extern bool zzbitvalue(ZVALUE pos, ZVALUE *res);
extern bool zisset(ZVALUE z, long n);
extern bool zisonebit(ZVALUE z);
extern bool zisallbits(ZVALUE z);
extern FLAG ztest(ZVALUE z);
extern FLAG zrel(ZVALUE z1, ZVALUE z2);
extern FLAG zabsrel(ZVALUE z1, ZVALUE z2);
extern bool zcmp(ZVALUE z1, ZVALUE z2);

/*
 * More complicated numeric functions.
 */
extern FULL uugcd(FULL i1, FULL i2);
extern long iigcd(long i1, long i2);
extern void zgcd(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zlcm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zreduce(ZVALUE z1, ZVALUE z2, ZVALUE *z1res, ZVALUE *z2res);
extern void zfact(ZVALUE z, ZVALUE *dest);
extern void zperm(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern int zcomb(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern FLAG zjacobi(ZVALUE z1, ZVALUE z2);
extern void zfib(ZVALUE z, ZVALUE *res);
extern void zpowi(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void ztenpow(long power, ZVALUE *res);
extern void zpowermod(ZVALUE z1, ZVALUE z2, ZVALUE z3, ZVALUE *res);
extern bool zmodinv(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern bool zrelprime(ZVALUE z1, ZVALUE z2);
extern long zlog(ZVALUE z1, ZVALUE z2);
extern long zlog10(ZVALUE z, bool *was_10_power);
extern long zdivcount(ZVALUE z1, ZVALUE z2);
extern long zfacrem(ZVALUE z1, ZVALUE z2, ZVALUE *rem);
extern long zgcdrem(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern long zdigits(ZVALUE z1);
extern long zdigit(ZVALUE z1, long n);
extern FLAG zsqrt(ZVALUE z1, ZVALUE *dest, long R);
extern void zroot(ZVALUE z1, ZVALUE z2, ZVALUE *dest);
extern bool zissquare(ZVALUE z);
extern void zhnrmod(ZVALUE v, ZVALUE h, ZVALUE zn, ZVALUE zr, ZVALUE *res);
extern bool zispowerof2(ZVALUE z, FULL *log2);

/*
 * Prime related functions.
 */
extern FLAG zisprime(ZVALUE z);
extern FULL znprime(ZVALUE z);
extern FULL next_prime(FULL v);
extern FULL zpprime(ZVALUE z);
extern void zpfact(ZVALUE z, ZVALUE *dest);
extern bool zprimetest(ZVALUE z, long count, ZVALUE skip);
extern bool zredcprimetest(ZVALUE z, long count, ZVALUE skip);
extern bool znextcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res, ZVALUE mod, ZVALUE *cand);
extern bool zprevcand(ZVALUE z1, long count, ZVALUE skip, ZVALUE res, ZVALUE mod, ZVALUE *cand);
extern FULL zlowfactor(ZVALUE z, long count);
extern FLAG zfactor(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern long zpix(ZVALUE z1);
extern void zlcmfact(ZVALUE z, ZVALUE *dest);

/*
 * miscellaneous functions. :-)
 */
extern void zsquaremod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zminmod(ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern bool zcmpmod(ZVALUE z1, ZVALUE z2, ZVALUE z3);
extern void zio_init(void);

/*
 * These functions are for internal use only.
 */
extern void ztrim(ZVALUE *z);
extern void zshiftr(ZVALUE z, long n);
extern void zshiftl(ZVALUE z, long n);
extern HALF *zalloctemp(LEN len);

/*
 * Modulo arithmetic definitions.
 * Structure holding state of REDC initialization.
 * Multiple instances of this structure can be used allowing
 * calculations with more than one modulus at the same time.
 * Len of zero means the structure is not initialized.
 */
typedef struct {
    LEN len;    /* number of words in binary modulus */
    ZVALUE mod; /* modulus REDC is computing with */
    ZVALUE inv; /* inverse of modulus in binary modulus */
    ZVALUE one; /* REDC format for the number 1 */
} REDC;

extern REDC *zredcalloc(ZVALUE z1);
extern void zredcfree(REDC *rp);
extern void zredcencode(REDC *rp, ZVALUE z1, ZVALUE *res);
extern void zredcdecode(REDC *rp, ZVALUE z1, ZVALUE *res);
extern void zredcmul(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);
extern void zredcsquare(REDC *rp, ZVALUE z1, ZVALUE *res);
extern void zredcpower(REDC *rp, ZVALUE z1, ZVALUE z2, ZVALUE *res);

/*
 * macro expansions to speed this thing up
 */
#  define ziseven(z) (!(*(z).v & 0x1))
#  define zisodd(z) (*(z).v & 0x1)
#  define ziszero(z) ((*(z).v == 0) && ((z).len == 1))
#  define zisneg(z) ((z).sign)
#  define zispos(z) (((z).sign == 0) && (*(z).v || ((z).len > 1)))
#  define zisunit(z) ((*(z).v == 1) && ((z).len == 1))
#  define zisone(z) ((*(z).v == 1) && ((z).len == 1) && !(z).sign)
#  define zisnegone(z) ((*(z).v == 1) && ((z).len == 1) && (z).sign)
#  define zltnegone(z) (zisneg(z) && (((z).len > 1) || *(z).v > 1))
#  define zistwo(z) ((*(z).v == 2) && ((z).len == 1) && !(z).sign)
#  define zisabstwo(z) ((*(z).v == 2) && ((z).len == 1))
#  define zisabsleone(z) ((*(z).v <= 1) && ((z).len == 1))
#  define zislezero(z) (zisneg(z) || ziszero(z))
#  define zisleone(z) (zisneg(z) || zisabsleone(z))
#  define zistiny(z) ((z).len == 1)

/*
 * zgtmaxfull(z)        true if abs(z) > MAXFULL
 */
#  define zgtmaxfull(z) (((z).len > 2) || (((z).len == 2) && (((SHALF)(z).v[1]) < 0)))

/*
 * zgtmaxufull(z)       true if abs(z) will not fit into a FULL (> MAXUFULL)
 */
#  define zgtmaxufull(z) ((z).len > 2)

/*
 * zgtmaxulong(z)       true if abs(z) > MAXULONG
 */
#  if BASEB >= LONG_BITS
#    define zgtmaxulong(z) ((z).len > 1)
#  else
#    define zgtmaxulong(z) zgtmaxufull(z)
#  endif

/*
 * zgtmaxlong(z)        true if abs(z) > MAXLONG
 */
#  if BASEB >= LONG_BITS
#    define zgtmaxlong(z) (((z).len > 1) || (((z).len == 1) && (((SHALF)(z).v[0]) < 0)))
#  else
#    define zgtmaxlong(z) zgtmaxfull(z)
#  endif

/*
 * Some algorithms testing for values of a certain length.  Macros such as
 * zistiny() do this well.  In other cases algorithms require tests for values
 * in comparison to a given power of 2.  In the later case, zistiny() compares
 * against a different power of 2 on a 64 bit machine.  The macros below
 * provide a tests against powers of 2 that are independent of the work size.
 *
 *      zge16b(z)       true if abs(z) >= 2^16
 *      zge24b(z)       true if abs(z) >= 2^24
 *      zge31b(z)       true if abs(z) >= 2^31
 *      zge32b(z)       true if abs(z) >= 2^32
 *      zge64b(z)       true if abs(z) >= 2^64
 *      zge128b(z)      true if abs(z) >= 2^128
 *      zge256b(z)      true if abs(z) >= 2^256
 *      zge512b(z)      true if abs(z) >= 2^512
 *      zge1024b(z)     true if abs(z) >= 2^1024
 *      zge2048b(z)     true if abs(z) >= 2^2048
 *      zge4096b(z)     true if abs(z) >= 2^4096
 *      zge8192b(z)     true if abs(z) >= 2^8192
 */
#  if BASEB == 32

#    define zge16b(z) (!zistiny(z) || ((z).v[0] >= (HALF)0x10000))
#    define zge24b(z) (!zistiny(z) || ((z).v[0] >= (HALF)0x1000000))
#    define zge31b(z) (!zistiny(z) || (((SHALF)(z).v[0]) < 0))
#    define zge32b(z) (!zistiny(z))
#    define zge64b(z) ((z).len > 2)
#    define zge128b(z) ((z).len > 4)
#    define zge256b(z) ((z).len > 8)
#    define zge512b(z) ((z).len > 16)
#    define zge1024b(z) ((z).len > 32)
#    define zge2048b(z) ((z).len > 64)
#    define zge4096b(z) ((z).len > 128)
#    define zge8192b(z) ((z).len > 256)

#  else

#    define zge16b(z) (!zistiny(z))
#    define zge24b(z) (((z).len > 2) || (((z).len == 2) && ((z).v[1] >= (HALF)0x100)))
#    define zge31b(z) (((z).len > 2) || (((z).len == 2) && (((SHALF)(z).v[1]) < 0)))
#    define zge32b(z) ((z).len > 2)
#    define zge64b(z) ((z).len > 4)
#    define zge128b(z) ((z).len > 8)
#    define zge256b(z) ((z).len > 16)
#    define zge512b(z) ((z).len > 32)
#    define zge1024b(z) ((z).len > 64)
#    define zge2048b(z) ((z).len > 128)
#    define zge4096b(z) ((z).len > 256)
#    define zge8192b(z) ((z).len > 512)

#  endif

/*
 * ztofull - convert an absolute value of a ZVALUE to a FULL if possible
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a FULL will be used.
 */
#  define ztofull(z) (zistiny(z) ? ((FULL)((z).v[0])) : ((FULL)((z).v[0]) + ((FULL)((z).v[1]) << BASEB)))

#  define z1tol(z) ((long)((z).v[0]))
#  define z2tol(z) ((long)(((z).v[0]) + (((z).v[1] & MAXHALF) << BASEB)))

/*
 * ztoulong - convert an absolute value of a ZVALUE to an unsigned long
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a long will be used.
 */
#  if BASEB >= LONG_BITS
#    define ztoulong(z) ((unsigned long)z1tol(z))
#  else
#    define ztoulong(z) ((unsigned long)ztofull(z))
#  endif

/*
 * ztolong - convert an absolute value of a ZVALUE to a long
 *
 * If the value is too large, only the low order bits that are able to
 * be converted into a long will be used.
 */
#  define ztolong(z) ((long)(ztoulong(z) & MAXLONG))

#  define zclearval(z) memset((z).v, 0, (z).len * sizeof(HALF))
#  define zcopyval(z1, z2) memcpy((z2).v, (z1).v, (z1).len * sizeof(HALF))
#  define zquicktrim(z)                                   \
      {                                                   \
          if (((z).len > 1) && ((z).v[(z).len - 1] == 0)) \
              (z).len--;                                  \
      }
#  define zfree(z)                             \
      {                                        \
          if ((z).len != 0 && (z).v != NULL) { \
              freeh((z).v);                    \
              (z).v = NULL;                    \
              (z).len = 0;                     \
              (z).sign = 0;                    \
          }                                    \
      }

/*
 * Output modes for numeric displays.
 */
#  define MODE_DEFAULT 0
#  define MODE_FRAC 1
#  define MODE_INT 2
#  define MODE_REAL 3
#  define MODE_EXP 4
#  define MODE_HEX 5
#  define MODE_OCTAL 6
#  define MODE_BINARY 7
#  define MODE_REAL_AUTO 8
#  define MODE_ENG 9
#  define MODE_MAX 9
#  define MODE2_OFF (MODE_MAX + 1)

#  define MODE_INITIAL MODE_REAL
#  define MODE2_INITIAL MODE2_OFF

/*
 * Output routines for either FILE handles or strings.
 */
extern void math_chr(int ch);
extern void math_str(char *str);
extern void math_fill(char *str, long width);
extern void math_flush(void);
extern void math_divertio(void);
extern void math_cleardiversions(void);
extern char *math_getdivertedio(void);
extern int math_setmode(int mode);
extern int math_setmode2(int mode);
extern LEN math_setdigits(LEN digits);
extern void math_fmt(char *, ...) __attribute__((format(printf, 1, 2)));

/*
 * external swap functions
 */
extern HALF *swap_b8_in_HALFs(HALF *dest, HALF *src, LEN len);
extern ZVALUE *swap_b8_in_ZVALUE(ZVALUE *dest, ZVALUE *src, bool all);
extern HALF *swap_b16_in_HALFs(HALF *dest, HALF *src, LEN len);
extern HALF *swap_HALFs(HALF *dest, HALF *src, LEN len);
extern ZVALUE *swap_b16_in_ZVALUE(ZVALUE *dest, ZVALUE *src, bool all);
extern ZVALUE *swap_HALF_in_ZVALUE(ZVALUE *dest, ZVALUE *src, bool all);

/*
 * constants used often by the arithmetic routines
 */
extern HALF _zeroval_[];
extern ZVALUE _zero_;

extern HALF _oneval_[];
extern ZVALUE _one_;
extern ZVALUE _neg_one_;

extern HALF _twoval_[];
extern ZVALUE _two_;

extern HALF _tenval_[];
extern ZVALUE _ten_;

extern HALF _sqbaseval_[];
extern ZVALUE _sqbase_;

extern HALF _pow4baseval_[];
extern ZVALUE _pow4base_;

extern HALF _pow8baseval_[];
extern ZVALUE _pow8base_;

extern HALF _basebval_[];
extern ZVALUE _baseb_;

/* _b32_ is _sqbaseval_ or _pow4baseval_ depending on BASEB */
extern ZVALUE _b32_;

/* _b64_ is _pow4baseval_ or _pow8baseval_ depending on BASEB */
extern ZVALUE _b64_;

extern HALF *half_tbl[]; /* preset HALF constants, NULL terminated list */

extern bool _math_abort_;    /* nonzero to abort calculations */
extern ZVALUE _tenpowers_[]; /* table of 10^2^n */

/*
 * Bit fiddling functions and types
 */
extern HALF bitmask[];        /* bit rotation, norm 0 */
extern HALF lowhalf[];        /* bit masks from low end of HALF */
extern HALF rlowhalf[];       /* reversed bit masks from low end of HALF */
extern HALF highhalf[];       /* bit masks from high end of HALF */
extern HALF rhighhalf[];      /* reversed bit masks from high end of HALF */
#  define HAVE_REVERSED_MASKS /* allows old code to know libcalc.a has them */

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
    HALF *loc; /* half address of most significant bit */
    int bit;   /* bit position within half of most significant bit */
    int len;   /* length of string in bits */
} BITSTR;

#endif
