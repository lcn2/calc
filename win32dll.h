/*
 * win32dll - definitions for building windoz dll files
 *
 * Copyright (C) 2001  Landon Curt Noll
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
 * @(#) $Revision: 29.1 $
 * @(#) $Id: win32dll.h,v 29.1 2001/03/18 03:03:11 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/win32dll.h,v $
 *
 * Under source code control:	2001/03/17 13:05:31
 * File existed as early as:	2001
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__WIN32DLL_H__)
#define __WIN32DLL_H__

#if defined(_WIN32)

# if defined(_EXPORTING)
#  define DLL __declspec(dllexport)
# else
#  define DLL __declspec(dllimport)
# endif

#else /* Windoz free systems */

# define DLL

#endif /* Windoz free systems */


#endif /* !__WIN32DLL_H__ */
