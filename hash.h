/* XXX - this code is currently not really used, but it will be soon */
/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined(HASH_H)
#define HASH_H

/*
 * hashstate - state of a hash system
 */
struct hashstate {
        int type;               /* hash type (see XYZ_HASH_TYPE below) */
        BOOL prevstr;           /* TRUE=>previous value hashed was a string */
        union {
		SHS_INFO hh_shs;        /* old Secure Hash Standard */
        } h_union;
};
typedef struct hashstate HASH;
/* For ease in referencing */
#define h_shs   h_union.hh_shs


/* 
 * XYZ_HASH_TYPE - hash types
 *
 * we support these hash types - must start with 0 
 */
#define SHS_HASH_TYPE 0
#define HASH_TYPE_MAX 0         /* must be number of XYZ_HASH_TYPE values */

#endif /* !HASH_H */
