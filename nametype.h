/*
 * nametype - associate names with values
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
 * @(#) $Revision: 29.1 $
 * @(#) $Id: nametype.h,v 29.1 1999/12/14 09:16:12 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/nametype.h,v $
 *
 * Under source code control:	1997/03/08 13:44:55
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://reality.sgi.com/chongo/
 * Share and enjoy!  :-)	http://reality.sgi.com/chongo/tech/comp/calc/
 */


#if !defined(__NAMETYPE_H__)
#define __NAMETYPE_H__


/*
 * Configuration parameter name and type.
 */
typedef struct {
	char *name;	/* name of configuration string */
	long type;	/* type for configuration */
} NAMETYPE;


#endif /* !__NAMETYPE_H__ */
