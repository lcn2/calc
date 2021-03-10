/*
 * have_fpos_pos - Determine if a __pos element in FILEPOS
 *
 * Copyright (C) 2000,2021  Landon Curt Noll
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
 * Under source code control:	2000/12/17 01:23
 * File existed as early as:	2000
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * If the symbol HAVE_NO_FPOS is defined, we will output nothing.
 * If the HAVE_FILEPOS_SCALAR is defined, we will output nothing.
 * If we are able to compile this program, then we must have the
 * __pos element in a non-scalar FILEPOS.
 */

#include <stdio.h>
#include "have_fpos.h"
#include "have_posscl.h"


#include "banned.h"	/* include after system header <> includes */


int
main(void)
{
#if !defined(HAVE_NO_FPOS) && !defined(HAVE_FILEPOS_SCALAR)
	fpos_t pos;		/* file position */

	/* print a __pos element in fpos_t */
	printf("#undef HAVE_FPOS_POS\n");
	printf("#define HAVE_FPOS_POS 1  /* yes */\n\n");

	/* determine __pos element size */
	printf("#undef FPOS_POS_BITS\n");
	printf("#undef FPOS_POS_LEN\n");
# if defined(FPOS_POS_BITS)
	printf("#define FPOS_POS_BITS %d\n", FPOS_POS_BITS);
	printf("#define FPOS_POS_LEN %d\n", int(FPOS_POS_BITS/8));
# else
	printf("#define FPOS_POS_BITS %d\n", sizeof(pos.__pos)*8);
	printf("#define FPOS_POS_LEN %d\n", sizeof(pos.__pos));
# endif

#else
	/* we have no __pos element */
	printf("#undef HAVE_FPOS_POS\t/* no */\n");
	printf("#undef FPOS_POS_BITS\n");
	printf("#undef FPOS_POS_LEN\n");
#endif
	/* exit(0); */
	return 0;
}
