/*
 * label - label handling routines
 *
 * Copyright (C) 1999  David I. Bell
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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: label.h,v 29.4 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/label.h,v $
 *
 * Under source code control:	1990/02/15 01:48:33
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__LABEL_H__)
#define __LABEL_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "zmath.h"
#else
# include <calc/zmath.h>
#endif


#define NULL_LABEL	((LABEL *) 0)


/*
 * Label structures.
 */
typedef struct {
	long l_offset;		  /* offset into code of label */
	long l_chain;		  /* offset into code of undefined chain */
	char *l_name;		  /* name of label if any */
} LABEL;


extern void initlabels(void);
extern void definelabel(char *name);
extern void addlabel(char *name);
extern void clearlabel(LABEL *lp);
extern void setlabel(LABEL *lp);
extern void uselabel(LABEL *lp);
extern void checklabels(void);


#endif /* !__LABEL_H__ */
