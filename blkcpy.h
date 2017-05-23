/*
 * blkcpy - general values and related routines used by the calculator
 *
 * Copyright (C) 1999-2007,2014  Landon Curt Noll and Ernest Bowen
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
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1997/04/18 20:41:25
 * File existed as early as:	1997
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_BLKCPY_H)
#define INCLUDE_BLKCPY_H

/*
 * the main copy gateway function
 */
E_FUNC int copystod(VALUE *, long, long, VALUE *, long);

/*
 * specific copy functions
 */
E_FUNC int copyblk2blk(BLOCK *, long, long, BLOCK *, long, BOOL);
E_FUNC int copyblk2file(BLOCK *, long, long, FILEID, long);
E_FUNC int copyblk2mat(BLOCK *, long, long, MATRIX *, long);
E_FUNC int copyblk2num(BLOCK *, long, long, NUMBER *, long, NUMBER **);
E_FUNC int copyblk2str(BLOCK *, long, long, STRING *, long);
E_FUNC int copyfile2blk(FILEID, long, long, BLOCK *, long, BOOL);
E_FUNC int copylist2list(LIST *, long, long, LIST *, long);
E_FUNC int copylist2mat(LIST *, long, long, MATRIX *, long);
E_FUNC int copymat2blk(MATRIX *, long, long, BLOCK *, long, BOOL);
E_FUNC int copymat2list(MATRIX *, long, long, LIST *, long);
E_FUNC int copymat2mat(MATRIX *, long, long, MATRIX *, long);
E_FUNC int copynum2blk(NUMBER *, long, long, BLOCK *, long, BOOL);
E_FUNC int copyostr2blk(char *, long, long, BLOCK *, long, BOOL);
E_FUNC int copyostr2str(char *, long, long, STRING *, long);
E_FUNC int copystr2blk(STRING *, long, long, BLOCK *, long, BOOL);
E_FUNC int copystr2file(STRING *, long, long, FILEID, long);
E_FUNC int copystr2str(STRING *, long, long, STRING *, long);

#endif /* !INCLUDE_BLKCPY_H */
