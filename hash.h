/*
 * hash - one-way hash routines
 *
 * Copyright (C) 1999  Landon Curt Noll
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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: hash.h,v 29.4 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/hash.h,v $
 *
 * Under source code control:	1995/11/14 23:57:45
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__HASH_H__)
#define __HASH_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "shs.h"
# include "shs1.h"
# include "md5.h"
# include "zmath.h"
#else
# include <calc/shs.h>
# include <calc/shs1.h>
# include <calc/md5.h>
# include <calc/zmath.h>
#endif


/* MAX_CHUNKSIZE is the largest chunksize of any hash */
#define MAX_CHUNKSIZE (SHS1_CHUNKSIZE)

/* max size of debugging strings in xyz_print() functions */
#define DEBUG_SIZE 127


/*
 * hashstate - state of a hash system
 *
 * Hashing some types of values requires a checkpoint (chkpt function call)
 * to be performed, which pads buffered data with 0's and performs an
 * update.  The checkpoint thus causes the value to start on a new hash
 * block boundary with no buffered data.
 *
 * Some data types (strings, BLOCKs and OCTETs) do not require a
 * checkpoint as long as the previous value hashed was a string,
 * BLOCK or OCTET.
 */
typedef struct hashstate HASH;
struct hashstate {
	int hashtype;		/* XYZ_HASH_TYPE debug value */
	BOOL bytes;		/* TRUE => reading bytes rather than words */
	void (*update)(HASH*, USB8*, USB32);	/* update arbitrary length */
	void (*chkpt)(HASH*);			/* checkpoint a state */
	void (*note)(int, HASH*);		/* note a special value */
	void (*type)(int, HASH*);		/* note a VALUE type */
	ZVALUE (*final)(HASH*);		/* complete hash state */
	int (*cmp)(HASH*,HASH*);	/* compare to states, TRUE => a!=b */
	void (*print)(HASH*);		/* print the value of a hash */
	int base;			/* XYZ_BASE special hash value */
	int chunksize;			/* XYZ_CHUNKSIZE input chunk size */
	int unionsize;			/* h_union element size */
	union {				/* hash dependent states */
		USB8 data[1];		/* used by hash_value to hash below */
		SHS_INFO h_shs;		/* old SHS/SHA internal state */
		SHS1_INFO h_shs1;	/* new SHS-1/SHA-1 internal state */
		MD5_CTX h_md5;		/* MD5 internal state */
	} h_union;
};


/*
 * what to xor to digest value when hashing special values
 *
 * IMPORTANT: To avoid overlap due to the HASH_XYZ macros below, the
 *	      XYZ_BASE values should be unique random hex values
 *	      that end in 00 (i.e., 0 mod 256).
 */
#define SHS_BASE 0x12face00		/* old SHS / SHA */
#define SHS1_BASE 0x23cafe00		/* new SHS-1 / SHA-1 */
#define MD5_BASE 0x34feed00		/* MD5 */


/*
 * XYZ_HASH_TYPE - hash types
 *
 * we support these hash types
 */
#define SHS_HASH_TYPE 1
#define SHS1_HASH_TYPE 2
#define MD5_HASH_TYPE 3


/*
 * Note a special value given the base value
 */
#define HASH_NEG(base) (1+base)		/* note a negative value */
#define HASH_COMPLEX(base) (2+base)	/* note a complex value */
#define HASH_DIV(base) (4+base)		/* note a division by a value */
#define HASH_ZERO(base) (8+base)	/* note a zero numeric value */
#define HASH_ZVALUE(base) (16+base)	/* note a ZVALUE */


/*
 * external functions
 */
extern HASH* hash_init(int, HASH*);
extern void hash_free(HASH*);
extern HASH* hash_copy(HASH*);
extern int hash_cmp(HASH*, HASH*);
extern void hash_print(HASH*);
extern ZVALUE hash_final(HASH*);
extern HASH* hash_long(int, long, HASH*);
extern HASH* hash_zvalue(int, ZVALUE, HASH*);
extern HASH* hash_number(int, void*, HASH*);
extern HASH* hash_complex(int, void*, HASH*);
extern HASH* hash_str(int, char*, HASH*);
extern HASH* hash_usb8(int, USB8*, int, HASH*);
extern HASH* hash_value(int, void*, HASH*);


#endif /* !__HASH_H__ */
