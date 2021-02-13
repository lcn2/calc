/*
 * block - fixed, dynamic, fifo and circular memory blocks
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll and Ernest Bowen
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
 * Under source code control:	1997/02/21 05:03:39
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_BLOCK_H)
#define INCLUDE_BLOCK_H


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
 *		Reduce storage down to 0 octets.
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
 *		overlapping moves are handled correctly
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
 *		copy successive octets from src into dest1, dest2, ...
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
EXTERN int blk_debug;	/* 0 => debug off */


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

/* block footprint in memory */
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
E_FUNC BLOCK *blkalloc(int, int);
E_FUNC void blk_free(BLOCK*);
E_FUNC BLOCK *blkrealloc(BLOCK*, int, int);
E_FUNC void blktrunc(BLOCK*);
E_FUNC BLOCK *blk_copy(BLOCK*);
E_FUNC int blk_cmp(BLOCK*, BLOCK*);
E_FUNC void blk_print(BLOCK*);
E_FUNC void nblock_print(NBLOCK *);
E_FUNC NBLOCK *createnblock(char *, int, int);
E_FUNC NBLOCK *reallocnblock(int, int, int);
E_FUNC int removenblock(int);
E_FUNC int findnblockid(char *);
E_FUNC NBLOCK *findnblock(int);
E_FUNC BLOCK *copyrealloc(BLOCK*, int, int);
E_FUNC int countnblocks(void);
E_FUNC void shownblocks(void);


#endif /* !INCLUDE_BLOCK_H */
