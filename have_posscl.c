/*
 * have_posscl - determine if we have a scalar FILEPOS element
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
 * @(#) $Id: have_posscl.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_posscl.c,v $
 *
 * Under source code control:	1996/07/13 12:57:22
 * File existed as early as:	1996
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *	have_posscl
 *
 * On some systems, FILEPOS is a scalar value on which one can perform
 * arithmetic operations, assignments and comparisons.	On some systems
 * FILEPOS is some sort of union or struct which must be converted into
 * a ZVALUE in order to perform arithmetic operations, assignments and
 * comparisons.
 *
 * This prog outputs several defines:
 *
 *	HAVE_FILEPOS_SCALAR
 *		defined ==> ok to perform arithmetic ops, = and comparisons
 *		undefined ==> convert to ZVALUE first
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "have_fpos.h"

int
main(void)
{
#if !defined(FILEPOS_NON_SCALAR)
	FILEPOS value;	/* an FILEPOS to perform arithmatic on */
	FILEPOS value2; /* an FILEPOS to perform arithmatic on */

	/*
	 * do some math opts on an FILEPOS
	 */
	value = (FILEPOS)getpid();
	value2 = (FILEPOS)-1;
	if (value > (FILEPOS)1) {
		--value;
	}
	if (value <= (FILEPOS)getppid()) {
		--value;
	}
	if (value == value2) {
		value += value2;
	}
	value <<= 1;
	if (!value) {
		printf("/* something for the FILEPOS to do */\n");
	}

	/*
	 * report FILEPOS as a scalar
	 */
	printf("#undef HAVE_FILEPOS_SCALAR\n");
	printf("#define HAVE_FILEPOS_SCALAR /* FILEPOS is a simple value */\n");
#else
	printf("#undef HAVE_FILEPOS_SCALAR /* FILEPOS is not a simple value */\n");
#endif
	/* exit(0); */
	return 0;
}
