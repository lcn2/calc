/*
 * lib_util - calc library utility routines
 *
 * These routines are here to support users of libcalc.a.  These routines
 * are not directly used by calc itself, however.
 */

/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
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
 *
 * chongo was here	/\../\
 */


#if !defined(__LIB_UTIL_H__)
#define __LIB_UTIL_H__

/* external functions in lib_util.c */
extern int lowhex2bin[256];
extern char lowbin2hex[256];
extern ZVALUE convstr2z(char*);
extern ZVALUE convhex2z(char *hex);
extern char *convz2hex(ZVALUE z);

#endif /* __LIB_UTIL_H__ */
