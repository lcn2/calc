/*
 * fposval - Determine information about the file position type
 *
 * Copyright (C) 1999,2021  Landon Curt Noll
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
 * Under source code control:	1994/11/05 03:19:52
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
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
 *	FILEPOS_BITS		length in bits of the type FILEPOS
 *	FILEPOS_LEN		length in octets of the type FILEPOS
 *	SWAP_HALF_IN_FILEPOS	will copy/swap FILEPOS into an HALF array
 *	OFF_T_BITS		length in bits of the st_size stat element
 *	OFF_T_LEN		length in octets of the st_size stat element
 *	SWAP_HALF_IN_OFF_T	will copy/swap st_size into an HALF array
 *	DEV_BITS		length in bits of the st_dev stat element
 *	DEV_LEN			length in bits of the st_dev stat element
 *	SWAP_HALF_IN_DEV	will copy/swap st_dev into an HALF array
 *	INODE_BITS		length in bits of the st_ino stat element
 *	INODE_LEN		length in octets of the st_ino stat element
 *	SWAP_HALF_IN_INODE	will copy/swap st_ino into an HALF array
 *
 * With regards to 'will copy/swap ... into an HALF array'.  Such macros
 * will either be a copy or a copy with HALFs swapped depending on the
 * Endian order of the hardware.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "have_fpos.h"
#include "endian_calc.h"
#include "have_offscl.h"
#include "have_posscl.h"
#include "have_fpos_pos.h"
#include "alloc.h"


#include "banned.h"	/* include after system header <> includes */


char *program;			/* our name */

int
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
#if defined(HAVE_FPOS_POS)
	fileposlen = FPOS_POS_BITS;
#else /* ! HAVE_FPOS_POS */
# if defined(FPOS_BITS)
	fileposlen = FPOS_BITS;
# else
	fileposlen = sizeof(FILEPOS)*8;
# endif
#endif /* ! HAVE_FPOS_POS */
	printf("#undef FILEPOS_BITS\n");
	printf("#define FILEPOS_BITS %d\n", fileposlen);
	printf("#undef FILEPOS_LEN\n");
	printf("#define FILEPOS_LEN %d\n", fileposlen/8);
#if CALC_BYTE_ORDER == BIG_ENDIAN
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
#else /* CALC_BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 */
#if defined(HAVE_FILEPOS_SCALAR)
	printf("#define SWAP_HALF_IN_FILEPOS(dest, src)\t\t%s\n",
	    "(*(dest) = *(src))");
#else /* HAVE_FILEPOS_SCALAR */
	/*
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems a FILEPOS is not a scalar hence we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_FILEPOS(dest, src)\t%s\n",
	    "\\\n\tmemcpy((void *)(dest), (void *)(src), "
	    "sizeof(FPOS_POS_LEN))");
#endif /* HAVE_FILEPOS_SCALAR */
#endif /* CALC_BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the stat file size information
	 */
#if defined(OFF_T_BITS)
	stsizelen = OFF_T_BITS;
#else
	stsizelen = sizeof(buf.st_size)*8;
#endif
	printf("#undef OFF_T_BITS\n");
	printf("#define OFF_T_BITS %d\n", stsizelen);
	printf("#undef OFF_T_LEN\n");
	printf("#define OFF_T_LEN %d\n", stsizelen/8);
#if CALC_BYTE_ORDER == BIG_ENDIAN
	/*
	 * Big Endian
	 */
	if (stsizelen == 64) {
		printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B64(dest, src)");
	} else if (stsizelen == 32) {
		printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n",
		    "SWAP_HALF_IN_B32(dest, src)");
	} else {
		fprintf(stderr, "%s: unexpected st_size bit size: %d\n",
		    program, stsizelen);
		exit(2);
	}
#else /* CALC_BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems an off_t is not a scalar hence we must memcpy.
	 */
#if defined(HAVE_OFF_T_SCALAR)
	printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n",
	    "(*(dest) = *(src))");
#else /* HAVE_OFF_T_SCALAR */
	/*
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a off_t is not a scalar hence we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), ", stsizelen/8, ")");
#endif /* HAVE_OFF_T_SCALAR */
#endif /* CALC_BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the dev_t size
	 */
#if defined(DEV_BITS)
	devlen = DEV_BITS;
#else
	devlen = sizeof(buf.st_dev)*8;
#endif
	printf("#undef DEV_BITS\n");
	printf("#define DEV_BITS %d\n", devlen);
	printf("#undef DEV_LEN\n");
	printf("#define DEV_LEN %d\n", devlen/8);
#if CALC_BYTE_ORDER == BIG_ENDIAN
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
#else /* CALC_BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a DEV is not a scalar hence we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_DEV(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), ", devlen/8, ")");
#endif /* CALC_BYTE_ORDER == BIG_ENDIAN */
	putchar('\n');

	/*
	 * print the ino_t size
	 */
#if defined(INODE_BITS)
	inodelen = INODE_BITS;
#else
	inodelen = sizeof(buf.st_ino)*8;
#endif
	printf("#undef INODE_BITS\n");
	printf("#define INODE_BITS %d\n", inodelen);
	printf("#undef INODE_LEN\n");
	printf("#define INODE_LEN %d\n", inodelen/8);
#if CALC_BYTE_ORDER == BIG_ENDIAN
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
#else /* CALC_BYTE_ORDER == BIG_ENDIAN */
	/*
	 * Little Endian
	 *
	 * Normally a "(*(dest) = *(src))" would do, but on some
	 * systems, a INODE is not a scalar hence we must memcpy.
	 */
	printf("#define SWAP_HALF_IN_INODE(dest, src)\t%s%d%s\n",
	    "memcpy((void *)(dest), (void *)(src), ", inodelen/8, ")");
#endif /* CALC_BYTE_ORDER == BIG_ENDIAN */
	/* exit(0); */
	return 0;
}
