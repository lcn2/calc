/*
 * block - fixed, dynamic, fifo and circular memory blocks
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
 * @(#) $Id: block.h,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/block.h,v $
 *
 * Under source code control:	1997/02/21 05:03:39
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__BLOCK_H__)
#define __BLOCK_H__


/*
 * block - the basic block structure
 *
 * A block comes is one of several types.  At the moment, only fixed
 * types are defined.
 *
 ***
 *
 * Block functions and operations:
 *
 *	x[i]
 *		(i-1)th octet
 *
 *	blk(len [, blkchunk])
 *		unnamed block
 *		len > 0
 *		blkchunk defaults to BLK_CHUNKSIZE
 *
 *	blk(name, [len [, blkchunk]])
 *		named block
 *		len > 0
 *		blkchunk defaults to BLK_CHUNKSIZE
 *
 *	blkfree(x)
 *		Reduce storage down to 0 octetes.
 *
 *	size(x)
 *		The length of data stored in the block.
 *
 *	sizeof(x) == blk->maxsize
 *		Allocation size in memory
 *
 *	isblk(x)
 *		returns 0 is x is not a BLOCK, 1 if x is an
 *		unnamed block, 2 if x is a named BLOCK
 *
 *	blkread(x, size, count, fd [, offset])
 *	blkwrite(x, size, count, fd [, offset])
 *		returns number of items written
 *		offset is restricted in value by block type
 *
 *	blkset(x, val, length [, offset])
 *		only the lower octet of val is used
 *		offset is restricted in value by block type
 *
 *	blkchr(x, val, length [, offset])
 *		only the lower octet of val is used
 *		offset is restricted in value by block type
 *
 *	blkcpy(dest, src, length [, dest_offset [, src_offset]])
 *		0 <= length <= blksize(x)
 *		offset's are restricted in value by block type
 *		dest may not == src
 *
 *	blkmove(dest, src, length [, dest_offset [, src_offset]])
 *		0 <= length <= blksize(x)
 *		offset's are restricted in value by block type
 *		overlapping moves are handeled correctly
 *
 *	blkccpy(dest, src, stopval, length [, dest_offset [, src_offset]])
 *		0 <= length <= blksize(x)
 *		offset's are restricted in value by block type
 *
 *	blkcmp(dest, src, length [, dest_offset [, src_offset]])
 *		0 <= length <= blksize(x)
 *		offset's are restricted in value by block type
 *
 *	blkswap(x, a, b)
 *		swaps groups of 'a' octets within each 'b' octets
 *		b == a is a noop
 *		b = a*k for some integer k >= 1
 *
 *	scatter(src, dest1, dest2 [, dest3 ] ...)
 *		copy sucessive octets from src into dest1, dest2, ...
 *		    restarting with dest1 after end of list
 *		stops at end of src
 *
 *	gather(dest, src1, src2 [, src3 ] ...)
 *		copy first octet from src1, src2, ...
 *		copy next octet from src1, src2, ...
 *		...
 *		copy last octet from src1, src2, ...
 *		copy 0 when there is no more data from a given source
 *
 *	blkseek(x, offset, {"in","out"})
 *		some seeks may not be allowed by block type
 *
 *	config("blkmaxprint", count)
 *		number of octets of a block to print, 0 means all
 *
 *	config("blkverbose", boolean)
 *		TRUE => print all lines, FALSE => skip dup lines
 *
 *	config("blkbase", "base")
 *		output block base = { "hex", "octal", "char", "binary", "raw" }
 *		    binary is base 2, raw is just octet values
 *
 *	config("blkfmt", "style")
 *		style of output = {
 *		    "line",	lines in blkbase with no spaces between octets
 *		    "string",	as one long line with no spaces between octets
 *		    "od_style", position, spaces between octets
 *		    "hd_style"} position, spaces between octets, chars on end
 */
struct block {
	LEN blkchunk;	/* allocation chunk size */
	LEN maxsize;	/* octets actually malloced for this block */
	LEN datalen;	/* octets of data held this block */
	USB8 *data;	/* pointer to the 1st octet of the allocated data */
};
typedef struct block BLOCK;


struct nblock {
	char *name;
	int subtype;
	int id;
	BLOCK *blk;
};
typedef struct nblock NBLOCK;


/*
 * block debug
 */
extern int blk_debug;	/* 0 => debug off */


/*
 * block defaults
 */
#define BLK_CHUNKSIZE 256	/* default allocation chunk size for blocks */

#define BLK_DEF_MAXPRINT 256	/* default octets to print */

#define BLK_BASE_HEX 0		/* output octets in a block in hex */
#define BLK_BASE_OCT 1		/* output octets in a block in octal */
#define BLK_BASE_CHAR 2		/* output octets in a block in characters */
#define BLK_BASE_BINARY 3	/* output octets in a block in base 2 chars */
#define BLK_BASE_RAW 4		/* output octets in a block in raw binary */

#define BLK_FMT_HD_STYLE 0	/* output in base with chars on end of line */
#define BLK_FMT_LINE 1		/* output is lines of up to 79 chars */
#define BLK_FMT_STRING 2	/* output is one long string */
#define BLK_FMT_OD_STYLE 3	/* output in base with chars */


/*
 * block macros
 */
/* length of data stored in a block */
#define blklen(blk) ((blk)->datalen)

/* block footpint in memory */
#define blksizeof(blk) ((blk)->maxsize)

/* block allocation chunk size */
#define blkchunk(blk) ((blk)->blkchunk)


/*
 * OCTET - what the INDEXADDR produces from a blk[offset]
 */
typedef USB8 OCTET;


/*
 * external functions
 */
extern BLOCK *blkalloc(int, int);
extern void blk_free(BLOCK*);
extern BLOCK *blkrealloc(BLOCK*, int, int);
extern void blktrunc(BLOCK*);
extern BLOCK *blk_copy(BLOCK*);
extern int blk_cmp(BLOCK*, BLOCK*);
extern void blk_print(BLOCK*);
extern void nblock_print(NBLOCK *);
extern NBLOCK *createnblock(char *, int, int);
extern NBLOCK *reallocnblock(int, int, int);
extern int removenblock(int);
extern int findnblockid(char *);
extern NBLOCK *findnblock(int);
extern BLOCK *copyrealloc(BLOCK*, int, int);
extern int countnblocks(void);
extern void shownblocks(void);


#endif /* !__BLOCK_H__ */
