/*
 * lib_util - calc link library utility routines
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
 * @(#) $Revision: 29.2 $
 * @(#) $Id: lib_util.h,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/lib_util.h,v $
 *
 * Under source code control:	1997/04/19 21:38:30
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * These routines are here to support users of libcalc.a.  These routines
 * are not directly used by calc itself, however.
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
