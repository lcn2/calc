/*
 * have_fpos - Determine if have fgetpos and fsetpos functions
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
 * @(#) $Id: have_fpos.c,v 29.2 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/have_fpos.c,v $
 *
 * Under source code control:	1994/11/05 03:19:52
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * If the symbol HAVE_NO_FPOS is defined, we will output nothing.
 * If we are able to compile this program, then we must have the
 * fgetpos and fsetpos functions and we will output the
 * appropriate have_fpos.h file body.
 */


#include <stdio.h>

int
main(void)
{
#if !defined(HAVE_NO_FPOS)
	fpos_t pos;		/* file position */

	/* get the current position */
	(void) fgetpos(stdin, &pos);

	/* set the current position */
	(void) fsetpos(stdin, &pos);

	/* print a have_fpos.h body that says we have the functions */
	printf("#undef HAVE_FPOS\n");
	printf("#define HAVE_FPOS 1  /* yes */\n\n");
	printf("typedef fpos_t FILEPOS;\n");
#endif
	/* exit(0); */
	return 0;
}
