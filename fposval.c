/*
 * fposval - Determine information about the file position type
 *
 * The include file have_pos.h, as built during the make process will
 * define the type FILEPOS as the type used to describe file positions.
 * We will print information regarding the size and byte order
 * of this definition.
 *
 * The stat system call returns a stat structure.  One of the elements
 * of the stat structure is the st_size element.  We will print information
 * regarding the size and byte order of st_size.
 *
 * We will #define of 8 symbols:
 *
 *	FILEPOS_BITS 		length in bits of the type FILEPOS
 *	SWAP_HALF_IN_FILEPOS	will copy/swap FILEPOS into an HALF array
 *	STSIZE_BITS		length in bits of the st_size stat element
 *	SWAP_HALF_IN_STSIZE	will copy/swap st_size into an HALF array
 *	DEV_BITS		length in bits of the st_dev stat element
 *	SWAP_HALF_IN_DEV	will copy/swap st_dev into an HALF array
 *	INODE_BITS		length in bits of the st_ino stat element
 *	SWAP_HALF_IN_INODE	will copy/swap st_ino into an HALF array
 *
 * With regards to 'will copy/swap ... into an HALF array'.  Such macros
 * will either be a copy or a copy with HALFs swapped depending on the
 * Endian order of the hardware.
 */
/*
 * Copyright (c) 1996 by Landon Curt Noll.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "have_fpos.h"
#include "endian_calc.h"

char *program;			/* our name */

MAIN
main(int argc, char **argv)
{
	int stsizelen;		/* bit length of st_size in buf */
	int fileposlen;		/* bit length of FILEPOS */
	int devlen;		/* bit length of st_dev in buf */
	int inodelen;		/* bit length of st_ino in buf */
	struct stat buf;	/* file status */

	/*
	 * parse args
	 */
	program = argv[0];

	/*
	 * print the file position information
	 */
	fileposlen = sizeof(FILEPOS)*8;
	printf("#undef FILEPOS_BITS\n");
	printf("#define FILEPOS_BITS %d\n", fileposlen);
#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * Big Endian
	 */
	if (fileposlen == 64) {
		printf("#define SWAP_HALF_IN_FILEPOS(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B64(dest, src)");
	} else if (fileposlen == 32) {
		printf("#define SWAP_HALF_IN_FILEPOS(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B32(dest, src)");
	} else {
		fprintf(stderr, "%s: unexpected FILEPOS bit size: %d\n",
		    program, fileposlen);
		exit(1);
	}
#else /* BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a FILEPOS is not a scalar hince we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_FILEPOS(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), sizeof(",fileposlen,"))");
#endif /* BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the stat file size information
	 */
	stsizelen = sizeof(buf.st_size)*8;
	printf("#undef STSIZE_BITS\n");
	printf("#define STSIZE_BITS %d\n", stsizelen);
#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * Big Endian
	 */
	if (stsizelen == 64) {
		printf("#define SWAP_HALF_IN_STSIZE(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B64(dest, src)");
	} else if (stsizelen == 32) {
		printf("#define SWAP_HALF_IN_STSIZE(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B32(dest, src)");
	} else {
		fprintf(stderr, "%s: unexpected st_size bit size: %d\n",
		    program, stsizelen);
		exit(2);
	}
#else /* BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a STSIZE is not a scalar hince we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_STSIZE(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), sizeof(",stsizelen,"))");
#endif /* BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the dev_t size
	 */
	devlen = sizeof(buf.st_dev)*8;
	printf("#undef DEV_BITS\n");
	printf("#define DEV_BITS %d\n", devlen);
#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * Big Endian
	 */
	if (devlen == 64) {
		printf("#define SWAP_HALF_IN_DEV(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B64(dest, src)");
	} else if (devlen == 32) {
		printf("#define SWAP_HALF_IN_DEV(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B32(dest, src)");
	} else if (devlen == 16) {
		printf("#define SWAP_HALF_IN_DEV(dest, src)\t\t%s\n",
		    "(*(dest) = *(src))");
	} else {
		fprintf(stderr, "%s: unexpected st_dev bit size: %d\n",
		    program, devlen);
		exit(3);
	}
#else /* BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a DEV is not a scalar hince we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_DEV(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), sizeof(",devlen,"))");
#endif /* BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the ino_t size
	 */
	inodelen = sizeof(buf.st_ino)*8;
	printf("#undef INODE_BITS\n");
	printf("#define INODE_BITS %d\n", inodelen);
#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * Big Endian
	 */
	if (inodelen == 64) {
		printf("#define SWAP_HALF_IN_INODE(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B64(dest, src)");
	} else if (inodelen == 32) {
		printf("#define SWAP_HALF_IN_INODE(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B32(dest, src)");
	} else if (inodelen == 16) {
		printf("#define SWAP_HALF_IN_INODE(dest, src)\t\t%s\n",
		    "(*(dest) = *(src))");
	} else {
		fprintf(stderr, "%s: unexpected st_ino bit size: %d\n",
		    program, inodelen);
		exit(4);
	}
#else /* BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a INODE is not a scalar hince we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_INODE(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), sizeof(",inodelen,"))");
#endif /* BYTE_ORDER == BIG_ENDIAN */
	exit(0);
}
