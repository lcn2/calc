/*
 * blkcpy - general values and related routines used by the calculator
 *
 * Copyright (C) 1999  Landon Curt Noll and Ernest Bowen
 *
 * Primary author:  Landon Curt Noll
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
 * @(#) $Id: blkcpy.h,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/blkcpy.h,v $
 *
 * Under source code control:	1997/04/18 20:41:25
 * File existed as early as:	1997
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__BLKCPY_H__)
#define __BLKCPY_H__

/*
 * the main copy gateway function
 */
extern int copystod(VALUE *, long, long, VALUE *, long);

/*
 * specific copy functions
 */
extern int copyblk2blk(BLOCK *, long, long, BLOCK *, long, BOOL);
extern int copyblk2file(BLOCK *, long, long, FILEID, long);
extern int copyblk2mat(BLOCK *, long, long, MATRIX *, long);
extern int copyblk2num(BLOCK *, long, long, NUMBER *, long, NUMBER **);
extern int copyblk2str(BLOCK *, long, long, STRING *, long);
extern int copyfile2blk(FILEID, long, long, BLOCK *, long, BOOL);
extern int copylist2list(LIST *, long, long, LIST *, long);
extern int copylist2mat(LIST *, long, long, MATRIX *, long);
extern int copymat2blk(MATRIX *, long, long, BLOCK *, long, BOOL);
extern int copymat2list(MATRIX *, long, long, LIST *, long);
extern int copymat2mat(MATRIX *, long, long, MATRIX *, long);
extern int copynum2blk(NUMBER *, long, long, BLOCK *, long, BOOL);
extern int copyostr2blk(char *, long, long, BLOCK *, long, BOOL);
extern int copyostr2str(char *, long, long, STRING *, long);
extern int copystr2blk(STRING *, long, long, BLOCK *, long, BOOL);
extern int copystr2file(STRING *, long, long, FILEID, long);
extern int copystr2str(STRING *, long, long, STRING *, long);

#endif /* !__BLKCPY_H__ */
