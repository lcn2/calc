/*
 * math_error - a simple libcalc math error routine
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
 * @(#) $Revision: 29.5 $
 * @(#) $Id: math_error.h,v 29.5 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/math_error.h,v $
 *
 * Under source code control:	1997/03/23 18:37:10
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__MATH_ERROR_H__)
#define __MATH_ERROR_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "win32dll.h"
#else
# include <calc/win32dll.h>
#endif


/*
 * Global data definitions.
 */
extern DLL jmp_buf jmpbuf;		/* for errors */


#endif /* !__MATH_ERROR_H__ */
