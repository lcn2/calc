/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Definitions of general values and related routines used by the calculator.
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
