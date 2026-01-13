/*
 * blkcpy - general values and related routines used by the calculator
 *
 * Copyright (C) 1999-2007,2014,2026  Landon Curt Noll and Ernest Bowen
 *
 * Primary author:  Landon Curt Noll
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
 * Under source code control:   1997/04/18 20:41:25
 * File existed as early as:    1997
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_BLKCPY_H)
#  define INCLUDE_BLKCPY_H

/*
 * the main copy gateway function
 */
extern int copystod(VALUE *, LEN, LEN, VALUE *, LEN);

/*
 * specific copy functions
 */
extern int copyblk2blk(BLOCK *, LEN, LEN, BLOCK *, LEN, bool);
extern int copyblk2file(BLOCK *, LEN, LEN, FILEID, LEN);
extern int copyblk2mat(BLOCK *, LEN, LEN, MATRIX *, LEN);
extern int copyblk2num(BLOCK *, LEN, LEN, NUMBER *, LEN, NUMBER **);
extern int copyblk2str(BLOCK *, LEN, LEN, STRING *, LEN);
extern int copyfile2blk(FILEID, LEN, LEN, BLOCK *, LEN, bool);
extern int copylist2list(LIST *, LEN, LEN, LIST *, LEN);
extern int copylist2mat(LIST *, LEN, LEN, MATRIX *, LEN);
extern int copymat2blk(MATRIX *, LEN, LEN, BLOCK *, LEN, bool);
extern int copymat2list(MATRIX *, LEN, LEN, LIST *, LEN);
extern int copymat2mat(MATRIX *, LEN, LEN, MATRIX *, LEN);
extern int copynum2blk(NUMBER *, LEN, LEN, BLOCK *, LEN, bool);
extern int copyostr2blk(char *, LEN, LEN, BLOCK *, LEN, bool);
extern int copyostr2str(char *, LEN, LEN, STRING *, LEN);
extern int copystr2blk(STRING *, LEN, LEN, BLOCK *, LEN, bool);
extern int copystr2file(STRING *, LEN, LEN, FILEID, LEN);
extern int copystr2str(STRING *, LEN, LEN, STRING *, LEN);

#endif
